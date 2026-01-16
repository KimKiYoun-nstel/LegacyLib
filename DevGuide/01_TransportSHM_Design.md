# 01_TransportSHM_Design.md
# TransportSHM 공용모듈 구현 설계 (VxWorks 전용) v4

## 0) 목적/원칙
TransportSHM은 **Data plane 전용 Transport**이며, Agent/Legacy/UI 상위 프로토콜(프레임 타입, JSON/Struct, corr_id 등)과 **완전히 분리**된 공용모듈이다.

- **TransportSHM은 오직 VxWorks에서만 동작/빌드 대상**이다.
  - Windows/Linux 빌드에서는 소스가 제외되거나(stub) 컴파일만 가능하도록 한다.
- 공용모듈은 **기존 프로젝트 코드(Agent/Legacy) 헤더에 의존하지 않는다.**
  - `dkmrtp::ipc::Header`, `LegacyFrameHeader` 등 include 금지
  - TransportSHM의 I/O 단위는 **bytes**(frame_bytes)뿐이다.
- (허용 마지노선) 로그 연결은 **콜백(함수 포인터)** 형태로만 제공한다.

---

## 1) 공용모듈이 운반하는 단위
TransportSHM은 다음 바이트열을 있는 그대로 운반한다.

```
frame_bytes = [FrameHeader][Payload]
```

- FrameHeader/Payload의 의미는 상위(Agent/Legacy)가 알고 있고,
- TransportSHM은 길이(`len`)와 bytes만 취급한다.

---

## 2) VxWorks에서의 “이름 기반 SHM/notify”
- `shm_name` (예: `/dds_fw_data`) : **named shared memory object name**
- `notify_la`, `notify_al` (예: `/dds_fw_sem_la`, `/dds_fw_sem_al`) : **named semaphore/event name**

중요:
- 이는 “주소”가 아니라 **이름(key)** 이다.
- 서로 다른 프로세스가 같은 이름으로 `shm_open/sem_open`을 호출하면 **같은 객체를 공유**한다.
- 실제 파일이 보이는지 여부는 OS 구현에 따라 다르며, 설계상 “파일 경로”로 취급하지 않는다.

---

## 3) 아키텍처: Full-duplex SPSC 링 2개
- LA ring: Legacy → Agent (Data REQ)
- AL ring: Agent → Legacy (Data RSP/EVT)

각 링은 SPSC로 고정:
- lock-free head/tail (atomic) 구현
- 구현 단순 + 성능 예측 가능

---

## 4) 공용모듈 디렉터리/빌드 트리(공유 단위)
> **Agent 저장소에서 구현**하고, LegacyLib은 동일 소스를 포함(복사/서브모듈/서브트리)하여 **LegacyLib의 빌드 시스템으로 다시 컴파일**한다.

권장 트리(Agent repo 기준):
```
Agent/
  common/
    transport_shm/
      include/
        ddsfw/transport_shm/
          shm_config.hpp
          shm_transport.hpp          # public bytes-only API
          shm_log.hpp                # log callback interface
      src/
        shm_transport.cpp            # core orchestration
        shm_region_posix.cpp         # shm_open/mmap/ftruncate (VxWorks POSIX)
        shm_notify_posix_sem.cpp     # sem_open/sem_post/sem_timedwait
        shm_ring_spsc.cpp            # ring push/pop + WRAP
        shm_layout.hpp               # ABI structs/offset calc (private)
        shm_crc32.cpp                # optional (private)
      CMakeLists.txt                 # Agent(CMake) 빌드용
      README.md                      # (선택) quick usage
```

LegacyLib에서는 동일 폴더를 그대로 포함:
```
LegacyLib/
  third_party/
    transport_shm/   # Agent/common/transport_shm 를 그대로 미러(복사/서브모듈)
      include/...
      src/...
```

공용모듈이 외부에 노출하는 헤더는 `include/ddsfw/transport_shm/*` 만.

---

## 5) 공용모듈 Public API (bytes-only)
### 5.1 설정 구조체
```cpp
namespace ddsfw::transport_shm {

enum class Side { Agent, Legacy };
enum class NotifyKind { PosixNamedSemaphore /* default */ };

struct Config {
  Side side;
  bool creator;              // 권장: Agent=true, Legacy=false(joiner)

  const char* shm_name;      // "/dds_fw_data"
  const char* notify_la;     // "/dds_fw_sem_la"
  const char* notify_al;     // "/dds_fw_sem_al"

  uint32_t ring_bytes;       // per-ring bytes
  uint32_t max_frame;        // max bytes for one frame_bytes
  uint32_t wait_ms;          // consumer wait timeout

  NotifyKind notify_kind;    // 확장 여지
};
} // namespace
```

