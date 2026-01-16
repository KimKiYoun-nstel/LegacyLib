/**
 * @file shm_layout.hpp
 * @brief TransportSHM ABI 구조체 및 레이아웃 정의 (내부용)
 *
 * SHM 메모리 레이아웃과 Record 형식을 정의한다.
 * 이 헤더는 공용모듈 내부(src/)에서만 사용되며 외부에 노출하지 않는다.
 *
 * 레이아웃 (ver=1):
 *   [ShmGlobalHeader]
 *   [LA ShmRingHeader][LA buffer (ring_bytes)]
 *   [AL ShmRingHeader][AL buffer (ring_bytes)]
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>

namespace ddsfw {
namespace transport_shm {

//------------------------------------------------------------------------------
// 상수 정의
//------------------------------------------------------------------------------

/// SHM 매직 넘버 ('SHM1')
constexpr uint32_t kShmMagic = 0x53484D31u;

/// 현재 ABI 버전
constexpr uint16_t kShmVersion = 1;

/// Record 정렬 단위
constexpr uint32_t kAlign = 16;

/// RingHeader 정렬 단위
constexpr uint32_t kRingHeaderAlign = 64;

/// 이름 필드 최대 길이
constexpr size_t kNameMaxLen = 64;

//------------------------------------------------------------------------------
// SHM 상태
//------------------------------------------------------------------------------

/**
 * @brief SHM 상태 열거형
 */
enum class ShmState : uint32_t {
    Uninit = 0,  ///< 초기화되지 않음
    Ready = 1    ///< 사용 가능
};

//------------------------------------------------------------------------------
// Global Header
//------------------------------------------------------------------------------

/**
 * @struct ShmGlobalHeader
 * @brief SHM 전역 헤더 (offset 0에 위치)
 *
 * creator가 초기화하고, joiner는 읽기만 수행.
 * magic/ver 확인으로 호환성 검증.
 */
struct ShmGlobalHeader {
    uint32_t magic;          ///< DDSFW_SHM_MAGIC (0x53484D31)
    uint16_t ver;            ///< ABI 버전 (1)
    uint16_t flags;          ///< 예약 (0)
    uint32_t state;          ///< ShmState (Ready=1이면 사용 가능)
    uint32_t epoch;          ///< creator가 재시작/리셋 시 증가

    uint32_t max_frame;      ///< 단일 프레임 최대 바이트
    uint32_t ring_bytes;     ///< 링 버퍼 크기 (per ring)

    uint32_t abi_hash;       ///< 선택적 ABI 해시 (0이면 미사용)

    char shm_name[kNameMaxLen];    ///< SHM 객체 이름
    char notify_la[kNameMaxLen];   ///< LA ring notify 이름
    char notify_al[kNameMaxLen];   ///< AL ring notify 이름

    uint32_t reserved[8];    ///< 예약 필드

    /**
     * @brief 헤더 초기화
     */
    void init(const char* shm, const char* la, const char* al,
              uint32_t max_frm, uint32_t ring_sz) noexcept {
        magic = kShmMagic;
        ver = kShmVersion;
        flags = 0;
        state = static_cast<uint32_t>(ShmState::Uninit);
        epoch = 1;
        max_frame = max_frm;
        ring_bytes = ring_sz;
        abi_hash = 0;

        std::memset(shm_name, 0, sizeof(shm_name));
        std::memset(notify_la, 0, sizeof(notify_la));
        std::memset(notify_al, 0, sizeof(notify_al));
        std::memset(reserved, 0, sizeof(reserved));

        if (shm) std::strncpy(shm_name, shm, kNameMaxLen - 1);
        if (la)  std::strncpy(notify_la, la, kNameMaxLen - 1);
        if (al)  std::strncpy(notify_al, al, kNameMaxLen - 1);
    }

    /**
     * @brief Ready 상태로 전환
     */
    void set_ready() noexcept {
        state = static_cast<uint32_t>(ShmState::Ready);
    }

    /**
     * @brief Ready 상태 확인
     */
    bool is_ready() const noexcept {
        return state == static_cast<uint32_t>(ShmState::Ready);
    }

    /**
     * @brief 매직/버전 검증
     */
    bool validate() const noexcept {
        return magic == kShmMagic && ver == kShmVersion;
    }
};

//------------------------------------------------------------------------------
// Ring Header
//------------------------------------------------------------------------------

/**
 * @struct ShmRingHeader
 * @brief SPSC 링 버퍼 헤더 (64-byte 정렬)
 *
 * head: producer가 다음 쓸 위치 (버퍼 내 오프셋)
 * tail: consumer가 다음 읽을 위치 (버퍼 내 오프셋)
 */
