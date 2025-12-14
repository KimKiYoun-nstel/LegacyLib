# DemoApp TCP CLI 사용 가이드

## 개요

DemoApp은 키보드 입력 없이 애플리케이션을 제어할 수 있는 **TCP 기반 CLI (Command Line Interface)** 를 제공합니다. 이는 다음과 같은 환경에서 필수적입니다:

- **VxWorks DKM**: 커널 모드에서 콘솔 I/O 사용 불가
- **Windows**: 원격 제어 및 자동화 테스트
- **로그 분리**: CLI 명령과 로그 출력이 서로 다른 TCP 포트 사용

## 아키텍처

```
┌─────────────────┐
│  Telnet 클라이언트│
│  (localhost)    │
└────────┬────────┘
         │
    ┌────┴─────────────────────┐
    │                          │
┌───▼────────┐        ┌────────▼────┐
│ CLI 포트   │        │  로그 포트   │
│  23000     │        │   24000     │
│  (명령)    │        │  (출력)     │
└───┬────────┘        └─────────────┘
    │
┌───▼──────────────────────────────┐
│  demo_app_cli.c                  │
│  - 명령어 파서                    │
│  - 명령 핸들러                    │
└───┬──────────────────────────────┘
    │
┌───▼──────────────────────────────┐
│  demo_app_core.c / demo_app_msg.c│
│  - DDS 엔티티 관리                │
│  - 메시지 송수신                  │
│  - BIT 제어                      │
└──────────────────────────────────┘
```

## 포트 구성

| 포트  | 용도              | 방향      | 프로토콜 |
|-------|-------------------|-----------|----------|
| 23000 | CLI 명령          | 양방향    | TCP      |
| 24000 | 로그 출력         | 출력 전용 | TCP      |
| 25000 | DDS Agent (UDP)   | 양방향    | UDP      |

## VxWorks 사용법

### 1. DKM 로드

```shell
-> ld < demo_app_dkm.out
```

### 2. 애플리케이션 시작

```shell
-> demoAppStart(23000, "127.0.0.1")
```

- **포트 23000**: CLI 서버 포트
- **IP "127.0.0.1"**: DDS Agent IP 주소

### 3. Host PC에서 접속

```bash
telnet <vxworks_target_ip> 23000
```

### 4. VxWorks Shell 명령 (선택사항)

VxWorks shell에서 직접 호출 가능한 명령들:

```c
demoAppStart(23000, "127.0.0.1")     // CLI 및 Log 서버와 함께 시작
demoAppConnect()                      // Agent에 연결
demoAppCreateEntities()               // DDS 엔티티 생성
demoAppStartScenario()                // 시나리오 시작
demoAppStopScenario()                 // 시나리오 중지
demoAppTestWrite("pbit")              // 테스트 전송 (pbit|cbit|result_bit|signal|all)
demoAppLogMode("console")             // 로그 모드 설정 (console|redirect|both)
demoAppStatus()                       // 상태 표시
demoAppStop()                         // 종료
```

## Windows 사용법

### 1. 애플리케이션 시작

```bash
demo_app.exe -cli_port 23000 -log_port 24000 -agent_host 127.0.0.1 -agent_port 25000
```

**명령줄 옵션:**

- `-cli_port <포트>`: TCP CLI 서버 포트 (기본값: 23000)
- `-log_port <포트>`: TCP 로그 서버 포트 (기본값: 24000)
- `-agent_host <IP>`: DDS Agent IP (기본값: 127.0.0.1)
- `-agent_port <포트>`: DDS Agent 포트 (기본값: 25000)
- `-log_mode <모드>`: 로그 출력 모드 - `console`, `redirect`, 또는 `both` (기본값: both)

### 2. CLI 접속

```bash
telnet localhost 23000
```

### 3. 로그 출력 접속 (선택사항)

```bash
telnet localhost 24000
```

## CLI 명령어

### 연결 명령

#### `connect [ip] [port]`

DDS Agent에 연결하고 hello + clear 메시지 전송

**파라미터:**

- `ip`: Agent IP 주소 (기본값: 127.0.0.1)
- `port`: Agent UDP 포트 (기본값: 25000)

