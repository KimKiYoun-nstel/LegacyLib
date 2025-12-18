# Legacy Agent DKM Demo (demo_dkm.c)

간단한 개요
- 이 예제(`demo_dkm.c`)는 Legacy Agent용 DKM(Domain Kernel Module) 환경에서 동작하는 TCP 기반 CLI 서버입니다.
- 목적: 로컬/원격에서 DDS/Legacy Agent 엔티티 생성·쓰기·구독을 손쉽게 테스트하기 위한 도구입니다.

주요 기능 (요약)
- 디스크의 JSON 샘플을 로드해 캐시 보관 후 필요 시 전송
- 타입 문자열 ↔ 파일명 매핑: `P_NSTEL::C_Type` → `P_NSTEL__C_Type.json`
- TCP로 접속 가능한 간단한 CLI 제공(기본 포트 23000)
- Agent 초기화/participant/writer/reader 생성, JSON 쓰기, 캐시 관리 등 명령 제공

동작 방식 (분석)
1. 초기화
   - `legacyAgentTcpStart(port, json_dir)` 호출로 시작
   - 내부적으로 `load_json_directory()`를 호출해 `examples/output`(기본)에서 `.json` 파일을 로딩하여 캐시화
   - JSON 캐시는 `g_json_cache[]` 배열에 동적 할당된 문자열로 보관
2. TCP 서버
   - 소켓 바인드 후 `accept()` 루프에서 클라이언트 연결을 대기
   - 클라이언트가 연결되면 간단한 프롬프트를 제공하고 명령을 수신
   - 입력은 라인 단위로 파싱(백스페이스/CR/LF 처리 포함)
3. CLI 명령 처리
   - `connect <ip> <port>`: `legacy_agent_init()`를 호출해 에이전트와 연결
   - `hello`: 비동기 헬로우 호출(`legacy_agent_hello`)
   - `create_p`: Participant 생성
   - `create_w <topic> <type>`: Writer 생성
   - `create_r <topic> <type>`: Reader 생성 + 이벤트 구독
   - `write <topic> <type> [file]`: 캐시 또는 지정 파일의 JSON을 이용해 `legacy_agent_write_json()` 호출
   - `cache_info`, `cache_reload`, `clear`, `quit` 등 캐시·엔티티 관리 명령
4. 동기화/안전
   - CLI 출력(`cli_print`)과 JSON 캐시 접근은 세마포어(`g_cli_mutex`, `g_cache_mutex`)로 보호
5. 정리
   - `legacyAgentTcpStop()` 호출 시 소켓과 태스크를 종료, 에이전트 연결 해제, 캐시 메모리 해제

운영 및 사용법

VxWorks 사용 예
1) 모듈 로드
-> ld < demo_tcp_cli_dkm.out

2) 서버 시작 (포트와 JSON 디렉토리 선택 가능)
-> legacyAgentTcpStart(23000)
# 또는
-> legacyAgentTcpStart(23000, "examples/output")

3) 원격에서 접속
- telnet <target_ip> 23000

주요 CLI 명령 (요약)
- `connect <ip> <port>`
  - 에이전트 초기화 (기본: 127.0.0.1 25000)
- `hello`
  - 비동기 Hello 요청 전송
- `create_p [domain] [qos]`
  - Participant 생성 (기본 domain=0)
- `create_w <topic> <type> [entity_name] [qos]`
  - Writer 생성 (예: `create_w Cannon_Signal P_NSTEL::C_Cannon_Actuator_Signal`)
- `create_r <topic> <type> [entity_name] [qos]`
  - Reader 생성 및 이벤트 수신 구독
- `write <topic> <type> [file_path]`
  - 캐시된 JSON 또는 지정 파일을 사용해 메시지 전송
- `cache_info`
  - 로드된 JSON 목록 출력
- `cache_reload`
  - 캐시 초기화 후 디렉토리 재로딩
- `clear`
  - 생성된 DDS 엔티티 모두 삭제
- `quit`
  - 클라이언트 연결 종료

JSON 파일 규칙
- 파일명: `P_NSTEL__C_<TypeName>.json` (스키마 명칭에서 `::`를 `__`로 변환)
- 내부에는 스키마에 맞는 JSON 페이로드를 넣어 두면 `write` 명령으로 바로 전송 가능

실행/디버깅 팁
- 서버가 포트를 바인드하지 못하면 이미 같은 포트에서 다른 프로세스가 실행 중인지 확인하세요.
- 캐시 메모리는 동적 할당되므로 많은 대형 JSON 파일을 로드하면 메모리 사용이 늘어납니다.
- VxWorks 환경에서는 `taskSpawn`으로 서버 태스크를 실행하므로 태스크가 정상적으로 종료되는지 `taskDelete`로 확인하세요.

제한 및 알려진 이슈
- 현재 포트(23000)과 기본 JSON 디렉토리가 하드코딩되어 있어 시작 시 다른 값으로 변경하려면 인자 전달 필요
- JSON 최대 크기 `MAX_JSON_SIZE`(8192 바이트) 제한 있음
- Telnet 클라이언트가 Telnet IAC 바이트(0xFF)를 보낼 때 필터링 처리하지만 모든 Telnet 옵션은 지원하지 않음

파일 위치
- 서버 소스: `examples/demo_dkm.c`
- 샘플 JSON: `examples/output/*.json`

문의
- 추가 기능(예: 인증, 더 많은 동시 클라이언트 지원, JSON 스트리밍 등)이 필요하면 알려주세요.
