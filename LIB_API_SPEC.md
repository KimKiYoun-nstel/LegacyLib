# LIB API SPEC

## 개요

이 문서는 `LegacyLib`(헤더: include/legacy_agent.h)가 DemoApp에 제공하는 공개 C API의 상세 사용 설명서입니다. 목적은 제3자 애플리케이션이 라이브러리를 정확히 초기화·구성하고 제어/데이터 API를 안전하게 호출하여 Agent와 통신하도록 안내하는 것입니다.

범위: 초기화/종료, 제어(hello, 엔티티 생성/삭제), QoS, 데이터 쓰기/구독, 타입 어댑터(부록), 에러 및 동기화/응답 매칭 규칙, 예제 코드(일부는 VxWorks DKM 대상).

대상 독자: DemoApp 개발자, 시스템 통합자

---

## 구조 요약

- 핵심 핸들: `LEGACY_HANDLE` (불투명 포인터)
- 반환형: `LegacyStatus` (열거형: 성공/각종 에러)
- 요청-응답: 대부분의 제어·쓰기 API는 비동기 요청/응답 패턴을 사용. 요청 시 `LegacyRequestId`가 발급될 수 있고, 결과는 등록된 콜백으로 전달됩니다.

비동기 흐름 모델(요약):
- 요청 API 호출 → 라이브러리 내부 전송 → Agent 응답 수신 → 등록된 콜백 호출(요청 ID 및 사용자 컨텍스트 전달)

---

## 빠른 시퀀스 (권장 사용 흐름)

1. `legacy_agent_init()` — 라이브러리 초기화
2. (선택) `legacy_agent_set_log_callback()` — 전역 로그 콜백 등록
3. `legacy_agent_hello()` — Agent 핸드셰이크(상태 확인)
4. `legacy_agent_create_participant()` — Participant 생성
5. `legacy_agent_create_publisher()` / `legacy_agent_create_subscriber()` — Pub/Sub 생성
6. `legacy_agent_create_writer()` / `legacy_agent_create_reader()` — Writer/Reader 생성
7. `legacy_agent_write_json()` 또는 `legacy_agent_write_struct()`로 데이터 송신
8. `legacy_agent_subscribe_event()` 또는 `legacy_agent_subscribe_typed()`로 이벤트 수신 등록
9. 모든 작업 완료 시 `legacy_agent_close()` — 블로킹 종료

---

## 타입 및 구조체 — 전체 상세

아래는 `include/legacy_agent.h`에 정의된 공개 구조체/타입들의 필드와 의미입니다.

1) LEGACY_HANDLE
- typedef: `typedef struct LegacyAgentHandleImpl* LEGACY_HANDLE;`
- 설명: 라이브러리 내부 상태를 가리키는 불투명 핸들. 모든 API 호출에 전달.

2) LegacyStatus
- 열거형 값:
  - `LEGACY_OK = 0`
  - `LEGACY_ERR_PARAM` - 잘못된 인자
  - `LEGACY_ERR_TRANSPORT` - 전송/연결 오류
  - `LEGACY_ERR_TIMEOUT` - 타임아웃
  - `LEGACY_ERR_PROTO` - 프로토콜/파싱 오류
  - `LEGACY_ERR_CLOSED` - 핸들이 이미 종료됨

3) LegacyLogCb
- 시그니처: `typedef void (*LegacyLogCb)(int level, const char* msg, void* user);`
- 의미: 라이브러리 로그 콜백. `level`: 0=TRACE,1=DEBUG,2=INFO,3=WARN,4=ERR. `user`는 `LegacyConfig.log_user` 또는 `legacy_agent_set_log_callback()`에서 설정한 포인터.

4) LegacyConfig
- 정의(주요 필드):
  - `const char* agent_ip` : Agent 접속 주소(예: "127.0.0.1")
  - `uint16_t agent_port` : Agent 포트
  - `uint32_t recv_task_priority` : 수신 스레드 우선순위(VxWorks 전용값 적용 가능)
  - `uint32_t recv_task_stack` : 수신 스레드 스택 크기
  - `uint32_t send_task_priority` : 송신 스레드 우선순위
  - `uint32_t send_task_stack` : 송신 스레드 스택 크기
  - `LegacyLogCb log_cb` : 초기화 시 등록할 로그 콜백(옵션)
  - `void* log_user` : 로그 콜백에 전달할 사용자 포인터

