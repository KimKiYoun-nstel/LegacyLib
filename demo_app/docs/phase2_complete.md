# Phase 2 완료 보고서

## 📅 작업 일시

- 완료일: 2025년 12월 10일
- 소요 시간: 약 2일 분량

## 🎯 Phase 2 목표

상태 머신 구현 및 DDS 초기화 완료 (Idle → Init → PowerOnBit → Run)

## ✅ 완료 작업

### 1. 상태 머신 구현 (demo_app_core.c)

#### 1.1 LegacyLib 초기화
```c
// demo_app_start() 함수에서
LegacyConfig cfg = {
    agent_ip, agent_port,
    100, 64*1024,  // recv task
    100, 64*1024,  // send task
    NULL, NULL     // log callbacks
};
legacy_agent_init(&cfg, &ctx->agent);
```

#### 1.2 DDS 엔티티 생성
새로운 헬퍼 함수 `create_dds_entities()`:
- **Participant** 생성 (domain 0)
- **Publisher** 생성 ("pub1")
- **Subscriber** 생성 ("sub1")
- 콜백으로 생성 결과 확인

#### 1.3 상태 전이 로직
```
Idle → Init (LegacyLib 초기화)
     ↓
     Init → PowerOnBit (DDS 엔티티 생성)
     ↓
     PowerOnBit → Run (PBIT 수행 및 발행)
```

#### 1.4 종료 로직
```c
demo_app_stop():
- demo_msg_cleanup() 호출
- DDS 엔티티 제거
- LegacyLib 종료
- Idle 상태로 복귀
```

### 2. 메시지 핸들러 구현 (demo_app_msg.c)

#### 2.1 7개 Topic DDS 엔티티 생성

**Writers (4개 - 송신)**:

1. **PBIT Writer**
   - Topic: `P_NSTEL__C_Cannon_Driving_Device_PBIT`
   - QoS: `NstelCustomQosLib::InitialStateProfile`
   - 용도: PowerOn BIT 결과 (초기 1회)

2. **CBIT Writer**
   - Topic: `P_NSTEL__C_Cannon_Driving_Device_CBIT`
   - QoS: `NstelCustomQosLib::LowFreqStatusProfile`
   - 용도: Continuous BIT 상태 (1Hz)

3. **ResultBIT Writer**
   - Topic: `P_NSTEL__C_Cannon_Driving_Device_resultBIT`
   - QoS: `NstelCustomQosLib::NonPeriodicEventProfile`
   - 용도: IBIT 결과 (비주기)

4. **Actuator Signal Writer**
   - Topic: `P_NSTEL__C_Cannon_Actuator_Signal`
   - QoS: `NstelCustomQosLib::HighFreqPeriodicProfile`
   - 용도: 피드백 신호 (200Hz)

**Readers (3개 - 수신)**:

1. **runBIT Reader**
   - Topic: `P_UCMS__C_Monitored_Entity_runBIT`
   - QoS: `NstelCustomQosLib::NonPeriodicEventProfile`
   - 콜백: `demo_msg_on_runbit()`

2. **Actuator Control Reader**
   - Topic: `P_NSTEL__C_Cannon_Actuator_Control`
   - QoS: `NstelCustomQosLib::HighFreqPeriodicProfile`
   - 콜백: `demo_msg_on_actuator_control()`

3. **Vehicle Speed Reader**
   - Topic: `P_NSTEL__C_Vehicle_Speed`
   - QoS: `NstelCustomQosLib::LowFreqVehicleProfile`
   - 콜백: `demo_msg_on_vehicle_speed()`

#### 2.2 QoS 프로파일 매핑

| 메시지 | 주기 | QoS 프로파일 |
|--------|------|-------------|
| PBIT | 초기 1회 | InitialStateProfile |
| CBIT | 1Hz | LowFreqStatusProfile |
| ResultBIT | 비주기 | NonPeriodicEventProfile |
| Actuator Signal | 200Hz | HighFreqPeriodicProfile |
| runBIT | 비주기 | NonPeriodicEventProfile |
| Actuator Control | 200Hz | HighFreqPeriodicProfile |
| Vehicle Speed | 1Hz | LowFreqVehicleProfile |

### 3. PBIT 로직 구현

#### 3.1 PBIT JSON 페이로드
```json
{
  "deviceId": "CANNON_DRIVE_01",
  "timestamp": 0,
  "bitType": "PowerOn",
  "overallStatus": "OK",
  "components": [
    {"name": "Azimuth", "status": "OK", "code": 0},
    {"name": "UpDown", "status": "OK", "code": 0},
    {"name": "Sensor", "status": "OK", "code": 0}
  ]
}
```

#### 3.2 PBIT 발행 시퀀스
1. PowerOnBit 상태 진입
2. 1초 대기 (PBIT 시뮬레이션)
3. `demo_msg_publish_pbit()` 호출
4. DDS로 PBIT 메시지 발행
5. Run 상태로 전이

