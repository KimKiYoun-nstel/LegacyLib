# Wire Struct 사용 가이드 (Legacy/UI용)

## 개요

이 문서는 Legacy 시스템 및 UI에서 Gateway와 바이너리 데이터를 교환하기 위한 Wire Struct 사용법을 설명합니다.

---

## 1. 필요한 파일

빌드 후 다음 파일들을 Legacy/UI 프로젝트에 복사하세요:

| 파일 | 위치 | 용도 |
|------|------|------|
| `idl_wire_structs.h` | `<build>/idlkit/gen/` | Wire Struct 정의 |
| `data_envelope.h` | (아래 제공) | DataEnvelope 헤더 |
| `topic_id_utils.h` | (아래 제공) | topic_id 해시 함수 |

---

## 2. 헤더 파일 (Legacy/UI용 C 코드)

### 2.1 data_envelope.h

```c
/**
 * @file data_envelope.h
 * @brief DataEnvelope struct definition (C compatible)
 */
#ifndef DATA_ENVELOPE_H
#define DATA_ENVELOPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/**
 * @brief Data Struct frame header (20 bytes)
 *
 * DataEnvelope and payload are serialized as Big-Endian (Network Byte Order).
 */
typedef struct DataEnvelope {
    uint32_t magic;      /**< magic: 0x44495043 ("DIPC" big-endian) */
    uint16_t ver;        /**< protocol version: 1 */
    uint16_t kind;       /**< 1=WRITE_REQ, 2=EVT */
    uint32_t topic_id;   /**< FNV-1a hash of topic_name */
    uint32_t abi_hash;   /**< wire struct ABI hash */
    uint32_t data_len;   /**< wire struct payload length (bytes) */
} DataEnvelope;

#pragma pack(pop)

/** DataEnvelope magic ('DIPC') */
#define DATA_ENVELOPE_MAGIC  0x44495043

/** DataEnvelope version */
#define DATA_ENVELOPE_VER    1

/** DataEnvelope kind values */
#define DATA_KIND_WRITE_REQ  1
#define DATA_KIND_EVT        2

#ifdef __cplusplus
}
#endif

#endif /* DATA_ENVELOPE_H */
```

### 2.2 topic_id_utils.h

```c
/**
 * @file topic_id_utils.h
 * @brief topic_id 계산용 FNV-1a 해시 함수 (C 호환)
 * 
 * topic_id는 topic_name 문자열의 FNV-1a 32-bit 해시값입니다.
 * Legacy/UI에서 런타임에 topic_name을 해싱하여 사용합니다.
 */
#ifndef TOPIC_ID_UTILS_H
#define TOPIC_ID_UTILS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FNV-1a 32-bit 해시 함수
 * @param data 입력 데이터 포인터
 * @param len 데이터 길이 (bytes)
 * @return 32-bit 해시값
 */
static inline uint32_t fnv1a_32(const uint8_t* data, size_t len)
{
    uint32_t hash = 2166136261u;  /* FNV offset basis */
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= 16777619u;  /* FNV prime */
    }
    return hash;
}

/**
 * @brief topic_name 문자열로부터 topic_id 계산
 * @param topic_name NULL 종료 문자열 (예: "MyTopic", "Alarms")
 * @return 32-bit topic_id
 * 
 * @note Gateway와 동일한 해시 함수를 사용하므로 
 *       같은 topic_name은 항상 같은 topic_id를 반환합니다.
 * 
 * @example
 *   uint32_t id = compute_topic_id("VehicleSpeed");
 *   // Gateway의 create_writer("VehicleSpeed", ...)와 동일한 id
 */
static inline uint32_t compute_topic_id(const char* topic_name)
{
    size_t len = 0;
    const char* p = topic_name;
    while (*p++) ++len;
    return fnv1a_32((const uint8_t*)topic_name, len);
}

#ifdef __cplusplus
}
#endif

#endif /* TOPIC_ID_UTILS_H */
```

---

## 3. 프레임 레이아웃

### 3.1 전체 구조

```
+--------------------------------------------------------------------+
| IPC Frame (UDP packet)                                              |
+--------------------------------------------------------------------+
| [RIPC Header: 24 bytes]                                             |
|   magic (4B): "RIPC"                                                |
|   version (2B)                                                      |
|   type (2B): 0x2100=REQ, 0x2101=RSP, 0x2102=EVT                     |
|   corr_id (4B): request/response correlation ID                     |
|   length (4B): payload bytes length                                 |
|   ts_ns (8B): timestamp (ns)                                        |
+--------------------------------------------------------------------+
| [DataEnvelope: 20 bytes]                                            |
|   magic (4B): 0x44495043 ("DIPC")                                   |
|   ver (2B): 1                                                       |
|   kind (2B): 1=WRITE_REQ, 2=EVT                                     |
|   topic_id (4B): fnv1a(topic_name)                                  |
|   abi_hash (4B): wire struct ABI hash                               |
|   data_len (4B): sizeof(Wire_XXX)                                   |
+--------------------------------------------------------------------+
| [Wire Struct Payload]                                               |
|   Wire_P_NSTEL_C_VehicleSpeed (packed, fixed size)                  |
+--------------------------------------------------------------------+
```