5) LegacyPerfStats
- 필드: ipc_parse_ns_total, ipc_parse_count, ipc_cbor_ns_total, ipc_cbor_count, transport_send_us_total, transport_send_count, write_ns_total, write_count
- 설명: 성능 계측 카운터(빌드 시 DEMO_PERF_INSTRUMENTATION 활성화 필요)

6) LegacyRequestId
- typedef: `typedef uint32_t LegacyRequestId;` — 요청 식별자

7) LegacySimpleResult
- 필드:
  - `bool ok` : 응답 'ok' 필드 (true면 요청 처리 성공)
  - `int err` : 에러 코드(0이면 없음)
  - `const char* msg` : 에러/상태 메시지(없으면 NULL)
  - `const char* raw_json` : 원문 JSON 응답(디버그용, NULL 가능)

8) LegacySimpleCb
- 시그니처: `typedef void (*LegacySimpleCb)(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, void* user);`

9) Hello 관련
- `LegacyHelloInfo` : `{ int proto; const char* caps_raw_json; }`
- `LegacyHelloCb` : `typedef void (*LegacyHelloCb)(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, const LegacyHelloInfo* info, void* user);`

10) Participant/Publisher/Subscriber/Writer/Reader Configs
- `LegacyParticipantConfig` : `{ int domain; const char* qos; }`
- `LegacyPublisherConfig` : `{ int domain; const char* name; const char* qos; }`
- `LegacySubscriberConfig` : `{ int domain; const char* name; const char* qos; }`
- `LegacyWriterConfig` : `{ int domain; const char* publisher; const char* topic; const char* type; const char* qos; }`
- `LegacyReaderConfig` : `{ int domain; const char* subscriber; const char* topic; const char* type; const char* qos; }`

11) QoS 타입
- `LegacyQosProfile` : `{ const char* library; const char* profile; const char* xml; }`
- `LegacyQosList` : `{ const LegacyQosProfile* profiles; size_t count; const char* raw_json; }`
- `LegacyQosSetOptions` : `{ const char* library; const char* profile; const char* xml; }`
- `LegacyQosListCb` : `typedef void (*LegacyQosListCb)(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, const LegacyQosList* list, void* user);`
- `LegacyQosSetCb` : `typedef void (*LegacyQosSetCb)(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, void* user);`

12) Write 옵션
- `LegacyWriteJsonOptions` :
  - `const char* topic` (필수)
  - `const char* type` (선택)
  - `const char* data_json` (필수, NUL-종료 JSON)
  - `int domain` (선택)
  - `const char* publisher` (선택)
  - `const char* qos` (선택)
- `LegacyWriteCb` : `typedef void (*LegacyWriteCb)(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, void* user);`

13) Event/구독 타입
- `LegacyEvent` : `{ const char* topic; const char* type; const char* data_json; const char* raw_json; }`
- `LegacyEventCb` : `typedef void (*LegacyEventCb)(LEGACY_HANDLE h, const LegacyEvent* evt, void* user);`
- `LegacyTypedEventCb` : `typedef void (*LegacyTypedEventCb)(LEGACY_HANDLE h, const LegacyEvent* evt, void* user_struct, void* user);`

14) Type Adapter
- `LegacyTypeKey` : `{ const char* topic; const char* type_name; }`
- `LegacyTypeAdapter` : 구조체 필드
  - `LegacyTypeKey key;`
  - `const char* (*encode)(const void* user_struct, void* user_ctx);`
  - `bool (*decode)(const char* data_json, void* out_user_struct, void* user_ctx);`
  - `const char* (*make_default)(void* user_ctx);`
  - `void* user_ctx;`
- 등록 함수: `legacy_agent_register_type_adapter(LEGACY_HANDLE h, const LegacyTypeAdapter* adapter);`
- 해제: `legacy_agent_unregister_type_adapter(LEGACY_HANDLE h, const char* topic, const char* type_name);`

---

## API별 상세 설명 및 예제

아래는 각 공개 API별로 시그니처, 파라미터 표, 동작 설명, VxWorks DKM 예제(가능한 경우)를 함께 제공합니다.

