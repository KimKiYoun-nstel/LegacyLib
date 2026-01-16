# IPC 프로토콜 규격서 v3.0

## 목적

이 문서는 Legacy 시스템 및 UI가 DDS Gateway Agent와 통신하기 위한 **완전한 개발 규격**을 제공합니다.

---

## 1. 개요

### 1.1 프로토콜 구조

```
┌─────────────────────────────────────────────────────────────┐
│                     IPC 프로토콜 v3.0                        │
├──────────────────────┬──────────────────────────────────────┤
│     Control Plane    │           Data Plane                 │
│     (JSON/CBOR)      │   JSON/CBOR   │   Binary Struct      │
├──────────────────────┼───────────────┼──────────────────────┤
│  MSG_FRAME_REQ/RSP   │ MSG_DATA_*_JSON│ MSG_DATA_*_STRUCT   │
│  엔티티 생성/관리      │  데이터 송수신   │  데이터 송수신        │
└──────────────────────┴───────────────┴──────────────────────┘
```

### 1.2 Control vs Data 분리

| 구분 | Control Plane | Data Plane |
|------|---------------|------------|
| **목적** | DDS 엔티티 생성/관리/설정 | DDS 샘플 송수신 |
| **프레임 타입** | `MSG_FRAME_REQ/RSP/EVT` | `MSG_DATA_*_JSON`, `MSG_DATA_*_STRUCT` |
| **페이로드** | JSON (CBOR 인코딩) | JSON 또는 Binary Struct |
| **사용 예** | hello, create.writer, get.qos | write sample, receive event |

### 1.3 JSON vs Struct 모드

**중요**: Agent는 런타임에 **하나의 data_codec 모드**만 지원합니다.

| 항목 | JSON 모드 | Struct 모드 |
|------|-----------|-------------|
| 설정값 | `"data_codec": "json"` | `"data_codec": "struct"` |
| 요청 프레임 | `MSG_DATA_REQ_JSON` | `MSG_DATA_REQ_STRUCT` |
| 응답 프레임 | `MSG_DATA_RSP_JSON` | `MSG_DATA_RSP_STRUCT` |
| 이벤트 프레임 | `MSG_DATA_EVT_JSON` | `MSG_DATA_EVT_STRUCT` |
| 페이로드 포맷 | CBOR 인코딩 JSON | DataEnvelope + Wire Struct |

**⚠️ 경고**: Legacy/UI는 **반드시 Agent와 동일한 모드**를 사용해야 합니다.

- Hello 응답의 `data_codec` 필드를 확인하세요.
- 모드가 불일치하면 Agent가 요청을 거부합니다.

---

## 2. IPC 헤더 포맷

### 2.1 Header 구조 (24 bytes, packed)

```c
#pragma pack(push, 1)
typedef struct IpcHeader {
    uint32_t magic;      // 'RIPC' = 0x52495043
    uint16_t version;    // 0x0001
    uint16_t type;       // Frame Type (아래 참조)
    uint32_t corr_id;    // Correlation ID (요청/응답 매칭)
    uint32_t length;     // Payload 길이 (bytes)
    uint64_t ts_ns;      // 타임스탬프 (ns, 디버깅용)
} IpcHeader;
#pragma pack(pop)
```

### 2.2 corr_id 규칙

| 상황 | 규칙 |
|------|------|
| 요청(REQ) 전송 | 송신측이 고유 ID 할당 |
| 응답(RSP) 전송 | 요청의 corr_id를 **그대로 사용** |
| 이벤트(EVT) 전송 | corr_id = 0 |

---

## 3. Frame Type 목록

### 3.1 전체 Frame Type 정의

```c
enum FrameType : uint16_t {
    // ===== Control Plane (CBOR/JSON) =====
    MSG_FRAME_REQ   = 0x1000,  // Control 요청
    MSG_FRAME_RSP   = 0x1001,  // Control 응답
    MSG_FRAME_EVT   = 0x1002,  // Control 이벤트 (예약)

    // ===== Data Plane: JSON (CBOR/JSON) =====
    MSG_DATA_REQ_JSON   = 0x2000,  // Data 요청 (JSON)
    MSG_DATA_RSP_JSON   = 0x2001,  // Data 응답 (JSON)
    MSG_DATA_EVT_JSON   = 0x2002,  // Data 이벤트 (JSON)

    // ===== Data Plane: Struct (Binary) =====
    MSG_DATA_REQ_STRUCT = 0x2100,  // Data 요청 (Struct)
    MSG_DATA_RSP_STRUCT = 0x2101,  // Data 응답 (Struct)
    MSG_DATA_EVT_STRUCT = 0x2102,  // Data 이벤트 (Struct)
};
```

