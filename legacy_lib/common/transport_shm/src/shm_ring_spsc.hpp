/**
 * @file shm_ring_spsc.hpp
 * @brief SPSC Ring Buffer 인터페이스 (내부용)
 *
 * Single-Producer Single-Consumer 링 버퍼.
 * lock-free atomic head/tail 기반 구현.
 */
#pragma once

#include "shm_layout.hpp"
#include "shm_notify.hpp"
#include <cstdint>

namespace ddsfw {
namespace transport_shm {

/**
 * @class ShmRingSPSC
 * @brief SPSC 링 버퍼 연산 클래스
 *
 * 외부에서 ShmRingHeader와 buffer 포인터를 제공받아 연산만 수행.
 * 메모리 할당은 하지 않는다.
 */
class ShmRingSPSC {
public:
    ShmRingSPSC() = default;

    /**
     * @brief 링 버퍼 초기화 (attach)
     *
     * @param ring RingHeader 포인터
     * @param buffer 버퍼 시작 주소
     * @param notify 알림 객체 (producer: post, consumer: wait)
     * @param max_frame 최대 프레임 크기
     */
    void attach(ShmRingHeader* ring, uint8_t* buffer,
                ShmNotify* notify, uint32_t max_frame) noexcept;

    /**
     * @brief Producer: 프레임 전송
     *
     * @param frame_bytes 전송할 바이트열
     * @param len 바이트열 길이
     * @return 성공 시 true, 실패(overflow) 시 false
     */
    bool push(const uint8_t* frame_bytes, uint32_t len) noexcept;

    /**
     * @brief Consumer: 프레임 수신 (블로킹)
     *
     * @param out_buf 수신 버퍼 (최소 max_frame 크기)
     * @param out_len 수신된 길이 (출력)
     * @param timeout_ms 타임아웃 (밀리초)
     * @return 성공 시 true, 타임아웃/오류 시 false
     */
    bool pop(uint8_t* out_buf, uint32_t& out_len, uint32_t timeout_ms) noexcept;

    /**
     * @brief Consumer: 논블로킹 drain (여러 레코드 연속 처리)
     *
     * @param callback 프레임 처리 콜백
     * @param user 사용자 컨텍스트
     * @param max_count 최대 처리 개수 (0이면 모두)
     * @return 처리된 레코드 수
     */
    using DrainFn = void (*)(const uint8_t* frame_bytes, uint32_t len, void* user) noexcept;
    uint32_t drain(DrainFn callback, void* user, uint32_t max_count = 0) noexcept;

    /**
     * @brief 비어있는지 확인
     */
    bool empty() const noexcept;

    /**
     * @brief drop 카운트 조회
     */
    uint32_t drops() const noexcept;

    /**
     * @brief 현재 시퀀스 번호
     */
    uint32_t seq() const noexcept;

private:
    /**
     * @brief WRAP marker 기록 및 head 리셋
     */
    void write_wrap_marker(uint32_t head) noexcept;

    /**
     * @brief 레코드 읽기 (내부)
     * @return 읽은 레코드 크기 (0이면 empty 또는 WRAP)
     */
    uint32_t read_record(uint8_t* out_buf, uint32_t& out_len) noexcept;

private:
    ShmRingHeader* ring_ = nullptr;
    uint8_t* buffer_ = nullptr;
    ShmNotify* notify_ = nullptr;
    uint32_t max_frame_ = 0;
};

}  // namespace transport_shm
}  // namespace ddsfw