주의: 모든 예제는 설명 목적이며, 실제 빌드 환경에 맞춰 include, 링커 옵션, 스택/우선순위 값을 조정해야 합니다.

### legacy_agent_init
- 시그니처:
  - `LegacyStatus legacy_agent_init(const LegacyConfig* cfg, LEGACY_HANDLE* outHandle);`
- 파라미터:
  - `cfg` (in): 포인터, 필수. `LegacyConfig` 구조체로 Agent 접속 정보 및 스레드 속성, 로그 콜백을 포함.
  - `outHandle` (out): `LEGACY_HANDLE*` — 초기화 성공 시 핸들이 설정됨.
- 반환값: `LEGACY_OK` 또는 에러 코드(`LEGACY_ERR_PARAM`, `LEGACY_ERR_TRANSPORT`, 등).
- 동작: 내부 리소스(버퍼, 스레드) 할당 및 초기 연결 준비. 실패 시 리소스는 정리.

VxWorks DKM 예제:
```c
#include <semLib.h>
#include <stdio.h>

LegacyConfig cfg = {0};
cfg.agent_ip = "192.168.0.10";
cfg.agent_port = 15000;
cfg.recv_task_priority = 100; // 예시 값
cfg.recv_task_stack = 0x4000;
cfg.send_task_priority = 90;
cfg.send_task_stack = 0x3000;

LEGACY_HANDLE h = NULL;
if (legacy_agent_init(&cfg, &h) != LEGACY_OK) {
    printf("legacy_agent_init failed\n");
    return;
}
```

### legacy_agent_close
- 시그니처: `void legacy_agent_close(LEGACY_HANDLE h);`
- 파라미터: `h` (in) — 유효한 핸들
- 동작: 블로킹 종료 — 내부 스레드/리소스 정리. 호출 후 `h` 사용 금지.

예제:
```c
legacy_agent_close(h);
h = NULL;
```

### legacy_agent_set_log_callback / legacy_agent_get_log_callback
- 시그니처:
  - `void legacy_agent_set_log_callback(LegacyLogCb cb, void* user);`
  - `void legacy_agent_get_log_callback(LegacyLogCb* out_cb, void** out_user);`
- 동작: 전역 로그 콜백 등록/조회. `user`는 로그 콜백이 호출될 때 그대로 전달됨.

예제:
```c
void my_log(int level, const char* msg, void* user) { printf("LOG:%d %s\n", level, msg); }
legacy_agent_set_log_callback(my_log, NULL);
```

### legacy_agent_hello
- 시그니처:
  - `LegacyStatus legacy_agent_hello(LEGACY_HANDLE h, uint32_t timeout_ms, LegacyHelloCb cb, void* user);`
- 파라미터:
  - `h` (in)
  - `timeout_ms` (in): 타임아웃 밀리초(0이면 내부 기본)
  - `cb` (in): 응답 콜백
  - `user` (in): 콜백에 그대로 전달되는 포인터(요청-응답 매칭 혹은 컨텍스트)
- 동작: Agent에 Hello 요청 전송. 응답은 `LegacyHelloCb`로 비동기 전달.

예제(간단 비동기):
```c
void hello_cb(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, const LegacyHelloInfo* info, void* user) {
    if (!res || !res->ok) { printf("Hello failed\n"); return; }
    printf("Agent proto=%d\n", info->proto);
}
legacy_agent_hello(h, 5000, hello_cb, NULL);
```

### 엔티티 생성 API (모두 동일 패턴: 비동기)

공통: 각 함수는 `uint32_t timeout_ms`, `LegacySimpleCb cb`, `void* user` 파라미터를 받음. 콜백은 `LegacySimpleResult`로 성공 여부 및 메시지를 전달.

