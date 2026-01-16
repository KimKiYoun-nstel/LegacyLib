# TransportSHM 공용모듈

VxWorks 전용 SHM 기반 Data plane Transport 공용모듈입니다.

## 개요

- **bytes-only API**: 상위 프로토콜(Header, JSON/Struct 등)과 완전 분리
- **Full-duplex SPSC Ring 2개**: LA(Legacy→Agent), AL(Agent→Legacy)
- **POSIX Named Semaphore**: 알림 메커니즘
- **lock-free**: atomic head/tail 기반

## 디렉터리 구조

```
common/transport_shm/
├── CMakeLists.txt              # 빌드 설정
├── README.md                   # 이 문서
├── include/
│   └── ddsfw/transport_shm/
│       ├── shm_config.hpp      # Config 구조체
│       ├── shm_log.hpp         # 로그 콜백 인터페이스
│       └── shm_transport.hpp   # Transport 클래스 (Public API)
└── src/
    ├── shm_layout.hpp          # ABI 구조체 (내부)
    ├── shm_region.hpp          # SHM Region 인터페이스
    ├── shm_region_posix.cpp    # shm_open/mmap 구현
    ├── shm_notify.hpp          # Notify 인터페이스
    ├── shm_notify_posix_sem.cpp # Named Semaphore 구현
    ├── shm_ring_spsc.hpp       # SPSC Ring 인터페이스
    ├── shm_ring_spsc.cpp       # Ring 알고리즘 구현
    └── shm_transport.cpp       # Transport 핵심 구현
```

## 사용법

### Agent 측 (creator)

```cpp
#include <ddsfw/transport_shm/shm_transport.hpp>

using namespace ddsfw::transport_shm;

void my_log(LogLevel level, const char* msg) noexcept {
    // 로그 출력
}

void my_on_frame(const uint8_t* data, uint32_t len, void* user) noexcept {
    // 수신 프레임 처리 (bytes-only)
}

int main() {
    Transport tx;
    
    auto cfg = Config::defaults(Side::Agent, true);  // creator
    if (!tx.open(cfg, my_log)) {
        return -1;
    }
    
    tx.set_on_frame(my_on_frame, nullptr);
    tx.start();  // RX thread 시작
    
    // 송신
    uint8_t frame[] = { ... };
    tx.send_bytes(frame, sizeof(frame));
    
    // ...
    
    tx.stop();
    tx.close();
    return 0;
}
```

### Legacy 측 (joiner)

```cpp
auto cfg = Config::defaults(Side::Legacy, false);  // joiner
```

## 설정 파라미터

| 항목 | 기본값 | 설명 |
|------|--------|------|
| `shm_name` | `/dds_fw_data` | SHM 객체 이름 |
| `notify_la` | `/dds_fw_sem_la` | LA ring 알림 이름 |
| `notify_al` | `/dds_fw_sem_al` | AL ring 알림 이름 |
| `ring_bytes` | 4MB | 링 버퍼 크기 (per ring) |
| `max_frame` | 64KB | 단일 프레임 최대 크기 |
| `wait_ms` | 100 | consumer wait timeout (ms) |

## ABI (ver=1)

메모리 레이아웃:
```
[ShmGlobalHeader]
[LA ShmRingHeader][LA buffer]
[AL ShmRingHeader][AL buffer]
```

상호운용을 위해 양쪽이 동일한 설정을 사용해야 합니다.

## 빌드

VxWorks에서만 빌드됩니다:

```cmake
if(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    add_subdirectory(common/transport_shm)
endif()
```

## LegacyLib에서 사용

동일 소스를 복사하거나 서브모듈로 포함하여 LegacyLib 빌드 시스템으로 컴파일합니다.
