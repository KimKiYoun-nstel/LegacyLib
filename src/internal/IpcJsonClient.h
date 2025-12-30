#pragma once
#include "DkmRtpIpc.h"
#include "legacy_agent.h"
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <functional>

#ifdef _VXWORKS_
extern "C" {
#include <vxWorks.h>
#include <taskLib.h>
#include <semLib.h>
#include <sysLib.h>
}

// RAII wrapper for VxWorks mutex semaphore
class SemLockGuard {
public:
    explicit SemLockGuard(SEM_ID sem) : sem_(sem) {
        if (sem_) semTake(sem_, WAIT_FOREVER);
    }
    ~SemLockGuard() {
        if (sem_) semGive(sem_);
    }
private:
    SEM_ID sem_;
    SemLockGuard(const SemLockGuard&) = delete;
    SemLockGuard& operator=(const SemLockGuard&) = delete;
};
#else
#include <thread>
#include <mutex>
#endif

struct PendingRequest {
    LegacySimpleCb simple_cb;
    LegacyHelloCb hello_cb;
    void* user;
    // Add other callback types as needed
};

class IpcJsonClient {
public:
    IpcJsonClient();
    ~IpcJsonClient();

    LegacyStatus init(const LegacyConfig* cfg);
    void close();

    // Control Plane
    LegacyStatus sendHello(uint32_t timeout_ms, LegacyHelloCb cb, void* user);
    
    LegacyStatus createParticipant(const LegacyParticipantConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);
    LegacyStatus createPublisher(const LegacyPublisherConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);
    LegacyStatus createSubscriber(const LegacySubscriberConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);
    LegacyStatus createWriter(const LegacyWriterConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);
    LegacyStatus createReader(const LegacyReaderConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);
    LegacyStatus clearEntities(uint32_t timeout_ms, LegacySimpleCb cb, void* user);

    // QoS
    LegacyStatus getQosList(bool include_builtin, bool detail, uint32_t timeout_ms, LegacyQosListCb cb, void* user);
    LegacyStatus setQosProfile(const LegacyQosSetOptions* opt, uint32_t timeout_ms, LegacyQosSetCb cb, void* user);

    // Data Plane
    LegacyStatus writeJson(const LegacyWriteJsonOptions* opt, uint32_t timeout_ms, LegacyWriteCb cb, void* user);
    LegacyStatus writeStruct(const char* topic, const char* type_name, const void* user_struct, uint32_t timeout_ms, LegacyWriteCb cb, void* user);
    
    // Events
    LegacyStatus subscribeEvent(const char* topic, const char* type, LegacyEventCb cb, void* user);
    LegacyStatus subscribeTyped(const char* topic, const char* type_name, LegacyTypedEventCb cb, void* user);

    // Type Adapters
    LegacyStatus registerTypeAdapter(const LegacyTypeAdapter* adapter);
    LegacyStatus unregisterTypeAdapter(const char* topic, const char* type_name);

private:
#ifdef _VXWORKS_
    static void recvTaskEntry(uintptr_t arg);
#endif
    void receiveLoop();
    uint32_t generateRequestId();
    void registerRequest(uint32_t reqId, const PendingRequest& req);
    
    // Logging helper (printf-style)
    void logInfo(const char* fmt, ...);
    
    // Helper to send raw JSON with header
    LegacyStatus sendRequest(const std::string& json_body, uint16_t type = 0x1000, uint32_t req_id = 0);

    // Type Adapter Helper
    const LegacyTypeAdapter* findTypeAdapter(const char* topic, const char* type_name);

private:
    DkmRtpIpc transport_;
    LegacyConfig config_;
    
#ifdef _VXWORKS_
    TASK_ID recv_task_;
#else
    std::thread recv_thread_;
#endif
    std::atomic<bool> running_;
    
#ifdef _VXWORKS_
    SEM_ID req_sem_;
#else
    std::mutex req_mutex_;
#endif
    std::map<uint32_t, PendingRequest> pending_requests_;
    std::atomic<uint32_t> next_req_id_;

    // Event Subscriptions
    struct Subscription {
        LegacyEventCb event_cb;
        LegacyTypedEventCb typed_cb;
        void* user;
    };
    // Key: "topic/type"
#ifdef _VXWORKS_
    SEM_ID sub_sem_;
#else
    std::mutex sub_mutex_;
#endif
    std::map<std::string, std::vector<Subscription>> subscriptions_;

    // Type Adapters
    // Key: "topic/type"
#ifdef _VXWORKS_
    SEM_ID adapter_sem_;
#else
    std::mutex adapter_mutex_;
#endif
    std::map<std::string, LegacyTypeAdapter> type_adapters_;
    // Perf accumulation (when DEMO_PERF_INSTRUMENTATION enabled)
    std::atomic<uint64_t> parse_ns_total_{0};
    std::atomic<uint32_t> parse_count_{0};
    std::atomic<uint64_t> cbor_ns_total_{0};
    std::atomic<uint32_t> cbor_count_{0};
    std::atomic<uint64_t> write_ns_total_{0};
    std::atomic<uint32_t> write_count_{0};

    // Per-instance reusable CBOR buffer to avoid per-call allocations
    std::vector<uint8_t> cbor_buf_;

public:
    // Fill a LegacyPerfStats structure with accumulated library perf counters
    void getPerfStats(LegacyPerfStats* out_stats);
};