### 3.2 Frame Type 값

| Frame Type | 값 | 방향 | 용도 |
|------------|-----|------|------|
| `MSG_DATA_REQ_STRUCT` | 0x2100 | Legacy/UI → Gateway | 데이터 전송 요청 |
| `MSG_DATA_RSP_STRUCT` | 0x2101 | Gateway → Legacy/UI | 요청에 대한 응답 |
| `MSG_DATA_EVT_STRUCT` | 0x2102 | Gateway → Legacy/UI | DDS 수신 데이터 이벤트 |

---

## 4. 데이터 송신 (Legacy/UI → Gateway)

### 4.1 단계별 절차

```c
#include "idl_wire_structs.h"
#include "data_envelope.h"
#include "topic_id_utils.h"
#include <string.h>

/**
 * @brief Send Wire Struct to Gateway
 * @param topic_name DDS topic name (must match create_writer name)
 * @param wire_data Wire Struct data (already converted to big-endian)
 * @param wire_size sizeof(Wire_XXX)
 * @param abi_hash Wire struct ABI hash (see idl_wire_abi.h)
 */
void send_wire_struct(const char* topic_name,
                      const void* wire_data,
                      uint32_t wire_size,
                      uint32_t abi_hash)
{
    // 1. Compute topic_id
    uint32_t topic_id = compute_topic_id(topic_name);

    // 2. Build DataEnvelope (big-endian on the wire)
    DataEnvelope env;
    env.magic = htonl(DATA_ENVELOPE_MAGIC);
    env.ver = htons(DATA_ENVELOPE_VER);
    env.kind = htons(DATA_KIND_WRITE_REQ);
    env.topic_id = htonl(topic_id);
    env.abi_hash = htonl(abi_hash);
    env.data_len = htonl(wire_size);

    // 3. Build payload (Envelope + Wire Struct)
    uint8_t payload[sizeof(DataEnvelope) + wire_size];
    memcpy(payload, &env, sizeof(DataEnvelope));
    memcpy(payload + sizeof(DataEnvelope), wire_data, wire_size);

    // 4. Send IPC frame
    send_ipc_frame(MSG_DATA_REQ_STRUCT,
                   next_corr_id(),
                   payload,
                   sizeof(payload));
}
```

### 4.2 사용 예시

```c
// VehicleSpeed 데이터 전송
void send_vehicle_speed(double speed_value)
{
    // 1. Wire Struct 채우기
    Wire_P_NSTEL_C_VehicleSpeed data = {0};
    data.A_sourceID.A_resourceId = 1;
    data.A_sourceID.A_instanceId = 0;
    data.A_timeOfDataGeneration.A_second = time(NULL);
    data.A_timeOfDataGeneration.A_nanoseconds = 0;
    data.A_value = speed_value;
    
    // 2. 전송 (topic_name은 Gateway에서 create_writer 시 사용한 이름)
    send_wire_struct("VehicleSpeed",        // topic_name
                     &data,                  // wire struct
                     sizeof(data),           // size
                     0x00000000);            // abi_hash (생략 가능)
}

// CannonDrivingDevice 신호 전송
void send_cannon_signal(void)
{
    Wire_P_NSTEL_C_CannonDrivingDevice_Signal sig = {0};
    sig.A_sourceID.A_resourceId = 100;
    sig.A_sourceID.A_instanceId = 1;
    sig.A_azAngleVelocity = 45.5;
    sig.A_e1AngleVelocity = 30.0;
    sig.A_energyStorage = 1;  // enum: T_EnergyStorageStatusType
    // ... 나머지 필드
    
    send_wire_struct("CannonSignal", &sig, sizeof(sig), 0);
}
```

---

## 5. 데이터 수신 (Gateway → Legacy/UI)

### 5.1 EVT 프레임 처리

