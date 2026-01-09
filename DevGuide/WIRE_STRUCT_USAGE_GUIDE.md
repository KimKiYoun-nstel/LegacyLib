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
 * @brief DataEnvelope 구조체 정의 (C 호환)
 */
#ifndef DATA_ENVELOPE_H
#define DATA_ENVELOPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

/**
 * @brief Data Struct 프레임의 헤더 (20 bytes)
 * 
 * Wire Struct 페이로드 앞에 반드시 이 헤더를 붙여야 합니다.
 */
typedef struct DataEnvelope {
    uint32_t magic;      /**< 매직 넘버: 0x43504944 ("DIPC" little-endian) */
    uint8_t  ver;        /**< 프로토콜 버전: 1 */
    uint8_t  kind;       /**< PayloadKind: 2 = DataStruct */
    uint32_t topic_id;   /**< topic_name의 FNV-1a 해시 */
    uint32_t abi_hash;   /**< Wire struct ABI 해시 (버전 검증용) */
    uint32_t data_len;   /**< Wire struct 페이로드 길이 (bytes) */
    uint16_t reserved;   /**< 예약 (0으로 설정) */
} DataEnvelope;

#pragma pack(pop)

/** DataEnvelope 매직 상수 */
#define DATA_ENVELOPE_MAGIC  0x43504944

/** DataEnvelope 버전 */
#define DATA_ENVELOPE_VER    1

/** PayloadKind 열거 */
#define PAYLOAD_KIND_CONTROL_JSON   0
#define PAYLOAD_KIND_DATA_JSON      1
#define PAYLOAD_KIND_DATA_STRUCT    2

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
┌────────────────────────────────────────────────────────────────┐
│ IPC 프레임 (UDP 패킷)                                          │
├────────────────────────────────────────────────────────────────┤
│ [IPC Header: 12 bytes]                                         │
│   ├─ magic (4B): "DIPC"                                        │
│   ├─ frame_type (2B): 0x2100=REQ, 0x2101=RSP, 0x2102=EVT      │
│   ├─ corr_id (4B): 요청-응답 매칭용 ID                          │
│   └─ payload_len (2B): 아래 페이로드 전체 길이                   │
├────────────────────────────────────────────────────────────────┤
│ [DataEnvelope: 20 bytes]                                       │
│   ├─ magic (4B): 0x43504944 ("DIPC")                           │
│   ├─ ver (1B): 1                                               │
│   ├─ kind (1B): 2 (DataStruct)                                 │
│   ├─ topic_id (4B): fnv1a(topic_name)                          │
│   ├─ abi_hash (4B): Wire struct 버전 해시                       │
│   ├─ data_len (4B): sizeof(Wire_XXX)                           │
│   └─ reserved (2B): 0                                          │
├────────────────────────────────────────────────────────────────┤
│ [Wire Struct Payload]                                          │
│   예: Wire_P_NSTEL_C_VehicleSpeed (packed, 고정 크기)          │
└────────────────────────────────────────────────────────────────┘
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
 * @brief Wire Struct 데이터를 Gateway로 전송
 * @param topic_name DDS topic 이름 (create_writer에 사용된 것과 동일)
 * @param wire_data Wire Struct 포인터
 * @param wire_size sizeof(Wire_XXX)
 * @param abi_hash Wire struct ABI 해시 (idl_wire_abi.h 참조)
 */
void send_wire_struct(const char* topic_name, 
                      const void* wire_data, 
                      uint32_t wire_size,
                      uint32_t abi_hash)
{
    // 1. topic_id 계산 (런타임)
    uint32_t topic_id = compute_topic_id(topic_name);
    
    // 2. DataEnvelope 생성
    DataEnvelope env;
    env.magic = DATA_ENVELOPE_MAGIC;
    env.ver = DATA_ENVELOPE_VER;
    env.kind = PAYLOAD_KIND_DATA_STRUCT;
    env.topic_id = topic_id;
    env.abi_hash = abi_hash;
    env.data_len = wire_size;
    env.reserved = 0;
    
    // 3. 페이로드 조립 (Envelope + Wire Struct)
    uint8_t payload[sizeof(DataEnvelope) + wire_size];
    memcpy(payload, &env, sizeof(DataEnvelope));
    memcpy(payload + sizeof(DataEnvelope), wire_data, wire_size);
    
    // 4. IPC 프레임 전송
    // (기존 IPC 전송 함수 사용)
    send_ipc_frame(0x2100,  // MSG_DATA_REQ_STRUCT
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
 * @brief IPC 프레임 수신 콜백
 * @param frame_type 프레임 타입 (0x2102 = MSG_DATA_EVT_STRUCT)
 * @param payload 페이로드 데이터
 * @param len 페이로드 길이
 */
void on_ipc_frame_received(uint16_t frame_type, 
                           const uint8_t* payload, 
                           uint32_t len)
{
    if (frame_type != 0x2102) {  // MSG_DATA_EVT_STRUCT
        return;
    }
    
    // 1. DataEnvelope 파싱
    if (len < sizeof(DataEnvelope)) {
        printf("Error: payload too short\n");
        return;
    }
    
    const DataEnvelope* env = (const DataEnvelope*)payload;
    
    // 2. 매직 검증
    if (env->magic != DATA_ENVELOPE_MAGIC) {
        printf("Error: invalid magic\n");
        return;
    }
    
    // 3. Wire Struct 데이터 추출
    const uint8_t* wire_data = payload + sizeof(DataEnvelope);
    uint32_t wire_len = env->data_len;
    
    // 4. topic_id로 타입 판별 후 처리
    handle_wire_struct(env->topic_id, wire_data, wire_len);
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
┌─────────────────────────────────────┐
│ RSP Payload (8 bytes)               │
├─────────────────────────────────────┤
│ status (1B): 결과 코드              │
│ topic_id (4B): 요청의 topic_id      │
│ reserved (3B): 0                    │
└─────────────────────────────────────┘
```

### 7.2 상태 코드

| 코드 | 이름 | 의미 |
|------|------|------|
| 0x00 | OK | 성공 |
| 0x01 | PARSE_ERROR | DataEnvelope 파싱 실패 |
| 0x02 | UNKNOWN_TOPIC | topic_id에 해당하는 topic 없음 |
| 0x03 | ABI_MISMATCH | Wire struct 버전 불일치 |
| 0x04 | CONVERT_FAIL | Wire → DDS 변환 실패 |
| 0x05 | PUBLISH_FAIL | DDS publish 실패 |

### 7.3 응답 처리 예시

```c
void on_struct_response(uint32_t corr_id, const uint8_t* payload, uint32_t len)
{
    if (len < 5) return;
    
    uint8_t status = payload[0];
    uint32_t topic_id;
    memcpy(&topic_id, &payload[1], sizeof(topic_id));
    
    if (status == 0x00) {
        printf("Request %u succeeded for topic_id 0x%08X\n", corr_id, topic_id);
    } else {
        printf("Request %u failed: status=0x%02X topic_id=0x%08X\n", 
               corr_id, status, topic_id);
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
| 바이트 순서 | Little-endian (x86, ARM 기본값) |
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
- [ ] 바이트 순서가 Little-endian인지 확인
- [ ] Wire struct 크기가 `sizeof()`와 일치하는지 확인
