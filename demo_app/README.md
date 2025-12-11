# DemoApp README

## 디렉토리 구조

```
demo_app/
├── include/
│   └── demo_app.h           # 공용 헤더 (타입 정의, API)
├── src/
│   ├── demo_app_core.c      # 상태 머신 + 핵심 로직
│   ├── demo_app_msg.c       # 메시지 핸들러 (7개 Topic)
│   └── demo_app_timer.c     # 주기 처리 (200Hz, 1Hz)
├── vxworks/
│   ├── demo_app_dkm.c       # VxWorks 모듈 진입점
│   └── demo_app_cli.c       # TCP CLI 서버
├── docs/
│   └── (문서 위치)
├── Makefile                 # 빌드 스크립트
└── README.md                # 이 파일
```

## 빌드 방법

```bash
# VxWorks 개발 환경에서
cd demo_app
make
```

산출물: `demo_app_dkm.out`

## VxWorks에서 실행

```
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")
```

## CLI 명령어

```
telnet <target_ip> 23000
> help                      # 도움말
> status                    # 상태 확인
> demo_init                 # 데모 시작
> demo_stop                 # 데모 중지
> run_ibit 1 0             # IBIT 실행
> fault_inject azimuth     # 고장 주입
> fault_clear all          # 고장 제거
```

## 개발 단계

- [x] Phase 1: 프로젝트 구조 생성 (완료)
- [ ] Phase 2: 상태 머신 + DDS 초기화
- [ ] Phase 3: 주기 메시지 송수신
- [ ] Phase 4: IBIT 흐름 구현
- [ ] Phase 5: 통합 테스트 & 완성