### 4. 추가 구현

#### 4.1 CBIT 발행 함수
- 1Hz 주기 발행 준비
- 현재 고장 상태 포함 (fault_azimuth, fault_updown, fault_sensor)
- JSON 동적 생성

#### 4.2 Actuator Signal 발행 함수
- 200Hz 주기 발행 준비
- 위치/속도 피드백 포함
- 타임스탬프 포함

#### 4.3 콜백 함수들
- `on_hello()` - Agent 연결 확인
- `on_entity_created()` - DDS 엔티티 생성 확인
- `on_writer_created()` - Writer 생성 확인
- `on_reader_created()` - Reader 생성 확인
- `on_write_complete()` - 메시지 발행 확인

## 📊 코드 통계

| 파일 | 변경 전 | 변경 후 | 증가 |
|------|---------|---------|------|
| demo_app_core.c | 186줄 | 320줄 | +134줄 |
| demo_app_msg.c | 124줄 | 330줄 | +206줄 |
| **합계** | 310줄 | 650줄 | **+340줄** |

## 🔧 구현 세부사항

### 타이밍 처리

VxWorks 시스템 함수 사용:
```c
#ifdef _VXWORKS_
taskDelay(sysClkRateGet());      // 1초 대기
taskDelay(sysClkRateGet() / 2);  // 500ms 대기
#endif
```

### 에러 처리

- 각 단계별 실패 시 Idle 상태로 복귀
- LegacyLib 리소스 정리
- 명확한 에러 메시지 출력

### 동기화

- 엔티티 생성 후 대기 시간 추가
- 콜백 기반 비동기 처리
- 상태 기반 흐름 제어

## 🎯 달성 목표

✅ **LegacyLib 초기화**: Agent 연결 및 Hello 교환  
✅ **DDS 엔티티 생성**: Participant, Publisher, Subscriber  
✅ **7개 Topic 설정**: 4개 Writer + 3개 Reader  
✅ **QoS 프로파일 적용**: 메시지 특성에 맞는 QoS  
✅ **PBIT 로직**: PowerOn BIT 수행 및 발행  
✅ **상태 전이**: Idle → Init → PowerOnBit → Run  
✅ **종료 로직**: 안전한 리소스 정리  

## 🔄 상태 전이 흐름

```
1. demoAppStart() 호출
   ↓
2. Idle → Init
   - LegacyLib 초기화
   - Agent Hello 교환
   ↓
3. Init → PowerOnBit
   - DDS 엔티티 생성 (Participant, Publisher, Subscriber)
   - 7개 Topic Writer/Reader 생성
   - 이벤트 구독 (3개 Reader)
   ↓
4. PowerOnBit → Run
   - 1초 PBIT 시뮬레이션
   - PBIT 메시지 발행
   - Run 상태 진입 완료
```

## 📝 다음 Phase 준비 사항

### Phase 3에서 구현할 항목

1. **주기 메시지 발행**
   - 200Hz: Actuator Signal
   - 1Hz: CBIT

2. **타이머 서브시스템**
   - `demo_app_timer.c` 구현
   - 1ms tick 기반 주기 분리

3. **수신 메시지 처리**
   - Control 명령 파싱 및 적용
   - Vehicle Speed 파싱 및 적용
   - 내부 상태 업데이트

4. **시뮬레이션 로직**
   - 제어 명령에 따른 위치 업데이트
   - 속도 계산
   - 피드백 신호 생성

## 🚀 테스트 시나리오

### 기본 시작 테스트

```
VxWorks Shell:
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")

CLI (telnet):
> demo_init 127.0.0.1 25000
> status

예상 결과:
- State: Run
- PBIT Completed: Yes
- Signal Published: 0 (Phase 3에서 증가)
```

### 종료 테스트

```
CLI:
> demo_stop

VxWorks Shell:
-> demoAppStop()
```

## ✨ 주요 특징

1. **완전한 초기화**: Agent → DDS → Messages
2. **비동기 콜백**: 모든 작업 결과 확인
3. **에러 복구**: 실패 시 안전한 종료
4. **상태 기반**: 명확한 상태 전이
5. **확장 가능**: Phase 3 주기 처리 준비 완료

## 📌 주의사항

### VxWorks 전용 코드

```c
#ifdef _VXWORKS_
taskDelay(sysClkRateGet());
#endif
```

### 대기 시간

- Hello 응답: 500ms
- 엔티티 생성: 500ms
- Message handlers: 1초
- PBIT 시뮬레이션: 1초

**총 초기화 시간**: 약 3초

---

**Phase 2 완료**: 상태 머신 및 DDS 초기화 완료 ✅

**다음**: Phase 3 (주기 메시지 송수신) 준비 완료
