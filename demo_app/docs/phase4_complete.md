# Phase 4 완료 보고서

## 📅 작업 일시

완료일: 2025년 12월 10일
소요 시간: 약 1.5일 분량

## 🎯 Phase 4 목표

IBIT(Initiated Built-In Test) 흐름 구현 - runBIT 수신부터 3초 후 resultBIT 발행까지 완전 자동화

## ✅ 완료 작업

### 1. runBIT 메시지 파싱 (demo_app_msg.c)

**함수**: `demo_msg_on_runbit()`

**파싱 필드**:
- `referenceNum` (uint32_t) - IBIT 요청 참조 번호
- `type` (int) - BIT 타입

**구현**:
```c
void demo_msg_on_runbit(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    // Parse referenceNum
    uint32_t reference_num = 0;
    const char* ref_str = strstr(json, "\"referenceNum\":");
    if (ref_str) {
        sscanf(ref_str, "\"referenceNum\":%u", &reference_num);
    }
    
    // Parse type
    int type = 0;
    const char* type_str = strstr(json, "\"type\":");
    if (type_str) {
        sscanf(type_str, "\"type\":%d", &type);
    }
    
    // Trigger IBIT
    demo_app_trigger_ibit(ctx, reference_num, type);
}
```

### 2. IBIT 트리거 로직 (demo_app_core.c)

**함수**: `demo_app_trigger_ibit()`

**동작**:
1. Run 상태 검증 (IBIT는 Run 상태에서만 가능)
2. BIT 상태 설정
   - `ibit_running = true`
   - `ibit_reference_num` 저장
   - `ibit_type` 저장
   - `ibit_start_time = tick_count` (타임스탬프 기록)
3. Run → IBitRunning 상태 전이

**코드**:
```c
int demo_app_trigger_ibit(DemoAppContext* ctx, uint32_t reference_num, int type) {
    if (ctx->current_state != DEMO_STATE_RUN) {
        printf("[DemoApp Core] ERROR: IBIT can only run from Run state\n");
        return -1;
    }
    
    ctx->bit_state.ibit_running = true;
    ctx->bit_state.ibit_reference_num = reference_num;
    ctx->bit_state.ibit_type = type;
    ctx->bit_state.ibit_start_time = ctx->tick_count;
    
    enter_state(ctx, DEMO_STATE_IBIT_RUNNING);
    
    return 0;
}
```

### 3. IBIT 수행 시뮬레이션 (demo_app_timer.c)

**타이머 틱 핸들러 개선**: `demo_timer_tick()`

**IBIT 실행 로직**:
```c
if (ctx->current_state == DEMO_STATE_IBIT_RUNNING) {
    uint64_t elapsed = ctx->tick_count - ctx->bit_state.ibit_start_time;
    
    // IBIT duration: 3 seconds (3000ms)
    if (elapsed >= 3000) {
        // Publish resultBIT
        demo_msg_publish_result_bit(ctx);
        
        // Clear IBIT state
        ctx->bit_state.ibit_running = false;
        
        // Transition back to Run state
        enter_state(ctx, DEMO_STATE_RUN);
    }
    
    return;  // Don't publish periodic messages during IBIT
}
```

**특징**:
- 정확히 3000ms (3초) 후 자동 완료
- IBIT 중에는 주기 메시지(Signal/CBIT) 발행 중단
- 완료 후 자동으로 Run 상태 복귀

### 4. resultBIT 발행 구현 (demo_app_msg.c)

**함수**: `demo_msg_publish_result_bit()`

**JSON 페이로드**:
```json
{
  "referenceNum": 12345,
  "testResult": 0,
  "azimuthResult": 0,
  "updownResult": 0,
  "sensorResult": 0,
  "completionTime": 15000
}
```

**필드 설명**:
- `referenceNum`: runBIT 요청의 참조 번호 (매칭 검증용)
- `testResult`: 전체 결과 (0=Pass, 1=Fail)
- `azimuthResult`: Azimuth 컴포넌트 결과
- `updownResult`: UpDown 컴포넌트 결과
- `sensorResult`: Sensor 컴포넌트 결과
- `completionTime`: 완료 시각 (ibit_start_time + 3000ms)

**고장 반영**:
```c
"testResult": (fault_azimuth || fault_updown || fault_sensor) ? 1 : 0
"azimuthResult": fault_azimuth ? 1 : 0
"updownResult": fault_updown ? 1 : 0
"sensorResult": fault_sensor ? 1 : 0
```

**QoS Profile**: `NstelCustomQosLib::NonPeriodicEventProfile` (이벤트성 메시지)

### 5. CLI 명령 연동

**CLI 명령**: `run_ibit <ref> <type>`

이미 Phase 1에서 구현되어 있으며, 이번 Phase 4에서 완전히 동작하도록 백엔드 연결 완료