### 5.2 로깅(마지노선)
```cpp
namespace ddsfw::transport_shm {
enum class LogLevel { Debug, Info, Warn, Error };
using LogFn = void(*)(LogLevel, const char* msg) noexcept;
} // namespace
```

### 5.3 송수신 콜백(bytes-only)
```cpp
namespace ddsfw::transport_shm {
using OnFrameFn = void(*)(const uint8_t* frame_bytes, uint32_t len, void* user) noexcept;
} // namespace
```

### 5.4 Transport 클래스
```cpp
namespace ddsfw::transport_shm {

class Transport {
public:
  Transport() = default;
  ~Transport();

  bool open(const Config& cfg, LogFn log = nullptr) noexcept;
  void close() noexcept;

  void set_on_frame(OnFrameFn cb, void* user) noexcept;

  bool start() noexcept;     // start rx thread (consumer)
  void stop() noexcept;

  bool send_bytes(const uint8_t* frame_bytes, uint32_t len) noexcept;

  uint32_t epoch() const noexcept;
  uint32_t drops_tx() const noexcept;
  uint32_t drops_rx() const noexcept;
};

} // namespace
```

---

## 6) SHM Layout / ABI (v1 고정)
### 6.1 전체 배치
```
[GlobalHeader]
[LA RingHeader][LA buffer (ring_bytes)]
[AL RingHeader][AL buffer (ring_bytes)]
```

### 6.2 구조체(고정)
- `ShmGlobalHeader` / `ShmRingHeader` / `ShmRecordHdr` 는 **04 문서(InterfaceSpec)** 와 동일.
- 정렬:
  - RingHeader: 64-byte align
  - Record 정렬: ALIGN=16

---

## 7) Ring 알고리즘 (구현 상세)
### 7.1 Record cost
```
record_cost = align16(sizeof(ShmRecordHdr) + frame_len)
```

### 7.2 Producer (send_bytes)
1) len==0 또는 len>max_frame → reject(false)
2) free < record_cost → drop(drops++) return false
3) contig < record_cost → WRAP marker(total_len=0) 기록 후 head=0
4) RecordHdr(total_len, seq, crc32=0) + frame_bytes memcpy
5) head.store(release)
6) notify.post()

### 7.3 Consumer (rx thread)
1) empty면 notify.wait(wait_ms)
2) record 읽기
   - total_len==0 → tail=0 (wrap) 후 continue
3) frame_bytes를 callback으로 전달
4) tail.store(release)
5) drain loop(가능하면 여러 개 연속 처리)

메모리 오더:
- producer: payload write → head store(release)
- consumer: head load(acquire)

---

## 8) Sizing 규칙(명확)
- `max_frame` = 한 번에 운반 가능한 **frame_bytes 최대 바이트**
- `ring_bytes` = 링 버퍼 data area 용량(바이트) (메시지 개수 아님)

“최대 프레임 N개 보장”:
```
ring_bytes >= N * align16(sizeof(ShmRecordHdr) + max_frame) + margin
margin: 1~2 record 권장
```

권장 기본값(초안):
- max_frame=65536 (64KB)
- ring_bytes=4*1024*1024 (4MB)

---

## 9) VxWorks 전용 구현 포인트
- `shm_open/ftruncate/mmap/munmap/close/shm_unlink` 사용 가능 여부는 VxWorks POSIX 구성에 따름.
- `sem_open/sem_post/sem_timedwait/sem_close/sem_unlink` 사용 가능 여부 확인.
- 불가 시, `shm_region_posix.cpp`, `shm_notify_posix_sem.cpp`를 VxWorks native API 래핑으로 교체할 수 있도록 파일 분리.

---

## 10) 단독 테스트(권장)
- 동일 HW에서 프로세스 2개로:
  - LA/AL 양방향 send/recv
  - WRAP 동작
  - overflow drop 카운트
  - 재시작(epoch) 복구

---

## 11) DoD
- 공용모듈이 Agent/Legacy 코드에 의존하지 않음(bytes-only)
- VxWorks에서만 실제 동작(타 플랫폼은 제외/스텁)
- SHM+notify만으로 독립 동작(UDP 깨우기 금지)
- sizing/max_frame/ring_bytes 규칙이 문서/코드에 반영