struct alignas(kRingHeaderAlign) ShmRingHeader {
    std::atomic<uint32_t> head;  ///< producer 위치
    std::atomic<uint32_t> tail;  ///< consumer 위치
    uint32_t capacity;           ///< == ring_bytes
    uint32_t drops;              ///< drop 카운트
    uint32_t seq;                ///< 시퀀스 번호 (producer 증가)
    uint32_t reserved[11];       ///< 패딩 (64바이트 맞춤)

    /**
     * @brief 헤더 초기화
     */
    void init(uint32_t cap) noexcept {
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
        capacity = cap;
        drops = 0;
        seq = 0;
        std::memset(reserved, 0, sizeof(reserved));
    }

    /**
     * @brief 사용 중인 바이트 수
     */
    uint32_t used() const noexcept {
        uint32_t h = head.load(std::memory_order_acquire);
        uint32_t t = tail.load(std::memory_order_acquire);
        return (h >= t) ? (h - t) : (capacity - t + h);
    }

    /**
     * @brief 남은 공간 (바이트)
     */
    uint32_t free_space() const noexcept {
        // 최소 1바이트는 남겨야 full/empty 구분 가능
        uint32_t u = used();
        return (capacity > u + 1) ? (capacity - u - 1) : 0;
    }

    /**
     * @brief head부터 버퍼 끝까지 연속 공간
     */
    uint32_t contig_space(uint32_t h) const noexcept {
        return capacity - h;
    }

    /**
     * @brief 비어있는지 확인
     */
    bool empty() const noexcept {
        return head.load(std::memory_order_acquire) ==
               tail.load(std::memory_order_acquire);
    }
};

//------------------------------------------------------------------------------
// Record Header
//------------------------------------------------------------------------------

/**
 * @struct ShmRecordHdr
 * @brief 링 버퍼 내 레코드 헤더
 *
 * total_len: frame_bytes 길이 (0이면 WRAP marker)
 * 레코드 전체 크기: align16(sizeof(ShmRecordHdr) + total_len)
 */
struct ShmRecordHdr {
    uint32_t total_len;  ///< 페이로드 길이 (0 = WRAP marker)
    uint32_t seq;        ///< 시퀀스 번호
    uint32_t crc32;      ///< CRC32 (0이면 미사용)
    uint32_t reserved;   ///< 예약

    /**
     * @brief WRAP marker 여부
     */
    bool is_wrap() const noexcept {
        return total_len == 0;
    }

    /**
     * @brief 레코드 전체 비용 (정렬 포함)
     */
    static uint32_t cost(uint32_t payload_len) noexcept {
        return align16(static_cast<uint32_t>(sizeof(ShmRecordHdr)) + payload_len);
    }

private:
    static constexpr uint32_t align16(uint32_t sz) noexcept {
        return (sz + 15u) & ~15u;
    }
};

//------------------------------------------------------------------------------
// 레이아웃 계산 헬퍼
//------------------------------------------------------------------------------

/**
 * @brief 전체 SHM 크기 계산
 * @param ring_bytes per-ring 버퍼 크기
 * @return 필요한 총 SHM 바이트
 */
inline size_t calc_shm_total_size(uint32_t ring_bytes) noexcept {
    // GlobalHeader + (RingHeader + buffer) * 2
    size_t global_sz = sizeof(ShmGlobalHeader);
    size_t ring_total = sizeof(ShmRingHeader) + ring_bytes;
    return global_sz + ring_total * 2;
}

/**
 * @brief LA RingHeader 오프셋
 */
inline size_t offset_la_ring() noexcept {
    return sizeof(ShmGlobalHeader);
}

/**
 * @brief LA 버퍼 오프셋
 */
inline size_t offset_la_buffer() noexcept {
    return offset_la_ring() + sizeof(ShmRingHeader);
}

/**
 * @brief AL RingHeader 오프셋
 * @param ring_bytes per-ring 버퍼 크기
 */
inline size_t offset_al_ring(uint32_t ring_bytes) noexcept {
    return offset_la_buffer() + ring_bytes;
}

/**
 * @brief AL 버퍼 오프셋
 * @param ring_bytes per-ring 버퍼 크기
 */
inline size_t offset_al_buffer(uint32_t ring_bytes) noexcept {
    return offset_al_ring(ring_bytes) + sizeof(ShmRingHeader);
}

}  // namespace transport_shm
}  // namespace ddsfw
