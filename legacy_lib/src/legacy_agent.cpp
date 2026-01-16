#include "legacy_agent.h"
#include "IpcJsonClient.h"
#include <new>

struct LegacyAgentHandleImpl {
    IpcJsonClient client;
};

// Global log callback storage
static LegacyLogCb g_global_log_cb = nullptr;
static void* g_global_log_user = nullptr;

void legacy_agent_set_log_callback(LegacyLogCb cb, void* user) {
    g_global_log_cb = cb;
    g_global_log_user = user;
}

void legacy_agent_get_log_callback(LegacyLogCb* out_cb, void** out_user) {
    if (out_cb) *out_cb = g_global_log_cb;
    if (out_user) *out_user = g_global_log_user;
}

extern "C" {

LegacyStatus legacy_agent_init(const LegacyConfig* cfg, LEGACY_HANDLE* outHandle) {
    if (!cfg || !outHandle) return LEGACY_ERR_PARAM;
    
    LegacyAgentHandleImpl* impl = new (std::nothrow) LegacyAgentHandleImpl();
    if (!impl) return LEGACY_ERR_PARAM;

    LegacyStatus status = impl->client.init(cfg);
    if (status != LEGACY_OK) {
        delete impl;
        return status;
    }

    *outHandle = impl;
    return LEGACY_OK;
}

void legacy_agent_close(LEGACY_HANDLE h) {
    if (h) {
        h->client.close();
        delete h;
    }
}

LegacyStatus legacy_agent_hello(LEGACY_HANDLE h, uint32_t timeout_ms, LegacyHelloCb cb, void* user) {
    if (!h) return LEGACY_ERR_PARAM;
    return h->client.sendHello(timeout_ms, cb, user);
}

LegacyStatus legacy_agent_enable_shm(LEGACY_HANDLE h, const LegacyHelloInfo* info) {
    if (!h) return LEGACY_ERR_PARAM;
    return h->client.enableShm(info);
}

bool legacy_agent_is_shm_active(LEGACY_HANDLE h) {
    if (!h) return false;
    return h->client.isShmActive();
}

LegacyStatus legacy_agent_create_participant(LEGACY_HANDLE h, const LegacyParticipantConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h || !cfg) return LEGACY_ERR_PARAM;
    return h->client.createParticipant(cfg, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_create_publisher(LEGACY_HANDLE h, const LegacyPublisherConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h || !cfg) return LEGACY_ERR_PARAM;
    return h->client.createPublisher(cfg, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_create_subscriber(LEGACY_HANDLE h, const LegacySubscriberConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h || !cfg) return LEGACY_ERR_PARAM;
    return h->client.createSubscriber(cfg, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_create_writer(LEGACY_HANDLE h, const LegacyWriterConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h || !cfg) return LEGACY_ERR_PARAM;
    return h->client.createWriter(cfg, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_create_reader(LEGACY_HANDLE h, const LegacyReaderConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h || !cfg) return LEGACY_ERR_PARAM;
    return h->client.createReader(cfg, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_clear_dds_entities(LEGACY_HANDLE h, uint32_t timeout_ms, LegacySimpleCb cb, void* user) {
    if (!h) return LEGACY_ERR_PARAM;
    return h->client.clearEntities(timeout_ms, cb, user);
}

LegacyStatus legacy_agent_get_qos_list(LEGACY_HANDLE h, bool include_builtin, bool detail, uint32_t timeout_ms, LegacyQosListCb cb, void* user) {
    if (!h) return LEGACY_ERR_PARAM;
    return h->client.getQosList(include_builtin, detail, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_set_qos_profile(LEGACY_HANDLE h, const LegacyQosSetOptions* opt, uint32_t timeout_ms, LegacyQosSetCb cb, void* user) {
    if (!h || !opt) return LEGACY_ERR_PARAM;
    return h->client.setQosProfile(opt, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_write_json(LEGACY_HANDLE h, const LegacyWriteJsonOptions* opt, uint32_t timeout_ms, LegacyWriteCb cb, void* user) {
    if (!h || !opt) return LEGACY_ERR_PARAM;
    return h->client.writeJson(opt, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_write_struct(LEGACY_HANDLE h, const char* topic, const char* type_name, const void* user_struct, size_t struct_size, uint32_t timeout_ms, LegacyWriteCb cb, void* user) {
    if (!h || !topic || !type_name || !user_struct) return LEGACY_ERR_PARAM;
    return h->client.writeStruct(topic, type_name, user_struct, struct_size, timeout_ms, cb, user);
}

LegacyStatus legacy_agent_subscribe_event(LEGACY_HANDLE h, const char* topic, const char* type, LegacyEventCb cb, void* user) {
    if (!h || !topic || !type) return LEGACY_ERR_PARAM;
    return h->client.subscribeEvent(topic, type, cb, user);
}

LegacyStatus legacy_agent_subscribe_typed(LEGACY_HANDLE h, const char* topic, const char* type_name, LegacyTypedEventCb cb, void* user) {
    if (!h || !topic || !type_name) return LEGACY_ERR_PARAM;
    return h->client.subscribeTyped(topic, type_name, cb, user);
}

LegacyStatus legacy_agent_register_type_adapter(LEGACY_HANDLE h, const LegacyTypeAdapter* adapter) {
    if (!h || !adapter) return LEGACY_ERR_PARAM;
    return h->client.registerTypeAdapter(adapter);
}

LegacyStatus legacy_agent_get_perf_stats(LEGACY_HANDLE h, LegacyPerfStats* out_stats) {
    if (!h || !out_stats) return LEGACY_ERR_PARAM;
    h->client.getPerfStats(out_stats);
    return LEGACY_OK;
}

LegacyStatus legacy_agent_unregister_type_adapter(LEGACY_HANDLE h, const char* topic, const char* type_name) {
    if (!h || !topic || !type_name) return LEGACY_ERR_PARAM;
    return h->client.unregisterTypeAdapter(topic, type_name);
}

LegacyStatus legacy_agent_publish_json(LEGACY_HANDLE h, const char* topic, const char* json_str) {
    if (!h || !topic || !json_str) return LEGACY_ERR_PARAM;
    LegacyWriteJsonOptions opt;
    memset(&opt, 0, sizeof(opt));
    opt.topic = topic;
    opt.data_json = json_str;
    // 간편 전송이므로 비동기(timeout=0) 또는 기본 타임아웃 사용. 여기선 0 (Fire & Forget 느낌)
    return h->client.writeJson(&opt, 0, nullptr, nullptr);
}

LegacyStatus legacy_agent_publish_struct(LEGACY_HANDLE h, const char* topic, const char* type_name, const void* data, size_t size) {
    if (!h || !topic || !type_name || !data) return LEGACY_ERR_PARAM;
    return h->client.writeStruct(topic, type_name, data, size, 0, nullptr, nullptr);
}

} // extern "C"
