# track1_demoapp.md
DemoApp (VxWorks 모듈) – 시연용 동작 설계

## 1. 목표

- 장비(VxWorks) 상에서 동작하는 DemoApp 모듈을 통해,
  - 시연 대상 7개 DDS 메시지를 **정해진 방향/주기/QoS**에 맞게 송수신한다.
  - 내부 상태 머신에 따라 PowerOn BIT, CBIT, IBIT, 포구동 제어/피드백, 차량속도 시뮬레이션을 수행한다.
- DemoApp은 **사용자 UI를 직접 가지지 않고**,  
  CLI(TCP) 명령에 의해 상태/동작이 트리거된다.
- DDS 연동은 기존 **LegacyLib (JSON IPC → DDS Agent(Vx))**를 이용한다.

---

## 2. 시연 대상 메시지 및 QoS

### 2.1. AgentUI → DemoApp (DDS 수신, DemoApp Subscriber)

1. **포구동장치통제명령 (runBIT 요청)**
  - Topic: `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT`
   - 방향: AgentUI → DemoApp
   - 주기: 비주기
   - QoS: Non-Periodic Event
   - 역할:
     - IBIT(C-BIT) 실행 요청
     - `referenceNum`, `type(BITType)` 등을 포함

2. **포구동장치제어명령**
  - Topic: `P_NSTEL__C_CannonDrivingDevice_commandDriving`
   - 방향: AgentUI → DemoApp
   - 주기: 200Hz
   - QoS: High Frequency Periodic
   - 역할:
     - 포신 위치/속도/운용모드 제어

3. **차량속도정보**
  - Topic: `P_NSTEL__C_VehicleSpeed`
   - 방향: AgentUI → DemoApp
   - 주기: 1Hz
   - QoS: Low Frequency Vehicle
   - 역할:
     - 차량 속도 값 (시뮬레이션 입력)

---

### 2.2. DemoApp → AgentUI (DDS 송신, DemoApp Publisher)

4. **포구동장치 IBIT 결과**
  - Topic: `P_NSTEL__C_CannonDrivingDevice_IBIT`
   - 방향: DemoApp → AgentUI
   - 주기: 비주기
   - QoS: Non-Periodic Event
   - 역할:
     - runBIT 요청(1번)에 대한 논리적 응답
     - `referenceNum`, 구성품별 BIT 결과 포함

5. **포구동장치 CBIT 상태**
  - Topic: `P_NSTEL__C_CannonDrivingDevice_PBIT`
   - 방향: DemoApp → AgentUI
   - 주기: 1Hz
   - QoS: Low Frequency Status
   - 역할:
     - 운용 중 BIT 상태(구성품별 상태 스냅샷)

6. **포구동장치 PowerOn BIT 결과**
  - Topic: `P_NSTEL__C_CannonDrivingDevice_PowerOnBIT`
   - 방향: DemoApp → AgentUI
   - 주기: 비주기 (모듈 초기화 시)
   - QoS: Initial State
   - 역할:
     - 전원 인가/모듈 초기화 후 수행되는 PBIT 결과
     - Initial State QoS로 AgentUI는 나중에 붙어도 마지막 결과 1개 수신 가능

7. **포구동장치 신호획득 (피드백)**
  - Topic: `P_NSTEL__C_CannonDrivingDevice_Signal`
   - 방향: DemoApp → AgentUI
   - 주기: 200Hz
   - QoS: High Frequency Periodic
   - 역할:
     - 포구동 장치의 위치/속도/자세/센서값 피드백

---

## 3. DemoApp 상태 머신 설계

### 3.1. 상태 정의

```cpp
enum class DemoState {
    Idle,           // 모듈 로드 후, 데모 미시작
    Init,           // 데모 초기화 (DDS 준비)
    PowerOnBit,     // PowerOn BIT 수행 + PBIT 메시지 송신
    Run,            // 주기 DDS 송신 + 제어/속도 수신 처리
    IBitRunning,    // IBIT 수행 중 (runBIT 응답 준비)
};
```

### 3.2. 상태 전이

- `Idle`  
  - 진입 조건: 모듈 로드 후 기본 상태  
  - 전이:
    - CLI 명령 `demo_init` 수신 → `Init`