### 3.2 프레임 타입 요약표

| 타입 | 값 | 방향 | 페이로드 |
|------|-----|------|----------|
| `MSG_FRAME_REQ` | 0x1000 | Client → Agent | CBOR(JSON) |
| `MSG_FRAME_RSP` | 0x1001 | Agent → Client | CBOR(JSON) |
| `MSG_DATA_REQ_JSON` | 0x2000 | Client → Agent | CBOR(JSON) |
| `MSG_DATA_RSP_JSON` | 0x2001 | Agent → Client | CBOR(JSON) |
| `MSG_DATA_EVT_JSON` | 0x2002 | Agent → Client | CBOR(JSON) |
| `MSG_DATA_REQ_STRUCT` | 0x2100 | Client → Agent | DataEnvelope + Wire |
| `MSG_DATA_RSP_STRUCT` | 0x2101 | Agent → Client | Binary |
| `MSG_DATA_EVT_STRUCT` | 0x2102 | Agent → Client | DataEnvelope + Wire |

---

## 4. Control 프로토콜 (MSG_FRAME_REQ/RSP)

### 4.1 Hello 요청/응답

#### 요청

```json
{
    "op": "hello"
}
```

#### 응답

```json
{
    "ok": true,
    "result": {
        "proto": 1,
        "data_codec": "json",       // 현재 Agent의 데이터 코덱 모드
        "abi_hash": 3047212491,     // Wire Struct ABI 해시 (Struct 모드용)
        "cap": [...]                // 지원 기능 목록
    }
}
```

**⚠️ 중요**: `data_codec` 값을 확인하여 Legacy/UI가 동일한 모드로 동작해야 합니다.

### 4.2 엔티티 생성 요청/응답

#### Writer 생성

```json
// 요청
{
    "op": "create",
    "target": { "kind": "writer" },
    "data": {
        "domain_id": 0,
        "pub_name": "MyPub",
        "topic": "AlarmTopic",
        "type": "C_AlarmMsg",
        "qos_lib": "NGVA_QoS_Library",
        "qos_profile": "Default_Profile"
    }
}

// 응답 (성공)
{
    "ok": true,
    "result": {
        "id": 12345678901234,
        "topic_id": 3141592653
    }
}
```

#### Reader 생성

```json
// 요청
{
    "op": "create",
    "target": { "kind": "reader" },
    "data": {
        "domain_id": 0,
        "sub_name": "MySub",
        "topic": "AlarmTopic",
        "type": "C_AlarmMsg",
        "qos_lib": "NGVA_QoS_Library",
        "qos_profile": "Default_Profile"
    }
}

// 응답 (성공)
{
    "ok": true,
    "result": {
        "id": 12345678901235,
        "topic_id": 3141592653
    }
}
```

### 4.3 에러 응답

```json
{
    "ok": false,
    "err": 4,                // 에러 코드
    "category": 1,           // 에러 카테고리
    "msg": "Writer creation failed: topic not found"
}
```

---

## 5. Data JSON 프로토콜 (MSG_DATA_*_JSON)

### 5.1 topic_id

`topic_id`는 DDS topic name의 **FNV-1a 32-bit 해시**입니다.

```c
// FNV-1a 32-bit 알고리즘
uint32_t compute_topic_id(const char* topic, size_t len) {
    uint32_t hash = 2166136261u;  // FNV_OFFSET_BASIS
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)topic[i];
        hash *= 16777619u;         // FNV_PRIME
    }
    return hash;
}
```

**⚠️ 주의**: topic_id는 런타임에 계산됩니다. 상수가 아닙니다.

#### 테스트 벡터

