# Phase 1.5 완료 보고서

## 📅 작업 일시

- 완료일: 2025년 12월 10일
- 소요 시간: 약 0.5일

## 🎯 Phase 1.5 목표

VxWorks 실제 빌드 환경에 맞춘 Makefile 수정 및 빌드 시스템 구축

## ✅ 완료 작업

### 1. 환경 분석

#### 1.1 vx_env.bat 분석
주요 환경 변수:
- `WIND_BASE=D:\WindRiver\vxworks\23.03` - VxWorks 베이스
- `WIND_CC_SYSROOT=D:\WindRiver\workspace_test` - VSB 루트
- `VSB_DIR=D:\WindRiver\workspace_test` - VSB 디렉토리
- `LLVM_ROOT=D:\WindRiver\compilers\llvm-15.0.0.1\WIN64` - 컴파일러
- `NDDSHOME_CTL=D:\rti_connext_dds-7.3.1` - DDS 라이브러리

#### 1.2 default.conf 분석
타겟 아키텍처 (VSB 설정):
- **Architecture**: PowerPC (ppc)
- **CPU**: e6500 (PPCE6500)
- **컴파일러 플래그**:
  - `-mcpu=e6500` - PowerPC e6500 CPU
  - `-D_VX_CPU=_VX_PPCE6500` - CPU 정의
  - `-mhard-float` - 하드웨어 부동소수점
  - `-mstrict-align` - 엄격한 정렬
  - `--secure-plt` - Secure PLT

### 2. 수정된 파일

#### 2.1 루트 Makefile 수정

**변경 사항**:
- 환경 변수 검증 로직 추가
  ```makefile
  ifndef WIND_BASE
      $(error WIND_BASE not set. Run vx_env.bat first)
  endif
  ```
- PowerPC e6500 타겟 설정
  ```makefile
  COMMON_FLAGS = -dkm \
                 -mcpu=e6500 \
                 -D_VX_CPU=_VX_PPCE6500 \
                 -D_WRS_KERNEL \
                 -D_VXWORKS_
  ```
- `check-env` 타겟 추가
- `config` 타겟 추가 (빌드 설정 출력)

**주요 개선**:
- vx_env.bat 실행 여부 자동 검증
- VSB 기반 정확한 CPU/ARCH 설정
- 명확한 에러 메시지

#### 2.2 demo_app/Makefile 수정

**변경 사항**:
- 상세한 주석 및 사용 가이드 추가
- 환경 변수 검증 (WIND_BASE, WIND_CC_SYSROOT)
- PowerPC e6500 타겟 설정
  ```makefile
  ARCH = ppc
  CPU = PPCE6500
  ```
- VSB include 경로 수정
  ```makefile
  INCLUDES = -I./include \
             -I../include \
             -I$(VSB_DIR)/krnl/h/published/UTILS_UNIX \
             -I$(VSB_DIR)/share/h \
             -I$(VSB_DIR)/krnl/h/public
  ```
- LegacyLib 의존성 처리 개선
  - 정적 라이브러리 대신 오브젝트 파일 직접 링크
  - DKM 특성에 맞는 부분 링크 (-dkm)
- `check-env` 타겟 추가
- 확장된 `config` 타겟

**주요 개선**:
- 실제 VSB 구조에 맞는 include 경로
- DKM 링크 방식 정확한 구현
- LegacyLib와의 의존성 자동 처리

### 3. 새로 작성된 문서

#### 3.1 BUILD_GUIDE.md

**내용**:
- 환경 설정 가이드 (vx_env.bat)
- 빌드 방법 (LegacyLib → DemoApp)
- 타겟 아키텍처 정보
- 문제 해결 가이드
- VxWorks 로드 방법
- 클린 빌드 방법

**주요 섹션**:
1. 환경 설정 (vx_env.bat 실행)
2. 빌드 방법 (make 명령)
3. 타겟 정보 (PowerPC e6500)
4. 문제 해결 (4가지 일반적 오류)
5. VxWorks 로드 (shell 명령)

## 📊 주요 변경 사항 요약

### CPU/Architecture 설정

