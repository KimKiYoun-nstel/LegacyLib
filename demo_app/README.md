# DemoApp — README

이 문서는 현재 저장소 기준 `demo_app`의 빌드·실행 방법, 디렉토리 구조, 지원 CLI 및 동작 방식을 구현 상태 그대로 정리한 최신 안내서입니다.

간단 개요
- 목적: 포구동장치 시뮬레이터로서 DDS(또는 Legacy Agent)를 통해 PBIT/CBIT/IBIT/Signal 및 제어 명령을 송수신합니다.
- JSON 처리: `nlohmann::json` (`json.hpp`) 사용 — 페이로드는 RefDoc XML 스키마의 열거자 문자열을 따릅니다 (예: `"L_BITResultType_NORMAL"`).

디렉토리(주요)
- `include/` — 공용 헤더 (`demo_app.h`, `demo_app_enums.h`, `msg_fields.h` 등)
- `src/` — 핵심 C/C++ 소스 (`demo_app_core.c`, `demo_app_msg.c` / `.cpp`, `demo_app_timer.c`, ...)
- `vxworks/` — VxWorks 전용 진입점 및 CLI (`demo_app_dkm.c`, `demo_app_cli.c`)
- `windows/` — Windows 전용 진입점/추가 구현 (`demo_app_main.c`, `demo_app_tcp_win.c`)
- `docs/` — 인터페이스 및 사용 문서 (INTERFACE_SPEC.md, FEATURE_ANALYSIS.md 등)
- `build_win/` — Windows 빌드 산출물

요구 사항 (개발 환경)
- Windows: MinGW toolchain (g++, mingw32-make), WinSock 라이브러리
- VxWorks: WindRiver 빌드 환경(예: `wrenv.exe`, VxWorks SDK) — `BuildRef` 스크립트가 환경 설정을 지원

빌드 방법

Windows (권장, 단일 demo_app 빌드)
- 작업 디렉터리: `d:\CodeDev\LegacyLib\demo_app`
- 방법 A — 워크스페이스 루트 빌드 스크립트 사용 (권장 전체 빌드):
```powershell
cd d:\CodeDev\LegacyLib
call BuildRef\build_win_demo.cmd
```
- 방법 B — `demo_app` 폴더에서 Makefile.windows 사용 (개별 빌드):
```powershell
cd d:\CodeDev\LegacyLib\demo_app
mingw32-make -f Makefile.windows
```
- 산출물: `demo_app\build_win\demo_app.exe`

실행 (Windows)
```powershell
cd demo_app
build_win\demo_app.exe -p 23000 -h 127.0.0.1 -d 0
```
- 주요 옵션: `-p <port>`, `-h <agent_host>`, `-d <domain_id>`

VxWorks (DKM) 빌드 및 실행
- 워크스페이스 루트에서 제공되는 빌드 스크립트를 사용하면 VxWorks 환경 변수를 자동 설정합니다:
```powershell
cd d:\CodeDev\LegacyLib
call BuildRef\build_vx_demo.cmd
```
- 또는 `demo_app/vxworks/` 내 Makefile을 사용해 빌드 후 생성된 `.out` 파일을 타겟에 로드합니다.

로드 & 실행 (VxWorks Shell)
```
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")
```

제공되는 CLI 및 제어 (요약)

Windows 콘솔 모드 (로컬 실행)
- 실행 시: 콘솔에서 키 입력 인터페이스 또는 TCP CLI가 활성화됩니다.
- 주요 키/명령:
	- `h` 또는 `?` : 도움말
	- `s` : 상태 출력 (`DemoApp Status`) — 상태, tick, pub 횟수, 컴포넌트 상태
	- `i` : IBIT 시작 (runBIT 시뮬레이션)
	- `f` : Fault 주입(프롬프트로 대상 입력)
	- `c` : Fault 해제(모두 또는 지정)
	- `q` : 종료(또는 TCP 클라이언트의 `quit`으로 분리)

VxWorks CLI / API
- VxWorks에서는 다음 함수로 제어/검사합니다:
	- `demoAppStart(port, agent_addr, [domain])` — 앱 시작
	- `demoAppStop()` — 앱 종료
	- `demoAppStatus()` — 상태 출력
	- `demoAppInjectFault("<name>")` — 특정 결함 주입 (예: `"round"`, `"power"`)
	- `demoAppClearFault("all" | "<name>")` — 결함 해제

예상 메시지/토픽
- 송신: `P_NSTEL__C_CannonDrivingDevice_PowerOnBIT` (PBIT), `P_NSTEL__C_CannonDrivingDevice_PBIT` (CBIT), `P_NSTEL__C_CannonDrivingDevice_IBIT` (resultBIT), `P_NSTEL__C_CannonDrivingDevice_Signal` (Signal)
- 수신: `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT`, `P_NSTEL__C_CannonDrivingDevice_commandDriving`, `P_NSTEL__C_VehicleSpeed`

데이터 형식 주의사항
- 모든 JSON 직렬화/역직렬화는 `nlohmann::json`으로 구현되어 있습니다 (`demo_app/src/demo_app_msg.cpp` / `demo_app/src/demo_app_msg.c`).
- BIT 결과 필드는 boolean이 아니라 스키마 열거자 문자열을 사용합니다. 예: `"L_BITResultType_NORMAL"` / `"L_BITResultType_ABNORMAL"`.

디버깅 & 문제해결
- 빌드 오류(링커 관련)가 발생하면 `build_win/demo_app.exe`가 이미 실행 중인지 확인하고 종료 후 재빌드하세요.
- 포트 바인드 실패 시 동일 포트(기본 23000)를 점유한 프로세스를 확인하세요.
- JSON 예제가 스키마와 일치하지 않으면 `docs/INTERFACE_SPEC.md`를 참고해 필드명과 열거자 문자열을 확인하세요.

관련 문서
- 인터페이스·스키마: `demo_app/docs/INTERFACE_SPEC.md`
- 기능 분석: `demo_app/docs/FEATURE_ANALYSIS.md`
- CLI 명세: `demo_app/docs/CLI_SPEC.md`

문의사항
- README에서 더 상세히 다루길 원하는 항목(예: 디버그 로그 레벨, 자세한 QoS 매핑, 샘플 JSON 생성 스크립트)이 있으면 알려주세요.


