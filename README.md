# LegacyLib + DemoApp Deployment Guide

이 문서는 DemoApp과 LegacyLib를 제3자에게 전달하여 빌드/실행까지 가능한 상태로 만드는 배포용 README입니다. LegacyLib를 중심으로 구조와 사용법을 설명하고, DemoApp은 참고용 실행/검증 앱으로 정리했습니다.

## 프로젝트 구성 요약

- LegacyLib: DDS Gateway Agent와 통신하는 C API 라이브러리 (JSON/Struct 모두 지원)
- DemoApp: VxWorks DKM/Windows 콘솔 기반 시뮬레이터 (LegacyLib 사용 예제)
- 문서/스키마: IPC 프로토콜, Wire Struct, XML 스키마, QoS 등 참고 자료 포함

## 빌드 환경 및 사전 구성

- 공통
  - Python 3 (API 코드 생성: `legacy_lib/tools/gen_api.py`)
    - 사전 필요성: 빌드 전에 `legacy_lib/generated/legacy_api.{c,h}`를 자동 생성합니다.
- Windows
  - MinGW (gcc, g++, mingw32-make)
    - 사전 필요성: DemoApp Windows 빌드 및 LegacyLib 정적 라이브러리 빌드에 사용합니다.
- VxWorks
  - Wind River VxWorks 23.03 + LLVM toolchain
  - VxWorks VSB (VxWorks Source Build) 필요
  - `BuildRef/vx_env.bat`로 환경 변수 설정 필요
    - 사전 필요성: `wr-cc`, `wr-c++` 컴파일러와 VSB 경로, SDK/툴체인 경로를 환경에 주입합니다.
    - 사용되는 변수 예: `WIND_BASE`, `WIND_CC_SYSROOT`, `VSB_DIR`, `LLVM_ROOT`, `NDDSHOME_CTL`
    - 자세한 절차: `BuildRef/BUILD_GUIDE.md` 참고

## 빌드 방법 (권장 스크립트)

### Windows

```powershell
# LegacyLib 생성 (API 생성 + 정적 라이브러리)
call BuildRef\build_win_lib.cmd

# DemoApp (Windows 콘솔)
call BuildRef\build_win_demo.cmd
```

### VxWorks

```powershell
# VxWorks 환경 설정
call BuildRef\vx_env.bat

# LegacyLib DKM
call BuildRef\build_vx_lib.cmd

# DemoApp DKM
call BuildRef\build_vx_demo.cmd
```

### 직접 빌드 (Makefile)

```powershell
# LegacyLib (VxWorks)
make -C legacy_lib MODE=vxworks

# DemoApp (VxWorks)
make -C demo_app

# LegacyLib (Linux/MinGW 정적 라이브러리)
mingw32-make -C legacy_lib MODE=native

# DemoApp (Windows)
mingw32-make -C demo_app -f Makefile.windows
```

## 최종 산출물

- LegacyLib (VxWorks)
  - `legacy_lib/liblegacy_agent_dkm.out`
  - `legacy_lib/demo_tcp_cli_dkm.out`
- DemoApp (VxWorks)
  - `demo_app/demo_app_dkm.out`
- LegacyLib (Linux/MinGW 정적 라이브러리)
  - `legacy_lib/liblegacy_agent.a`
  - `legacy_lib/example_app`
- DemoApp (Windows)
  - `demo_app/build_win/demo_app.exe`

## LegacyLib (핵심)

### 목적

- DDS Gateway Agent와 IPC(UDP/RTP)로 통신하는 C API 제공
- Control Plane(엔티티 생성, QoS 설정)과 Data Plane(데이터 송수신) 지원
- JSON/Struct 모드 모두 지원, Hello 응답의 `data_codec`로 동작 모드 결정

### 설계 구조

- 공개 API: `legacy_lib/include/legacy_agent.h`
- C API 래퍼: `legacy_lib/src/legacy_agent.cpp`
- IPC/프로토콜: `legacy_lib/src/internal/IpcJsonClient.*`
- 전송 계층: `legacy_lib/src/internal/DkmRtpIpc.*`
- 자동 생성 API: `legacy_lib/generated/legacy_api.{c,h}`

### Agent 연동 스펙

- IPC 프로토콜: `DevGuide/IPC_PROTOCOL_SPECIFICATION_v3.0.md`
- Wire Struct 사용법: `DevGuide/WIRE_STRUCT_USAGE_GUIDE.md`
- API 상세 스펙: `RefDoc/LIB_API_SPEC.md`

핵심 포인트

- `legacy_agent_hello()` 응답의 `data_codec`과 `abi_hash`를 확인
- JSON/Struct 모드가 Agent와 일치해야 EVT 수신이 정상 동작
- Struct 모드에서는 `abi_hash`가 동일한 IDL 버전을 사용해야 함

### 기본 사용 흐름 (DDS 엔티티 생성 + 송수신)

