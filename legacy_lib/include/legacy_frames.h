#ifndef LEGACY_FRAMES_H
#define LEGACY_FRAMES_H

#include <stdint.h>

/**
 * @brief RIPC 공통 패킷 헤더 정의
 */
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;     ///< 'RIPC' (0x52495043)
    uint16_t version;   ///< 프로토콜 버전 (현재 1)
    uint16_t type;      ///< 프레임 타입 (MSG_FRAME_*, MSG_DATA_*)
    uint32_t corr_id;   ///< 요청/응답 매칭용 ID
    uint32_t length;    ///< 페이로드 길이 (바이트)
    uint64_t ts_ns;     ///< 타임스탬프 (나노초)
} LegacyFrameHeader;
#pragma pack(pop)

#define RIPC_MAGIC 0x52495043
#define RIPC_VERSION 1

/* --- Frame Types --- */

// Control Plane (CBOR/JSON)
#define MSG_FRAME_REQ      0x1000
#define MSG_FRAME_RSP      0x1001
#define MSG_FRAME_EVT      0x1002

// Data Plane JSON (CBOR/JSON)
#define MSG_DATA_REQ_JSON  0x2000
#define MSG_DATA_RSP_JSON  0x2001
#define MSG_DATA_EVT_JSON  0x2002

// Data Plane STRUCT (Binary)
#define MSG_DATA_REQ_STRUCT 0x2100
#define MSG_DATA_RSP_STRUCT 0x2101
#define MSG_DATA_EVT_STRUCT 0x2102

/**
 * @brief Data STRUCT용 엔벨로프 (Data Plane Payload 시작점) - v3.0 Spec
 */
#pragma pack(push, 1)
typedef struct {
    uint32_t magic;      ///< 매직 넘버: 0x44495043 ("DIPC" network byte order via htonl)
    uint8_t  ver;        ///< 프로토콜 버전: 1
    uint8_t  kind;       ///< PayloadKind: 2 = DataStruct
    uint32_t topic_id;   ///< topic_name의 FNV-1a 해시
    uint32_t abi_hash;   ///< Wire struct ABI 해시 (버전 검증용)
    uint32_t data_len;   ///< Wire struct 페이로드 길이 (bytes)
    uint16_t reserved;   ///< 예약 (0으로 설정)
} DataEnvelope;

/** DataEnvelope 매직 상수 ('DIPC') */
#define DATA_ENVELOPE_MAGIC  0x44495043
#define DATA_ENVELOPE_VER    1

/** PayloadKind 열거 */
#define PAYLOAD_KIND_CONTROL_JSON   0
#define PAYLOAD_KIND_DATA_JSON      1
#define PAYLOAD_KIND_DATA_STRUCT    2

/**
 * @brief Data STRUCT 응답 구조체 (8 bytes) - v3.0 Spec
 */
typedef struct {
    uint8_t  status;     ///< 0=OK, 1=PARSE_ERROR, 2=UNKNOWN_TOPIC, 3=ABI_MISMATCH, 4=CONVERT_FAIL, 5=PUBLISH_FAIL
    uint32_t topic_id;   ///< 요청한 topic_id
    uint8_t  reserved[3];
} DataRspStruct;

#pragma pack(pop)

/**
 * @brief FNV-1a 32-bit 해시 계산 (topic_id 생성용)
 */
static inline uint32_t legacy_fnv1a_32(const char* str) {
    if (!str) return 0;
    uint32_t hash = 2166136261U;
    while (*str) {
        hash ^= (uint8_t)(*str++);
        hash *= 16777619U;
    }
    return hash;
}

#endif // LEGACY_FRAMES_H
