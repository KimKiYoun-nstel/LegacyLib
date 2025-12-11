# Agent JSON 메시지 포맷 명세

## 개요

본 문서는 DemoApp이 INIT 단계에서 Agent(RTP)로 전송하는 JSON 메시지 포맷을 정의합니다.  
모든 메시지는 **IPC JSON Protocol**을 따르며, QoS 프로파일은 `NstelCustomQosLib::ProfileName` 형식을 사용합니다.

---

## 메시지 공통 구조

모든 요청 메시지는 다음 필드를 포함합니다:

```json
{
  "op": "operation_name",     // 작업 타입 (hello, create 등)
  "proto": 1,                  // 프로토콜 버전 (고정값 1)
  "target": {                  // 대상 엔티티
    "kind": "entity_kind",     // participant, publisher, subscriber, writer, reader
    "topic": "...",            // (Writer/Reader만 해당)
    "type": "..."              // (Writer/Reader만 해당)
  },
  "args": {                    // 작업 인자
    "domain": 0,               // Domain ID
    "qos": "LIB::Profile",     // QoS 프로파일 (선택)
    ...
  },
  "data": null                 // 데이터 페이로드 (현재 사용 안 함)
}
```

**중요**: 
- `proto`: 항상 `1`
- `data`: 항상 `null` (DDS 엔티티 생성 시)
- QoS는 **"LibraryName::ProfileName"** 형식 (예: `"NstelCustomQosLib::HighFreqPeriodicProfile"`)

---

## 1. Hello 메시지

### 목적
Agent와의 초기 핸드셰이크

### JSON 포맷
```json
{
  "op": "hello",
  "proto": 1,
  "target": {
    "kind": "agent"
  },
  "args": null,
  "data": null
}
```

### 응답 예상
```json
{
  "ok": true,
  "proto": 1,
  "result": {
    "proto": 1
  }
}
```

**코드 위치**: `IpcJsonClient::sendHello()`

---

## 2. Participant 생성

### 목적
DDS Domain Participant 생성 (Domain 0)

### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "participant"
  },
  "args": {
    "domain": 0,
    "qos": "TriadQosLib::DefaultReliable"
  },
  "data": null
}
```

### 필드 설명
- `args.domain`: Domain ID (기본값: 0)
- `args.qos`: QoS 프로파일 (`TriadQosLib::DefaultReliable` - RELIABLE, UDPv4)

**코드 위치**: `demo_app_core.c:create_dds_entities()` → `IpcJsonClient::createParticipant()`

---

## 3. Publisher 생성

### 목적
송신용 Publisher 생성 (이름: "pub1")

### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "publisher"
  },
  "args": {
    "domain": 0,
    "publisher": "pub1",
    "qos": "TriadQosLib::DefaultReliable"
  },
  "data": null
}
```

### 필드 설명
- `args.publisher`: Publisher 이름 (DemoApp은 "pub1" 사용)
- `args.qos`: QoS 프로파일 (`TriadQosLib::DefaultReliable`)

**코드 위치**: `demo_app_core.c:create_dds_entities()` → `IpcJsonClient::createPublisher()`

---

## 4. Subscriber 생성

### 목적
수신용 Subscriber 생성 (이름: "sub1")

### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "subscriber"
  },
  "args": {
    "domain": 0,
    "subscriber": "sub1",
    "qos": "TriadQosLib::DefaultReliable"
  },
  "data": null
}
```

### 필드 설명
- `args.subscriber`: Subscriber 이름 (DemoApp은 "sub1" 사용)
- `args.qos`: QoS 프로파일 (`TriadQosLib::DefaultReliable`)

**코드 위치**: `demo_app_core.c:create_dds_entities()` → `IpcJsonClient::createSubscriber()`

---

## 5. DataWriter 생성 (4개)

### 5.1 PBIT Writer

#### 목적
PowerOn BIT 결과 송신 (초기화 시 1회)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "writer",
    "topic": "P_NSTEL__C_Cannon_Driving_Device_PBIT",
    "type": "P_NSTEL::C_Cannon_Driving_Device_PBIT"
  },
  "args": {
    "domain": 0,
    "publisher": "pub1",
    "qos": "NstelCustomQosLib::InitialStateProfile"
  },
  "data": null
}
```

#### QoS 특성 (nstel_custom_qos.xml 참조)
- **Reliability**: RELIABLE
- **History**: KEEP_LAST (depth=1)
- **Durability**: TRANSIENT_LOCAL (늦게 연결된 Subscriber도 수신)
- **Deadline**: 없음

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - PBIT Writer