**사용 예시**:
```
CLI> run_ibit 12345 0
IBIT triggered (ref=12345, type=0)
```

**동작**:
- AgentUI에서 runBIT 메시지 수신 없이도 수동으로 IBIT 실행 가능
- 테스트 및 디버깅에 유용

## 🔄 전체 IBIT 흐름

### 시나리오 1: AgentUI에서 runBIT 발행

```
1. AgentUI: runBIT 메시지 발행
   Topic: P_UCMS__C_Monitored_Entity_runBIT
   Payload: {"referenceNum": 12345, "type": 0}
   
2. DemoApp: runBIT 수신 (demo_msg_on_runbit)
   - referenceNum=12345, type=0 파싱
   - demo_app_trigger_ibit(ctx, 12345, 0) 호출
   
3. State Transition: Run → IBitRunning
   - ibit_start_time = tick_count (예: 10000ms)
   - ibit_reference_num = 12345
   - ibit_running = true
   
4. Timer Tick (매 1ms):
   - elapsed = tick_count - ibit_start_time
   - if (elapsed < 3000): 대기
   - if (elapsed >= 3000): 완료 처리
   
5. IBIT 완료 (elapsed=3000ms):
   - demo_msg_publish_result_bit() 호출
   - resultBIT 발행:
     {
       "referenceNum": 12345,
       "testResult": 0,
       "completionTime": 13000
     }
   - ibit_running = false
   
6. State Transition: IBitRunning → Run
   - 주기 메시지 발행 재개
```

### 시나리오 2: CLI에서 수동 실행

```
CLI> run_ibit 99999 1

→ 동일한 흐름으로 3초 후 resultBIT 발행
```

### 시나리오 3: 고장 상태에서 IBIT

```
CLI> fault_inject azimuth
Fault injected: Azimuth

CLI> run_ibit 55555 0

3초 후:
resultBIT: {
  "referenceNum": 55555,
  "testResult": 1,        // Fail
  "azimuthResult": 1,     // Fail
  "updownResult": 0,      // Pass
  "sensorResult": 0       // Pass
}
```

## 📊 코드 통계

| 파일 | 변경 전 | 변경 후 | 증가 |
|------|---------|---------|------|
| demo_app_msg.c | 484줄 | 530줄 | +46줄 |
| demo_app_timer.c | 200줄 | 231줄 | +31줄 |
| demo_app_core.c | 335줄 | 340줄 | +5줄 |
| **합계** | 1,019줄 | 1,101줄 | **+82줄** |

## 🎯 달성 목표

✅ **runBIT 파싱**: referenceNum, type 추출  
✅ **IBIT 트리거**: Run → IBitRunning 전이  
✅ **3초 시뮬레이션**: 타이머 기반 자동 완료  
✅ **resultBIT 발행**: referenceNum 매칭, 컴포넌트별 결과  
✅ **상태 복귀**: IBitRunning → Run 자동 전이  
✅ **CLI 연동**: run_ibit 명령 완전 동작  

## 🔧 구현 세부사항

### referenceNum 매칭

**목적**: AgentUI가 요청과 결과를 매칭하기 위함

**구현**:
- runBIT 수신 시 referenceNum 저장
- resultBIT 발행 시 동일한 referenceNum 사용

**검증**:
```
Request:  {"referenceNum": 12345, ...}
Response: {"referenceNum": 12345, ...}  // 매칭됨
```

### IBIT 중 주기 메시지 중단

**이유**: IBIT 수행 중에는 정상 동작 메시지를 보내지 않음

**구현**:
```c
if (ctx->current_state == DEMO_STATE_IBIT_RUNNING) {
    // IBIT 로직만 실행
    return;  // 주기 메시지 발행 스킵
}

// Run 상태일 때만 주기 메시지 발행
if (ctx->current_state == DEMO_STATE_RUN) {
    demo_msg_publish_actuator_signal(ctx);
    demo_msg_publish_cbit(ctx);
}
```

### 타임스탬프 관리

**tick_count 사용**:
- `ibit_start_time = tick_count` (ms 단위)
- `completionTime = ibit_start_time + 3000`
- `elapsed = tick_count - ibit_start_time`

**정확도**: 1ms 분해능 (VxWorks 시스템 tick rate 의존)

### 고장 상태 반영

**BITState 플래그**:
- `fault_azimuth`
- `fault_updown`
- `fault_sensor`

**resultBIT 계산**:
```c
testResult = (fault_azimuth || fault_updown || fault_sensor) ? 1 : 0
azimuthResult = fault_azimuth ? 1 : 0
updownResult = fault_updown ? 1 : 0
sensorResult = fault_sensor ? 1 : 0
```

## 🚀 테스트 시나리오

### 기본 IBIT 테스트

