# 제어(Command) 및 신호(Signal) 메시지 상호 운용 및 시뮬레이션 규격서

## 1. 개요
본 문서는 구동장치 모의기(**DemoApp**)와 통제/모니터링 장치(**UI**) 간의 상호 작용을 정의한다. `CommandDriving` 메시지(UI $\rightarrow$ DemoApp)와 `Signal` 메시지(DemoApp $\rightarrow$ UI)의 필드별 생성 규칙, 처리 로직, 그리고 기댓값(Expectation)을 상세히 기술하여, 양측 개발자가 동일한 시나리오를 구현할 수 있도록 한다.

---

## 2. DemoApp 관점 (Receiver & Simulator)

DemoApp은 실제 하드웨어 없이 구동장치의 물리적 특성을 모의해야 한다. 이를 위해 수신된 Command를 해석하여 내부 상태를 갱신하고, 이를 기반으로 Signal 메시지를 생성한다.

### 2.1. CommandDriving 메시지의 수신 및 활용
DemoApp은 200Hz 주기로 수신되는 메시지에서 다음 필드를 추출하여 제어 입력으로 사용한다.

| 수신 필드명 | 중요도 | 활용 용도 및 우선순위 |
|:--- |:---:|:--- |
| `A_roundAngleVelocity` | **최우선** | **[속도 제어]** 방위각 구동을 위한 속도 지령. <br> - 이 값이 0이 아니면, **즉시 속도 제어 모드**로 진입한다. <br> - `A_drivingPosition` 값은 무시된다. |
| `A_drivingPosition` | 차선 | **[위치 제어]** 방위각 목표 위치 지령. <br> - `A_roundAngleVelocity`가 0일 때만 유효하다. <br> - 현재 위치와의 오차에 비례하여 내부 속도를 생성한다(P-Control). |
| `A_upDownAngleVelocity` | 단순 | **[고각 자이로 모의]** 고각 구동은 물리 모델 없이 입력값을 그대로 센서 출력으로 바이패스(Bypass)한다. |
| `A_operationMode` | 상태 | 현재 장비의 운용 모드를 저장했다가 Signal 메시지의 상태 비트나 로그에 활용한다. |

### 2.2. 내부 시뮬레이션 로직 (1ms 주기)
DemoApp은 1ms 타이머 인터럽트 내에서 다음 물리 엔진을 수행한다.

1.  **방위각(Azimuth) 물리 연산**
    *   **Case 1: 속도 명령 수신 시** ($V_{cmd} \neq 0$)
        *   내부 속도($V_{curr}$) = $V_{cmd}$
    *   **Case 2: 위치 명령 수신 시** ($V_{cmd} = 0$)
        *   오차($Error$) = $Pos_{target} - Pos_{curr}$
        *   내부 속도($V_{curr}$) = $Error \times Gain_P$ (단, $|V_{curr}| \le 10.0$ 제한)
    *   **위치 적분**: $Pos_{curr} = Pos_{curr} + V_{curr} \times 0.001$

2.  **센서 데이터 생성**
    *   방위각 자이로 = $V_{curr}$
    *   고각 자이로 = `A_upDownAngleVelocity` 수신값 그대로 사용

### 2.3. Signal 메시지의 생성 및 송신
DemoApp은 내부 물리 상태를 기반으로 Signal 메시지를 생성하여 200Hz로 송신한다.

| 송신 필드명 | 매핑 소스 (내부 변수) | 변환/제한 규칙 | DemoApp의 의도 |
|:--- |:--- |:--- |:--- |
| `A_azAngle` | **$V_{curr}$ (속도)** | **[주의]** 위치가 아닌 **속도**를 매핑함. <br> 양자화: 0.01 단위 | UI가 이를 '현재 구동 속도'로 해석하기를 기대함. (레거시 호환성) |
| `A_e1AngleVelocity` | $V_{curr}$ (속도) | 범위제한: $\pm 450.0$ <br> 양자화: 0.01 단위 | 실제 모터의 구동 속도 피드백 |
| `A_roundGyro` | $V_{curr}$ (속도) | 범위제한: $\pm 655.0$ <br> 양자화: 0.02 단위 | 방위각 자이로 센서의 계측값 |
| `A_upDownGyro` | `A_upDownAngleVelocity` | 범위제한: $\pm 655.0$ <br> 양자화: 0.02 단위 | 고각 자이로 센서의 계측값 |

---

## 3. UI 관점 (Controller & Monitor)

UI는 운용자의 조작(조이스틱, 지도 클릭 등)을 Command로 변환하여 송신하고, 돌아오는 Signal을 통해 장비가 명령대로 움직이는지 감시한다.