---

### 5.2 CBIT Writer

#### 목적
Continuous BIT 상태 송신 (1Hz 주기)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "writer",
    "topic": "P_NSTEL__C_Cannon_Driving_Device_CBIT",
    "type": "P_NSTEL::C_Cannon_Driving_Device_CBIT"
  },
  "args": {
    "domain": 0,
    "publisher": "pub1",
    "qos": "NstelCustomQosLib::LowFreqStatusProfile"
  },
  "data": null
}
```

#### QoS 특성 (nstel_custom_qos.xml 참조)
- **Reliability**: RELIABLE
- **History**: KEEP_LAST (depth=1)
- **Durability**: TRANSIENT_LOCAL
- **Deadline**: 1초 (1Hz)

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - CBIT Writer

---

### 5.3 ResultBIT Writer

#### 목적
IBIT 결과 송신 (IBIT 완료 시)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "writer",
    "topic": "P_NSTEL__C_Cannon_Driving_Device_resultBIT",
    "type": "P_NSTEL::C_Cannon_Driving_Device_resultBIT"
  },
  "args": {
    "domain": 0,
    "publisher": "pub1",
    "qos": "NstelCustomQosLib::NonPeriodicEventProfile"
  },
  "data": null
}
```

#### QoS 특성 (nstel_custom_qos.xml 참조)
- **Reliability**: RELIABLE
- **History**: KEEP_ALL
- **Durability**: VOLATILE
- **Resource Limits**: max_samples=1000, max_instances=10, max_samples_per_instance=100

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - ResultBIT Writer

---

### 5.4 Actuator Signal Writer

#### 목적
포구동 피드백 송신 (200Hz 주기)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "writer",
    "topic": "P_NSTEL__C_Cannon_Actuator_Signal",
    "type": "P_NSTEL::C_Cannon_Actuator_Signal"
  },
  "args": {
    "domain": 0,
    "publisher": "pub1",
    "qos": "NstelCustomQosLib::HighFreqPeriodicProfile"
  },
  "data": null
}
```

#### QoS 특성 (nstel_custom_qos.xml 참조)
- **Reliability**: BEST_EFFORT (고속 전송)
- **History**: KEEP_LAST (depth=1)
- **Durability**: VOLATILE
- **Deadline**: 5ms (200Hz)

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - Actuator Signal Writer

---

## 6. DataReader 생성 (3개)

### 6.1 runBIT Reader

#### 목적
IBIT 요청 수신 (AgentUI → DemoApp)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "reader",
    "topic": "P_UCMS__C_Monitored_Entity_runBIT",
    "type": "P_UCMS::C_Monitored_Entity_runBIT"
  },
  "args": {
    "domain": 0,
    "subscriber": "sub1",
    "qos": "NstelCustomQosLib::NonPeriodicEventProfile"
  },
  "data": null
}
```

#### QoS 특성
- **Reliability**: RELIABLE
- **History**: KEEP_ALL
- **Durability**: VOLATILE

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - runBIT Reader

---

### 6.2 Actuator Control Reader