1) legacy_agent_create_participant
- 시그니처:
  - `LegacyStatus legacy_agent_create_participant(LEGACY_HANDLE h, const LegacyParticipantConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- `LegacyParticipantConfig` 필드: `int domain; const char* qos;`
- 예제:
```c
void part_cb(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, void* user) {
    if (!res->ok) { printf("participant create failed\n"); }
}
LegacyParticipantConfig pcfg = { .domain = 0, .qos = "default" };
legacy_agent_create_participant(h, &pcfg, 5000, part_cb, NULL);
```

2) legacy_agent_create_publisher
- 시그니처:
  - `LegacyStatus legacy_agent_create_publisher(LEGACY_HANDLE h, const LegacyPublisherConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- `LegacyPublisherConfig` 필드: `int domain; const char* name; const char* qos;`
- 예제:
```c
LegacyPublisherConfig pub = { .domain=0, .name="my_pub", .qos="default" };
legacy_agent_create_publisher(h, &pub, 5000, part_cb, NULL);
```

3) legacy_agent_create_subscriber
- 시그니처:
  - `LegacyStatus legacy_agent_create_subscriber(LEGACY_HANDLE h, const LegacySubscriberConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- `LegacySubscriberConfig` 필드: `int domain; const char* name; const char* qos;`

4) legacy_agent_create_writer
- 시그니처:
  - `LegacyStatus legacy_agent_create_writer(LEGACY_HANDLE h, const LegacyWriterConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- `LegacyWriterConfig` 필드: `int domain; const char* publisher; const char* topic; const char* type; const char* qos;`
- 예제:
```c
LegacyWriterConfig wcfg = { .domain=0, .publisher="my_pub", .topic="C_Command", .type="C_StringMsg", .qos="default" };
legacy_agent_create_writer(h, &wcfg, 5000, part_cb, NULL);
```

5) legacy_agent_create_reader
- 시그니처:
  - `LegacyStatus legacy_agent_create_reader(LEGACY_HANDLE h, const LegacyReaderConfig* cfg, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- `LegacyReaderConfig` 필드: `int domain; const char* subscriber; const char* topic; const char* type; const char* qos;`

6) legacy_agent_clear_dds_entities
- 시그니처: `LegacyStatus legacy_agent_clear_dds_entities(LEGACY_HANDLE h, uint32_t timeout_ms, LegacySimpleCb cb, void* user);`
- 동작: 등록된 DDS 엔티티를 정리 요청.

### QoS API

1) legacy_agent_get_qos_list
- 시그니처: `LegacyStatus legacy_agent_get_qos_list(LEGACY_HANDLE h, bool include_builtin, bool detail, uint32_t timeout_ms, LegacyQosListCb cb, void* user);`
- 설명: 사용 가능한 QoS 프로파일 목록 조회. 결과는 `LegacyQosList`로 전달.

2) legacy_agent_set_qos_profile
- 시그니처: `LegacyStatus legacy_agent_set_qos_profile(LEGACY_HANDLE h, const LegacyQosSetOptions* opt, uint32_t timeout_ms, LegacyQosSetCb cb, void* user);`

### 데이터 쓰기 API

1) legacy_agent_write_json
- 시그니처: `LegacyStatus legacy_agent_write_json(LEGACY_HANDLE h, const LegacyWriteJsonOptions* opt, uint32_t timeout_ms, LegacyWriteCb cb, void* user);`
- 파라미터 표:
  - `h`: 핸들
  - `opt`: `LegacyWriteJsonOptions` 포인터(필수: topic, data_json)
  - `timeout_ms`: 응답 대기 시간
  - `cb`: 결과 콜백
  - `user`: 콜백에 전달되는 사용자 포인터
- 동작: JSON 페이로드를 Agent로 전송. 응답(ack/err)은 콜백으로 전달.

VxWorks DKM 예제(동기 대기 using binary semaphore):
```c
#include <semLib.h>
#include <stdio.h>

typedef struct { SEM_ID sem; LegacySimpleResult res; } VxPending;

void vx_write_cb(LEGACY_HANDLE h, LegacyRequestId reqId, const LegacySimpleResult* res, void* user) {
    VxPending* p = (VxPending*)user;
    if (res) p->res = *res; // 구조체 복사
    semGive(p->sem);
}

