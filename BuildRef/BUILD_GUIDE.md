# VxWorks 빌드 가이드

## 환경 설정

### 1. VxWorks 개발 환경 준비

필수 구성 요소:
- Wind River VxWorks 23.03
- LLVM Compiler 15.0.0.1
- VSB (VxWorks Source Build) 프로젝트: `workspace_test`

### 2. 환경 변수 설정

**방법 1: vx_env.bat 실행 (권장)**

```batch
cd D:\CodeDev\LegacyLib
vx_env.bat
```

이 명령은 다음 환경 변수를 설정합니다:
- `WIND_BASE=D:\WindRiver\vxworks\23.03`
- `WIND_CC_SYSROOT=D:\WindRiver\workspace_test`
- `VSB_DIR=D:\WindRiver\workspace_test`
- `LLVM_ROOT=D:\WindRiver\compilers\llvm-15.0.0.1\WIN64`
- `NDDSHOME_CTL=D:\rti_connext_dds-7.3.1`

**방법 2: 수동 설정**

```batch
set WIND_BASE=D:\WindRiver\vxworks\23.03
set WIND_CC_SYSROOT=D:\WindRiver\workspace_test
set VSB_DIR=D:\WindRiver\workspace_test
```

### 3. 환경 검증

```batch
# 컴파일러 확인
wr-cc --version
wr-c++ --version

# 환경 변수 확인
echo %WIND_BASE%
echo %WIND_CC_SYSROOT%
```

## 빌드 방법

### LegacyLib 빌드

```batch
# VxWorks 환경에서
cd D:\CodeDev\LegacyLib

# 빌드 설정 확인
make config

# LegacyLib 빌드
make

# 또는 명시적으로
make MODE=vxworks
```

산출물:
- `liblegacy_agent_dkm.out` - LegacyLib DKM
- `demo_tcp_cli_dkm.out` - 범용 CLI 데모 DKM

### DemoApp 빌드

```batch
# LegacyLib가 먼저 빌드되어 있어야 함
cd demo_app

# 빌드 설정 확인
make config

# DemoApp 빌드
make
```

산출물:
- `demo_app_dkm.out` - DemoApp DKM

## 타겟 아키텍처 정보

### CPU 설정 (default.conf 기반)

- **Architecture**: PowerPC (ppc)
- **CPU Model**: e6500 (PPCE6500)
- **Floating Point**: Hard Float (-mhard-float)
- **ABI**: Secure PLT (--secure-plt)
- **Endian**: Big Endian

### 컴파일러 플래그

**DKM 공통 플래그**:
```
-dkm                    # Downloadable Kernel Module
-mcpu=e6500             # PowerPC e6500
-D_VX_CPU=_VX_PPCE6500  # CPU 정의
-D_WRS_KERNEL           # Kernel 모드
-D_VXWORKS_             # VxWorks 플랫폼
```

## 빌드 문제 해결

### 1. "WIND_BASE not set" 오류

**원인**: 환경 변수가 설정되지 않음

**해결**:
```batch
# vx_env.bat 재실행
vx_env.bat
```

### 2. "wr-cc not found" 오류

**원인**: PATH에 컴파일러가 없음

**해결**:
```batch
# Wind River 환경 확인
where wr-cc

# PATH에 추가 (vx_env.bat이 자동으로 수행)
set PATH=%WIND_BASE%\host\x86-win64\bin;%PATH%
```

### 3. VSB 경로 오류

**원인**: VSB_DIR이 잘못된 경로를 가리킴

**해결**:
```batch
# VSB 디렉토리 확인
dir %VSB_DIR%\krnl\h\public

# 경로가 없으면 VSB_DIR 수정
set VSB_DIR=D:\WindRiver\workspace_test
```

### 4. 링크 오류

**원인**: LegacyLib가 먼저 빌드되지 않음

**해결**:
```batch
# 루트에서 먼저 빌드
cd D:\CodeDev\LegacyLib
make

# 그 다음 DemoApp
cd demo_app
make
```

## Makefile 의존성 구조

```
LegacyLib/Makefile
    ↓ (빌드)
liblegacy_agent_dkm.out
    ↓ (링크)
demo_app/Makefile
    ↓ (빌드)
demo_app_dkm.out
```

## VxWorks에 로드

```
# VxWorks Target Shell에서
-> ld < liblegacy_agent_dkm.out
-> ld < demo_app_dkm.out
-> demoAppStart(23000, "127.0.0.1")

# 또는 범용 CLI 데모
-> ld < demo_tcp_cli_dkm.out
-> legacyAgentTcpStart(23000, NULL)
```

## 클린 빌드

```batch
# LegacyLib 클린
cd D:\CodeDev\LegacyLib
make clean

# DemoApp 클린
cd demo_app
make clean

# 전체 재빌드
cd D:\CodeDev\LegacyLib
make clean
make
cd demo_app
make clean
make
```

## 참고 파일

- `vx_env.bat` - 환경 설정 스크립트
- `default.conf` - VSB 컴파일러 설정 (참고용)
- `Makefile` - LegacyLib 빌드 스크립트
- `demo_app/Makefile` - DemoApp 빌드 스크립트
