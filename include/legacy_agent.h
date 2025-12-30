#ifndef LEGACY_AGENT_H
#define LEGACY_AGENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Instrumentation: use single macro `DEMO_PERF_INSTRUMENTATION`.
 * Older `DEMO_TIMING_INSTRUMENTATION` uses were removed to keep
 * instrumentation controlled by the single Makefile macro.
 */

/* --- Basic Types & Initialization --- */

typedef struct LegacyAgentHandleImpl* LEGACY_HANDLE;

typedef enum {
    LEGACY_OK = 0,
    LEGACY_ERR_PARAM,
    LEGACY_ERR_TRANSPORT,
    LEGACY_ERR_TIMEOUT,
    LEGACY_ERR_PROTO,
    LEGACY_ERR_CLOSED
} LegacyStatus;

typedef void (*LegacyLogCb)(int level, const char* msg, void* user);
// level: 0=TRACE, 1=DEBUG, 2=INFO, 3=WARN, 4=ERR

typedef struct {
    const char* agent_ip;
    uint16_t    agent_port;

    uint32_t    recv_task_priority;
    uint32_t    recv_task_stack;
    uint32_t    send_task_priority;
    uint32_t    send_task_stack;

    LegacyLogCb log_cb;
    void*       log_user;
} LegacyConfig;

LegacyStatus legacy_agent_init(const LegacyConfig* cfg, LEGACY_HANDLE* outHandle);
void         legacy_agent_close(LEGACY_HANDLE h);

// Global log callback registration (optional)
// If demos/apps want a global adapter for library logs they can register here.
void legacy_agent_set_log_callback(LegacyLogCb cb, void* user);
void legacy_agent_get_log_callback(LegacyLogCb* out_cb, void** out_user);

/* --- Performance Instrumentation Exposure --- */
typedef struct {
    uint64_t ipc_parse_ns_total;    // total time spent parsing JSON -> ns
    uint32_t ipc_parse_count;
    uint64_t ipc_cbor_ns_total;     // total time spent json::to_cbor -> ns
    uint32_t ipc_cbor_count;
    uint64_t transport_send_us_total; // total transport send time (microseconds)
    uint32_t transport_send_count;
    uint64_t write_ns_total;        // total time spent in writeJson() (nanoseconds)
    uint32_t write_count;
} LegacyPerfStats;

/* Query library-side accumulated perf counters. Returns LEGACY_OK if handle valid.
 * Counters are accumulated only when code is built with DEMO_PERF_INSTRUMENTATION.
 */
LegacyStatus legacy_agent_get_perf_stats(LEGACY_HANDLE h, LegacyPerfStats* out_stats);

/* --- Common Response Structures --- */

typedef uint32_t LegacyRequestId;

typedef struct {
    bool        ok;       // Response JSON 'ok' field
    int         err;      // Response JSON 'err' (0 if missing)
    const char* msg;      // Response JSON 'msg' (NULL if missing)

    const char* raw_json; // Full response JSON (optional, for debug)
} LegacySimpleResult;

typedef void (*LegacySimpleCb)(LEGACY_HANDLE h,
                               LegacyRequestId reqId,
                               const LegacySimpleResult* res,
                               void* user);

/* --- Control Plane API --- */

// Hello
typedef struct {
    int         proto;          // Hello response proto version (-1 if missing)
    const char* caps_raw_json;  // Capability JSON
} LegacyHelloInfo;

typedef void (*LegacyHelloCb)(
    LEGACY_HANDLE h,
    LegacyRequestId reqId,
    const LegacySimpleResult* res,
    const LegacyHelloInfo* info,
    void* user);

LegacyStatus legacy_agent_hello(
    LEGACY_HANDLE h,
    uint32_t timeout_ms,
    LegacyHelloCb cb,
    void* user);

// Create Entities
typedef struct {
    int         domain;
    const char* qos;
} LegacyParticipantConfig;

