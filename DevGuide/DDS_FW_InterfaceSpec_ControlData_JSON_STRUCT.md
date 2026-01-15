# DDS FW 연동 규격 & 개발 사항 (Legacy/UI 공통 기준)
## 목적
Legacy 및 UI가 **Control/Data 분리 + Data(JSON/Struct) 지원**을 동일하게 구현할 수 있도록,
Agent와의 인터페이스 규격과 개발 항목을 정리한다.

- **Control plane**: 기존 JSON 프로토콜 유지 (CBOR encoding)
- **Data plane**: JSON 또는 Struct 선택 가능 (프레임 타입으로 구분)

> 이 문서만 보고 Legacy/UI가 동일한 프레임을 생성/파싱할 수 있어야 한다.

---

# 1. 공통 프레이밍(UDP/SHM 무관)
## 1.1 Packet Layout
모든 패킷은 아래 형태를 따른다.

```
[Header][Payload(length bytes)]
```

Header는 고정 크기이며, Payload 길이는 `Header.length`로 정의된다.

## 1.2 Header
필드(예시):
- `magic` : 'RIPC'
- `version`
- `type` : frame type
- `corr_id` : request/response correlation id
- `length` : payload bytes length
- `ts_ns` : timestamp (옵션/디버깅)

### corr_id 규칙
- 요청(Request) 전송 측이 corr_id를 채운다.
- 응답(Response)은 반드시 **동일 corr_id를 유지**한다.
- 이벤트(Event)는 corr_id=0 또는 별도 규칙(필요 시 문서화).

---

# 2. Frame Type 정의(권장 모델)
## 2.1 Control plane (CBOR/JSON)
- `MSG_FRAME_REQ` : control request
- `MSG_FRAME_RSP` : control response
- `MSG_FRAME_EVT` : control event (reserved, 필요 시 사용)

> Control은 현재 구현과 동일하게 JSON object를 CBOR로 인코딩하여 payload로 사용한다.

## 2.2 Data plane (JSON/Struct 분리)
### Data JSON (CBOR/JSON)
- `MSG_DATA_REQ_JSON`
- `MSG_DATA_RSP_JSON`
- `MSG_DATA_EVT_JSON`

### Data STRUCT (binary)
- `MSG_DATA_REQ_STRUCT`
- `MSG_DATA_RSP_STRUCT`
- `MSG_DATA_EVT_STRUCT`

---

# 3. Control(JSON) 프로토콜 (현행 유지)
## 3.1 범위
- DDS Participant/Publisher/Subscriber/Writer/Reader 생성/해제
- QoS get/set
- clear/reset 등 엔티티 관리

## 3.2 변경 방침
- Control 메시지는 “기존 스키마 유지”가 원칙
- 단, **write/evt 데이터 송수신은 Data plane으로 이동**하는 것을 권장/목표로 한다.
  - (이행 기간 동안 write가 control에 남아있다면, 중복 경로를 허용할지 여부를 프로젝트 정책으로 결정)

---

# 4. Data(JSON) 프로토콜
## 4.1 Data Write Request (MSG_DATA_REQ_JSON)
Payload는 CBOR로 인코딩된 JSON object:

```json
{
  "topic_id": 1234567890,
  "data": { ... }
}
```

- `topic_id`: 필수
- `data`: 필수 (DDS 타입에 대응하는 JSON object)

## 4.2 Data Write Response (MSG_DATA_RSP_JSON)
성공:
```json
{ "ok": true }
```

실패:
```json
{ "ok": false, "err": 4, "msg": "reason..." }
```

## 4.3 Data Event (MSG_DATA_EVT_JSON)
```json
{
  "topic_id": 1234567890,
  "type": "TypeName",
  "data": { ... }
}
```

---

# 5. Data(STRUCT) 프로토콜
## 5.1 DataEnvelope
`MSG_DATA_REQ_STRUCT / MSG_DATA_EVT_STRUCT` payload는 DataEnvelope로 시작한다.

