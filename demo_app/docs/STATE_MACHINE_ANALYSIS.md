## DemoApp 상태머신 분석

요약
- 상태: `Idle` -> `Init` -> `PowerOnBit` -> `Run` -> `IBitRunning`
- 주요 진입 경로:
  1. `demo_app_start()` 호출 → `Idle` -> `Init` (LegacyLib 초기화, Agent 연결)
  2. `demo_app_create_entities()` 호출 → `Init` 상태에서 DDS 엔티티 생성, 메시지 핸들러 초기화
  3. `demo_app_start_scenario()` 호출 → `Init` -> `PowerOnBit` (PBIT 발행) -> `Run` (타이머 시작)

동작 세부사항
- `demo_app_start()`
  - `ctx->current_state`가 `DEMO_STATE_IDLE`일 때만 동작.
  - LegacyLib 초기화 (`legacy_agent_init`) 및 Hello 전송.
  - DDS 엔티티는 자동 생성하지 않음(별도 `demo_app_create_entities()` 필요).

- `demo_app_create_entities()`
  - `current_state`가 `DEMO_STATE_INIT`일 때만 호출 허용.
  - Participant/Publisher/Subscriber 생성 후 `demo_msg_init()`로 Writer/Reader 생성.

- `demo_app_start_scenario()`
  - `current_state`가 `DEMO_STATE_INIT`일 때만 호출 가능.
  - PowerOn BIT(PBIT) 시뮬레이션 실행 후 `DEMO_STATE_RUN`으로 전환.
  - `demo_timer_init()`을 호출해 1ms 틱 타이머(또는 VxWorks 태스크) 시작.
  - 기본 주기: Signal 200Hz, CBIT 1Hz (현재는 CLI로 변경 가능).

- `demo_app_stop()` (pause) / `cmd_reset()`
  - `demo_timer_cleanup()`로 타이머 중단하고, 상태를 `DEMO_STATE_PEND`로 설정(엔티티/에이전트는 유지).
  - `demo_app_reset()`은 메시지/엔티티 정리 및 에이전트 닫기까지 수행하고 `DEMO_STATE_IDLE`로 복귀.
  - `disconnect` 명령은 제거됨(사용자에게는 `reset`을 사용하도록 권장).

- IBIT (인터럽트형 BIT)
  - `demo_app_trigger_ibit()`는 `DEMO_STATE_RUN`에서만 동작.
  - 호출 시 `ibit_running=true`, `ibit_start_time = tick_count`, 상태를 `DEMO_STATE_IBIT_RUNNING`으로 전환.
  - 타이머 틱에서 `DEMO_STATE_IBIT_RUNNING`이면 3초(3000ms) 경과 후 `demo_msg_publish_result_bit()`를 호출하고 `DEMO_STATE_RUN`으로 복귀.

발견된 문제와 동작 불분명한 점
1) "stop 후 start가 안 되는" 현상
- 원인(상식적 설명): `demo_app_stop()`은 DDS 엔티티와 LegacyLib 연결을 정리(cleanup)하고 `current_state`를 `Idle`로 만든다.
- 재시작하려면 순서가 필요함: `demo_app_start()` (Agent 연결) → `demo_app_create_entities()` (엔티티 생성) → `demo_app_start_scenario()` (시나리오 시작).
- 사용자가 `stop_scenario` 후 곧바로 `start_scenario`만 호출하면 실패한다(요구 상태가 `Init`이므로). 즉 "stop → start"가 즉시 동작하려면 `start()`와 `create_entities()`가 자동으로 호출되도록 로직 변경이 필요.

2) `reset` 동작 정의
- 현재 구현은 `reset`과 `disconnect`가 혼재되어 있었으나, `disconnect`는 제거되었습니다.
- `reset`은 내부 상태(시뮬레이션 값, 카운터) 초기화, 메시지/엔티티 정리 및 에이전트 닫기를 수행하고 `DEMO_STATE_IDLE`로 복귀합니다.

3) 재연성과 동시성 염려
- `demo_app_stop()`이 타이머/메시지 정리 중에 외부 이벤트(예: IBIT 완료 콜백, 수신 메시지)가 동시에 발생하면 경쟁 상태가 생길 수 있음.
- 권장: 정리 순서에 락(또는 atomic 플래그) 적용, `stop` 시작 시 `accept_msgs=false` 같은 플래그로 수신 핸들러에서 신속히 무시하도록 변경.

4) 상태 전이 검증 부족
- `demo_app_transition_to()`는 현재 검증 없이 상태 전환을 허용함(임의 상태 이동 가능).
- 권장: 허용 가능한 전이 테이블을 도입하여 잘못된 전이(예: `Idle` -> `Run` 직접 이동)를 방지.

권장 개선안 (우선순위 순)
1. `stop` 후 `start_scenario`로 자동 재시작 옵션
   - `demo_app_stop()`이 End-to-End 정리 후에도 필요한 경우 `demo_app_start()` + `create_entities()`를 자동으로 호출하도록 선택적 플래그 추가.
2. `reset` vs `disconnect` 기능 분리
   - `reset`: 내부 상태/시뮬레이션(센서값, 카운터)만 초기화
   - `disconnect`: 네트워크/엔티티 정리 후 `Idle`
3. 안전한 정리(atomic stop)
   - `stopping` 플래그 도입: 수신 핸들러/타이머는 `stopping==true`이면 조용히 무시
4. 상태 전이 검증 테이블 추가
   - 명시적 전이 테이블로 `enter_state()` 전에 validation 수행

결론
- 현재 설계는 "명시적 단계(연결→엔티티 생성→시나리오 시작)"를 요구하므로 사용자가 `stop` 후 단순히 `start_scenario`만 호출하는 패턴에서는 재시작이 실패합니다.
- 위 권장 개선을 적용하면 사용자 기대(단순 stop→start) 동작을 지원하면서도 안전성(정리/동기)을 유지할 수 있습니다.

---
파일 참조
- 주요 함수: `demo_app_start()`, `demo_app_create_entities()`, `demo_app_start_scenario()`, `demo_app_stop()`, `demo_app_trigger_ibit()`
- 타이머/퍼블리시: `demo_timer_init()`, `demo_timer_cleanup()`, `demo_timer_tick()`

원하시면 이 문서의 권장안(예: `stop` 후 자동복구, `reset`/`disconnect` 분리)을 코드로 구현해 드리겠습니다.
