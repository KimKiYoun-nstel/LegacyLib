/**
 * @file data_transport_shm.hpp
 * @brief SHM 기반 Data Transport (VxWorks 전용)
 *
 * TransportSHM 공용모듈을 래핑하여 IDataTransport 인터페이스를 구현한다.
 * VxWorks에서만 활성화되며, 타 플랫폼에서는 컴파일되지 않는다.
 *
 * 설계 참조: DevGuide/01_TransportSHM_Design.md
 *            DevGuide/03_LegacyLib_SHM_Integration_Design.md
 */
#pragma once

#include "data_transport_iface.hpp"

#if DDSFW_ENABLE_SHM

#include "ddsfw/transport_shm/shm_transport.hpp"
#include <atomic>

namespace legacy {
namespace transport {

/**
 * @class DataTransportShm
 * @brief SHM 기반 Data Transport 구현 (VxWorks 전용)
 *
 * TransportSHM 공용모듈을 사용하여 Agent와 Data Plane 통신을 수행한다.
 * - 송신: LegacyFrameHeader + Payload를 bytes로 조립하여 send_bytes() 호출
 * - 수신: bytes를 파싱하여 OnDataFrameFn 콜백으로 전달
 */
class DataTransportShm : public IDataTransport {
public:
    DataTransportShm();
    ~DataTransportShm() override;

    // Non-copyable
    DataTransportShm(const DataTransportShm&) = delete;
    DataTransportShm& operator=(const DataTransportShm&) = delete;

    // IDataTransport 인터페이스 구현
    bool open(const DataTransportConfig& cfg, LogFn log = nullptr) noexcept override;
    void close() noexcept override;
    void set_on_frame(OnDataFrameFn cb, void* user) noexcept override;
    bool start() noexcept override;
    void stop() noexcept override;
    bool send_frame(const LegacyFrameHeader& hdr,
                    const uint8_t* payload,
                    uint32_t payload_len) noexcept override;
    bool is_open() const noexcept override;
    bool is_running() const noexcept override;
    DataTransportKind kind() const noexcept override { return DataTransportKind::SHM; }

private:
    /**
     * @brief SHM 프레임 수신 콜백 (static)
     *
     * TransportSHM의 OnFrameFn 시그니처에 맞춘 static 함수.
     * bytes를 LegacyFrameHeader + Payload로 파싱하여 사용자 콜백 호출.
     */
    static void on_shm_frame(const uint8_t* frame_bytes, uint32_t len, void* user) noexcept;

    /**
     * @brief 로그 콜백 변환 (static)
     */
    static void on_shm_log(ddsfw::transport_shm::LogLevel level, const char* msg) noexcept;

private:
    ddsfw::transport_shm::Transport shm_transport_;
    OnDataFrameFn on_frame_cb_;
    void* on_frame_user_;
    LogFn log_fn_;
    std::atomic<bool> opened_;

    // static 콜백에서 인스턴스 접근용 (thread_local 또는 user ptr 사용)
    static DataTransportShm* s_instance_;
};

}  // namespace transport
}  // namespace legacy

#endif  // DDSFW_ENABLE_SHM