- `Init`
  - 동작:
    - LegacyLib 초기화 (Agent(Vx)와 JSON IPC 연결)
    - 7개 Topic에 대한 Writer/Reader 생성
  - 전이:
    - DDS 엔티티 생성 성공 → `PowerOnBit`
    - 실패 시: 로그 출력 후 `Idle` 복귀 또는 재시도 정책

- `PowerOnBit`
  - 동작:
    - 내부 PowerOn BIT 로직 수행
    - 결과를 PBIT 메시지로 1회(또는 소수회) DDS publish
  - 전이:
    - PBIT 완료 → `Run`

- `Run`
  - 동작:
    - 주기 업무:
      - 200Hz: `Cannon_Actuator_Signal` publish
      - 1Hz: `Cannon_Driving_Device_CBIT` publish
    - 수신 처리:
      - `Actuator_Control` 수신 → 내부 포구동 상태 업데이트
      - `Vehicle_Speed` 수신 → 내부 속도 상태 업데이트
      - `runBIT` 수신 → `IBitRunning` 전이
  - 전이:
    - `runBIT` 수신 → `IBitRunning`
    - CLI 명령 `demo_stop` 등으로 종료 가능 (선택 사항)

- `IBitRunning`
  - 동작:
    - `runBIT` 메시지에 기반하여 IBIT 수행
      - `referenceNum` 등 요청 필드 저장
    - 수행 완료 시:
      - `Cannon_Driving_Device_resultBIT`로 결과 DDS publish
      - 결과에 `referenceNum` 그대로 반영
  - 전이:
    - IBIT 완료 → `Run`

※ Fault 상태는 별도의 플래그로 관리:
- `bool faultLatched` + BIT 결과 메시지 내 비트로 표현

---

## 4. 주기/타이머 설계

- 기본 시스템 tick: 1ms 또는 5ms (플랫폼 상황에 맞게)
- 주기 분해:
  - 200Hz (5ms 단위): `Actuator_Signal` publish
  - 1Hz (1000ms 단위): `CBIT` publish

예:

```cpp
void demo_tick_1ms()
{
    ++tick_;

    if (tick_ % 5 == 0) on_200Hz();
    if (tick_ % 1000 == 0) on_1Hz();
}
```

---

## 5. DDS 수신 처리 설계

각 Topic 별 핸들러:

- `onRunBitReceived(msg)`
  - `DemoState::Run` 상태에서만 유효
  - IBIT 실행 시작 → 상태 `IBitRunning` 진입
  - `msg.referenceNum` 저장

- `onActuatorControlReceived(msg)`
  - 내부 포구동 상태 업데이트
  - 향후 200Hz Signal 계산에 반영

- `onVehicleSpeedReceived(msg)`
  - 내부 차량 속도 상태 업데이트
  - 시뮬레이션용(포신 안정화, 에너지 저장 등)에 사용

각 수신 시 CLI 로그 출력 (TCP로 전달용):

```cpp
cli_log("[RX] RunBIT ref=%d type=%d ...", msg.referenceNum, msg.type);
cli_log("[RX] VehicleSpeed=%.2f", msg.speed);
cli_log("[RX] ActuatorCtrl az=... upDown=...");
```

---

## 6. CLI 명령 설계 (시연용 기본)

향후 AgentUI와 연계할 CLI 문자열들:

- `demo_init`
  - 동작:
    - DDS Agent(Vx) 초기화
    - 엔티티 생성
    - 상태 `Idle` → `Init` → `PowerOnBit`
- `demo_start` (선택)
  - 필요 시 Init/PowerOnBit를 합쳐서 시작
- `run_ibit`
  - 내부 IBIT 강제 실행 또는  
    외부 runBIT 수신을 시뮬레이션하는 데 사용 가능 (정책에 따라)
- `fault_inject <target>`
  - 특정 구성품 fault 강제 발생 (향후 시연용)

추가 명령은 Track 3에서 AgentUI 설계와 맞물려 확장.

---

## 7. 산출물

- DemoApp 상태 머신 설계서
- LegacyLib 이용한 DDS I/O 초기화/해제 시퀀스 문서
- 주기 처리/타이머 구조 설계
- CLI 명령 목록 및 동작 정의
