# 03_LegacyLib_SHM_Integration_Design.md
# LegacyLib에서 TransportSHM 반영 설계 (VxWorks 전용 SHM) v4

## 0) 목표/범위
- Control plane: 기존 UDP(JSON/CBOR) 유지
- Data plane: 설정에 따라 UDP 또는 **SHM(VxWorks only)** 선택
- TransportSHM 공용모듈은 Agent에서 구현한 소스를 LegacyLib로 공유하여,
  LegacyLib의 Makefile 빌드로 **동일 소스를 재컴파일**하여 사용

이번 문서에는:
- LegacyLib 코드 연결 설계(Transport 선택/라우팅)
- LegacyLib(Makefile)에서 공용모듈 소스 포함 방식
- VxWorks 전용 컴파일 가드

을 포함한다.

---

## 1) 공용모듈 공유 방식(LegacyLib)
권장: Agent repo의 `common/transport_shm`를 그대로 미러링

LegacyLib 트리:
```
LegacyLib/third_party/transport_shm/
  include/ddsfw/transport_shm/...
  src/...
```

공유 방식 예시:
- git submodule / subtree / vendor copy 중 택1
- 중요한 건 “동일 소스”가 LegacyLib에서 **자체 빌드**된다는 점

---

## 2) Makefile 반영(필수)
### 2.1 include path
예:
```make
CXXFLAGS += -ILegacyLib/third_party/transport_shm/include
```

### 2.2 sources
예:
```make
SHM_SRC_CPP =   third_party/transport_shm/src/shm_transport.cpp   third_party/transport_shm/src/shm_region_posix.cpp   third_party/transport_shm/src/shm_notify_posix_sem.cpp   third_party/transport_shm/src/shm_ring_spsc.cpp

LIB_SRC_CPP += $(SHM_SRC_CPP)
```

### 2.3 VxWorks 전용 가드
예:
```make
ifeq ($(OS),VXWORKS)
  CXXFLAGS += -DDDSFW_ENABLE_SHM=1 -DDDSFW_TRANSPORT_SHM_VXWORKS=1
else
  CXXFLAGS += -DDDSFW_ENABLE_SHM=0
  # (권장) SHM_SRC_CPP를 포함하지 않는다.
endif
```

---

## 3) LegacyLib 연결 구조(bytes-only)
공용모듈은 bytes-only이므로 LegacyLib 쪽에도 얇은 어댑터를 둔다.

권장 구성:
```
legacy_lib/src/internal/data_transport/
  data_transport_iface.hpp
  data_transport_udp.cpp
  data_transport_shm.cpp     # bytes <-> frame assembly/parsing only
```

`data_transport_shm.cpp` 역할:
- 기존 코드가 생성하는 (Header+Payload) frame_bytes를 조립
- `ddsfw::transport_shm::Transport::send_bytes()` 호출
- 수신 콜백에서 frame_bytes를 파싱하여 기존 dispatcher/pending 매칭으로 전달

> 프레임 헤더 정의 include는 LegacyLib 내부이므로 OK.  
> 공용모듈은 절대 include하지 않는다.

---

## 4) 라우팅 규칙(세트3)
- UDP 수신: Control만 처리, Data는 drop/warn
- SHM 수신: Data만 처리, Control은 drop/warn

송신:
- Control: UDP
- Data: 선택된 Data transport(SHM)

---

## 5) 설정/헬로 기반 파라미터 공유
LegacyLib은 다음 SHM 파라미터를 알아야 한다(양쪽 일치 필요):

- shm_name
- notify_la / notify_al
- ring_bytes
- max_frame
- wait_ms

획득:
- UI/앱 설정 파일 또는
- Control hello 응답에서 Agent가 노출한 값을 사용(권장: 일치 검증)

---

## 6) VxWorks 전용 동작 정책
- data_transport=shm인데 VxWorks가 아니면:
  - init 실패(fail-fast) 권장

---

## 7) DoD
- LegacyLib이 공용모듈 소스를 포함하여 Makefile로 정상 빌드
- VxWorks에서만 SHM 경로 활성화
- Data 송수신이 SHM으로 라우팅되고 Control은 UDP 유지