#### 목적
포구동 제어 명령 수신 (AgentUI → DemoApp, 200Hz)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "reader",
    "topic": "P_NSTEL__C_Cannon_Actuator_Control",
    "type": "P_NSTEL::C_Cannon_Actuator_Control"
  },
  "args": {
    "domain": 0,
    "subscriber": "sub1",
    "qos": "NstelCustomQosLib::HighFreqPeriodicProfile"
  },
  "data": null
}
```

#### QoS 특성
- **Reliability**: BEST_EFFORT (고속 전송)
- **History**: KEEP_LAST (depth=1)
- **Durability**: VOLATILE
- **Deadline**: 5ms (200Hz)

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - Actuator Control Reader

---

### 6.3 Vehicle Speed Reader

#### 목적
차량 속도 수신 (AgentUI → DemoApp, 1Hz)

#### JSON 포맷
```json
{
  "op": "create",
  "proto": 1,
  "target": {
    "kind": "reader",
    "topic": "P_NSTEL__C_Vehicle_Speed",
    "type": "P_NSTEL::C_Vehicle_Speed"
  },
  "args": {
    "domain": 0,
    "subscriber": "sub1",
    "qos": "NstelCustomQosLib::LowFreqVehicleProfile"
  },
  "data": null
}
```

#### QoS 특성
- **Reliability**: RELIABLE
- **History**: KEEP_LAST (depth=5) ← LowFreqStatusProfile 기반 확장
- **Durability**: TRANSIENT_LOCAL
- **Deadline**: 1초 (1Hz)

**코드 위치**: `demo_app_msg.c:demo_msg_init()` - Vehicle Speed Reader

---

## QoS 프로파일 요약

### DDS 엔티티 QoS

| 엔티티 | QoS Profile | Reliability | Transport |
|--------|-------------|-------------|----------|
| Participant | `TriadQosLib::DefaultReliable` | RELIABLE | UDPv4 |
| Publisher | `TriadQosLib::DefaultReliable` | RELIABLE | - |
| Subscriber | `TriadQosLib::DefaultReliable` | RELIABLE | - |

**특징** (`qos_profiles.xml` 참조):
- `base_name`: `BuiltinQosLibExp::Generic.StrictReliable`
- `publish_mode`: ASYNCHRONOUS
- `transport`: UDPv4 only
- `is_default_qos`: true (기본 QoS)

### 메시지 QoS

| 메시지 | QoS Profile | Reliability | History | Durability | Deadline |
|--------|-------------|-------------|---------|------------|----------|
| PBIT | `NstelCustomQosLib::InitialStateProfile` | RELIABLE | KEEP_LAST(1) | TRANSIENT_LOCAL | - |
| CBIT | `NstelCustomQosLib::LowFreqStatusProfile` | RELIABLE | KEEP_LAST(1) | TRANSIENT_LOCAL | 1s |
| resultBIT | `NstelCustomQosLib::NonPeriodicEventProfile` | RELIABLE | KEEP_ALL | VOLATILE | - |
| Actuator Signal | `NstelCustomQosLib::HighFreqPeriodicProfile` | BEST_EFFORT | KEEP_LAST(1) | VOLATILE | 5ms |
| runBIT | `NstelCustomQosLib::NonPeriodicEventProfile` | RELIABLE | KEEP_ALL | VOLATILE | - |
| Actuator Control | `NstelCustomQosLib::HighFreqPeriodicProfile` | BEST_EFFORT | KEEP_LAST(1) | VOLATILE | 5ms |
| Vehicle Speed | `NstelCustomQosLib::LowFreqVehicleProfile` | RELIABLE | KEEP_LAST(5) | TRANSIENT_LOCAL | 1s |

**XML 참조**: `d:\CodeDev\LegacyLib\RefDoc\nstel_custom_qos.xml`

---

## 메시지 송신 순서

DemoApp 시작 시 다음 순서로 메시지가 Agent로 전송됩니다:

```
1. Hello                          → Agent 핸드셰이크
   ↓
2. Create Participant (domain=0, qos=TriadQosLib::DefaultReliable)  → DDS Domain 참여
   ↓
3. Create Publisher (pub1, qos=TriadQosLib::DefaultReliable)        → 송신용 엔티티
   ↓
4. Create Subscriber (sub1, qos=TriadQosLib::DefaultReliable)       → 수신용 엔티티
   ↓
5. Create Writer: PBIT            → 송신 Topic 1
6. Create Writer: CBIT            → 송신 Topic 2
7. Create Writer: resultBIT       → 송신 Topic 3
8. Create Writer: Actuator Signal → 송신 Topic 4
   ↓
9. Create Reader: runBIT          → 수신 Topic 1
10. Create Reader: Actuator Control → 수신 Topic 2
11. Create Reader: Vehicle Speed  → 수신 Topic 3
   ↓