```c
/**
 * @brief IPC frame receive callback
 * @param frame_type frame type (0x2102 = MSG_DATA_EVT_STRUCT)
 * @param payload payload bytes
 * @param len payload length
 */
void on_ipc_frame_received(uint16_t frame_type,
                           const uint8_t* payload,
                           uint32_t len)
{
    if (frame_type != MSG_DATA_EVT_STRUCT) {
        return;
    }

    // 1. Basic length check
    if (len < sizeof(DataEnvelope)) {
        printf("Error: payload too short\n");
        return;
    }

    // 2. Parse DataEnvelope (big-endian on the wire)
    DataEnvelope env;
    memcpy(&env, payload, sizeof(env));
    env.magic = ntohl(env.magic);
    env.ver = ntohs(env.ver);
    env.kind = ntohs(env.kind);
    env.topic_id = ntohl(env.topic_id);
    env.abi_hash = ntohl(env.abi_hash);
    env.data_len = ntohl(env.data_len);

    // 3. Validate header
    if (env.magic != DATA_ENVELOPE_MAGIC ||
        env.ver != DATA_ENVELOPE_VER ||
        env.kind != DATA_KIND_EVT) {
        printf("Error: invalid DataEnvelope\n");
        return;
    }

    if (len < sizeof(DataEnvelope) + env.data_len) {
        printf("Error: payload length mismatch\n");
        return;
    }

    // 4. Extract Wire Struct
    const uint8_t* wire_data = payload + sizeof(DataEnvelope);
    uint32_t wire_len = env.data_len;

    // NOTE: Convert wire_data fields from big-endian before use.
    handle_wire_struct(env.topic_id, wire_data, wire_len);
}
```

### 5.2 topic_id 기반 타입 판별

```c
/**
 * @brief topic_id를 기반으로 Wire Struct 타입을 판별하여 처리
 * 
 * @note topic_id는 topic_name의 해시이므로, 
 *       사용할 topic_name을 미리 알고 해시값을 계산해두거나
 *       런타임에 비교해야 합니다.
 */
void handle_wire_struct(uint32_t topic_id, 
                        const uint8_t* wire_data, 
                        uint32_t wire_len)
{
    // 자주 사용하는 topic의 id를 미리 계산 (또는 전역 변수로)
    static uint32_t id_vehicle_speed = 0;
    static uint32_t id_cannon_signal = 0;
    static int initialized = 0;
    
    if (!initialized) {
        id_vehicle_speed = compute_topic_id("VehicleSpeed");
        id_cannon_signal = compute_topic_id("CannonSignal");
        initialized = 1;
    }
    
    // topic_id로 분기
    if (topic_id == id_vehicle_speed) {
        if (wire_len >= sizeof(Wire_P_NSTEL_C_VehicleSpeed)) {
            const Wire_P_NSTEL_C_VehicleSpeed* data = 
                (const Wire_P_NSTEL_C_VehicleSpeed*)wire_data;
            
            printf("VehicleSpeed: %.2f\n", data->A_value);
            // UI 업데이트 등
        }
    }
    else if (topic_id == id_cannon_signal) {
        if (wire_len >= sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_Signal)) {
            const Wire_P_NSTEL_C_CannonDrivingDevice_Signal* sig = 
                (const Wire_P_NSTEL_C_CannonDrivingDevice_Signal*)wire_data;
            
            printf("Cannon Az: %.2f, El: %.2f\n", 
                   sig->A_azAngleVelocity, sig->A_e1AngleVelocity);
        }
    }
    else {
        printf("Unknown topic_id: 0x%08X\n", topic_id);
    }
}
```

---

## 6. topic_id 동작 원리

### 6.1 핵심 개념

| 항목 | 설명 |
|------|------|
| `topic_name` | 런타임에 지정되는 문자열 (예: "VehicleSpeed", "MyCustomTopic") |
| `topic_id` | `topic_name`의 FNV-1a 32-bit 해시값 |
| `type_name` | IDL에서 정의된 고정된 타입명 (예: "P_NSTEL::C_VehicleSpeed") |

### 6.2 동작 흐름

```
[Legacy/UI]                              [Gateway]                              [DDS]
    │                                        │                                     │
    │  topic_name = "VehicleSpeed"           │                                     │
    │  topic_id = fnv1a("VehicleSpeed")      │                                     │
    │  ────────────────────────────────────► │                                     │
    │  MSG_DATA_REQ_STRUCT                   │                                     │
    │  [DataEnvelope + Wire_XXX]             │                                     │
    │                                        │  topic_id_map_[topic_id]            │
    │                                        │   → {"VehicleSpeed",                │
    │                                        │      "P_NSTEL::C_VehicleSpeed"}     │
    │                                        │                                     │
    │                                        │  wire_to_dds(type_name, ...)        │
    │                                        │  ─────────────────────────────────► │
    │                                        │  publish(topic_name, sample)        │
    │                                        │                                     │
```

### 6.3 중요 사항

1. **topic_name 일치 필수**: Legacy/UI가 사용하는 `topic_name`과 Gateway에서 `create_writer`/`create_reader` 시 사용한 `topic_name`이 **정확히 일치**해야 합니다.

