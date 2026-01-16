/**
 * @file shm_config.hpp
 * @brief TransportSHM 설정 구조체 정의
 *
 * VxWorks 전용 SHM Data Transport의 설정 파라미터를 정의한다.
 * 이 헤더는 Agent와 LegacyLib 양쪽에서 동일하게 사용된다.
 */
#pragma once

#include <cstdint>

namespace ddsfw {
namespace transport_shm {

/**
 * @brief Transport 역할 (Agent 또는 Legacy)
 */
enum class Side {
    Agent,  ///< Agent 측 (일반적으로 creator)
    Legacy  ///< Legacy 측 (일반적으로 joiner)
};

/**
 * @brief Notify 메커니즘 종류
 * @details v1에서는 POSIX Named Semaphore만 지원
 */
enum class NotifyKind {
    PosixNamedSemaphore  ///< sem_open/sem_post/sem_timedwait 사용
};

/**
 * @brief TransportSHM 설정 구조체
 *
 * Agent와 Legacy 양쪽에서 동일한 값을 사용해야 상호운용이 가능하다.
 * 특히 shm_name, notify_la, notify_al, ring_bytes, max_frame은
 * 반드시 양쪽이 일치해야 한다.
 */
struct Config {
    Side side;           ///< Agent 또는 Legacy
    bool creator;        ///< SHM 생성자 여부 (권장: Agent=true, Legacy=false)

    const char* shm_name;    ///< SHM 객체 이름 (예: "/dds_fw_data")
    const char* notify_la;   ///< LA ring notify 이름 (예: "/dds_fw_sem_la")
    const char* notify_al;   ///< AL ring notify 이름 (예: "/dds_fw_sem_al")

    uint32_t ring_bytes;     ///< 링 버퍼 크기 (per ring, 바이트)
    uint32_t max_frame;      ///< 단일 프레임 최대 크기 (바이트)
    uint32_t wait_ms;        ///< consumer wait timeout (밀리초)

    NotifyKind notify_kind;  ///< Notify 메커니즘 종류

    /**
     * @brief 기본값으로 초기화된 Config 생성
     * @return 기본 설정이 적용된 Config
     */
    static Config defaults(Side s, bool is_creator) noexcept {
        Config cfg{};
        cfg.side = s;
        cfg.creator = is_creator;
        cfg.shm_name = "/dds_fw_data";
        cfg.notify_la = "/dds_fw_sem_la";
        cfg.notify_al = "/dds_fw_sem_al";
        cfg.ring_bytes = 4 * 1024 * 1024;  // 4MB
        cfg.max_frame = 65536;              // 64KB
        cfg.wait_ms = 100;
        cfg.notify_kind = NotifyKind::PosixNamedSemaphore;
        return cfg;
    }
};

/**
 * @brief Record 정렬 상수
 */
constexpr uint32_t kRecordAlign = 16;

/**
 * @brief 정렬된 크기 계산
 * @param size 원본 크기
 * @param align 정렬 단위
 * @return 정렬된 크기
 */
constexpr uint32_t align_up(uint32_t size, uint32_t align) noexcept {
    return (size + align - 1) & ~(align - 1);
}

}  // namespace transport_shm
}  // namespace ddsfw
