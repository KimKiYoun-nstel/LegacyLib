/**
 * @file shm_ring_spsc.cpp
 * @brief SPSC Ring Buffer 구현
 *
 * lock-free SPSC 링 버퍼 알고리즘.
 * 메모리 오더: producer store(release) / consumer load(acquire)
 */

#include "shm_ring_spsc.hpp"
#include <cstring>
#include <algorithm>

namespace ddsfw {
namespace transport_shm {

void ShmRingSPSC::attach(ShmRingHeader* ring, uint8_t* buffer,
                         ShmNotify* notify, uint32_t max_frame) noexcept {
    ring_ = ring;
    buffer_ = buffer;
    notify_ = notify;
    max_frame_ = max_frame;
}

bool ShmRingSPSC::push(const uint8_t* frame_bytes, uint32_t len) noexcept {
    if (!ring_ || !buffer_ || !frame_bytes) {
        return false;
    }

    // 길이 검증
    if (len == 0 || len > max_frame_) {
        return false;
    }

    const uint32_t record_cost = ShmRecordHdr::cost(len);
    const uint32_t capacity = ring_->capacity;

    // head/tail 읽기
    uint32_t head = ring_->head.load(std::memory_order_relaxed);
    uint32_t tail = ring_->tail.load(std::memory_order_acquire);

    // free space 계산
    uint32_t used = (head >= tail) ? (head - tail) : (capacity - tail + head);
    uint32_t free_space = (capacity > used + 1) ? (capacity - used - 1) : 0;

    if (free_space < record_cost) {
        // overflow - drop
        ring_->drops++;
        return false;
    }

    // 연속 공간 확인
    uint32_t contig = capacity - head;
    if (contig < record_cost) {
        // WRAP marker 기록
        write_wrap_marker(head);
        head = 0;

        // WRAP 후 다시 공간 확인
        used = (head >= tail) ? (head - tail) : (capacity - tail + head);
        free_space = (capacity > used + 1) ? (capacity - used - 1) : 0;
        if (free_space < record_cost) {
            ring_->drops++;
            return false;
        }
    }

    // 레코드 헤더 작성
    ShmRecordHdr* hdr = reinterpret_cast<ShmRecordHdr*>(buffer_ + head);
    hdr->total_len = len;
    hdr->seq = ++ring_->seq;
    hdr->crc32 = 0;  // CRC 미사용
    hdr->reserved = 0;

    // 페이로드 복사
    std::memcpy(buffer_ + head + sizeof(ShmRecordHdr), frame_bytes, len);

    // head 업데이트 (release)
    uint32_t new_head = head + record_cost;
    if (new_head >= capacity) {
        new_head = 0;  // 정확히 맞는 경우
    }
    ring_->head.store(new_head, std::memory_order_release);

    // notify
    if (notify_) {
        notify_->post();
    }

    return true;
}

void ShmRingSPSC::write_wrap_marker(uint32_t head) noexcept {
    // WRAP marker: total_len = 0
    ShmRecordHdr* hdr = reinterpret_cast<ShmRecordHdr*>(buffer_ + head);
    hdr->total_len = 0;
    hdr->seq = ring_->seq;
    hdr->crc32 = 0;
    hdr->reserved = 0;

    // head를 0으로 리셋 (release)
    ring_->head.store(0, std::memory_order_release);
}

bool ShmRingSPSC::pop(uint8_t* out_buf, uint32_t& out_len, uint32_t timeout_ms) noexcept {
    if (!ring_ || !buffer_ || !out_buf) {
        return false;
    }

    out_len = 0;

    // 먼저 non-blocking 시도
    uint32_t read_len = read_record(out_buf, out_len);
    if (read_len > 0) {
        return true;
    }

    // empty - wait
    if (notify_) {
        if (!notify_->wait(timeout_ms)) {
            return false;  // 타임아웃
        }
    }

    // 다시 시도
    read_len = read_record(out_buf, out_len);
    return (read_len > 0);
}

uint32_t ShmRingSPSC::read_record(uint8_t* out_buf, uint32_t& out_len) noexcept {
    const uint32_t capacity = ring_->capacity;

retry:
    uint32_t head = ring_->head.load(std::memory_order_acquire);
    uint32_t tail = ring_->tail.load(std::memory_order_relaxed);

    if (head == tail) {
        return 0;  // empty
    }

    // 레코드 헤더 읽기
    const ShmRecordHdr* hdr = reinterpret_cast<const ShmRecordHdr*>(buffer_ + tail);

    // WRAP marker 처리
    if (hdr->is_wrap()) {
        // tail을 0으로 리셋
        ring_->tail.store(0, std::memory_order_release);
        goto retry;
    }

    uint32_t len = hdr->total_len;
    if (len > max_frame_) {
        // 비정상 - 손상된 데이터
        // tail을 다음으로 이동 (복구 시도)
        ring_->tail.store(0, std::memory_order_release);
        return 0;
    }

    // 페이로드 복사
    std::memcpy(out_buf, buffer_ + tail + sizeof(ShmRecordHdr), len);
    out_len = len;

    // tail 업데이트 (release)
    uint32_t record_cost = ShmRecordHdr::cost(len);
    uint32_t new_tail = tail + record_cost;
    if (new_tail >= capacity) {
        new_tail = 0;
    }
    ring_->tail.store(new_tail, std::memory_order_release);

    return record_cost;
}

uint32_t ShmRingSPSC::drain(DrainFn callback, void* user, uint32_t max_count) noexcept {
    if (!ring_ || !buffer_ || !callback) {
        return 0;
    }

    uint32_t count = 0;
    const uint32_t capacity = ring_->capacity;
    const uint32_t limit = (max_count > 0) ? max_count : UINT32_MAX;

    while (count < limit) {
        uint32_t head = ring_->head.load(std::memory_order_acquire);
        uint32_t tail = ring_->tail.load(std::memory_order_relaxed);

        if (head == tail) {
            break;  // empty
        }

        const ShmRecordHdr* hdr = reinterpret_cast<const ShmRecordHdr*>(buffer_ + tail);

        // WRAP marker 처리
        if (hdr->is_wrap()) {
            ring_->tail.store(0, std::memory_order_release);
            continue;
        }

        uint32_t len = hdr->total_len;
        if (len > max_frame_) {
            // 손상 - 복구
            ring_->tail.store(0, std::memory_order_release);
            break;
        }

        // 콜백 호출 (zero-copy)
        callback(buffer_ + tail + sizeof(ShmRecordHdr), len, user);

        // tail 업데이트
        uint32_t record_cost = ShmRecordHdr::cost(len);
        uint32_t new_tail = tail + record_cost;
        if (new_tail >= capacity) {
            new_tail = 0;
        }
        ring_->tail.store(new_tail, std::memory_order_release);

        ++count;
    }

    return count;
}

bool ShmRingSPSC::empty() const noexcept {
    if (!ring_) return true;
    return ring_->empty();
}

uint32_t ShmRingSPSC::drops() const noexcept {
    return ring_ ? ring_->drops : 0;
}

uint32_t ShmRingSPSC::seq() const noexcept {
    return ring_ ? ring_->seq : 0;
}

}  // namespace transport_shm
}  // namespace ddsfw