**Phase 1 (초기)**:
```makefile
ARCH = simlinux
CPU = SIMLINUX
```

**Phase 1.5 (수정 후)**:
```makefile
# 루트 Makefile: 플래그로 지정
-mcpu=e6500
-D_VX_CPU=_VX_PPCE6500

# demo_app/Makefile: 변수로 명시
ARCH = ppc
CPU = PPCE6500
```

### 컴파일러 플래그 비교

| 항목 | Phase 1 | Phase 1.5 |
|------|---------|-----------|
| 기본 | `-dkm -fPIC` | `-dkm -mcpu=e6500` |
| CPU | `-DCPU=$(CPU)` | `-D_VX_CPU=_VX_PPCE6500` |
| 모드 | (없음) | `-D_WRS_KERNEL` |
| 플랫폼 | (없음) | `-D_VXWORKS_` |

### Include 경로 개선

**Phase 1**:
```makefile
-I$(VSB_DIR)/h
-I$(VSB_DIR)/h/wrn/coreip
```

**Phase 1.5**:
```makefile
-I$(VSB_DIR)/krnl/h/published/UTILS_UNIX
-I$(VSB_DIR)/share/h
-I$(VSB_DIR)/krnl/h/public
```

## 🔧 빌드 프로세스

### 1. 환경 준비
```batch
vx_env.bat
```

### 2. LegacyLib 빌드
```batch
cd D:\CodeDev\LegacyLib
make config  # 설정 확인
make         # 빌드
```

### 3. DemoApp 빌드
```batch
cd demo_app
make config  # 설정 확인
make         # 빌드
```

### 4. VxWorks 로드
```
-> ld < liblegacy_agent_dkm.out
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")
```

## 🎯 검증 항목

### Makefile 검증
- [x] 환경 변수 검증 로직 (`check-env`)
- [x] CPU/ARCH 정확한 설정 (PowerPC e6500)
- [x] VSB 기반 include 경로
- [x] DKM 링크 플래그 (-dkm)
- [x] LegacyLib 의존성 처리

### 문서 검증
- [x] BUILD_GUIDE.md 작성
- [x] 환경 설정 방법 명시
- [x] 빌드 순서 명확화
- [x] 문제 해결 가이드 포함

## 🚀 다음 단계

**Phase 2 준비 완료**:
- ✅ 빌드 환경 구축 완료
- ✅ Makefile 정확성 확보
- ✅ 빌드 가이드 문서화

**Phase 2에서 구현**:
- LegacyLib 초기화 및 DDS 연결
- 7개 Topic Writer/Reader 생성
- PBIT 로직 및 메시지 발행
- 상태 전이 (`Idle` → `Init` → `PowerOnBit` → `Run`)

## 📝 참고 사항

### default.conf에서 발췌한 주요 설정

```conf
PROG_cc = $(BINDIR)/clang$(EXE)
PROG_c++ = $(BINDIR)/clang++$(EXE)
PROG_ld = $(BINDIR)/$(BIN_linker)$(EXE) -m elf32ppc --secure-plt

[KERNEL] Compile = $(CC) -mcpu=e6500 -mno-altivec \
                   -D_VX_CPU=_VX_PPCE6500 \
                   -mlong-double-64 -mno-spe \
                   -D_WRS_PPC_NO_MCRXR \
                   -mhard-float -D__ppc -D__ppc__ \
                   -mstrict-align -mlongcall \
                   --target=ppc32
```

이 중 필수 항목만 Makefile에 반영:
- `-mcpu=e6500`
- `-D_VX_CPU=_VX_PPCE6500`
- `-D_WRS_KERNEL`

### 빌드 의존성 그래프

```
vx_env.bat
    ↓
LegacyLib/Makefile (check-env)
    ↓
liblegacy_agent_dkm.out
src/legacy_agent.o
src/internal/*.o
    ↓
demo_app/Makefile (check-env)
    ↓
demo_app_dkm.out
```

---

**Phase 1.5 완료**: VxWorks 실제 빌드 환경 구축 완료 ✅

**다음**: Phase 2 (상태 머신 + DDS 초기화) 진행 가능