2. **런타임 해시 계산**: `topic_id`는 상수가 아니라 런타임에 `compute_topic_id(topic_name)`으로 계산합니다.

3. **해시 충돌 가능성**: FNV-1a 32-bit 해시는 충돌 가능성이 있으나, 실제 사용 범위에서는 무시할 수 있는 수준입니다.

---

## 7. 응답 처리 (MSG_DATA_RSP_STRUCT)

### 7.1 응답 포맷

Gateway가 `MSG_DATA_REQ_STRUCT` 요청에 대해 보내는 응답:

```
+-------------------------------+
| RSP Payload (12 bytes)        |
+-------------------------------+
| magic  (4B): 0x44525350 "DRSP"|
| ver    (2B): 1                |
| status (2B): 0=OK, 1=ERR      |
| err    (4B): error code       |
+-------------------------------+
```
### 7.2 Status and error codes

- `status`: 0=OK, 1=ERR
- `err`: detail code when status=ERR

| err | name | meaning |
|------|------|---------|
| 0x00 | OK | success (only when status=OK) |
| 0x01 | PARSE_ERROR | DataEnvelope parse failed |
| 0x02 | UNKNOWN_TOPIC | topic_id not found |
| 0x03 | ABI_MISMATCH | wire struct version mismatch |
| 0x04 | CONVERT_FAIL | wire->DDS conversion failed |
| 0x05 | PUBLISH_FAIL | DDS publish failed |

### 7.3 응답 처리 예시

```c
void on_struct_response(uint32_t corr_id, const uint8_t* payload, uint32_t len)
{
    if (len < sizeof(DataRspStruct)) {
        return;
    }

    DataRspStruct rsp;
    memcpy(&rsp, payload, sizeof(rsp));

    rsp.magic = ntohl(rsp.magic);
    rsp.ver = ntohs(rsp.ver);
    rsp.status = ntohs(rsp.status);
    rsp.err = ntohl(rsp.err);

    if (rsp.magic != DATA_RSP_MAGIC || rsp.ver != DATA_ENVELOPE_VER) {
        printf("Request %u failed: invalid response\n", corr_id);
        return;
    }

    if (rsp.status == 0) {
        printf("Request %u succeeded\n", corr_id);
    } else {
        printf("Request %u failed: err=0x%08X\n", corr_id, rsp.err);
    }
}
```

---

## 8. 바이트 정렬 주의사항

### 8.1 #pragma pack(push, 1)

`idl_wire_structs.h`의 모든 구조체는 `#pragma pack(push, 1)`로 1바이트 정렬됩니다.

```c
#pragma pack(push, 1)
typedef struct Wire_Example {
    int32_t field1;   // offset 0
    int8_t  field2;   // offset 4
    int64_t field3;   // offset 5 (패딩 없음!)
} Wire_Example;       // total size = 13 bytes
#pragma pack(pop)
```

### 8.2 플랫폼 호환성

| 항목 | 요구사항 |
|------|---------|
| 바이트 순서 | Big-endian (x86, ARM 기본값) |
| 정렬 | 1-byte aligned (pack 필수) |
| 컴파일러 | C99 이상, C++11 이상 |

---

## 9. 타입 매핑 참조

### 9.1 기본 타입

| IDL 타입 | Wire Struct 타입 | 크기 |
|----------|------------------|------|
| `boolean` | `bool` | 1B |
| `char` | `char` | 1B |
| `octet` | `uint8_t` | 1B |
| `short` | `int16_t` | 2B |
| `unsigned short` | `uint16_t` | 2B |
| `long` | `int32_t` | 4B |
| `unsigned long` | `uint32_t` | 4B |
| `long long` | `int64_t` | 8B |
| `float` | `float` | 4B |
| `double` | `double` | 8B |
| `enum` | `int32_t` | 4B |

### 9.2 복합 타입

| IDL 타입 | Wire Struct 표현 |
|----------|-----------------|
| `string<N>` | `char[N+1]` |
| `sequence<T, N>` | `uint32_t XXX_len; T XXX[N];` |
| `T[N]` (array) | `T[N]` |
| nested struct | `Wire_XXX` |

---

## 10. 체크리스트

Legacy/UI 연동 시 확인사항:

- [ ] `idl_wire_structs.h`를 프로젝트에 포함
- [ ] `data_envelope.h`, `topic_id_utils.h` 복사
- [ ] 컴파일러 설정: `#pragma pack` 지원 확인
- [ ] topic_name이 Gateway와 정확히 일치하는지 확인
- [ ] 바이트 순서가 Big-endian인지 확인
- [ ] Wire struct 크기가 `sizeof()`와 일치하는지 확인