**예제:**

```
> connect
Connecting to Agent at 127.0.0.1:25000...
Connected successfully
```

```
> connect 192.168.1.100 25001
Connecting to Agent at 192.168.1.100:25001...
Connected successfully
```

#### `status`

현재 애플리케이션 상태 표시

**출력:**

```
=== DemoApp Status ===
State: RUN
Tick: 12345
Transmit:
  Signal: 2468 (200Hz)
  CBIT: 12 (1Hz)
Receive:
  Control: 2468 (200Hz)
  Speed: 12 (1Hz)
BIT:
  PBIT: Completed
  CBIT: Active
  IBIT: Not Running
Components:
  roundMotor: OK
  upDownMotor: OK
  baseGyro: OK
  powerController: OK
======================
```

### 엔티티 관리

#### `create_entities`

모든 DDS 엔티티 생성 (Participant, Publisher, Subscriber, Writers, Readers)

**예제:**

```
> create_entities
Creating DDS entities...
Entities created successfully
State: POWERON_BIT
```

### 시나리오 제어

#### `start_scenario`

시나리오 실행 시작:

1. PBIT 발행 (1회)
2. 주기적 타이머 시작:
   - CBIT: 1 Hz
   - Actuator Signal: 200 Hz

**예제:**

```
> start_scenario
Starting scenario...
PBIT published
CBIT timer started (1 Hz)
Signal timer started (200 Hz)
State: RUN
```

#### `stop_scenario`

모든 주기적 타이머 및 시나리오 실행 중지

**예제:**

```
> stop_scenario
Stopping scenario...
Timers stopped
State: INIT
```

### 테스트 명령

#### `test_write <topic>`

전체 시나리오를 시작하지 않고 단일 테스트 메시지 전송

**파라미터:**

- `topic`: 메시지 타입
  - `pbit` - Power-On BIT
  - `cbit` - Continuous BIT
  - `result_bit` - BIT Result (refNum=999)
  - `signal` - Actuator Signal
  - `all` - 4개 메시지 모두 전송

**요구사항:**

- 먼저 `create_entities` 호출 필요
- `start_scenario` 불필요

**예제:**

```
> test_write pbit
[TX] PBIT published

> test_write all
[TX] PBIT published
[TX] CBIT published (count=1)
[TX] resultBIT published (refNum=999, result=PASS)
[TX] Actuator Signal published (count=1, 200Hz)
```

#### `run_ibit <ref_num> [type]`

수동으로 IBIT (Initiated BIT) 실행

**파라미터:**

- `ref_num`: 참조 번호 (1-65535)
- `type`: BIT 타입 (선택, 기본값: 2=I_BIT)
  - 0 = P_BIT
  - 1 = C_BIT
  - 2 = I_BIT

**예제:**

```
> run_ibit 1234
Starting IBIT: ref=1234, type=I_BIT
IBIT started
```

```
> run_ibit 5678 2
Starting IBIT: ref=5678, type=I_BIT
IBIT started
```

### 고장 주입

#### `fault_inject <component>`

BIT 실패 시나리오 테스트를 위해 컴포넌트에 고장 주입

**컴포넌트:**

- `round` - Round 모터
- `updown` - UpDown 모터
- `sensor` - 자이로 센서
- `power` - 전원 컨트롤러
- `motor` - 모든 모터

**예제:**

```
> fault_inject round
Fault injected: roundMotor
Component state: FAULT
```

#### `fault_clear <component|all>`

주입된 고장 제거

**파라미터:**

- `component`: 특정 컴포넌트 이름 (fault_inject와 동일)
- `all`: 모든 고장 제거

**예제:**

```
> fault_clear round
Fault cleared: roundMotor
Component state: OK

> fault_clear all
All faults cleared
```

### 로그 제어

#### `log_mode <mode>`

로그 출력 모드 동적 변경

**모드:**

- `console` - DemoApp 콘솔만 출력
- `redirect` - TCP 로그 포트(24000)만 출력
- `both` - 콘솔과 TCP 로그 포트 모두 출력

**예제:**

```
> log_mode redirect
Log mode changed to: redirect

> log_mode both
Log mode changed to: both
```