```c
#include "legacy_agent.h"

static void on_simple(LEGACY_HANDLE h, LegacyRequestId reqId,
                      const LegacySimpleResult* res, void* user) {
    (void)h; (void)reqId; (void)user;
    if (!res || !res->ok) {
        // 실패 처리
    }
}

static void on_event(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    (void)h; (void)user;
    // evt->data_json 사용 (콜백 반환 전 복사 권장)
}

void example_basic_flow(void) {
    LEGACY_HANDLE h = NULL;

    LegacyConfig cfg = {0};
    cfg.agent_ip = "127.0.0.1";
    cfg.agent_port = 25000;
    cfg.data_codec = LEGACY_CODEC_JSON; // Hello 결과와 일치시킬 것

    if (legacy_agent_init(&cfg, &h) != LEGACY_OK) {
        return;
    }

    legacy_agent_hello(h, 3000, NULL, NULL);

    LegacyParticipantConfig pcfg = { .domain = 0, .qos = "default" };
    legacy_agent_create_participant(h, &pcfg, 3000, on_simple, NULL);

    LegacyPublisherConfig pub = { .domain = 0, .name = "pub1", .qos = "default" };
    legacy_agent_create_publisher(h, &pub, 3000, on_simple, NULL);

    LegacyWriterConfig wcfg = {
        .domain = 0,
        .publisher = "pub1",
        .topic = "P_NSTEL__C_CannonDrivingDevice_Signal",
        .type = "P_NSTEL::C_CannonDrivingDevice_Signal",
        .qos = "HighFreqPeriodicProfile"
    };
    legacy_agent_create_writer(h, &wcfg, 3000, on_simple, NULL);

    LegacyWriteJsonOptions wopt = {0};
    wopt.topic = "P_NSTEL__C_CannonDrivingDevice_Signal";
    wopt.type = "P_NSTEL::C_CannonDrivingDevice_Signal";
    wopt.data_json = "{\"A_value\": 1}";
    legacy_agent_write_json(h, &wopt, 1000, on_simple, NULL);

    legacy_agent_subscribe_event(h,
        "P_NSTEL__C_VehicleSpeed",
        "P_NSTEL::C_VehicleSpeed",
        on_event, NULL);

    legacy_agent_close(h);
}
```

Struct 모드 사용 시

- `LegacyTypeAdapter` 등록 후 `legacy_agent_write_struct()` 또는 `legacy_agent_subscribe_typed()` 사용
- `struct_size`를 전달하거나 `LegacyTypeAdapter.struct_size`를 활용 가능

```c
#include "legacy_agent.h"

typedef struct {
    int32_t value;
} MyStruct;

static const char* encode_struct(const void* user_struct, void* user_ctx) {
    (void)user_ctx;
    const MyStruct* s = (const MyStruct*)user_struct;
    static char buf[64];
    snprintf(buf, sizeof(buf), "{\"value\":%d}", s->value);
    return buf;
}

static bool decode_struct(const char* data_json, void* out_user_struct, void* user_ctx) {
    (void)user_ctx;
    MyStruct* out = (MyStruct*)out_user_struct;
    if (!data_json || !out) return false;
    // 간단 파싱 예시 (실사용 시 JSON 파서 권장)
    int v = 0;
    if (sscanf(data_json, "{\"value\":%d}", &v) != 1) return false;
    out->value = v;
    return true;
}

static void on_typed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user_struct, void* user) {
    (void)h; (void)evt; (void)user;
    const MyStruct* s = (const MyStruct*)user_struct;
    // 수신된 struct 사용
    (void)s;
}

void example_struct_flow(LEGACY_HANDLE h) {
    LegacyTypeAdapter adapter = {0};
    adapter.key.topic = "MyTopic";
    adapter.key.type_name = "MyType";
    adapter.encode = encode_struct;
    adapter.decode = decode_struct;
    adapter.struct_size = sizeof(MyStruct);
    adapter.user_ctx = NULL;

    legacy_agent_register_type_adapter(h, &adapter);

    MyStruct data = { .value = 42 };
    legacy_agent_write_struct(h, "MyTopic", "MyType", &data, sizeof(data), 1000, NULL, NULL);

    legacy_agent_subscribe_typed(h, "MyTopic", "MyType", on_typed, NULL);
}
```

### DemoApp에서 LegacyLib 사용 위치

- Agent 초기화/엔티티 생성: `demo_app/src/demo_app_core.c`
- JSON 송수신 및 파싱: `demo_app/src/demo_app_msg.cpp`
- CLI 명령 -> LegacyLib 호출 연결: `demo_app/src/demo_app_cli.c`

### 로그 및 설정

