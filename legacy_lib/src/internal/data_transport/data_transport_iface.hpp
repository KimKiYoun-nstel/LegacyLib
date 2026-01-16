/**
 * @file data_transport_iface.hpp
 * @brief Data Plane Transport 공통 인터페이스
 *
 * Data Plane 전송을 위한 추상 인터페이스를 정의한다.
 * UDP 또는 SHM(VxWorks 전용) 중 하나를 선택하여 사용할 수 있다.
 *
 * 설계 참조: DevGuide/03_LegacyLib_SHM_Integration_Design.md
 */
#pragma once

#include "legacy_frames.h"
#include <cstdint>
#include <cstddef>
#include <functional>

namespace legacy {
namespace transport {

/**
 * @brief Data Transport 종류
 */
enum class DataTransportKind {
    UDP,    ///< UDP 기반 전송 (기본값, 모든 플랫폼)
    SHM     ///< SHM 기반 전송 (VxWorks 전용)
};

/**
 * @brief Data Transport 초기화 설정
 */
struct DataTransportConfig {
    DataTransportKind kind;    ///< Transport 종류

    // UDP 설정
    const char* agent_ip;      ///< Agent IP 주소
    uint16_t agent_port;       ///< Agent 포트

    // SHM 설정 (VxWorks 전용)
    const char* shm_name;      ///< SHM 객체 이름 (예: "/dds_fw_data")
    const char* notify_la;     ///< LA ring notify 이름
    const char* notify_al;     ///< AL ring notify 이름
    uint32_t ring_bytes;       ///< 링 버퍼 크기 (per ring)
    uint32_t max_frame;        ///< 단일 프레임 최대 크기
    uint32_t wait_ms;          ///< consumer wait timeout

    /**
     * @brief UDP 기본 설정으로 초기화
     */
    static DataTransportConfig udp_defaults(const char* ip, uint16_t port) {
        DataTransportConfig cfg{};
        cfg.kind = DataTransportKind::UDP;
        cfg.agent_ip = ip;
        cfg.agent_port = port;
        return cfg;
    }

#if DDSFW_ENABLE_SHM
    /**
     * @brief SHM 기본 설정으로 초기화 (VxWorks 전용)
     */
    static DataTransportConfig shm_defaults() {
        DataTransportConfig cfg{};
        cfg.kind = DataTransportKind::SHM;
        cfg.shm_name = "/dds_fw_data";
        cfg.notify_la = "/dds_fw_sem_la";
        cfg.notify_al = "/dds_fw_sem_al";
        cfg.ring_bytes = 4 * 1024 * 1024;  // 4MB per ring
        cfg.max_frame = 65536;              // 64KB
        cfg.wait_ms = 100;
        return cfg;
    }
#endif
};

/**
 * @brief 프레임 수신 콜백
 *
 * @param hdr 프레임 헤더
 * @param payload 페이로드 바이트열
 * @param len 페이로드 길이
 * @param user 사용자 컨텍스트
 */
using OnDataFrameFn = void (*)(const LegacyFrameHeader* hdr,
                                const uint8_t* payload,
                                uint32_t len,
                                void* user);

/**
 * @brief 로그 콜백
 */
using LogFn = void (*)(int level, const char* msg);

/**
 * @class IDataTransport
 * @brief Data Plane Transport 추상 인터페이스
 *
 * Data Plane 전송(STRUCT/JSON 데이터)을 위한 공통 인터페이스.
 * Control Plane(Hello, Create 등)은 항상 UDP를 사용한다.
 */
class IDataTransport {
public:
    virtual ~IDataTransport() = default;

    /**
     * @brief Transport 초기화
     * @param cfg 설정
     * @param log 로그 콜백 (nullable)
     * @return 성공 시 true
     */
    virtual bool open(const DataTransportConfig& cfg, LogFn log = nullptr) noexcept = 0;

    /**
     * @brief Transport 종료
     */
    virtual void close() noexcept = 0;

    /**
     * @brief 수신 콜백 설정
     * @param cb 콜백 함수
     * @param user 사용자 컨텍스트
     */
    virtual void set_on_frame(OnDataFrameFn cb, void* user) noexcept = 0;

    /**
     * @brief 수신 스레드 시작
     * @return 성공 시 true
     */
    virtual bool start() noexcept = 0;

    /**
     * @brief 수신 스레드 중지
     */
    virtual void stop() noexcept = 0;

    /**
     * @brief 프레임 전송 (헤더 + 페이로드)
     *
     * @param hdr 프레임 헤더
     * @param payload 페이로드 바이트열
     * @param payload_len 페이로드 길이
     * @return 성공 시 true
     */
    virtual bool send_frame(const LegacyFrameHeader& hdr,
                            const uint8_t* payload,
                            uint32_t payload_len) noexcept = 0;

    /**
     * @brief 열린 상태 확인
     */
    virtual bool is_open() const noexcept = 0;

    /**
     * @brief 실행 중 상태 확인
     */
    virtual bool is_running() const noexcept = 0;

    /**
     * @brief Transport 종류 반환
     */
    virtual DataTransportKind kind() const noexcept = 0;
};

/**
 * @brief DataTransport 팩토리 함수
 *
 * @param kind Transport 종류
 * @return Transport 인스턴스 (소유권 이전)
 *
 * @note SHM은 VxWorks에서만 사용 가능. 타 플랫폼에서 SHM 요청 시 nullptr 반환.
 */
IDataTransport* create_data_transport(DataTransportKind kind) noexcept;

}  // namespace transport
}  // namespace legacy