| topic_name | topic_id (hex) |
|------------|----------------|
| "AlarmTopic" | 0xA1B2C3D4 (예시) |
| "StringTopic" | 0x12345678 (예시) |

### 5.2 Data Write 요청 (MSG_DATA_REQ_JSON)

```json
{
    "topic_id": 3141592653,
    "data": {
        "level": 1,
        "text": "Hello World"
    }
}
```

### 5.3 Data Write 응답 (MSG_DATA_RSP_JSON)

#### 성공

```json
{ "ok": true }
```

#### 실패

```json
{
    "ok": false,
    "err": 13,
    "msg": "unknown topic_id"
}
```

### 5.4 Data Event (MSG_DATA_EVT_JSON)

Agent가 DDS에서 샘플을 수신하면 전송:

```json
{
    "topic_id": 3141592653,
    "topic": "AlarmTopic",        // 디버깅용
    "type": "C_AlarmMsg",         // 디버깅용
    "data": {
        "level": 2,
        "text": "Warning message"
    }
}
```

---

## 6. Data Struct 프로토콜 (MSG_DATA_*_STRUCT)

### 6.1 DataEnvelope 헤더 (20 bytes)

```c
#pragma pack(push, 1)
typedef struct DataEnvelope {
    uint32_t magic;      // 'DIPC' = 0x44495043
    uint16_t ver;        // 버전: 1
    uint16_t kind;       // 1=WRITE_REQ, 2=EVT
    uint32_t topic_id;   // FNV-1a hash(topic_name)
    uint32_t abi_hash;   // Wire Struct ABI 해시
    uint32_t data_len;   // Wire Struct 데이터 길이
} DataEnvelope;
#pragma pack(pop)
```

### 6.1.1 Byte Order (Important)
- DataEnvelope 및 Wire Struct payload(DataRspStruct 포함)는 **Big-Endian(Network Byte Order)**으로 고정한다.
- `uint16/uint32/uint64`, `int16/int32/int64`, `float/double` 등 **모든 숫자 필드**는 Big-Endian으로 직렬화한다.
- `bool`, `char[]`(string) 등 **1바이트 데이터**는 변환하지 않는다.
- 부동소수점은 **IEEE 754 표현을 Big-Endian 바이트 순서**로 전송한다.

### 6.2 ABI Hash 검증

**⚠️ 중요**: `abi_hash`가 Agent의 값과 일치하지 않으면 요청이 거부됩니다.

- Hello 응답의 `abi_hash` 값을 확인하세요.
- IDL이 변경되면 abi_hash도 변경됩니다.
- 양측(Legacy/UI와 Agent)이 동일한 IDL 버전을 사용해야 합니다.

### 6.3 Data Write 요청 (MSG_DATA_REQ_STRUCT)

```
[IpcHeader (24 bytes)]
[DataEnvelope (20 bytes)]
[Wire Struct bytes (data_len bytes)]
```

#### 예시: C_AlarmMsg 전송

```
IpcHeader:
  magic    = 0x52495043 ('RIPC')
  version  = 0x0001
  type     = 0x2100 (MSG_DATA_REQ_STRUCT)
  corr_id  = 42
  length   = 280 (DataEnvelope + Wire_C_AlarmMsg)
  ts_ns    = ...

DataEnvelope:
  magic    = 0x44495043 ('DIPC')
  ver      = 1
  kind     = 1 (WRITE_REQ)
  topic_id = 0xA1B2C3D4
  abi_hash = 0xB5A245CB
  data_len = 260

Wire_C_AlarmMsg:
  level    = 1 (4 bytes, int32)
  text     = "Hello" (256 bytes, char[256])
```

### 6.4 Data Write 응답 (MSG_DATA_RSP_STRUCT)

Binary 응답 (8 bytes):

```c
#pragma pack(push, 1)
typedef struct DataRspStruct {
    uint8_t  status;     // 0=OK, 1=PARSE_ERROR, 2=UNKNOWN_TOPIC, 3=ABI_MISMATCH, 4=CONVERT_FAIL, 5=PUBLISH_FAIL
    uint32_t topic_id;   // 요청한 topic_id
    uint8_t  reserved[3];
} DataRspStruct;
#pragma pack(pop)
```

#### 응답 상태 코드