- `LegacyConfig`로 Agent IP/포트, data codec, 스레드 우선순위, 로그 콜백 설정
- 전역 로그 콜백: `legacy_agent_set_log_callback()`
- 로그 레벨: 0=TRACE, 1=DEBUG, 2=INFO, 3=WARN, 4=ERR
- 성능 계측: `DEMO_PERF_INSTRUMENTATION` 정의 시 `legacy_agent_get_perf_stats()` 사용 가능

### LegacyLib 폴더 트리 (요약)

```
legacy_lib/
├── include/               # 공개 API 헤더
├── generated/             # gen_api.py 결과물
├── src/
│   ├── legacy_agent.cpp   # C API 래퍼
│   └── internal/          # IPC/전송 구현
├── tools/
│   ├── gen_api.py         # API 자동 생성 스크립트
│   ├── struct/            # IDL 기반 Wire Struct
│   └── xml/               # DDS 스키마 XML
└── Makefile
```

## DemoApp (참고/검증용)

### 목적

- 포구동장치 시뮬레이터
- LegacyLib로 DDS Agent와 연결하여 7개 메시지 송수신

### 구조적 설계

- 상태 머신: Idle → Init → PowerOnBit → Run → IBitRunning → Pend
- 핵심 모듈
  - `demo_app_core.c`: 상태 머신/시나리오 제어
  - `demo_app_timer.c`: 1Hz/200Hz 주기 송신 타이머
  - `demo_app_msg.cpp`: JSON 생성/파싱
  - `demo_app_publisher.c`: DDS 전송 관리
  - `demo_app_cli.c`: CLI 명령 처리

### 제공 기능

- 송신 토픽: PBIT, CBIT, resultBIT, Signal
- 수신 토픽: runBIT, commandDriving, VehicleSpeed
- Fault Injection 및 IBIT 시나리오
- TCP CLI 및 TCP 로그 포트 제공

### VxWorks (DKM)

- 모듈 로드
  - `ld < demo_app_dkm.out`
- 시작/정지
  - `demoAppStart(23000, "127.0.0.1")`
  - `demoAppStop()`
- CLI 접속
  - `telnet <target_ip> 23000`
- 주요 Shell 명령
  - `demoAppConnect()`, `demoAppCreateEntities()`, `demoAppStartScenario()`
  - `demoAppStatus()`, `demoAppInjectFault("round")`, `demoAppClearFault("all")`
  - `demoAppLogMode("console|redirect|both")`
- 참고: Agent 포트는 CLI의 `connect <ip> <port>`로 지정(기본 25000)하는 구성이 가장 안전합니다. VxWorks Shell의 `demoAppConnect()`는 코드 기본값(현재 23000)을 사용하므로, Agent 포트가 다른 경우 CLI 명령으로 연결하세요.

### Windows

- 실행

```powershell
cd demo_app
build_win\demo_app.exe -cli_port 23000 -log_port 24000 -agent_host 127.0.0.1 -agent_port 25000
```

- CLI/로그 접속

```powershell
# CLI
telnet localhost 23000
# 로그
telnet localhost 24000
```

### CLI 사용 요약

- `connect [ip] [port]`
- `create_entities`
- `start_scenario` / `stop_scenario`
- `status`
- `run_ibit <ref>`
- `codec [json|struct]`
- `fault_inject <component>` / `fault_clear <component|all>`
- `set_hz <topic> <hz>` / `reset_hz`
- `test_write <topic>`
- `log_mode <console|redirect|both>`
- `log_level <error|info|debug>`
- `log on|off`
- `reset`
- `help`
- `quit`

### 로그 및 설정

- 기본 포트
  - CLI: 23000
  - 로그: 24000
  - Agent: 25000
- 로그 모드: `console`, `redirect`, `both`
- 로그 레벨/출력 제어: `log_level`, `log on|off`

### DemoApp 폴더 트리 (요약)

```
demo_app/
├── include/        # DemoApp API/공용 헤더
├── src/            # 핵심 로직 (core/timer/msg/cli)
├── vxworks/        # DKM 진입점 및 TCP 서버
├── windows/        # Windows 콘솔 진입점
├── docs/           # 기능/CLI/인터페이스 문서
├── tools/          # 테스트 스크립트
└── Makefile*       # VxWorks/Windows 빌드
```

## 참고 문서

- LegacyLib API 스펙: `RefDoc/LIB_API_SPEC.md`
- IPC 프로토콜: `DevGuide/IPC_PROTOCOL_SPECIFICATION_v3.0.md`
- Wire Struct 사용법: `DevGuide/WIRE_STRUCT_USAGE_GUIDE.md`
- DemoApp 기능 분석: `demo_app/docs/FEATURE_ANALYSIS.md`
- DemoApp CLI 명세: `demo_app/docs/CLI_SPEC.md`
- DemoApp 인터페이스: `demo_app/docs/INTERFACE_SPEC.md`
- Agent 메시지 샘플: `demo_app/docs/AGENT_JSON_MESSAGES.md`
```
