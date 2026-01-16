/**
 * @file shm_transport.hpp
 * @brief TransportSHM 공용 API (bytes-only)
 *
 * VxWorks 전용 SHM Data Transport의 공개 인터페이스.
 * 이 헤더만 외부에 노출되며, 내부 구현은 src/ 하위에 위치한다.
 *
 * 설계 원칙:
 * - bytes-only: 상위 프로토콜(Header 구조, JSON/Struct 등)과 완전 분리
 * - 공용모듈은 Agent/Legacy 헤더에 의존하지 않음
 * - VxWorks에서만 실제 동작 (타 플랫폼은 스텁 또는 컴파일 제외)
 */
#pragma once

#include "shm_config.hpp"
#include "shm_log.hpp"
#include <cstdint>
#include <memory>

namespace ddsfw {
namespace transport_shm {

/**
 * @brief 프레임 수신 콜백 함수 타입 (bytes-only)
 *
 * @param frame_bytes 수신된 프레임 바이트열 (Header+Payload 포함)
 * @param len 바이트열 길이
 * @param user 사용자 컨텍스트 포인터
 *
 * @note noexcept 보장 필수 - 콜백 예외가 rx thread를 중단시키면 안 됨
 */
using OnFrameFn = void (*)(const uint8_t* frame_bytes, uint32_t len, void* user) noexcept;

// Forward declaration
class TransportImpl;

/**
 * @class Transport
 * @brief SHM 기반 양방향 Transport (SPSC Ring 2개)
 *
 * 사용 예:
 * @code
 * ddsfw::transport_shm::Transport tx;
 * auto cfg = ddsfw::transport_shm::Config::defaults(Side::Agent, true);
 * if (!tx.open(cfg, my_log_fn)) { ... }
 * tx.set_on_frame(my_frame_cb, this);
 * tx.start();  // rx thread 시작
 * tx.send_bytes(frame, len);
 * tx.stop();
 * tx.close();
 * @endcode
 *
 * 스레드 모델:
 * - send_bytes(): 호출 스레드에서 직접 producer 동작
 * - start()/stop(): 내부 rx thread 관리
 * - on_frame 콜백: rx thread에서 호출됨
 */
class Transport {
public:
    Transport();
    ~Transport();

    // Non-copyable, non-movable
    Transport(const Transport&) = delete;
    Transport& operator=(const Transport&) = delete;
    Transport(Transport&&) = delete;
    Transport& operator=(Transport&&) = delete;

    /**
     * @brief SHM 및 Notify 리소스 초기화
     *
     * @param cfg 설정 구조체
     * @param log 로그 콜백 (nullptr 가능)
     * @return 성공 시 true
     *
     * @details creator=true인 경우 SHM 생성 및 초기화 수행
     *          creator=false인 경우 기존 SHM에 조인
     */
    bool open(const Config& cfg, LogFn log = nullptr) noexcept;

    /**
     * @brief 리소스 해제 및 종료
     * @details stop()이 호출되지 않았으면 내부에서 호출
     */
    void close() noexcept;

    /**
     * @brief 프레임 수신 콜백 설정
     *
     * @param cb 콜백 함수
     * @param user 사용자 컨텍스트 (콜백에 전달됨)
     *
     * @note start() 전에 호출해야 함
     */
    void set_on_frame(OnFrameFn cb, void* user) noexcept;

    /**
     * @brief rx thread 시작 (consumer)
     * @return 성공 시 true
     * @details open() 후에만 호출 가능
     */
    bool start() noexcept;

    /**
     * @brief rx thread 정지
     * @details 블로킹 호출 - thread 종료까지 대기
     */
    void stop() noexcept;

    /**
     * @brief 프레임 전송 (bytes-only)
     *
     * @param frame_bytes 전송할 바이트열
     * @param len 바이트열 길이
     * @return 성공 시 true, 실패(overflow 등) 시 false
     *
     * @details len==0 또는 len>max_frame인 경우 거부
     *          Ring overflow 시 drop하고 false 반환
     */
    bool send_bytes(const uint8_t* frame_bytes, uint32_t len) noexcept;

    /**
     * @brief 현재 epoch 값 조회
     * @return SHM GlobalHeader의 epoch 값
     * @details creator가 재시작/리셋 시 증가함
     */
    uint32_t epoch() const noexcept;

    /**
     * @brief 송신 drop 카운트 조회
     * @return 로컬 producer에서 발생한 drop 횟수
     */
    uint32_t drops_tx() const noexcept;

    /**
     * @brief 수신 drop 카운트 조회
     * @return 로컬 consumer에서 발생한 drop 횟수 (현재는 0)
     */
    uint32_t drops_rx() const noexcept;

    /**
     * @brief open() 성공 여부
     * @return 열린 상태이면 true
     */
    bool is_open() const noexcept;

    /**
     * @brief rx thread 실행 중 여부
     * @return 실행 중이면 true
     */
    bool is_running() const noexcept;

private:
    std::unique_ptr<TransportImpl> impl_;
};

}  // namespace transport_shm
}  // namespace ddsfw