12. Subscribe Event: runBIT       → 이벤트 콜백 등록
13. Subscribe Event: Control      → 이벤트 콜백 등록
14. Subscribe Event: Speed        → 이벤트 콜백 등록
```

**총 14개 JSON 메시지** (Hello 포함)

---

## 검증 체크리스트

### Agent 수신 시 확인사항

#### ✅ QoS 포맷 검증
- [ ] QoS가 `"LibraryName::ProfileName"` 형식인가?
- [ ] 프로파일 이름이 nstel_custom_qos.xml과 일치하는가?
- [ ] Profile 이름에 오타가 없는가? (예: HighFrequencyPeriodicProfile → HighFreqPeriodicProfile)

#### ✅ Topic/Type 매칭 검증
- [ ] Topic 이름이 정확한가? (네임스페이스 구분자: `__`)
- [ ] Type 이름이 정확한가? (네임스페이스 구분자: `::`)
- [ ] Writer의 Topic과 Reader의 Topic이 일치하는가?

#### ✅ Domain/Publisher/Subscriber 검증
- [ ] 모든 엔티티가 동일한 domain(0)을 사용하는가?
- [ ] Writer가 올바른 Publisher("pub1")를 참조하는가?
- [ ] Reader가 올바른 Subscriber("sub1")를 참조하는가?

#### ✅ 프로토콜 검증
- [ ] `"proto": 1`이 모든 메시지에 포함되어 있는가?
- [ ] `"data": null`이 모든 create 요청에 포함되어 있는가?

---

## 트러블슈팅

### 문제 1: QoS 프로파일을 찾을 수 없음
```
Agent Error: Unknown QoS profile "InitialStateProfile"
```

**원인**: QoS를 프로파일 이름만 전송 (Library 이름 누락)  
**해결**: `"NstelCustomQosLib::InitialStateProfile"` 형식으로 전송

---

### 문제 2: Topic 이름 불일치
```
Agent Warning: No matching DataWriter for topic "P_NSTEL__C_Cannon_..."
```

**원인**: 네임스페이스 구분자 오류 (`::` vs `__`)  
**해결**: 
- Topic 이름: `P_NSTEL__C_Cannon_...` (언더스코어 2개)
- Type 이름: `P_NSTEL::C_Cannon_...` (콜론 2개)

---

### 문제 3: Writer/Reader 생성 실패
```
Agent Error: Publisher "pub1" not found
```

**원인**: Publisher/Subscriber 생성 전에 Writer/Reader 생성 시도  
**해결**: 순서 준수 (Participant → Publisher/Subscriber → Writer/Reader)

---

## 부록: 실제 전송 예제

### QoS 프로파일 상세 (qos_profiles.xml)

#### TriadQosLib::DefaultReliable
```xml
<qos_profile name="DefaultReliable" is_default_qos="true" 
             base_name="BuiltinQosLibExp::Generic.StrictReliable">
  <participant_qos>
    <transport_builtin>
      <mask>UDPv4</mask>
    </transport_builtin>
  </participant_qos>
  <datawriter_qos>
    <publish_mode>
      <kind>ASYNCHRONOUS_PUBLISH_MODE_QOS</kind>
    </publish_mode>
  </datawriter_qos>
  <datareader_qos>
    <reliability>
      <kind>RELIABLE_RELIABILITY_QOS</kind>
    </reliability>
  </datareader_qos>
</qos_profile>
```

**사용처**:
- Participant: Transport 설정 (UDPv4만 사용)
- Publisher/Subscriber: 기본 RELIABLE 설정
- Writer/Reader 개별 QoS로 오버라이드됨

---

### PBIT Writer 생성 로그 예제
```
[DemoApp Msg] Initializing message handlers...
[IpcJsonClient] Sending: {"args":{"domain":0,"publisher":"pub1","qos":"NstelCustomQosLib::InitialStateProfile"},"data":null,"op":"create","proto":1,"target":{"kind":"writer","topic":"P_NSTEL__C_Cannon_Driving_Device_PBIT","type":"P_NSTEL::C_Cannon_Driving_Device_PBIT"}}
[Agent] Writer created for P_NSTEL__C_Cannon_Driving_Device_PBIT
[DemoApp Msg] Writer created: P_NSTEL__C_Cannon_Driving_Device_PBIT
```

### runBIT Reader 생성 로그 예제
```
[IpcJsonClient] Sending: {"args":{"domain":0,"qos":"NstelCustomQosLib::NonPeriodicEventProfile","subscriber":"sub1"},"data":null,"op":"create","proto":1,"target":{"kind":"reader","topic":"P_UCMS__C_Monitored_Entity_runBIT","type":"P_UCMS::C_Monitored_Entity_runBIT"}}
[Agent] Reader created for P_UCMS__C_Monitored_Entity_runBIT
[DemoApp Msg] Reader created: P_UCMS__C_Monitored_Entity_runBIT
```

---

## 결론

본 문서는 DemoApp이 Agent로 전송하는 **14개 JSON 메시지**의 정확한 포맷을 정의합니다.

**핵심 요약**:
1. **QoS 형식**: `"LibraryName::ProfileName"` (Library::Profile)
2. **DDS 엔티티 QoS**: `"TriadQosLib::DefaultReliable"` (Participant, Publisher, Subscriber)
3. **메시지 QoS**: `"NstelCustomQosLib::ProfileName"` (Writer, Reader)
4. **Topic 구분자**: `__` (언더스코어 2개)
5. **Type 구분자**: `::` (콜론 2개)
6. **프로토콜 버전**: 항상 `"proto": 1`
7. **데이터 필드**: 엔티티 생성 시 `"data": null`

모든 JSON 메시지가 이 규격을 준수하면 Agent와 정상적으로 통신할 수 있습니다.
