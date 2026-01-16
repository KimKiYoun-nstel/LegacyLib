/**
 * @file shm_transport.cpp
 * @brief TransportSHM 핵심 구현
 *
 * Transport 클래스 구현 (오케스트레이션 및 rx thread 관리).
 * 공용모듈 API를 bytes-only로 유지하며 내부 모듈을 조합한다.
 */

#include "ddsfw/transport_shm/shm_transport.hpp"
#include "shm_layout.hpp"
#include "shm_region.hpp"
#include "shm_notify.hpp"
#include "shm_ring_spsc.hpp"

#include <atomic>
#include <thread>
#include <cstring>
#include <cstdio>

namespace ddsfw {
namespace transport_shm {

//------------------------------------------------------------------------------
// 로그 헬퍼 매크로
//------------------------------------------------------------------------------

#define SHM_LOG(level, fmt, ...)                                              \
    do {                                                                       \
        if (log_fn_) {                                                         \
            char buf[256];                                                     \
            std::snprintf(buf, sizeof(buf), "[TransportSHM] " fmt, ##__VA_ARGS__); \
            log_fn_(level, buf);                                               \
        }                                                                      \
    } while (0)

#define SHM_DEBUG(fmt, ...) SHM_LOG(LogLevel::Debug, fmt, ##__VA_ARGS__)
#define SHM_INFO(fmt, ...)  SHM_LOG(LogLevel::Info, fmt, ##__VA_ARGS__)
#define SHM_WARN(fmt, ...)  SHM_LOG(LogLevel::Warn, fmt, ##__VA_ARGS__)
#define SHM_ERROR(fmt, ...) SHM_LOG(LogLevel::Error, fmt, ##__VA_ARGS__)

//------------------------------------------------------------------------------
// TransportImpl 클래스
//------------------------------------------------------------------------------

/**
 * @class TransportImpl
 * @brief Transport 내부 구현 (pimpl)
 */
class TransportImpl {
public:
    TransportImpl() = default;
    ~TransportImpl() { close(); }

    bool open(const Config& cfg, LogFn log) noexcept;
    void close() noexcept;

    void set_on_frame(OnFrameFn cb, void* user) noexcept {
        on_frame_ = cb;
        on_frame_user_ = user;
    }

    bool start() noexcept;
    void stop() noexcept;

    bool send_bytes(const uint8_t* frame_bytes, uint32_t len) noexcept;

    uint32_t epoch() const noexcept;
    uint32_t drops_tx() const noexcept { return tx_ring_.drops(); }
    uint32_t drops_rx() const noexcept { return 0; }  // 현재 미사용

    bool is_open() const noexcept { return region_.is_open(); }
    bool is_running() const noexcept { return running_.load(std::memory_order_acquire); }

private:
    void rx_thread_func() noexcept;

private:
    // 설정
    Config cfg_{};
    LogFn log_fn_ = nullptr;

    // SHM Region
    ShmRegion region_;

    // Notify (LA/AL 각각)
    ShmNotify notify_la_;  // Legacy→Agent (Agent가 consumer)
    ShmNotify notify_al_;  // Agent→Legacy (Legacy가 consumer)

    // Ring 버퍼
    ShmRingSPSC tx_ring_;  // 송신용 (Agent: AL, Legacy: LA)
    ShmRingSPSC rx_ring_;  // 수신용 (Agent: LA, Legacy: AL)

    // 포인터 캐시
    ShmGlobalHeader* global_hdr_ = nullptr;

    // 콜백
    OnFrameFn on_frame_ = nullptr;
    void* on_frame_user_ = nullptr;

    // rx thread
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_requested_{false};
    std::thread rx_thread_;
};

//------------------------------------------------------------------------------
// TransportImpl 구현
//------------------------------------------------------------------------------

bool TransportImpl::open(const Config& cfg, LogFn log) noexcept {
    cfg_ = cfg;
    log_fn_ = log;

    SHM_INFO("Opening SHM transport (side=%s, creator=%s)",
             cfg.side == Side::Agent ? "Agent" : "Legacy",
             cfg.creator ? "true" : "false");

    // SHM 크기 계산
    size_t total_size = calc_shm_total_size(cfg.ring_bytes);
    SHM_DEBUG("SHM total size: %zu bytes (ring_bytes=%u)", total_size, cfg.ring_bytes);

    // SHM Region 열기
    if (!region_.open(cfg.shm_name, total_size, cfg.creator)) {
        SHM_ERROR("Failed to open SHM region: %s", cfg.shm_name);
        return false;
    }

    uint8_t* base = static_cast<uint8_t*>(region_.ptr());

    // Global Header
    global_hdr_ = reinterpret_cast<ShmGlobalHeader*>(base);

    if (cfg.creator) {
        // 초기화
        global_hdr_->init(cfg.shm_name, cfg.notify_la, cfg.notify_al,
                          cfg.max_frame, cfg.ring_bytes);

        // LA Ring 초기화
        ShmRingHeader* la_hdr = reinterpret_cast<ShmRingHeader*>(
            base + offset_la_ring());
        la_hdr->init(cfg.ring_bytes);

        // AL Ring 초기화
        ShmRingHeader* al_hdr = reinterpret_cast<ShmRingHeader*>(
            base + offset_al_ring(cfg.ring_bytes));
        al_hdr->init(cfg.ring_bytes);

        global_hdr_->set_ready();
        SHM_INFO("SHM initialized (epoch=%u)", global_hdr_->epoch);
    } else {
        // Joiner: 검증
        if (!global_hdr_->validate()) {
            SHM_ERROR("SHM validation failed (magic/ver mismatch)");
            region_.close();
            return false;
        }
        if (!global_hdr_->is_ready()) {
            SHM_ERROR("SHM not ready");
            region_.close();
            return false;
        }
        if (global_hdr_->max_frame != cfg.max_frame ||
            global_hdr_->ring_bytes != cfg.ring_bytes) {
            SHM_ERROR("SHM config mismatch (max_frame=%u/%u, ring_bytes=%u/%u)",
                      global_hdr_->max_frame, cfg.max_frame,
                      global_hdr_->ring_bytes, cfg.ring_bytes);
            region_.close();
            return false;
        }
        SHM_INFO("Joined SHM (epoch=%u)", global_hdr_->epoch);
    }

    // Notify 열기
    // Agent: notify_la는 consumer(wait), notify_al는 producer(post)
    // Legacy: notify_la는 producer(post), notify_al는 consumer(wait)
    bool la_creator = cfg.creator;
    bool al_creator = cfg.creator;

    if (!notify_la_.open(cfg.notify_la, la_creator)) {
        SHM_ERROR("Failed to open notify_la: %s", cfg.notify_la);
        region_.close();
        return false;
    }
    if (!notify_al_.open(cfg.notify_al, al_creator)) {
        SHM_ERROR("Failed to open notify_al: %s", cfg.notify_al);
        notify_la_.close();
        region_.close();
        return false;
    }

    // Ring 연결
    ShmRingHeader* la_hdr = reinterpret_cast<ShmRingHeader*>(
        base + offset_la_ring());
    uint8_t* la_buf = base + offset_la_buffer();

    ShmRingHeader* al_hdr = reinterpret_cast<ShmRingHeader*>(
        base + offset_al_ring(cfg.ring_bytes));
    uint8_t* al_buf = base + offset_al_buffer(cfg.ring_bytes);

    if (cfg.side == Side::Agent) {
        // Agent: TX=AL, RX=LA
        tx_ring_.attach(al_hdr, al_buf, &notify_al_, cfg.max_frame);
        rx_ring_.attach(la_hdr, la_buf, &notify_la_, cfg.max_frame);
    } else {
        // Legacy: TX=LA, RX=AL
        tx_ring_.attach(la_hdr, la_buf, &notify_la_, cfg.max_frame);
        rx_ring_.attach(al_hdr, al_buf, &notify_al_, cfg.max_frame);
    }

    SHM_INFO("SHM transport opened successfully");
    return true;
}

void TransportImpl::close() noexcept {
    stop();

    notify_al_.close();
    notify_la_.close();
    region_.close();

    global_hdr_ = nullptr;

    SHM_INFO("SHM transport closed");
}

bool TransportImpl::start() noexcept {
    if (!is_open()) {
        SHM_ERROR("Cannot start: transport not open");
        return false;
    }

    if (running_.load(std::memory_order_acquire)) {
        SHM_WARN("Already running");
        return true;
    }

    stop_requested_.store(false, std::memory_order_release);
    running_.store(true, std::memory_order_release);

    rx_thread_ = std::thread([this]() { rx_thread_func(); });

    SHM_INFO("RX thread started");
    return true;
}

void TransportImpl::stop() noexcept {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    stop_requested_.store(true, std::memory_order_release);

    // notify를 post해서 wait 중인 스레드 깨우기
    if (cfg_.side == Side::Agent) {
        notify_la_.post();  // Agent RX는 LA에서 wait
    } else {
        notify_al_.post();  // Legacy RX는 AL에서 wait
    }

    if (rx_thread_.joinable()) {
        rx_thread_.join();
    }

    running_.store(false, std::memory_order_release);
    SHM_INFO("RX thread stopped");
}

bool TransportImpl::send_bytes(const uint8_t* frame_bytes, uint32_t len) noexcept {
    if (!is_open()) {
        return false;
    }

    return tx_ring_.push(frame_bytes, len);
}

uint32_t TransportImpl::epoch() const noexcept {
    return global_hdr_ ? global_hdr_->epoch : 0;
}

void TransportImpl::rx_thread_func() noexcept {
    SHM_DEBUG("RX thread enter");

    while (!stop_requested_.load(std::memory_order_acquire)) {
        // drain 방식으로 여러 레코드 연속 처리
        uint32_t processed = rx_ring_.drain(
            [](const uint8_t* frame_bytes, uint32_t len, void* user) noexcept {
                auto* self = static_cast<TransportImpl*>(user);
                if (self->on_frame_) {
                    self->on_frame_(frame_bytes, len, self->on_frame_user_);
                }
            },
            this,
            16  // 한 번에 최대 16개
        );

        if (processed == 0) {
            // empty - wait
            ShmNotify* wait_notify = (cfg_.side == Side::Agent)
                                         ? &notify_la_
                                         : &notify_al_;
            wait_notify->wait(cfg_.wait_ms);
        }
    }

    SHM_DEBUG("RX thread exit");
}

//------------------------------------------------------------------------------
// Transport 구현 (pimpl wrapper)
//------------------------------------------------------------------------------

Transport::Transport() : impl_(std::make_unique<TransportImpl>()) {}

Transport::~Transport() {
    close();
}

bool Transport::open(const Config& cfg, LogFn log) noexcept {
    return impl_->open(cfg, log);
}

void Transport::close() noexcept {
    impl_->close();
}

void Transport::set_on_frame(OnFrameFn cb, void* user) noexcept {
    impl_->set_on_frame(cb, user);
}

bool Transport::start() noexcept {
    return impl_->start();
}

void Transport::stop() noexcept {
    impl_->stop();
}

bool Transport::send_bytes(const uint8_t* frame_bytes, uint32_t len) noexcept {
    return impl_->send_bytes(frame_bytes, len);
}

uint32_t Transport::epoch() const noexcept {
    return impl_->epoch();
}

uint32_t Transport::drops_tx() const noexcept {
    return impl_->drops_tx();
}

uint32_t Transport::drops_rx() const noexcept {
    return impl_->drops_rx();
}

bool Transport::is_open() const noexcept {
    return impl_->is_open();
}

bool Transport::is_running() const noexcept {
    return impl_->is_running();
}

}  // namespace transport_shm
}  // namespace ddsfw