| 코드 | 의미 | 설명 |
|------|------|------|
| 0x00 | OK | 성공 |
| 0x01 | PARSE_ERROR | DataEnvelope 파싱 실패 |
| 0x02 | UNKNOWN_TOPIC | topic_id 미등록 |
| 0x03 | ABI_MISMATCH | abi_hash 불일치 |
| 0x04 | CONVERT_FAIL | Wire → DDS 변환 실패 |
| 0x05 | PUBLISH_FAIL | DDS publish 실패 |

### 6.5 Data Event (MSG_DATA_EVT_STRUCT)

Agent가 DDS에서 샘플을 수신하면 전송:

```
[IpcHeader (24 bytes)]
[DataEnvelope (20 bytes)]  // kind=2 (EVT)
[Wire Struct bytes]
```

---

## 7. Wire Struct 규격

### 7.1 생성 규칙

Wire Struct는 IDL에서 자동 생성되며, 다음 규칙을 따릅니다:

| IDL 타입 | Wire Struct 타입 |
|----------|------------------|
| `int32` | `int32_t` |
| `int64` | `int64_t` |
| `float` | `float` |
| `double` | `double` |
| `boolean` | `bool` (1 byte) |
| `string<N>` | `char[N+1]` |
| `sequence<T, N>` | `uint32_t len; T data[N];` |
| `enum` | `int32_t` |

### 7.2 예시: C_AlarmMsg

IDL:

```idl
struct C_AlarmMsg {
    int32 level;
    string<255> text;
};
```

Wire Struct:

```c
#pragma pack(push, 1)
typedef struct Wire_C_AlarmMsg {
    int32_t level;
    char text[256];
} Wire_C_AlarmMsg;
#pragma pack(pop)
```

### 7.3 파일 위치

빌드 후 다음 파일들을 사용하세요:

| 파일 | 경로 | 용도 |
|------|------|------|
| `idl_wire_structs.h` | `<build>/idlkit/gen/` | Wire Struct 정의 |
| `idl_wire_abi.hpp` | `<build>/idlkit/gen/` | ABI 해시 상수 |

---

## 8. 시퀀스 다이어그램

### 8.1 연결 및 초기화

```
Legacy/UI                          Agent
    │                                │
    │──── MSG_FRAME_REQ (hello) ────▶│
    │                                │
    │◀─── MSG_FRAME_RSP ────────────│
    │     { data_codec: "json",     │
    │       abi_hash: 0xB5A245CB }  │
    │                                │
    │  [data_codec 확인 후 모드 설정]  │
    │                                │
```

### 8.2 Writer 생성 및 데이터 전송 (JSON 모드)

```
Legacy/UI                          Agent                          DDS
    │                                │                              │
    │── MSG_FRAME_REQ (create.writer)──▶│                           │
    │                                │                              │
    │◀── MSG_FRAME_RSP ──────────────│  (topic_id 매핑 등록)         │
    │    { topic_id: 0xA1B2C3D4 }    │                              │
    │                                │                              │
    │── MSG_DATA_REQ_JSON ──────────▶│                              │
    │   { topic_id, data: {...} }    │──── DDS publish ───────────▶│
    │                                │                              │
    │◀── MSG_DATA_RSP_JSON ──────────│                              │
    │    { ok: true }                │                              │
    │                                │                              │
```

### 8.3 Reader 생성 및 이벤트 수신 (Struct 모드)

```
Legacy/UI                          Agent                          DDS
    │                                │                              │
    │── MSG_FRAME_REQ (create.reader)─▶│                            │
    │                                │                              │
    │◀── MSG_FRAME_RSP ──────────────│                              │
    │                                │                              │
    │                                │◀───── DDS sample ───────────│
    │                                │                              │
    │◀── MSG_DATA_EVT_STRUCT ────────│                              │
    │   [DataEnvelope + WireStruct]  │                              │
    │                                │                              │
```

---

## 9. 에러 코드 목록

### 9.1 Control 에러 코드

| 코드 | 의미 |
|------|------|
| 1 | Invalid JSON/CBOR |
| 2 | Missing required field |
| 3 | Invalid operation |
| 4 | DDS operation failed |
| 5 | Entity not found |
| 6 | Invalid data format |
| 7 | No command sink |