#### `log_status`

현재 로그 설정 표시

**출력:**

```
=== Log Status ===
Mode: BOTH
CLI Port: 23000
Log Port: 24000
Client Connected: Yes
==================
```

### 일반 명령

#### `help`

사용 가능한 모든 명령어와 간단한 설명 표시

#### `quit` / `exit`

CLI 서버 연결 해제 (DemoApp 종료 안 함)

## 로그 출력 형식

모든 로그는 메시지 방향을 구분하기 위한 prefix 사용:

- `[TX]` - 전송 메시지 (DDS로 발행)
- `[RX]` - 수신 메시지 (DDS 구독)
- `[INFO]` - 일반 정보

**로그 출력 예제:**

```
[INFO] DemoApp started
[INFO] DDS entities created
[TX] PBIT published
[TX] CBIT published (count=1)
[TX] Actuator Signal published (count=200, 200Hz)
[RX] Actuator Control: driving=0.50, updown=0.30, mode=1 (rx=100)
[RX] Vehicle Speed: A_speed=15.20 m/s (rx=12)
[RX] runBIT received: {...}
[RX] runBIT parsed: refNum=1234, type=2
[TX] resultBIT published (refNum=1234, result=PASS)
```

## 일반적인 사용 순서

### VxWorks

```bash
# 1. 타겟에서
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "192.168.1.100")

# 2. Host PC에서
telnet 192.168.1.50 23000

# 3. Telnet 세션에서
> connect 192.168.1.100 25000
> create_entities
> start_scenario
> status
> log_mode redirect
> quit
```

### Windows

```bash
# 터미널 1: DemoApp 시작
demo_app.exe -cli_port 23000 -log_port 24000

# 터미널 2: CLI 접속
telnet localhost 23000
> connect
> create_entities
> test_write all
> start_scenario
> status

# 터미널 3: 로그 모니터 (선택)
telnet localhost 24000
```

## 테스트 절차

### 시나리오 전 테스트

```
> connect
> create_entities
> test_write pbit        # PBIT 메시지 테스트
> test_write signal      # Signal 메시지 테스트
> test_write all         # 4개 메시지 모두 테스트
```

### 전체 시나리오 테스트

```
> connect
> create_entities
> start_scenario         # 자동으로 PBIT 발행 + 타이머 시작
> status                 # 메시지 카운트 확인
> run_ibit 1234         # 수동으로 IBIT 실행
> fault_inject round    # 고장 주입
> status                 # 고장 상태 확인
> fault_clear all       # 고장 제거
> stop_scenario
```

## 오류 처리

### 일반적인 오류

**"Not connected to Agent"**

- 먼저 `connect` 명령 실행

**"Must create entities first"**

- `start_scenario` 또는 `test_write` 전에 `create_entities` 실행

**"Already running"**

- 시나리오를 두 번 시작할 수 없음
- 먼저 `stop_scenario` 사용

**"Invalid parameter"**

- `help`로 명령 문법 확인

## 플랫폼 차이점

| 기능                 | VxWorks | Windows |
|----------------------|---------|---------|
| Shell 명령           | ✅      | ❌      |
| TCP CLI              | ✅      | ✅      |
| TCP Log 서버         | ✅      | ✅      |
| 콘솔 출력            | 제한적  | 전체    |
| Ctrl+C 처리          | 해당없음| ✅      |

## 구현 파일

- `src/demo_app_cli.c` - 명령 파싱 및 핸들러 (공통)
- `vxworks/demo_app_tcp_vx.c` - VxWorks TCP 서버
- `windows/demo_app_tcp_win.c` - Windows TCP 서버
- `vxworks/demo_app_dkm.c` - VxWorks shell 명령
- `windows/demo_app_main.c` - Windows 진입점
- `src/demo_app_log.c` - TX/RX prefix 로그 시스템

## 향후 개선 사항

- [ ] 인증/권한 관리
- [ ] 명령 히스토리 (readline 지원)
- [ ] Tab 자동완성
- [ ] 파일에서 배치 명령 실행
- [ ] JSON 형식 상태 출력
- [ ] 브라우저 기반 제어를 위한 WebSocket 지원