int write_sync(LEGACY_HANDLE h, const LegacyWriteJsonOptions* wopt, uint32_t timeout_ms) {
    VxPending p;
    p.sem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
    if (!p.sem) return -1;
    legacy_agent_write_json(h, wopt, timeout_ms, vx_write_cb, &p);
    // semTake에 타임아웃 구현이 환경에 따라 다르지만 예시는 blocking
    semTake(p.sem, WAIT_FOREVER);
    semDelete(p.sem);
    return p.res.ok ? 0 : -1;
}
```

2) legacy_agent_write_struct
- 시그니처: `LegacyStatus legacy_agent_write_struct(LEGACY_HANDLE h, const char* topic, const char* type_name, const void* user_struct, uint32_t timeout_ms, LegacyWriteCb cb, void* user);`
- 설명: 미리 등록된 `LegacyTypeAdapter`의 `encode`를 호출해 JSON을 얻고 전송.

### 데이터 수신 (구독) API

1) legacy_agent_subscribe_event
- 시그니처: `LegacyStatus legacy_agent_subscribe_event(LEGACY_HANDLE h, const char* topic, const char* type, LegacyEventCb cb, void* user);`
- 동작: 지정 토픽/타입에 대해 이벤트 수신 콜백 등록. 콜백은 `LegacyEvent` 포인터를 전달받음.

2) legacy_agent_subscribe_typed
- 시그니처: `LegacyStatus legacy_agent_subscribe_typed(LEGACY_HANDLE h, const char* topic, const char* type_name, LegacyTypedEventCb cb, void* user);`
- 동작: 라이브러리가 등록된 타입 어댑터를 사용해 `user_struct`로 디코드한 결과를 콜백에 전달.

예제 — VxWorks DKM에서 간단한 이벤트 처리(비동기)
```c
void alarm_cb(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    // 받은 JSON을 즉시 복사하여 작업 큐에 전달
    char* copy = malloc(strlen(evt->data_json) + 1);
    strcpy(copy, evt->data_json);
    // enqueue_work는 앱이 구현한 작업 큐 함수
    enqueue_work(process_alarm, copy);
}