### 9.2 Data JSON 에러 코드

| 코드 | 의미 |
|------|------|
| 11 | Missing or invalid topic_id |
| 12 | Missing or invalid data object |
| 13 | Unknown topic_id |
| 14 | Publish failed |
| 15 | JSON/CBOR parse error |
| 16 | Internal error |

### 9.3 Data Struct 상태 코드

| 코드 | 의미 |
|------|------|
| 0x00 | OK |
| 0x01 | PARSE_ERROR |
| 0x02 | UNKNOWN_TOPIC |
| 0x03 | ABI_MISMATCH |
| 0x04 | CONVERT_FAIL |
| 0x05 | PUBLISH_FAIL |

---

## 10. 런타임 설정

### 10.1 Agent 설정 파일 (agent_config.json)

```json
{
    "ipc": {
        "data_codec": "json"   // "json" 또는 "struct"
    }
}
```

### 10.2 모드 전환

**⚠️ 주의**: 런타임 중 모드 전환은 **지원되지 않습니다**.

1. Agent 설정 파일에서 `data_codec` 변경
2. Agent 재시작
3. Legacy/UI가 Hello 응답으로 새 모드 확인
4. Legacy/UI가 해당 모드로 동작

---

## 11. 연동 주의사항

### 11.1 모드 불일치 문제

| 상황 | 결과 |
|------|------|
| UI가 JSON 전송, Agent가 Struct 모드 | 요청 처리됨 (프레임 타입으로 구분) |
| Agent가 Struct EVT 전송, UI가 JSON 기대 | **이벤트 수신 실패** |

**⚠️ 결론**: EVT 수신 방향이 문제입니다. 반드시 모드를 일치시키세요.

### 11.2 topic_id 매핑

- topic_id는 **엔티티(Writer/Reader) 생성 시점**에 Agent에 등록됩니다.
- 엔티티 생성 전에 Data 요청을 보내면 `UNKNOWN_TOPIC` 에러가 발생합니다.
- Control로 엔티티를 먼저 생성한 후 Data 요청을 보내세요.

### 11.3 ABI 호환성

- IDL 변경 시 `abi_hash`가 변경됩니다.
- Legacy/UI와 Agent가 동일한 IDL 버전을 사용해야 합니다.
- Hello 응답의 `abi_hash`를 확인하세요.

---

## 부록 A: C 헤더 파일

### A.1 topic_id_utils.h

```c
#ifndef TOPIC_ID_UTILS_H
#define TOPIC_ID_UTILS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t compute_topic_id(const char* topic, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)topic[i];
        hash *= 16777619u;
    }
    return hash;
}

#ifdef __cplusplus
}
#endif

#endif /* TOPIC_ID_UTILS_H */
```

### A.2 data_envelope.h

```c
#ifndef DATA_ENVELOPE_H
#define DATA_ENVELOPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_ENVELOPE_MAGIC  0x44495043  /* 'DIPC' */
#define DATA_ENVELOPE_VER    1

#pragma pack(push, 1)
typedef struct DataEnvelope {
    uint32_t magic;
    uint16_t ver;
    uint16_t kind;       /* 1=WRITE_REQ, 2=EVT */
    uint32_t topic_id;
    uint32_t abi_hash;
    uint32_t data_len;
} DataEnvelope;

typedef struct DataRspStruct {
    uint8_t  status;
    uint32_t topic_id;
    uint8_t  reserved[3];
} DataRspStruct;
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* DATA_ENVELOPE_H */
```

---

## 부록 B: 버전 이력

| 버전 | 날짜 | 변경 내용 |
|------|------|----------|
| 3.0 | 2026-01-09 | Control/Data 분리, JSON/Struct 모드 도입 |
| 2.x | - | 기존 통합 JSON 프로토콜 |

---

## 부록 C: 관련 문서

| 문서 | 설명 |
|------|------|
| `WIRE_STRUCT_USAGE_GUIDE.md` | Wire Struct 상세 사용법 |
| `DDS_FW_InterfaceSpec_*.md` | 프로토콜 설계 문서 |
| `idl_wire_structs.h` | 생성된 Wire Struct 정의 |