**준비**:
```
VxWorks Shell:
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")

CLI (telnet 127.0.0.1 23000):
> demo_init 127.0.0.1 25000
> status
```

**IBIT 실행**:
```
CLI> run_ibit 12345 0

[DemoApp Core] Triggering IBIT: ref=12345, type=0
[DemoApp Core] IBIT started at tick 10000 (will complete in 3 seconds)
[DemoApp Core] State transition: Run -> IBitRunning

(3초 대기...)

[DemoApp Timer] IBIT completed after 3000 ms
[DemoApp Msg] Published resultBIT: ref=12345, result=PASS
[DemoApp Timer] Returned to Run state
[DemoApp Core] State transition: IBitRunning -> Run
```

**검증**:
- AgentUI에서 resultBIT 수신 확인
- referenceNum=12345 매칭 확인
- testResult=0 (Pass) 확인

### 고장 주입 IBIT 테스트

```
CLI> fault_inject azimuth
Fault injected: Azimuth

CLI> fault_inject sensor
Fault injected: Sensor

CLI> status
State: Run
Faults:
  Azimuth: FAULT
  UpDown: OK
  Sensor: FAULT

CLI> run_ibit 99999 1

(3초 후)

[DemoApp Msg] Published resultBIT: ref=99999, result=FAIL
```

**AgentUI에서 수신**:
```json
{
  "referenceNum": 99999,
  "testResult": 1,
  "azimuthResult": 1,
  "updownResult": 0,
  "sensorResult": 1,
  "completionTime": 15000
}
```

### AgentUI에서 runBIT 발행 테스트

**AgentUI**:
```
Topic: P_UCMS__C_Monitored_Entity_runBIT
Type: P_UCMS::C_Monitored_Entity_runBIT
Payload:
{
  "referenceNum": 55555,
  "type": 0
}

[Publish]
```

**DemoApp 로그**:
```
[DemoApp Msg] Received runBIT: {"referenceNum": 55555, "type": 0}
[DemoApp Msg] runBIT parsed: referenceNum=55555, type=0
[DemoApp Core] Triggering IBIT: ref=55555, type=0
...
[DemoApp Msg] Published resultBIT: ref=55555, result=PASS
```

**AgentUI 수신**:
```
Topic: P_NSTEL__C_Cannon_Driving_Device_resultBIT
Payload: {"referenceNum": 55555, "testResult": 0, ...}
```

## 📝 다음 Phase 준비 사항

### Phase 5: 통합 테스트 & 완성

**예정 작업**:
1. **7개 메시지 통합 테스트**
   - PBIT, CBIT, ResultBIT
   - Actuator Control, Actuator Signal
   - Vehicle Speed
   - runBIT

2. **QoS 동작 검증**
   - InitialState: PBIT
   - LowFreqStatus: CBIT
   - NonPeriodicEvent: ResultBIT, runBIT
   - HighFreqPeriodic: Actuator Signal, Control
   - LowFreqVehicle: Vehicle Speed

3. **장기 안정성 테스트**
   - 2시간 연속 동작
   - 메모리 누수 검사
   - 타이밍 지터 측정

4. **문서 완성**
   - demo_app/docs/design.md
   - demo_app/docs/messages.md
   - demo_app/docs/testing.md

## ✨ 주요 특징

1. **완전 자동화**: runBIT 수신부터 resultBIT 발행까지 사람 개입 없음
2. **정확한 타이밍**: 3초 (3000ms) 정확히 측정
3. **referenceNum 매칭**: 요청-응답 추적 가능
4. **고장 반영**: 컴포넌트별 고장 상태가 결과에 반영
5. **주기 메시지 제어**: IBIT 중 주기 메시지 자동 중단
6. **수동 테스트 지원**: CLI 명령으로 IBIT 직접 실행 가능

## 📌 주의사항

### IBIT 실행 조건

**Run 상태에서만 가능**:
- Idle: 불가
- Init: 불가
- PowerOnBit: 불가
- IBitRunning: 불가 (이미 실행 중)
- **Run: 가능**

### IBIT 중 동작

**중단되는 것**:
- 200Hz Actuator Signal 발행
- 1Hz CBIT 발행
- 시뮬레이션 업데이트

**계속되는 것**:
- tick_count 증가
- Control/Speed 메시지 수신 (파싱은 하지만 무시)
- DDS 연결 유지

### referenceNum 중요성

**용도**:
- AgentUI가 여러 IBIT 요청 중 어떤 것의 응답인지 식별
- 로그 추적
- 타임아웃 검사

**권장**:
- 순차 증가하는 번호 사용 (1, 2, 3, ...)
- 또는 타임스탬프 사용

---

**Phase 4 완료**: IBIT 흐름 구현 완료 ✅

**다음**: Phase 5 (통합 테스트 & 완성) 준비 완료