legacy_agent_subscribe_event(h, "P_Alarms", "AlarmType", alarm_cb, NULL);
```

---

## 비동기 응답 매칭 (reqId와 user 포인터) — 이해하기 쉬운 설명

1) 목표: 요청을 보낸 호출 시점과 Agent에서 돌아오는 응답 콜백을 "연결"(매칭)해야 합니다.

2) 사용 가능한 방법
- A. per-request user-context (간단): 호출 시 `user`에 요청 전용 구조체 포인터를 전달. 콜백은 동일한 포인터를 받아 처리. 동기 대기가 필요하면 세마포어/뮤텍스/조건변수 등을 이 구조체에 포함시켜 대기.
- 장점: 구현 단순, 스코프가 명확.
- 단점: 요청이 많을 때 메모리 관리 필요.

- B. reqId 기반 맵(확장성): 요청 호출 시 전역(또는 핸들별) 스레드 안전 맵에 `reqId -> context`를 저장. 콜백에서 `reqId`로 조회하여 처리하고 맵에서 제거.
- 장점: 공통 콜백 구조로 여러 요청 처리 가능.
- 단점: 맵 관리(동기화, 타임아웃 정리) 필요.

3) 실전 권장
- 단건 요청(예: 설정/엔티티 생성 등)에는 A 패턴 권장.
- 많은 동시 요청 및 공통 콜백 사용 시 B 패턴 권장.

4) VxWorks 예제(간단 재요청-대기: per-request semaphore) — 이미 제공된 write_sync 예제 참조.

---

## 구독 콜백 사용 가이드 (실무 팁)

- 콜백은 라이브러리 내부 I/O 스레드에서 실행될 수 있음. 콜백 내에서 오래 걸리는 동작(파일 I/O, 긴 계산, 블로킹 동기화 등)은 반드시 별도 작업 스레드로 위임하세요.
- 콜백에 전달되는 포인터(`evt->data_json` 등)는 콜백이 반환되면 무효화될 수 있으니, 장기 보관 시 즉시 복사하십시오.
- 설계 제안:
  - 토픽별 전용 콜백: 이벤트별로 명확한 처리가 가능, 가독성 높음.
  - 공통 콜백 + 분기: 등록이 많은 경우 코드 중복을 줄이기 좋음. 분기 비용은 낮음.

예: 권장 패턴 — 콜백은 최소 역할(복사 및 enqueue)만 수행.

---

## 콜백 스레드·동시성 규칙 — 주의사항 상세

- 콜백은 라이브러리 내부의 I/O/워커 스레드에서 호출됩니다. 콜백이 오래 머무르면 그 스레드가 차단되어 다른 I/O가 지연됩니다.
- 콜백에서 호출하면 안 되는 작업 예:
  - 긴 파일 또는 네트워크 I/O
  - 대기 시간 불명확한 동기화(다른 락 획득 대기 등)
  - 메모리 압박을 유발하는 대용량 할당(가능하면 풀 사용)
- 안전한 콜백 작업:
  - 이벤트 JSON을 복사하여 작업 큐에 넣기
  - 상태 플래그 설정 및 빠른 반환

스레드 안전한 API 사용:
- `legacy_agent_*` 호출은 초기화 후 여러 스레드에서 안전하게 호출할 수 있도록 설계되어 있습니다(핸들 공유 가능). 다만 `legacy_agent_close()` 호출 시점은 외부에서 동기화해야 합니다.

---

## 메모리·소유권 규칙 (요약)

- 호출자가 전달하는 `const char*` (예: `topic`, `type`, `data_json`)는 호출이 완료될 때 라이브러리가 내부로 필요한 경우 복사합니다. 안전을 위해 호출 후 호출자 메모리의 수명을 책임지십시오.
- 콜백으로 전달되는 포인터(`LegacyEvent::data_json`, `LegacySimpleResult::raw_json`)는 콜백 루틴이 리턴할 때까지만 유효합니다. 복사 없이 장기 보관하지 마십시오.
- `LegacyTypeAdapter::encode`/`make_default`가 반환하는 문자열은 라이브러리가 내부로 복사하므로 어댑터는 스택 버퍼를 사용해도 됩니다(동시에 멀티스레드에서 같은 버퍼를 쓰지 않도록 주의).

---

## 권장 사용 패턴(요약)

- 콜백은 빠르게 반환; 복잡한 처리는 작업 큐로 위임.
- 단건 요청에는 per-request `user` 컨텍스트(세마포어 등) 사용.
- 동시 다중 요청 환경에는 `reqId` 맵 사용.
- `legacy_agent_close()` 호출 전 모든 비동기 요청이 완료되었거나 취소되었는지 확인.

---

## 성능 계측

- API: `LegacyStatus legacy_agent_get_perf_stats(LEGACY_HANDLE h, LegacyPerfStats* out_stats);`
- 조건: 빌드 시 `DEMO_PERF_INSTRUMENTATION` 활성화 시 해당 카운터가 수집됩니다.

---

## 에러 코드

- `LEGACY_OK` (0) — 성공
- `LEGACY_ERR_PARAM` — 전달 인자가 유효하지 않음
- `LEGACY_ERR_TRANSPORT` — 전송/연결 레벨 오류
- `LEGACY_ERR_TIMEOUT` — 요청 타임아웃
- `LEGACY_ERR_PROTO` — 프로토콜/파싱 오류
- `LEGACY_ERR_CLOSED` — 핸들이 이미 닫혀 있음

에러 처리 권장:
- API 반환값을 즉시 확인하고, 비동기 콜백의 `LegacySimpleResult` 내부 `res->ok` 값을 반드시 확인하세요.

---

## 부록: 타입 어댑터 (설명 및 사용 예제)

부속 기능: `LegacyTypeAdapter`는 사용자의 C 구조체와 JSON(또는 라이브러리 내부 포맷) 간 인코딩/디코딩을 담당합니다. 라이브러리에 등록하면 `legacy_agent_write_struct()`와 `legacy_agent_subscribe_typed()`에서 사용됩니다.

구성:
- `key.topic`, `key.type_name` — 어댑터가 적용될 토픽/타입 이름
- `encode()` — user_struct -> JSON 문자열 반환
- `decode()` — JSON -> out_user_struct
- `make_default()` — 디폴트 JSON

간단 등록 예제:
```c
LegacyTypeAdapter adapter = {0};
adapter.key.topic = "C_Command";
adapter.key.type_name = "C_MyCmd";
adapter.encode = my_encode;
adapter.decode = my_decode;
adapter.make_default = my_make_default;
adapter.user_ctx = NULL;

legacy_agent_register_type_adapter(h, &adapter);
```

사용 시 주의:
- `encode()`가 반환한 포인터는 즉시 복사되므로 스택 버퍼 사용 가능하나, 재진입/동시성 문제를 피하기 위해 각 호출마다 독립 버퍼를 권장합니다.

---

끝.
