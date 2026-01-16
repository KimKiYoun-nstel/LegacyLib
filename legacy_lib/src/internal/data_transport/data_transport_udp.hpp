/**
 * @file data_transport_udp.hpp
 * @brief UDP 기반 Data Transport
 *
 * 기존 DkmRtpIpc를 래핑하여 IDataTransport 인터페이스를 구현한다.
 * 모든 플랫폼에서 사용 가능하며, Control/Data 모두 처리 가능.
 *
 * 설계 참조: DevGuide/03_LegacyLib_SHM_Integration_Design.md
 */
#pragma once

#include "data_transport_iface.hpp"
#include "../DkmRtpIpc.h"
#include <atomic>

#ifdef _VXWORKS_
extern "C" {
#include <vxWorks.h>
#include <taskLib.h>
}
#else
#include <thread>
#endif

namespace legacy {
namespace transport {

/**
 * @class DataTransportUdp
 * @brief UDP 기반 Data Transport 구현
 *
 * 기존 DkmRtpIpc를 사용하여 Agent와 통신한다.
 * Data Plane 전용으로 사용 시, SHM이 활성화된 경우에는 사용되지 않는다.
 */
class DataTransportUdp : public IDataTransport {
public:
    DataTransportUdp();
    ~DataTransportUdp() override;

    // Non-copyable
    DataTransportUdp(const DataTransportUdp&) = delete;
    DataTransportUdp& operator=(const DataTransportUdp&) = delete;

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
    DataTransportKind kind() const noexcept override { return DataTransportKind::UDP; }

private:
    /**
     * @brief 수신 루프 (스레드/태스크에서 실행)
     */
    void receive_loop() noexcept;

#ifdef _VXWORKS_
    static void recv_task_entry(uintptr_t arg);
#endif

private:
    DkmRtpIpc transport_;
    OnDataFrameFn on_frame_cb_;
    void* on_frame_user_;
    LogFn log_fn_;
    std::atomic<bool> opened_;
    std::atomic<bool> running_;

#ifdef _VXWORKS_
    TASK_ID recv_task_;
#else
    std::thread recv_thread_;
#endif

    uint32_t recv_timeout_ms_;
};

}  // namespace transport
}  // namespace legacy
