# Build commands (minimal, copy-paste)

다음은 각 플랫폼별로 동작하는 최소한의 빌드/클린 명령 모음입니다. 각각의 명령은 **프로젝트 루트** 또는 **demo_app** 폴더 기준으로 동작하도록 두 가지 변형을 제공합니다.

주의: VxWorks 빌드는 반드시 같은 셸에서 `vx_env.bat` 를 먼저 호출해 환경 변수를 설정해야 합니다.

---

## VxWorks (Wind River)

목표: liblegacy_agent (DKM) 빌드 및 demo_app DKM 빌드

A) 프로젝트 루트에서 (한줄씩 복사해서 실행)

```batch
call vx_env.bat

REM 1) LegacyLib (DKM) 빌드
make MODE=vxworks

REM 2) DemoApp 빌드
make -C demo_app

REM LegacyLib 클린
make clean MODE=vxworks

REM DemoApp 클린
make -C demo_app clean
```

B) `demo_app` 폴더에서

```batch
cd demo_app
call ..\vx_env.bat

REM (옵션) 필요 시 상위 라이브러리 빌드
make -C .. MODE=vxworks

REM DemoApp 빌드 (demo_app/Makefile 기준)
make

REM DemoApp 클린
make clean

REM (옵션) 상위 라이브러리 클린
make -C .. clean MODE=vxworks
```

설명: `demo_app/Makefile` 은 필요 시 상위 디렉터리의 `Makefile` 을 호출해 라이브러리 객체를 생성하도록 되어 있으므로, 루트에서 `make MODE=vxworks`를 수동으로 먼저 돌리지 않아도 `make -C demo_app`로 충분히 빌드되는 경우가 많습니다. 하지만 `vx_env.bat` 는 반드시 같은 셸에서 먼저 실행하세요.

---

## Windows (MinGW)

목표: `demo_app` 빌드 및 클린 (MinGW)

전제: `mingw32-make` (권장) 또는 `make` 가 PATH에 있어야 합니다.

A) 프로젝트 루트에서 (권장 방식 — 한 줄)

```powershell
REM demo_app 빌드 (demo_app 디렉터리에서 실행되는 Makefile.windows 사용)
mingw32-make -C demo_app -f Makefile.windows

REM demo_app 클린
mingw32-make -C demo_app -f Makefile.windows clean
```

B) `demo_app` 폴더에서 (동일한 작업, 권장)

```powershell
cd demo_app

REM demo_app 빌드
mingw32-make -f Makefile.windows

REM demo_app 클린
mingw32-make -f Makefile.windows clean
```

설명: `demo_app/Makefile.windows`는 내부에서 `../src`의 legacy 소스들을 직접 컴파일하도록 작성되어 있으므로, Windows(MinGW)에서는 별도의 루트 `Makefile`의 `MODE=linux` 빌드를 실행할 필요가 없습니다. 루트에서 `-C demo_app` 또는 `cd demo_app` 후 빌드하세요.

---

## 요약 (한줄 복사용)

- VxWorks (루트 기준):
```
call vx_env.bat && make MODE=vxworks && make -C demo_app
```

- VxWorks (demo_app 기준):
```
cd demo_app && call ..\vx_env.bat && make
```

- Windows(MinGW, 루트 기준):
```
mingw32-make -C demo_app -f Makefile.windows
```

- Windows(MinGW, demo_app 기준):
```
cd demo_app && mingw32-make -f Makefile.windows
```

---

필요하면 이 파일을 더 간결하게 하거나, PowerShell 전용 스니펫(에러 체크 포함)으로 바꿔 드리겠습니다.