### Byte Order (Important)
- DataEnvelope 및 struct payload(DataRsp 포함)는 **Big-Endian(Network Byte Order)**으로 고정한다.
- `uint16/uint32/uint64`, `int16/int32/int64`, `float/double` 등 **모든 숫자 필드**는 Big-Endian으로 직렬화한다.
- `bool`, `char[]`(string) 등 **1바이트 데이터**는 변환하지 않는다.
- 부동소수점은 **IEEE 754 표현을 Big-Endian 바이트 순서**로 전송한다.

```c
#pragma pack(push, 1)
typedef struct {
  uint32_t magic;     // 'DIPC'
  uint16_t ver;       // 1
  uint16_t kind;      // 1=WRITE_REQ, 2=EVT
  uint32_t topic_id;  // hash(topic)
  uint32_t abi_hash;  // IDL 기반 wire struct 세트 식별
  uint32_t data_len;  // bytes length
  // uint8_t data[data_len];
} DataEnvelope;
#pragma pack(pop)
```

- `abi_hash`가 mismatch이면 수신측은 에러로 응답하고 처리하지 않는다(fail-fast 권장).

## 5.2 Data Write Response (MSG_DATA_RSP_STRUCT)
binary 고정 구조(예시):

```c
#pragma pack(push, 1)
typedef struct {
  uint32_t magic;   // 'DRSP'
  uint16_t ver;     // 1
  uint16_t status;  // 0=OK, 1=ERR
  uint32_t err;     // 상세 코드
} DataRsp;
#pragma pack(pop)
```

---

# 6. topic_id 규칙(필수)
## 6.1 생성 규칙
권장: **FNV-1a 32-bit** over UTF-8 topic string

- 입력: DDS topic name (Control plane에서 생성 시 사용한 동일 문자열)
- 출력: uint32

## 6.2 충돌 정책
- 기본: 충돌은 “극히 희박”하나, 발생 시:
  - Agent가 topic_id 재할당/테이블 배포를 할지,
  - 혹은 충돌 감지 시 에러로 fail할지
- 프로젝트 정책으로 명시해야 한다(권장: 충돌 감지 시 fail-fast)

---

# 7. 런타임/빌드 선택(권장)
Legacy/UI는 아래 중 하나를 선택해 동작해야 한다.

- `data_codec = json`
  - Data 프레임은 `MSG_DATA_*_JSON` 사용
- `data_codec = struct`
  - Data 프레임은 `MSG_DATA_*_STRUCT` 사용

> Control은 항상 `MSG_FRAME_REQ/RSP` (CBOR/JSON)

---

# 8. Legacy/UI 개발 사항(구현 체크리스트)
## 8.1 필수 공통 구현
- Header serialize/deserialize
- frame type dispatch
- corr_id 관리
- topic_id 계산(FNV-1a)

## 8.2 data_codec=json 구현
- CBOR encode/decode
- Data JSON 스키마 처리(write/evt)

## 8.3 data_codec=struct 구현
- DataEnvelope encode/decode
- ABI_HASH 검증 및 mismatch 처리
- IDL 기반 wire struct header include 및 구조체 채움/해석

---

# 9. 상위 서비스 로직 관점 코멘트(참고)
- **Control**은 “엔티티 관리” 중심:
  - Participant/Writer/Reader를 먼저 생성하고 준비가 끝나야 Data 송수신이 정상
- **Data**는 “샘플 송수신” 중심:
  - write 요청 시 topic_id가 유효해야 하며(Agent에 매핑이 존재), 그렇지 않으면 에러가 정상 동작
- UI와 Legacy는 내부 로직이 달라도,
  - 프레임 타입/스키마/encode 규칙만 동일하면 상호운용 가능

---

# 10. 변경 이행 전략(권장)
- 1차: control 그대로 유지 + data json 도입(Phase 2 대응)
- 2차: data struct 도입(Phase 3 대응)
- 3차: 필요 시 control EVT 또는 control 바이너리화는 “별도 프로젝트”로 분리 검토