LegacyStatus legacy_agent_create_participant(
    LEGACY_HANDLE h,
    const LegacyParticipantConfig* cfg,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

typedef struct {
    int         domain;
    const char* name;
    const char* qos;
} LegacyPublisherConfig;

LegacyStatus legacy_agent_create_publisher(
    LEGACY_HANDLE h,
    const LegacyPublisherConfig* cfg,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

typedef struct {
    int         domain;
    const char* name;
    const char* qos;
} LegacySubscriberConfig;

LegacyStatus legacy_agent_create_subscriber(
    LEGACY_HANDLE h,
    const LegacySubscriberConfig* cfg,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

typedef struct {
    int         domain;
    const char* publisher;
    const char* topic;
    const char* type;
    const char* qos;
} LegacyWriterConfig;

LegacyStatus legacy_agent_create_writer(
    LEGACY_HANDLE h,
    const LegacyWriterConfig* cfg,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

typedef struct {
    int         domain;
    const char* subscriber;
    const char* topic;
    const char* type;
    const char* qos;
} LegacyReaderConfig;

LegacyStatus legacy_agent_create_reader(
    LEGACY_HANDLE h,
    const LegacyReaderConfig* cfg,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

LegacyStatus legacy_agent_clear_dds_entities(
    LEGACY_HANDLE h,
    uint32_t timeout_ms,
    LegacySimpleCb cb,
    void* user);

/* --- QoS API --- */

typedef struct {
    const char* library;
    const char* profile;
    const char* xml;
} LegacyQosProfile;

typedef struct {
    const LegacyQosProfile* profiles;
    size_t                  count;
    const char*             raw_json;
} LegacyQosList;

typedef void (*LegacyQosListCb)(
    LEGACY_HANDLE h,
    LegacyRequestId reqId,
    const LegacySimpleResult* res,
    const LegacyQosList* list,
    void* user);

LegacyStatus legacy_agent_get_qos_list(
    LEGACY_HANDLE h,
    bool include_builtin,
    bool detail,
    uint32_t timeout_ms,
    LegacyQosListCb cb,
    void* user);

typedef struct {
    const char* library;
    const char* profile;
    const char* xml;
} LegacyQosSetOptions;

typedef void (*LegacyQosSetCb)(
    LEGACY_HANDLE h,
    LegacyRequestId reqId,
    const LegacySimpleResult* res,
    void* user);

LegacyStatus legacy_agent_set_qos_profile(
    LEGACY_HANDLE h,
    const LegacyQosSetOptions* opt,
    uint32_t timeout_ms,
    LegacyQosSetCb cb,
    void* user);

/* --- Data Plane API (Write) --- */

typedef struct {
    const char* topic;
    const char* type;
    const char* data_json;
    // Optional args for identifying writer if needed
    int         domain;
    const char* publisher;
    const char* qos;
} LegacyWriteJsonOptions;

typedef void (*LegacyWriteCb)(
    LEGACY_HANDLE h,
    LegacyRequestId reqId,
    const LegacySimpleResult* res,
    void* user);

LegacyStatus legacy_agent_write_json(
    LEGACY_HANDLE h,
    const LegacyWriteJsonOptions* opt,
    uint32_t timeout_ms,
    LegacyWriteCb cb,
    void* user);

LegacyStatus legacy_agent_write_struct(
    LEGACY_HANDLE h,
    const char* topic,
    const char* type_name,
    const void* user_struct,
    uint32_t timeout_ms,
    LegacyWriteCb cb,
    void* user);

/* --- Data Plane API (Events/Read) --- */

typedef struct {
    const char* topic;
    const char* type;
    const char* data_json;
    const char* raw_json;
} LegacyEvent;

typedef void (*LegacyEventCb)(
    LEGACY_HANDLE h,
    const LegacyEvent* evt,
    void* user);

LegacyStatus legacy_agent_subscribe_event(
    LEGACY_HANDLE h,
    const char* topic,
    const char* type,
    LegacyEventCb cb,
    void* user);

typedef void (*LegacyTypedEventCb)(
    LEGACY_HANDLE h,
    const LegacyEvent* evt,
    void* user_struct,
    void* user);

LegacyStatus legacy_agent_subscribe_typed(
    LEGACY_HANDLE h,
    const char* topic,
    const char* type_name,
    LegacyTypedEventCb cb,
    void* user);

/* --- Type Adapter API --- */

typedef struct {
    const char* topic;
    const char* type_name;
} LegacyTypeKey;

typedef struct LegacyTypeAdapter {
    LegacyTypeKey key;

    const char* (*encode)(const void* user_struct, void* user_ctx);
    bool        (*decode)(const char* data_json, void* out_user_struct, void* user_ctx);
    const char* (*make_default)(void* user_ctx);

    void* user_ctx;
} LegacyTypeAdapter;

LegacyStatus legacy_agent_register_type_adapter(
    LEGACY_HANDLE h,
    const LegacyTypeAdapter* adapter);

LegacyStatus legacy_agent_unregister_type_adapter(
    LEGACY_HANDLE h,
    const char* topic,
    const char* type_name);

#ifdef __cplusplus
}
#endif

#endif // LEGACY_AGENT_H
