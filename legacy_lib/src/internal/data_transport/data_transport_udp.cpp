/**
 * @file data_transport_udp.cpp
 * @brief UDP 기반 Data Transport 구현
 *
 * 기존 DkmRtpIpc를 래핑하여 Data Plane 통신을 수행한다.
 */

#include "data_transport_udp.hpp"
#include <cstring>
#include <cstdio>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#ifdef _VXWORKS_
#include <taskLib.h>
#include <sysLib.h>
#endif

namespace legacy {
namespace transport {

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------

DataTransportUdp::DataTransportUdp()
    : on_frame_cb_(nullptr)
    , on_frame_user_(nullptr)
    , log_fn_(nullptr)
    , opened_(false)
    , running_(false)
#ifdef _VXWORKS_
    , recv_task_(TASK_ID_ERROR)
#endif
    , recv_timeout_ms_(100)
{
}

DataTransportUdp::~DataTransportUdp() {
    close();
}

//------------------------------------------------------------------------------
// IDataTransport 구현
//------------------------------------------------------------------------------

bool DataTransportUdp::open(const DataTransportConfig& cfg, LogFn log) noexcept {
    if (opened_.load()) {
        return false;  // 이미 열림
    }

    log_fn_ = log;
    recv_timeout_ms_ = cfg.wait_ms > 0 ? cfg.wait_ms : 100;

    // UDP 소켓 초기화
    if (!transport_.init(cfg.agent_ip, cfg.agent_port)) {
        if (log_fn_) {
            log_fn_(4, "[DataTransportUdp] Failed to init UDP transport");
        }
        return false;
    }

    opened_.store(true);

    if (log_fn_) {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
                      "[DataTransportUdp] Opened: %s:%u",
                      cfg.agent_ip ? cfg.agent_ip : "null",
                      cfg.agent_port);
        log_fn_(2, buf);
    }

    return true;
}

void DataTransportUdp::close() noexcept {
    if (!opened_.load()) {
        return;
    }

    stop();
    transport_.close();
    opened_.store(false);

    if (log_fn_) {
        log_fn_(2, "[DataTransportUdp] Closed");
    }
}

void DataTransportUdp::set_on_frame(OnDataFrameFn cb, void* user) noexcept {
    on_frame_cb_ = cb;
    on_frame_user_ = user;
}

bool DataTransportUdp::start() noexcept {
    if (!opened_.load() || running_.load()) {
        return false;
    }

    running_.store(true);

#ifdef _VXWORKS_
    recv_task_ = taskSpawn(
        (char*)"tDataUdpRecv",
        100,   // priority
        0,     // options
        16384, // stack size
        (FUNCPTR)&DataTransportUdp::recv_task_entry,
        (int)(uintptr_t)this,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    );

    if (recv_task_ == TASK_ID_ERROR) {
        running_.store(false);
        if (log_fn_) {
            log_fn_(4, "[DataTransportUdp] Failed to spawn recv task");
        }
        return false;
    }
#else
    recv_thread_ = std::thread(&DataTransportUdp::receive_loop, this);
#endif

    if (log_fn_) {
        log_fn_(2, "[DataTransportUdp] Started");
    }

    return true;
}

void DataTransportUdp::stop() noexcept {
    if (!running_.load()) {
        return;
    }

    running_.store(false);

#ifdef _VXWORKS_
    if (recv_task_ != TASK_ID_ERROR) {
        // 태스크 종료 대기 (최대 500ms)
        for (int i = 0; i < 50 && taskIdVerify(recv_task_) == OK; ++i) {
            taskDelay(sysClkRateGet() / 100);  // 10ms
        }
        recv_task_ = TASK_ID_ERROR;
    }
#else
    if (recv_thread_.joinable()) {
        recv_thread_.join();
    }
#endif

    if (log_fn_) {
        log_fn_(2, "[DataTransportUdp] Stopped");
    }
}

bool DataTransportUdp::send_frame(const LegacyFrameHeader& hdr,
                                   const uint8_t* payload,
                                   uint32_t payload_len) noexcept {
    if (!opened_.load()) {
        return false;
    }

    // DkmRtpIpc::send()는 내부적으로 헤더를 생성하므로,
    // 페이로드만 전달하고 type/corr_id를 지정
    return transport_.send(payload, payload_len, hdr.type, hdr.corr_id);
}

bool DataTransportUdp::is_open() const noexcept {
    return opened_.load();
}

bool DataTransportUdp::is_running() const noexcept {
    return running_.load();
}

//------------------------------------------------------------------------------
// 수신 루프
//------------------------------------------------------------------------------

#ifdef _VXWORKS_
void DataTransportUdp::recv_task_entry(uintptr_t arg) {
    auto* self = reinterpret_cast<DataTransportUdp*>(arg);
    if (self) {
        self->receive_loop();
    }
}
#endif

void DataTransportUdp::receive_loop() noexcept {
    uint8_t buffer[65536 + sizeof(LegacyFrameHeader)];
    LegacyFrameHeader hdr;

    while (running_.load()) {
        int len = transport_.receive(buffer, sizeof(buffer),
                                     static_cast<int>(recv_timeout_ms_), &hdr);
        if (len <= 0) {
            // 타임아웃 또는 에러
            continue;
        }

        // 콜백 호출
        if (on_frame_cb_) {
            on_frame_cb_(&hdr, buffer, static_cast<uint32_t>(len), on_frame_user_);
        }
    }
}

}  // namespace transport
}  // namespace legacy
