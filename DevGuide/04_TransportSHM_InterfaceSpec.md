# 04_TransportSHM_InterfaceSpec.md
# TransportSHM 연동 규격 및 구조 설계 (Interface Spec) v2 (VxWorks)

## 0) 목적
Agent / LegacyLib / UI가 TransportSHM을 동일하게 구현/운영할 수 있도록 **상호운용 규격**을 고정한다.

- 공유해야 하는 이름/키
- 레이아웃/레코드/WRAP 규칙
- 동기화 규칙(메모리 오더 포함)
- sizing 산정 규칙

TransportSHM은 **VxWorks 전용 Data plane transport**다.

---

## 1) 공유 식별자(이름 기반)
### 1.1 SHM 객체 이름
- `shm_name` : 예) `/dds_fw_data`

### 1.2 Notify 이름(링별 1개)
- `notify_la` : 예) `/dds_fw_sem_la` (LA ring consumer 깨우기)
- `notify_al` : 예) `/dds_fw_sem_al` (AL ring consumer 깨우기)

의미:
- 주소가 아니라 이름(key)
- 같은 HW/같은 OS 인스턴스에서 동일 name으로 open하면 같은 객체 공유

---

## 2) 고정 파라미터(양쪽 일치 필수)
- `ver` = 1
- `ring_bytes` (per ring bytes)
- `max_frame` (max frame_bytes bytes)
- `ALIGN` = 16 (record alignment)
- `notify_kind` = POSIX named semaphore (v1 구현 기본)

불일치 시 fail-fast 권장.

---

## 3) 메모리 레이아웃(ver=1)
```
offset 0:
  ShmGlobalHeader
then:
  LA ShmRingHeader + LA buffer[ring_bytes]
  AL ShmRingHeader + AL buffer[ring_bytes]
```

---

## 4) ABI 구조체(고정)
### 4.1 Global Header
```c
#define DDSFW_SHM_MAGIC 0x53484D31u /* 'SHM1' */

enum : uint32_t {
  DDSFW_SHM_STATE_UNINIT = 0,
  DDSFW_SHM_STATE_READY  = 1
};

struct ShmGlobalHeader {
  uint32_t magic;        // DDSFW_SHM_MAGIC
  uint16_t ver;          // 1
  uint16_t flags;        // reserved
  uint32_t state;        // READY when initialized
  uint32_t epoch;        // creator increments on recreate/reset

  uint32_t max_frame;    // max [FrameHeader+Payload] bytes
  uint32_t ring_bytes;   // per ring bytes

  uint32_t abi_hash;     // optional (0)

  char shm_name[64];
  char notify_la[64];
  char notify_al[64];

  uint32_t reserved[8];
};
```

### 4.2 Ring Header
```c
struct alignas(64) ShmRingHeader {
  std::atomic<uint32_t> head;
  std::atomic<uint32_t> tail;
  uint32_t capacity;     // == ring_bytes
  uint32_t drops;
  uint32_t seq;
  uint32_t reserved[11];
  // uint8_t buffer[capacity];
};
```

### 4.3 Record Header
```c
struct ShmRecordHdr {
  uint32_t total_len;  // 0 => WRAP marker
  uint32_t seq;
  uint32_t crc32;      // 0 if unused
  uint32_t reserved;
  // uint8_t frame_bytes[];
};
```

---

## 5) Record 규격
- `frame_bytes` = 상위가 만든 바이트열 (Header+Payload 포함)
- Transport는 bytes만 운반

### 5.1 WRAP 규칙
- producer: contig 부족 시 WRAP record(total_len=0) 기록 후 head=0
- consumer: WRAP record를 만나면 tail=0 후 계속

---

## 6) 동기화 규칙(필수)
### 6.1 Producer 순서
1) record data memcpy
2) `head.store(release)`
3) `notify.post()`

### 6.2 Consumer 순서
1) empty면 `notify.wait(timeout)`
2) record 읽기
3) 소비 후 `tail.store(release)`

---

## 7) Sizing 산정 규칙
- `max_frame`: 최대 frame_bytes bytes
- `ring_bytes`: ring buffer bytes

record_cost:
```
record_cost = align16(sizeof(ShmRecordHdr) + frame_len)
```

최대 프레임 N개 보장:
```
ring_bytes >= N * align16(sizeof(ShmRecordHdr)+max_frame) + margin
```

---

## 8) Drop 규칙
- free < need → drop, drops++
- send_bytes 실패 반환

---

## 9) Epoch 복구
- creator reset/recreate 시 epoch 증가
- joiner는 epoch 변경 감지 시 head/tail 재동기화(리셋)

---

## 10) 상호운용 체크리스트
- [ ] shm_name/notify_la/notify_al 동일
- [ ] ring_bytes/max_frame/ver/ALIGN 동일
- [ ] WRAP 처리 동일
- [ ] memory order 규칙 준수
- [ ] drop 정책(drops 카운터) 확인 가능
