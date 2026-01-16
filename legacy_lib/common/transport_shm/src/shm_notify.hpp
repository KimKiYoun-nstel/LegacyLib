/**
 * @file shm_notify.hpp
 * @brief SHM Notify 인터페이스 (내부용)
 *
 * POSIX Named Semaphore 기반 알림 메커니즘.
 * producer가 post(), consumer가 wait()를 호출한다.
 */
#pragma once

#include <cstdint>

namespace ddsfw {
namespace transport_shm {

/**
 * @class ShmNotify
 * @brief Named Semaphore 기반 알림 클래스
 *
 * producer: post()로 consumer 깨우기
 * consumer: wait()로 데이터 대기
 */
class ShmNotify {
public:
    ShmNotify() = default;
    ~ShmNotify();

    // Non-copyable
    ShmNotify(const ShmNotify&) = delete;
    ShmNotify& operator=(const ShmNotify&) = delete;

    /**
     * @brief Named Semaphore 열기/생성
     *
     * @param name 세마포어 이름 (예: "/dds_fw_sem_la")
     * @param creator true이면 생성, false이면 기존에 조인
     * @return 성공 시 true
     */
    bool open(const char* name, bool creator) noexcept;

    /**
     * @brief 세마포어 닫기
     * @details creator인 경우 sem_unlink도 수행
     */
    void close() noexcept;

    /**
     * @brief consumer 깨우기 (producer 호출)
     * @return 성공 시 true
     */
    bool post() noexcept;

    /**
     * @brief 데이터 대기 (consumer 호출)
     * @param timeout_ms 타임아웃 (밀리초), 0이면 무한 대기
     * @return 성공 시 true, 타임아웃 시 false
     */
    bool wait(uint32_t timeout_ms) noexcept;

    /**
     * @brief 열린 상태 확인
     */
    bool is_open() const noexcept { return sem_ != nullptr; }

private:
    void* sem_ = nullptr;  // sem_t* (opaque)
    bool creator_ = false;
    char name_[64] = {};
};

}  // namespace transport_shm
}  // namespace ddsfw