### 3.1. CommandDriving 메시지의 생성 및 송신 전략
UI는 구동 목적에 따라 필드를 다르게 설정해야 한다.

1.  **조이스틱 모드 (속도 제어)**
    *   **설정**: `A_roundAngleVelocity`에 조이스틱 기울기에 비례한 속도값 설정.
    *   **상태**: `A_drivingPosition`은 무시되므로 0 또는 현재값 유지.
    *   **기대**: 장비가 해당 속도로 즉시 가속하여 회전하기를 기대함.

2.  **지도 클릭/동기화 모드 (위치 제어)**
    *   **설정**: `A_roundAngleVelocity`를 반드시 **0.0**으로 설정. `A_drivingPosition`에 목표 각도 설정.
    *   **기대**: 장비가 스스로 속도를 조절하여 목표 위치에 부드럽게 도달하고 정지하기를 기대함.

3.  **고각/자이로 테스트**
    *   **설정**: 고각 모터 구동을 테스트하고 싶다면 `A_upDownAngleVelocity`에 값을 설정.

### 3.2. Signal 메시지의 수신 및 시각화 활용
UI는 수신된 Signal 메시지를 통해 다음 정보를 시각화한다.

| 수신 필드명 | UI 해석 및 시각화 방법 | 검증 포인트 (Validation) |
|:--- |:--- |:--- |
| `A_e1AngleVelocity` | **[메인 속도계]** 현재 장비의 방위각 회전 속도로 표시 (Gauge). | 내가 보낸 `A_roundAngleVelocity` 명령값과 일치(또는 근접)하는가? |
| `A_azAngle` | **[보조 속도계]** 명칭은 각도이나, 속도 데이터로 처리해야 함. | `A_e1AngleVelocity`와 동일한 추세를 보이어야 함. |
| `A_roundGyro` | **[센서 모니터]** 방위각 자이로 센서 데이터 그래프. | 구동 속도와 자이로 값이 동기화되어 움직이는가? |
| `A_upDownGyro` | **[센서 모니터]** 고각 자이로 센서 데이터 그래프. | 내가 보낸 `A_upDownAngleVelocity` 명령값과 일치하는가? |

---

## 4. 상호 운용 시나리오 (Interoperability Scenarios)

### 시나리오 A: 방위각 정속 주행 테스트 (Velocity Mode)
*   **목적**: UI의 조이스틱 명령에 대해 DemoApp이 정확한 속도로 반응하는지 검증.
1.  **[UI Send]**: `A_roundAngleVelocity` = **30.0** (deg/s), `A_drivingPosition` = N/A
2.  **[DemoApp Process]**: 
    *   속도 모드 감지 ($V_{cmd} \neq 0$).
    *   내부 속도 $V_{curr}$를 30.0으로 설정.
    *   내부 위치를 계속 증가시킴 (1초당 30도).
3.  **[DemoApp Send]**: `A_e1AngleVelocity` = **30.0**, `A_roundGyro` = **30.0**
4.  **[UI Validate]**: 수신된 `Signal`의 속도값이 30.0 $\pm$ 오차범위 내인지 확인.

### 시나리오 B: 방위각 위치 제어 테스트 (Position Mode)
*   **목적**: UI의 목표 위치 지령에 대해 DemoApp이 감속하며 정지하는지 검증.
1.  **[UI Send]**: `A_roundAngleVelocity` = **0.0**, `A_drivingPosition` = **100.0** (현재 위치가 90.0이라고 가정)
2.  **[DemoApp Process]**:
    *   위치 모드 감지 ($V_{cmd} == 0$).
    *   오차 = 10.0 $\rightarrow$ P제어에 의해 속도 $V_{curr}$ = 10.0 발생.
    *   위치가 100.0에 근접할수록 오차가 줄어들어 $V_{curr}$도 0으로 수렴.
3.  **[DemoApp Send]**: `A_e1AngleVelocity` 값이 초기엔 10.0이었다가 점차 **0.0**으로 감소.
4.  **[UI Validate]**: 속도 그래프가 상승했다가 하강곡선을 그리며 0이 되는지 확인 (위치 도달 확인은 별도 위치 피드백이 없으므로 속도가 0이 됨을 통해 간접 확인).

### 시나리오 C: 고각 자이로 루프백 테스트
*   **목적**: 고각 명령이 자이로 센서 데이터로 올바르게 피드백되는지 검증.
1.  **[UI Send]**: `A_upDownAngleVelocity` = **-15.5**
2.  **[DemoApp Process]**: 별도 연산 없이 해당 값을 `upDownGyro` 출력 레지스터에 복사.
3.  **[DemoApp Send]**: `A_upDownGyro` = **-15.5**
4.  **[UI Validate]**: 고각 자이로 그래프에 -15.5가 표시되는지 확인.
