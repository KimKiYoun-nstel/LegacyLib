/**
 * @file data_transport_shm.cpp
 * @brief SHM 기반 Data Transport 구현 (VxWorks 전용)
 *
 * TransportSHM 공용모듈을 래핑하여 Data Plane 통신을 수행한다.
 */

#include "data_transport_shm.hpp"

#if DDSFW_ENABLE_SHM

#include <cstring>
#include <cstdio>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace legacy {
namespace transport {

// Static 인스턴스 포인터 (단일 인스턴스 가정)
DataTransportShm* DataTransportShm::s_instance_ = nullptr;

//------------------------------------------------------------------------------
// 생성자/소멸자
//------------------------------------------------------------------------------

DataTransportShm::DataTransportShm()
    : on_frame_cb_(nullptr)
    , on_frame_user_(nullptr)
    , log_fn_(nullptr)
    , opened_(false)
{
}

DataTransportShm::~DataTransportShm() {
    close();
}

//------------------------------------------------------------------------------
// IDataTransport 구현
//------------------------------------------------------------------------------

bool DataTransportShm::open(const DataTransportConfig& cfg, LogFn log) noexcept {
    if (opened_.load()) {
        return false;  // 이미 열림
    }

    log_fn_ = log;
    s_instance_ = this;

    // TransportSHM Config 생성
    ddsfw::transport_shm::Config shm_cfg{};
    shm_cfg.side = ddsfw::transport_shm::Side::Legacy;
    shm_cfg.creator = false;  // Legacy는 joiner
    shm_cfg.shm_name = cfg.shm_name ? cfg.shm_name : "/dds_fw_data";
    shm_cfg.notify_la = cfg.notify_la ? cfg.notify_la : "/dds_fw_sem_la";
    shm_cfg.notify_al = cfg.notify_al ? cfg.notify_al : "/dds_fw_sem_al";
    shm_cfg.ring_bytes = cfg.ring_bytes > 0 ? cfg.ring_bytes : (4 * 1024 * 1024);
    shm_cfg.max_frame = cfg.max_frame > 0 ? cfg.max_frame : 65536;
    shm_cfg.wait_ms = cfg.wait_ms > 0 ? cfg.wait_ms : 100;
    shm_cfg.notify_kind = ddsfw::transport_shm::NotifyKind::PosixNamedSemaphore;

    // 로그 콜백 변환
    ddsfw::transport_shm::LogFn shm_log = nullptr;
    if (log_fn_) {
        shm_log = &DataTransportShm::on_shm_log;
    }

    // SHM Transport 열기
    if (!shm_transport_.open(shm_cfg, shm_log)) {
        if (log_fn_) {
            log_fn_(4, "[DataTransportShm] Failed to open SHM transport");
        }
        return false;
    }

    // 프레임 수신 콜백 설정
    shm_transport_.set_on_frame(&DataTransportShm::on_shm_frame, this);

    opened_.store(true);

    if (log_fn_) {
        char buf[256];
        std::snprintf(buf, sizeof(buf),
                      "[DataTransportShm] Opened: shm=%s, ring=%u, max_frame=%u",
                      shm_cfg.shm_name, shm_cfg.ring_bytes, shm_cfg.max_frame);
        log_fn_(2, buf);
    }

    return true;
}

void DataTransportShm::close() noexcept {
    if (!opened_.load()) {
        return;
    }

    stop();
    shm_transport_.close();
    opened_.store(false);
    s_instance_ = nullptr;

    if (log_fn_) {
        log_fn_(2, "[DataTransportShm] Closed");
    }
}

void DataTransportShm::set_on_frame(OnDataFrameFn cb, void* user) noexcept {
    on_frame_cb_ = cb;
    on_frame_user_ = user;
}

bool DataTransportShm::start() noexcept {
    if (!opened_.load()) {
        return false;
    }

    if (!shm_transport_.start()) {
        if (log_fn_) {
            log_fn_(4, "[DataTransportShm] Failed to start rx thread");
        }
        return false;
    }

    if (log_fn_) {
        log_fn_(2, "[DataTransportShm] Started");
    }

    return true;
}

void DataTransportShm::stop() noexcept {
    shm_transport_.stop();

    if (log_fn_) {
        log_fn_(2, "[DataTransportShm] Stopped");
    }
}

bool DataTransportShm::send_frame(const LegacyFrameHeader& hdr,
                                   const uint8_t* payload,
                                   uint32_t payload_len) noexcept {
    if (!opened_.load()) {
        return false;
    }

    // frame_bytes = [LegacyFrameHeader][Payload]
    constexpr size_t HDR_SIZE = sizeof(LegacyFrameHeader);
    const size_t total_len = HDR_SIZE + payload_len;

    // 스택 버퍼 사용 (max_frame 이내라고 가정)
    // 대용량은 heap 할당 필요 시 수정
    uint8_t frame_buf[65536 + HDR_SIZE];
    if (total_len > sizeof(frame_buf)) {
        if (log_fn_) {
            log_fn_(4, "[DataTransportShm] Frame too large");
        }
        return false;
    }

    // 헤더 복사 (네트워크 바이트 오더 변환은 호출자 책임)
    std::memcpy(frame_buf, &hdr, HDR_SIZE);

    // 페이로드 복사
    if (payload && payload_len > 0) {
        std::memcpy(frame_buf + HDR_SIZE, payload, payload_len);
    }

    // SHM으로 전송
    return shm_transport_.send_bytes(frame_buf, static_cast<uint32_t>(total_len));
}

bool DataTransportShm::is_open() const noexcept {
    return opened_.load();
}

bool DataTransportShm::is_running() const noexcept {
    return shm_transport_.is_running();
}

//------------------------------------------------------------------------------
// Static 콜백
//------------------------------------------------------------------------------

void DataTransportShm::on_shm_frame(const uint8_t* frame_bytes, uint32_t len, void* user) noexcept {
    auto* self = static_cast<DataTransportShm*>(user);
    if (!self || !self->on_frame_cb_) {
        return;
    }

    constexpr size_t HDR_SIZE = sizeof(LegacyFrameHeader);
    if (len < HDR_SIZE) {
        if (self->log_fn_) {
            self->log_fn_(3, "[DataTransportShm] Frame too short, discarded");
        }
        return;
    }

    // 헤더 파싱
    LegacyFrameHeader hdr;
    std::memcpy(&hdr, frame_bytes, HDR_SIZE);

    // 매직 넘버 검증
    if (hdr.magic != RIPC_MAGIC) {
        if (self->log_fn_) {
            self->log_fn_(3, "[DataTransportShm] Invalid magic, discarded");
        }
        return;
    }

    // 페이로드 포인터 및 길이
    const uint8_t* payload = frame_bytes + HDR_SIZE;
    uint32_t payload_len = len - HDR_SIZE;

    // 사용자 콜백 호출
    self->on_frame_cb_(&hdr, payload, payload_len, self->on_frame_user_);
}

void DataTransportShm::on_shm_log(ddsfw::transport_shm::LogLevel level, const char* msg) noexcept {
    if (s_instance_ && s_instance_->log_fn_) {
        int legacy_level = 2;  // Info
        switch (level) {
            case ddsfw::transport_shm::LogLevel::Debug: legacy_level = 1; break;
            case ddsfw::transport_shm::LogLevel::Info:  legacy_level = 2; break;
            case ddsfw::transport_shm::LogLevel::Warn:  legacy_level = 3; break;
            case ddsfw::transport_shm::LogLevel::Error: legacy_level = 4; break;
        }
        s_instance_->log_fn_(legacy_level, msg);
    }
}

}  // namespace transport
}  // namespace legacy

#endif  // DDSFW_ENABLE_SHM
