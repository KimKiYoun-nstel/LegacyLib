# DemoApp ↔ AgentUI 인터페이스 규격서

## 📋 목차
# Interface Specification

This document describes the runtime JSON interfaces between DemoApp and the Agent. Field names and enum strings follow the XML schema in `RefDoc/Nstel_PSM.xml`.

Topics and Types
-----------------
- `P_NSTEL__C_CannonDrivingDevice_PowerOnBIT` : `P_NSTEL::C_CannonDrivingDevice_PowerOnBIT`
- `P_NSTEL__C_CannonDrivingDevice_PBIT` : `P_NSTEL::C_CannonDrivingDevice_PBIT`
- `P_NSTEL__C_CannonDrivingDevice_IBIT` : `P_NSTEL::C_CannonDrivingDevice_IBIT`
- `P_NSTEL__C_CannonDrivingDevice_Signal` : `P_NSTEL::C_CannonDrivingDevice_Signal`
- `P_NSTEL__C_CannonDrivingDevice_commandDriving` : `P_NSTEL::C_CannonDrivingDevice_commandDriving`
- `P_NSTEL__C_VehicleSpeed` : `P_NSTEL::C_VehicleSpeed`
- `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT` : `P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT`

JSON Payload Examples
----------------------
Below payloads are canonical examples — producers and consumers MUST use the exact field names shown. Numeric fields declared as `T_Double` in the XML are `double` in code.

PBIT (PowerOn BIT) — example

{
  "A_sourceID": { "A_systemID": "DEMO", "A_unitID": 1 },
  "A_timeOfDataGeneration": { "A_seconds": 123456789 },
  "A_BITResults": {
    "roundMotor": "L_BITResultType_NORMAL",
    "roundAmp": "L_BITResultType_NORMAL",
    "upDownMotor": "L_BITResultType_NORMAL",
    "upDownAmp": "L_BITResultType_NORMAL",
    "powerController": "L_BITResultType_NORMAL",
    "energyStorage": "L_BITResultType_NORMAL",
    "directPower": "L_BITResultType_NORMAL",
    "vehicleForwardGyro": "L_BITResultType_NORMAL",
    "baseGyro": "L_BITResultType_NORMAL",
    "commFault": "L_BITResultType_NORMAL",
    "mainCannonLock": "L_BITResultType_NORMAL",
    "roundEncoder": "L_BITResultType_NORMAL"
  }
}

CBIT (Continuous BIT) — example (partial)

{
  "A_sourceID": { "A_systemID": "DEMO", "A_unitID": 1 },
  "A_timeOfDataGeneration": { "A_seconds": 123456789 },
  "A_upDownPark": "L_DekClearanceType_INSIDE",
  "A_round_Park": "L_DekClearanceType_OUTSIDE",
  "A_commFault": false
}

Actuator Signal — example (partial)

{
  "A_roundGyro": 0.123,                 // double
  "A_upDownGyro": -0.05,               // double
  "A_roundVelocity": 0.0,              // double
  "A_upDownVelocity": 0.0,             // double
  "A_roundMotorStatus": "L_ArmPositionType_NORMAL"
}

runBIT (incoming) — example

{
  "A_referenceNum": 42,
  "A_type": "L_BITType_PBIT"
}

Field Name Source
------------------
All field names and enum string values are defined centrally in `demo_app/include/msg_fields.h` and `demo_app/include/demo_app_enums.h`.

Notes
-----
- Implementation uses `nlohmann::json` (`json.hpp`) for parsing/serializing JSON.
- Producers must serialize enum values using schema-aligned strings (e.g., `L_OperationModeType_EMERGENCY`).
- Numeric precision: `T_Double` → `double` in code; `T_Float` → `float` only where schema specifies.

  "A_roundMotor": "L_BITResultType_NORMAL",
  "A_upDownAmp": "L_BITResultType_NORMAL",
  "A_roundAmp": "L_BITResultType_NORMAL",
  "A_baseGyro": "L_BITResultType_NORMAL",
  "A_topForwardGyro": "L_BITResultType_NORMAL",
  "A_vehicleForwardGyro": "L_BITResultType_NORMAL",
  "A_powerController": "L_BITResultType_NORMAL",
  "A_energyStorage": "L_BITResultType_NORMAL",
  "A_directPower": "L_BITResultType_NORMAL",
  "A_cableLoop": "L_BITResultType_NORMAL",
  "A_upDownPark": "L_BITResultType_NORMAL",
  "A_roundPark": "L_BITResultType_NORMAL",
  "A_mainCannonLock": "L_BITResultType_NORMAL",
  "A_controllerNetwork": "L_BITResultType_NORMAL",
  "A_commFault": false
}
```

#### 필드 요약
- CBIT은 PBIT(12개) + 파킹/잠금/네트워크 상태를 포함합니다.
- `A_commFault`는 boolean이며 `true`는 통신 고장(오류)을 의미합니다.

---

### 3. resultBIT (IBIT 결과)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_IBIT`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_IBIT`
- **QoS**: `NonPeriodicEventProfile` (RELIABLE)
- **주기**: 비주기 (IBIT 완료 시)

#### 메시지 구조 (예시)
```json
{
  "A_sourceID": { /* identifier */ },
  "A_timeOfDataGeneration": "...",
  "A_cannonDrivingDevice_sourceID": { /* identifier */ },
  "A_referenceNum": 1234,
  "A_BITRunning": false,
  "A_upDownMotor": "L_BITResultType_NORMAL",
  "A_roundMotor": "L_BITResultType_NORMAL",
  "A_upDownAmp": "L_BITResultType_NORMAL",
  "A_roundAmp": "L_BITResultType_NORMAL",
  "A_baseGyro": "L_BITResultType_NORMAL",
  "A_topForwardGyro": "L_BITResultType_NORMAL",
  "A_vehicleForwardGyro": "L_BITResultType_NORMAL",
  "A_powerController": "L_BITResultType_NORMAL",
  "A_energyStorage": "L_BITResultType_NORMAL",
  "A_directPower": "L_BITResultType_NORMAL",
  "A_cableLoop": "L_BITResultType_NORMAL"
}
```

#### 필드 요약
- `A_referenceNum` (`P_LDM_Common::T_Int32`)는 runBIT 요청에서 전달된 값을 그대로 반환하여 요청-응답 매칭에 사용됩니다.
- 나머지 BIT 필드는 PBIT와 동일한 `T_BITResultType` 열거형을 사용합니다。

---

### 4. Actuator Signal (피드백)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_Signal`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_Signal`
- **QoS**: `HighFreqPeriodicProfile` (200Hz, BEST_EFFORT)
- **주기**: 5ms (200Hz)

#### 메시지 구조 (예시)
```json
{
  "A_sourceID": { /* identifier */ },
  "A_timeOfDataGeneration": "...",
  "A_recipientID": { /* identifier */ },
  "A_azAngle": 45.5,
  "A_e1AngleVelocity": 2.3,
  "A_roundGyro": 2.3,
  "A_upDownGyro": 0.5,
  "A_energyStorage": "L_EnergyStorageStatusType_NORMAL",
  "A_mainCannonFixStatus": "L_CannonFixType_RELEASE",
  "A_deckCleance": "L_DeckClearanceType_OUT_OF_DECK",
  "A_autoArmPositionComplement": "L_CannonDrivingType_DRIVING",
  "A_manualArmPositionComplement": "L_CannonDrivingType_DRIVING",
  "A_mainCannonRestoreComplement": "L_CannonDrivingType_RUNNING",
  "A_armSafetyMainCannonLock": "L_CannonLockType_NORMAL",
  "A_shutdown": "L_ShutdownType_UNKNOWN"
}
```

#### 필드 요약
- 자이로 표기는 스키마 기준으로 `Gyro`로 통일: `A_roundGyro`, `A_upDownGyro`.
- `A_roundGyro`/`A_upDownGyro`는 `P_LDM_Common::T_Double` (double) 타입입니다。

---

## AgentUI → DemoApp 수신 메시지

### 1. runBIT (IBIT 요청)

#### Topic 정보
- **Topic**: `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT`
- **Type**: `P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT`
- **QoS**: `NonPeriodicEventProfile` (RELIABLE)
- **주기**: 비주기 (사용자 명령)

#### 메시지 구조 (AgentUI 송신)
```json
{
  "A_recipientID": { /* DemoApp identifier */ },
  "A_sourceID": { /* Agent identifier */ },
  "A_referenceNum": 1234,
  "A_timeOfDataGeneration": "...",
  "A_type": "L_BITType_I_BIT"
}
```

#### 필드 요약
- `A_referenceNum`는 반드시 포함되며, DemoApp은 IBIT 완료 시 resultBIT의 `A_referenceNum`를 동일 값으로 반환합니다。

---

### 2. Actuator Control (제어 명령)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_commandDriving`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_commandDriving`
- **QoS**: `HighFreqPeriodicProfile` (200Hz, BEST_EFFORT)
- **주기**: 권장 200Hz (최소 10Hz)

#### 메시지 구조 (AgentUI 송신, 예시)
```json
{
  "A_sourceID": { /* AgentUI */ },
  "A_recipientID": { /* DemoApp */ },
  "A_timeOfDataGeneration": "...",
  "A_referenceNum": 1234,
  "A_roundPosition": 0.0,
  "A_upDownPosition": 0.0,
  "A_roundAngleVelocity": 0.0,
  "A_upDownAngleVelocity": 0.0,
  "A_operationMode": "L_OperationModeType_NORMAL",
  "A_parm": "L_PalmModeType_OFF",
  "A_targetFix": "L_TargetFixType_ETC",
  "A_autoArmPosition": "L_ArmPositionType_RELEASE",
  "A_manualArmPosition": "L_ArmPositionType_RELEASE",
  "A_mainCannonRestore": "L_CannonRestoreType_RESTORE",
  "A_mainCannonFix": "L_CannonFixType_RELEASE"
}
```

#### 필드 요약
- 제어 메시지는 정밀한 Topic/Type 식별자가 중요합니다. 반드시 스키마의 `module__struct` / `module::struct` 표기를 사용하세요。

---

### 3. Vehicle Speed (차량 속도)

#### Topic 정보
- **Topic**: `P_NSTEL__C_VehicleSpeed`
- **Type**: `P_NSTEL::C_VehicleSpeed`
- **QoS**: `LowFreqVehicleProfile` (1Hz, RELIABLE)
- **주기**: 권장 1Hz

#### 메시지 구조 (AgentUI 송신)
```json
{
  "A_sourceID": { /* identifier */ },
  "A_timeOfDataGeneration": "...",
  "A_value": 30.5
}
```

---

## 시뮬레이션 동작 로직

(시뮬레이션/타이머/IBIT 처리 로직은 기존 구현을 따르되, 메시지 식별자 및 필드 이름은 본 문서의 스펙을 우선으로 적용합니다.)

**코드 위치**: `demo_app/src/demo_app_timer.c`, `demo_app/src/demo_app_msg.c`, `demo_app/src/demo_app_core.c`

---

## 테스트 시나리오 (간단)
- IBIT 요청: AgentUI가 `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT`로 `A_referenceNum` 포함하여 전송 → DemoApp이 `P_NSTEL__C_CannonDrivingDevice_IBIT`로 동일 `A_referenceNum` 반환
+ Signal 수신/표시: AgentUI는 `P_NSTEL__C_CannonDrivingDevice_Signal`의 `A_roundGyro`/`A_upDownGyro` 값을 사용하여 실시간 게이지를 갱신

---

(문서 끝)

#### AgentUI 표시 방법

**표시 형태 1: 상태 인디케이터**
```
┌─ PowerOn BIT 결과 ──────────────┐
│ 상하 모터:      ● 정상           │
│ 회전 모터:      ● 정상           │
│ 상하 앰프:      ● 정상           │
│ 회전 앰프:      ● 정상           │
│ 베이스 자이로:   ● 정상           │
│ 전원 컨트롤러:   ● 정상           │
│ 에너지 저장:    ● 정상           │
│ 직접 전원:      ● 정상           │
└────────────────────────────────┘
```

**표시 형태 2: 개요 요약**
```
PBIT: 12/12 정상 ✓
최종 수신: 2025-12-11 14:30:15
```

#### 동작 의도
- **목적**: 시스템 초기화 시 12개 서브시스템의 상태를 운용자에게 알림
- **시점**: DemoApp 시작 직후 1회 (POWERON_BIT 상태)
- **기대 동작**: 
  - 정상: 모든 컴포넌트 `"L_BITResultType_NORMAL"` → 녹색 표시
  - 고장: 일부 컴포넌트 `"L_BITResultType_ABNORMAL"` → 빨간색 표시 + 경고음

---

### 2. CBIT (Continuous BIT)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_PBIT`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_PBIT`
- **QoS**: `LowFreqStatusProfile` (1Hz)
- **주기**: 1초 (1000ms)

#### 메시지 구조
```json
{
  "A_sourceID": {...},
  "A_timeOfDataGeneration": {...},
  "A_BITRunning": false,
  "A_upDownMotor": "L_BITResultType_NORMAL",
  "A_roundMotor": "L_BITResultType_NORMAL",
  "A_upDownAmp": "L_BITResultType_NORMAL",
  "A_roundAmp": "L_BITResultType_NORMAL",
  "A_baseGyro": "L_BITResultType_NORMAL",
  "A_topForwardGyro": "L_BITResultType_NORMAL",
  "A_vehicleForwardGyro": "L_BITResultType_NORMAL",
  "A_powerController": "L_BITResultType_NORMAL",
  "A_energyStorage": "L_BITResultType_NORMAL",
  "A_directPower": "L_BITResultType_NORMAL",
  "A_cableLoop": "L_BITResultType_NORMAL",
  "A_upDownPark": "L_BITResultType_NORMAL",
  "A_round_Park": "L_BITResultType_NORMAL",
  "A_mainCannon_Lock": "L_BITResultType_NORMAL",
  "A_commFault": false
}
```

#### 필드 설명 (15개 컴포넌트 = PBIT 12개 + 추가 3개)

**추가 필드** (PBIT 외):

| 필드 | 타입 | 설명 | 정상값 | 고장값 |
|------|------|------|--------|--------|
| `A_upDownPark` | T_BITResultType | 상하 파킹 상태 | `L_BITResultType_NORMAL` | `L_BITResultType_ABNORMAL` |
| `A_round_Park` | T_BITResultType | 회전 파킹 상태 | `L_BITResultType_NORMAL` | `L_BITResultType_ABNORMAL` |
| `A_mainCannon_Lock` | T_BITResultType | 주포 잠금 상태 | `L_BITResultType_NORMAL` | `L_BITResultType_ABNORMAL` |
| `A_commFault` | boolean | 통신 고장 | `false` | `true` (반대) |

**주의**: `A_commFault`는 반대 의미 (true = 고장, false = 정상)

#### AgentUI 표시 방법

**실시간 모니터링**
```
┌─ 시스템 상태 (CBIT) ──────────────┐
│ 모터 시스템:        ● 정상 (4/4)   │
│ 센서 시스템:        ● 정상 (3/3)   │
│ 전원 시스템:        ● 정상 (3/3)   │
│ 파킹/잠금:          ● 정상 (2/2)   │
│ 통신:              ● 정상          │
│                                   │
│ 마지막 업데이트: 1초 전            │
└───────────────────────────────────┘
```

**타임라인 그래프** (옵션)
```
14:30:00  ────────────────────  정상
14:30:05  ─────●───────────────  회전 모터 고장
14:30:10  ─────●●──────────────  회전 앰프 고장
14:30:15  ────────────────────  복구
```

#### 동작 의도
- **목적**: 1초마다 15개 서브시스템의 현재 상태를 실시간으로 전송
- **기대 동작**: 
  - AgentUI는 1Hz로 수신하여 실시간 상태 갱신
  - 고장 발생 시 1초 이내에 감지 가능
  - 고장 이력 저장 (타임스탬프와 함께)

---

### 3. resultBIT (IBIT 결과)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_IBIT`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_IBIT`
- **QoS**: `NonPeriodicEventProfile` (RELIABLE)
- **주기**: 비주기 (IBIT 완료 시)

#### 메시지 구조
```json
{
  "A_sourceID": {...},
  "A_timeOfDataGeneration": {...},
  "A_referenceNum": 1234,
  "A_BITRunning": false,
  "A_upDownMotor": "L_BITResultType_NORMAL",
  "A_roundMotor": "L_BITResultType_NORMAL",
  "A_upDownAmp": "L_BITResultType_NORMAL",
  "A_roundAmp": "L_BITResultType_NORMAL",
  "A_baseGyro": "L_BITResultType_NORMAL",
  "A_topForwardGyro": "L_BITResultType_NORMAL",
  "A_vehicleForwardGyro": "L_BITResultType_NORMAL",
  "A_powerController": "L_BITResultType_NORMAL",
  "A_energyStorage": "L_BITResultType_NORMAL",
  "A_directPower": "L_BITResultType_NORMAL",
  "A_cableLoop": "L_BITResultType_NORMAL"
}
```

#### 필드 설명

| 필드 | 타입 | 설명 |
|------|------|------|
| `A_referenceNum` | int32 | runBIT 요청 시 받은 참조 번호 (그대로 반환) |
| 나머지 12개 | boolean | PBIT와 동일 (12개 BIT 컴포넌트) |

**주의**: `A_powerController` 필드는 RefDoc XML 스키마의 표기와 일치합니다.

#### AgentUI 표시 방법

**IBIT 결과 다이얼로그**
```
┌─ IBIT 결과 (Ref: 1234) ────────────┐
│                                    │
│  소요 시간: 3.0초                   │
│                                    │
│  결과: ✓ 정상                       │
│                                    │
│  ┌─ 상세 결과 ────────────────┐   │
│  │ 상하 모터:      ● 정상       │   │
│  │ 회전 모터:      ● 정상       │   │
│  │ 상하 앰프:      ● 정상       │   │
│  │ 회전 앰프:      ● 정상       │   │
│  │ 베이스 자이로:   ● 정상       │   │
│  │ 전원 컨트롤러:   ● 정상       │   │
│  │ 에너지 저장:    ● 정상       │   │
│  │ 직접 전원:      ● 정상       │   │
│  └────────────────────────────┘   │
│                                    │
│              [ 확인 ]              │
└────────────────────────────────────┘
```

#### 동작 의도
- **목적**: AgentUI에서 IBIT 요청 후 3초 대기 → 결과 수신
- **상관 관계**: `A_referenceNum`을 사용하여 요청-응답 매칭
- **기대 동작**: 
  1. AgentUI가 runBIT 송신 (referenceNum=1234)
  2. DemoApp이 3초 대기 (IBIT 수행 시뮬레이션)
  3. DemoApp이 resultBIT 송신 (referenceNum=1234로 응답)
  4. AgentUI가 referenceNum 확인 후 결과 표시

---

### 4. Actuator Signal (피드백)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_Signal`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_Signal`
- **QoS**: `HighFreqPeriodicProfile` (200Hz, BEST_EFFORT)
- **주기**: 5ms (200Hz)

#### 메시지 구조
```json
{
  "A_sourceID": {...},
  "A_timeOfDataGeneration": {...},
  "A_azAngle": 45.5,
  "A_e1AngleVelocity": 2.3,
  "A_roundGyro": 2.3,
  "A_upDownGyro": 0.5,
  "A_energyStorage": "L_ChangingStatusType_NORMAL",
  "A_mainCannonFixStatus": "L_MainCannonFixStatusType_NORMAL",
  "A_deckClearance": "L_DekClearanceType_OUTSIDE",
  "A_autoArmPositionComplement": "L_ArmPositionType_NORMAL",
  "A_manualArmPositionComple": "L_ArmPositionType_NORMAL",
  "A_mainCannonRestoreComplement": "L_MainCannonReturnStatusType_RUNNING",
  "A_armSafetyMainCannonLock": "L_ArmSafetyMainCannonLock_NORMAL",
  "A_shutdown": "L_CannonDrivingDeviceShutdownType_UNKNOWN"
}
```

#### 필드 설명

**Float 필드 (4개)**:

| 필드 | 타입 | 단위 | 설명 | 범위 |
|------|------|------|------|------|
| `A_azAngle` | float | deg | 현재 방위각 (E1 각도) | 0~360 |
| `A_e1AngleVelocity` | float | deg/s | E1 각속도 | -360~360 |
| `A_roundGyro` | float | deg/s | 회전 자이로 값 | -360~360 |
| `A_upDownGyro` | float | deg/s | 상하 자이로 값 | -90~90 |

**Enum 필드 (8개)**:

| 필드 | 타입 | 설명 | 가능한 값 |
|------|------|------|----------|
| `A_energyStorage` | T_ChangingStatusType | 에너지 저장 상태 | NORMAL, DISCHARGE, CHARGE |
| `A_mainCannonFixStatus` | T_MainCannonFixStatusType | 주포 고정 상태 | NORMAL, FIX |
| `A_deckClearance` | T_DekClearanceType | 갑판 여유 상태 | INSIDE, OUTSIDE, ETC |
| `A_autoArmPositionComplement` | T_ArmPositionType | 자동 암 위치 보정 | NORMAL, ABNORMAL |
| `A_manualArmPositionComple` | T_ArmPositionType | 수동 암 위치 보정 (오타 보존) | NORMAL, ABNORMAL |
| `A_mainCannonRestoreComplement` | T_MainCannonReturnStatusType | 주포 복귀 보정 | STANDBY, RUNNING |
| `A_armSafetyMainCannonLock` | T_ArmSafetyMainCannonLock | 암 안전 주포 잠금 | NORMAL, ABNORMAL |
| `A_shutdown` | T_CannonDrivingDeviceShutdownType | 셧다운 상태 | NORMAL, UNKNOWN |

**Enum 포맷**: `"L_TypeName_VALUE"` (예: `"L_ChangingStatusType_NORMAL"`)

#### Enum 값과 BIT 상태 매핑

DemoApp은 내부 BIT 상태를 기반으로 Enum 값을 설정합니다:

| Enum 필드 | BIT 컴포넌트 | true일 때 | false일 때 |
|-----------|--------------|-----------|------------|
| `A_energyStorage` | `pbit.energyStorage` | DISCHARGE | NORMAL |
| `A_mainCannonFixStatus` | `pbit.roundMotor` | FIX | NORMAL |

나머지 Enum은 고정값 사용:
- `A_deckClearance`: `OUTSIDE`
- `A_autoArmPositionComplement`: `NORMAL`
- `A_manualArmPositionComple`: `NORMAL`
- `A_mainCannonRestoreComplement`: `RUNNING`
- `A_armSafetyMainCannonLock`: `NORMAL`
- `A_shutdown`: `UNKNOWN`

#### AgentUI 표시 방법

**실시간 게이지 (200Hz 갱신)**
```
┌─ 포구동 상태 ─────────────────────┐
│                                   │
│  방위각 (E1):     45.5°            │
│  ┌─────────────────────────────┐  │
│  │░░░░░░░░░░░░░█░░░░░░░░░░░░░│  │
│  └─────────────────────────────┘  │
│  0°           90°          180°   │
│                                   │
│  각속도:        2.3 deg/s          │
│  회전 자이로:   2.3 deg/s          │
│  상하 자이로:   0.5 deg/s          │
│                                   │
│  에너지:       ● NORMAL            │
│  주포 고정:     ● NORMAL            │
│  갑판 여유:     ● OUTSIDE           │
└───────────────────────────────────┘
```

**타임 시리즈 차트** (옵션)
```
각도
  │   /\        /\
90│  /  \      /  \
  │ /    \    /    \
 0├──────\/──/──────\─→ 시간
  0    5s   10s   15s
```

#### 동작 의도
- **목적**: 200Hz 고속 피드백으로 포구동 상태를 실시간 반영
- **시뮬레이션 로직**: 
  - 제어 명령(`A_drivingPosition`) → 목표 각도
  - 비례 제어 (P gain = 1.0) → 각속도 계산
  - 1ms마다 적분 → 현재 각도 업데이트
- **기대 동작**: 
  - AgentUI는 200Hz로 수신하여 부드러운 애니메이션
  - 제어 명령 송신 시 즉시 (5ms 이내) 피드백 반영
  - Enum 값 변화 시 색상/아이콘 변경

---

## AgentUI → DemoApp 수신 메시지

### 1. runBIT (IBIT 요청)

#### Topic 정보
- **Topic**: `P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT`
- **Type**: `P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT`
- **QoS**: `NonPeriodicEventProfile` (RELIABLE)
- **주기**: 비주기 (사용자 명령)

#### 메시지 구조 (AgentUI 송신)
```json
{
  "A_sourceID": {
    "A_resourceId": 99,
    "A_instanceId": 1
  },
  "A_timeOfDataGeneration": {
    "A_second": 1733900000,
    "A_nanoseconds": 0
  },
  "A_referenceNum": 1234,
  "A_type": "L_BITType_I_BIT"
}
```

#### 필드 설명

| 필드 | 타입 | 설명 | 예제 값 | 필수 |
|------|------|------|---------|------|
| `A_referenceNum` | int32 | 요청 참조 번호 (resultBIT에서 동일값 반환) | 1234 | ✓ |
| `A_type` | T_BITType | BIT 타입 (현재 I_BIT만 지원) | `"L_BITType_I_BIT"` | ✓ |

**BIT 타입 Enum**:
- `"L_BITType_P_BIT"`: PowerOn BIT (DemoApp 자동 수행)
- `"L_BITType_C_BIT"`: Continuous BIT (DemoApp 자동 수행)
- `"L_BITType_I_BIT"`: Initiated BIT (운용자 요청)

#### DemoApp 동작 로직

```c
void demo_msg_on_runbit(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    // 1. JSON 파싱
    int32_t referenceNum = json["A_referenceNum"];
    const char* type = json["A_type"];
    
    // 2. I_BIT 타입 검증
    if (strcmp(type, "L_BITType_I_BIT") != 0) {
        printf("ERROR: Unsupported BIT type: %s\n", type);
        return;
    }
    
    // 3. Run 상태에서만 수용
    if (ctx->current_state != DEMO_STATE_RUN) {
        printf("WARNING: Cannot start IBIT in state %s\n", 
               demo_state_name(ctx->current_state));
        return;
    }
    
    // 4. IBIT 시작
    ctx->ibit_reference_num = referenceNum;
    ctx->ibit_type = parse_bit_type(type);
    ctx->ibit_start_time = ctx->tick_count;
    ctx->ibit_running = true;
    
    // 5. 상태 전이: Run → IBitRunning
    enter_state(ctx, DEMO_STATE_IBIT_RUNNING);
    
    // 6. 3초(3000ms) 대기 후 resultBIT 송신
    // (타이머에서 tick_count 확인)
}
```

#### AgentUI 송신 절차

1. **사용자 버튼 클릭**: "IBIT 시작"
2. **참조 번호 생성**: 고유한 int32 값 (예: 타임스탬프 기반)
3. **runBIT 송신**: 위 JSON 포맷
4. **타임아웃 설정**: 5초 (3초 대기 + 2초 여유)
5. **resultBIT 대기**: `A_referenceNum` 일치 확인
6. **결과 표시**: 다이얼로그 팝업

**타임아웃 처리**:
```javascript
// AgentUI 예제 코드
let referenceNum = Date.now() & 0x7FFFFFFF;
let timeout = setTimeout(() => {
  showError("IBIT 타임아웃: 응답 없음");
}, 5000);

sendRunBIT(referenceNum);

onResultBIT((msg) => {
  if (msg.A_referenceNum === referenceNum) {
    clearTimeout(timeout);
    showIBITResult(msg);
  }
});
```

---

### 2. Actuator Control (제어 명령)

#### Topic 정보
- **Topic**: `P_NSTEL__C_CannonDrivingDevice_commandDriving`
- **Type**: `P_NSTEL::C_CannonDrivingDevice_commandDriving`
- **QoS**: `HighFreqPeriodicProfile` (200Hz, BEST_EFFORT)
- **주기**: 권장 200Hz (최소 10Hz)

#### 메시지 구조 (AgentUI 송신)
```json
{
  "A_sourceID": {...},
  "A_timeOfDataGeneration": {...},
  "A_drivingPosition": 90.0,
  "A_upDownPosition": 15.0,
  "A_roundAngleVelocity": 0.0,
  "A_upDownAngleVelocity": 0.0,
  "A_cannonUpDownAngle": 0.0,
  "A_topRelativeAngle": 0.0,
  "A_operationMode": "L_OperationModeType_NORMAL",
  "A_parm": "L_OnOffType_OFF",
  "A_targetDesingation": "L_TargetAllotType_ETC",
  "A_cannonSafetyDevice": "L_CannonSafetyDeviceType_NORMAL",
  "A_aim": "L_AimType_NONE",
  "A_aimStop": "L_BoolType_FALSE",
  "A_mainCannonLock": "L_OnOffType_OFF",
  "A_stopShutdown": "L_OnOffType_OFF"
}
```

#### 필드 설명

**Float 제어 필드 (6개)**:

| 필드 | 타입 | 단위 | 설명 | 사용 모드 |
|------|------|------|------|----------|
| `A_drivingPosition` | float | deg | 목표 방위각 | 위치 제어 |
| `A_upDownPosition` | float | deg | 목표 고각 | 위치 제어 |
| `A_roundAngleVelocity` | float | deg/s | 방위 각속도 명령 | 속도 제어 |
| `A_upDownAngleVelocity` | float | deg/s | 고각 각속도 명령 | 속도 제어 |
| `A_cannonUpDownAngle` | float | deg | 포신 상하각 | 미사용 |
| `A_topRelativeAngle` | float | deg | 상단 상대각 | 미사용 |

**Enum 제어 필드 (8개)**:

| 필드 | 타입 | 설명 | 기본값 |
|------|------|------|--------|
| `A_operationMode` | T_OperationModeType | 운용 모드 | NORMAL |
| `A_parm` | T_OnOffType | PARM 상태 | OFF |
| `A_targetDesingation` | T_TargetAllotType | 목표 지정 | ETC |
| `A_cannonSafetyDevice` | T_CannonSafetyDeviceType | 포 안전장치 | NORMAL |
| `A_aim` | T_AimType | 조준 타입 | NONE |
| `A_aimStop` | T_BoolType | 조준 정지 | FALSE |
| `A_mainCannonLock` | T_OnOffType | 주포 잠금 | OFF |
| `A_stopShutdown` | T_OnOffType | 정지/셧다운 | OFF |

#### 제어 모드

**모드 1: 위치 제어** (기본 모드)
```json
{
  "A_drivingPosition": 90.0,      // 목표 90도
  "A_roundAngleVelocity": 0.0,    // 속도 명령 0 (위치 모드)
  ...
}
```

**DemoApp 동작**:
```c
float error = control.drivingPosition - signal.azAngle;  // 오차 계산
signal.e1AngleVelocity = error * 1.0f;  // 비례 제어 (P gain = 1.0)
signal.azAngle += signal.e1AngleVelocity * 0.001f;  // 1ms 적분
```

**결과**: 목표값(90도)으로 서서히 수렴

---

**모드 2: 속도 제어**
```json
{
  "A_drivingPosition": 0.0,          // 위치 명령 무시
  "A_roundAngleVelocity": 10.0,      // 10 deg/s 속도 명령
  ...
}
```

**DemoApp 동작**:
```c
signal.e1AngleVelocity = control.roundAngleVelocity;  // 속도 직접 설정
signal.azAngle += signal.e1AngleVelocity * 0.001f;  // 1ms 적분
```

**결과**: 일정 속도(10 deg/s)로 계속 회전

---

#### AgentUI 송신 패턴

**패턴 1: 조이스틱 제어** (속도 모드)
```javascript
// 조이스틱 X축: -1.0 ~ 1.0
let maxSpeed = 30.0;  // deg/s
let velocity = joystick.x * maxSpeed;

sendControl({
  A_roundAngleVelocity: velocity,
  A_drivingPosition: 0.0,  // 무시
  ...
});
```

**패턴 2: 각도 슬라이더** (위치 모드)
```javascript
// 슬라이더: 0 ~ 360
let targetAngle = slider.value;

sendControl({
  A_drivingPosition: targetAngle,
  A_roundAngleVelocity: 0.0,  // 위치 모드
  ...
});
```

**패턴 3: 정지 명령**
```javascript
sendControl({
  A_roundAngleVelocity: 0.0,
  A_upDownAngleVelocity: 0.0,
  A_drivingPosition: currentAngle,  // 현재 위치 유지
  ...
});
```

---

### 3. Vehicle Speed (차량 속도)

#### Topic 정보
- **Topic**: `P_NSTEL__C_VehicleSpeed`
- **Type**: `P_NSTEL::C_VehicleSpeed`
- **QoS**: `LowFreqVehicleProfile` (1Hz, RELIABLE)
- **주기**: 권장 1Hz

#### 메시지 구조 (AgentUI 송신)
```json
{
  "A_sourceID": {...},
  "A_timeOfDataGeneration": {...},
  "A_speed": 30.5
}
```

#### 필드 설명

| 필드 | 타입 | 단위 | 설명 | 범위 |
|------|------|------|------|------|
| `A_speed` | float | m/s | 차량 속도 | 0~50 |

#### DemoApp 동작 로직

```c
void demo_msg_on_vehicle_speed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    // 1. JSON 파싱
    float speed = json["A_speed"];
    
    // 2. 내부 상태 저장
    ctx->vehicle_speed = speed;
    
    // 3. 로그 출력
    printf("[DemoApp Msg] Vehicle speed received: %.2f m/s\n", speed);
    
    // 현재는 저장만 수행 (추가 처리 없음)
}
```

**현재 용도**: 
- 내부 상태 저장만 수행
- 향후 확장: 차량 속도에 따른 포 안정화 보정

#### AgentUI 송신 절차

**시나리오 1: GPS 데이터 연동**
```javascript
setInterval(() => {
  let speed = getGPSSpeed();  // 실제 차량 GPS
  sendVehicleSpeed(speed);
}, 1000);  // 1Hz
```

**시나리오 2: 시뮬레이션 모드**
```javascript
let simulatedSpeed = 0;
let acceleration = 2.0;  // m/s²

setInterval(() => {
  simulatedSpeed += acceleration * 1.0;  // 1초마다
  simulatedSpeed = Math.min(simulatedSpeed, 50);  // 최대 50 m/s
  sendVehicleSpeed(simulatedSpeed);
}, 1000);
```

---

## 시뮬레이션 동작 로직

### 타이머 구조

```
1ms Tick
  ↓
시뮬레이션 업데이트 (위치/속도 적분)
  ↓
5ms 경과? → Actuator Signal 송신 (200Hz)
  ↓
1000ms 경과? → CBIT 송신 (1Hz)
  ↓
IBIT 수행 중? → 3000ms 경과 시 resultBIT 송신
```

**코드 위치**: `demo_app_timer.c:demo_timer_tick()`

---

### 위치 제어 시뮬레이션

#### 입력
- `control.drivingPosition`: 목표 각도 (AgentUI 송신)
- `signal.azAngle`: 현재 각도 (내부 상태)

#### 처리 (1ms마다)
```c
const float dt = 0.001f;  // 1ms

// 오차 계산
float error = control.drivingPosition - signal.azAngle;

// 비례 제어 (P 제어)
signal.e1AngleVelocity = error * 1.0f;  // P gain = 1.0

// 적분 (위치 업데이트)
signal.azAngle += signal.e1AngleVelocity * dt;

// 자이로 값 업데이트
signal.roundGyro = signal.e1AngleVelocity;
```

#### 출력 (5ms마다)
- `signal.azAngle`: 현재 각도
- `signal.e1AngleVelocity`: 현재 각속도
-- `signal.roundGyro`: 자이로 값 (각속도와 동일)

---

### 속도 제어 시뮬레이션

#### 입력
- `control.roundAngleVelocity`: 목표 각속도 (AgentUI 송신)

#### 처리 (1ms마다)
```c
const float dt = 0.001f;

// 속도 직접 설정
signal.e1AngleVelocity = control.roundAngleVelocity;

// 적분 (위치 업데이트)
signal.azAngle += signal.e1AngleVelocity * dt;

// 자이로 값 업데이트
signal.roundGyro = signal.e1AngleVelocity;
```

#### 출력 (5ms마다)
- `signal.azAngle`: 계속 증가/감소
- `signal.e1AngleVelocity`: 목표 속도
-- `signal.roundGyro`: 목표 속도

---

### Fault → Enum 매핑

#### 로직
```c
// BIT 상태 → Enum 변환
signal.energyStorage = pbit.energyStorage ? 
    L_ChangingStatusType_DISCHARGE : L_ChangingStatusType_NORMAL;

signal.mainCannonFixStatus = pbit.roundMotor ? 
    L_MainCannonFixStatusType_FIX : L_MainCannonFixStatusType_NORMAL;
```

#### 동작 예제
```
초기 상태:
  pbit.energyStorage = true (정상)
  → signal.energyStorage = "L_ChangingStatusType_DISCHARGE"

Fault 주입 (power):
  pbit.energyStorage = false (고장)
  → signal.energyStorage = "L_ChangingStatusType_NORMAL"
```

**주의**: 코드에서 사용되는 매핑은 다음과 같이 구현되어 있습니다 — 내부 PBIT 값이 `L_BITResultType_NORMAL`(정상)일 때 일부 신호(enum)로는 특수 상태(`L_ChangingStatusType_DISCHARGE` 또는 `L_MainCannonFixStatusType_FIX`)로 매핑됩니다. 이 동작은 현재 구현 규약이며, 직관과 다르므로 사양 담당자에게 의도 확인을 권장합니다.

---

## AgentUI 구현 가이드

### 메시지 수신 처리

#### PBIT 수신
```javascript
agent.subscribe("P_NSTEL__C_CannonDrivingDevice_PowerOnBIT", (msg) => {
  // 1. 타임스탬프 저장
  lastPBIT = {
    time: new Date(),
    data: msg
  };
  
  // 2. 상태 표시
  updatePBITIndicator(msg);
  
  // 3. 고장 감지
  let faults = [];
  if (!msg.A_upDownMotor) faults.push("상하 모터");
  if (!msg.A_roundMotor) faults.push("회전 모터");
  
  if (faults.length > 0) {
    showWarning("PBIT 고장: " + faults.join(", "));
  }
});
```

---

#### CBIT 수신 (1Hz)
```javascript
agent.subscribe("P_NSTEL__C_CannonDrivingDevice_PBIT", (msg) => {
  // 1. 실시간 상태 갱신
  updateSystemStatus(msg);
  
  // 2. 변화 감지
  if (previousCBIT) {
    detectChanges(previousCBIT, msg);
  }
  
  // 3. 이력 저장
  cbitHistory.push({
    time: new Date(),
    data: msg
  });
  
  // 4. 그래프 업데이트
  updateTimelineChart(cbitHistory);
  
  previousCBIT = msg;
});

function detectChanges(prev, curr) {
  // 고장 발생 감지
  if (prev.A_roundMotor && !curr.A_roundMotor) {
    showAlert("회전 모터 고장 발생!");
    playAlertSound();
  }
  
  // 복구 감지
  if (!prev.A_roundMotor && curr.A_roundMotor) {
    showInfo("회전 모터 복구됨");
  }
}
```

---

#### resultBIT 수신
```javascript
let pendingIBIT = {};

// IBIT 요청 함수
function requestIBIT() {
  let refNum = Date.now() & 0x7FFFFFFF;
  
  agent.publish("P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT", {
    A_referenceNum: refNum,
    A_type: "L_BITType_I_BIT",
    ...
  });
  
  // 타임아웃 설정
  pendingIBIT[refNum] = {
    startTime: Date.now(),
    timeout: setTimeout(() => {
      delete pendingIBIT[refNum];
      showError("IBIT 타임아웃 (5초 초과)");
    }, 5000)
  };
  
  showProgress("IBIT 수행 중... (3초 예상)");
}

// resultBIT 수신
agent.subscribe("P_NSTEL__C_CannonDrivingDevice_IBIT", (msg) => {
  let refNum = msg.A_referenceNum;
  
  if (pendingIBIT[refNum]) {
    let elapsed = Date.now() - pendingIBIT[refNum].startTime;
    clearTimeout(pendingIBIT[refNum].timeout);
    delete pendingIBIT[refNum];
    
    hideProgress();
    showIBITResult(msg, elapsed);
  }
});
```

---

#### Actuator Signal 수신 (200Hz)
```javascript
let frameCount = 0;

agent.subscribe("P_NSTEL__C_CannonDrivingDevice_Signal", (msg) => {
  frameCount++;
  
  // 1. 게이지 업데이트 (매 프레임)
  updateAngleGauge(msg.A_azAngle);
  updateVelocityGauge(msg.A_e1AngleVelocity);
  
  // 2. 차트 업데이트 (10Hz로 다운샘플링)
  if (frameCount % 20 === 0) {
    addToChart(msg.A_azAngle, msg.A_e1AngleVelocity);
  }
  
  // 3. Enum 상태 표시
  updateStatusIndicator("에너지", msg.A_energyStorage);
  updateStatusIndicator("주포 고정", msg.A_mainCannonFixStatus);
});
```

**성능 최적화**:
- 200Hz 전체 처리: 게이지, 숫자 표시
- 10Hz 다운샘플링: 차트, 이력 저장
- 1Hz 다운샘플링: 통계, 로그

---

### 메시지 송신 처리

#### Actuator Control 송신
```javascript
// 조이스틱 입력 처리 (60Hz)
setInterval(() => {
  let control = {
    A_sourceID: { A_resourceId: 99, A_instanceId: 1 },
    A_timeOfDataGeneration: getCurrentTimestamp(),
    A_drivingPosition: targetAngle,
    A_upDownPosition: targetElevation,
    A_roundAngleVelocity: joystick.x * 30.0,  // -30 ~ +30 deg/s
    A_upDownAngleVelocity: joystick.y * 10.0,  // -10 ~ +10 deg/s
    A_cannonUpDownAngle: 0.0,
    A_topRelativeAngle: 0.0,
    A_operationMode: "L_OperationModeType_NORMAL",
    A_parm: "L_OnOffType_OFF",
    A_targetDesingation: "L_TargetAllotType_ETC",
    A_cannonSafetyDevice: "L_CannonSafetyDeviceType_NORMAL",
    A_aim: "L_AimType_NONE",
    A_aimStop: "L_BoolType_FALSE",
    A_mainCannonLock: "L_OnOffType_OFF",
    A_stopShutdown: "L_OnOffType_OFF"
  };
  
  agent.publish("P_NSTEL__C_CannonDrivingDevice_commandDriving", control);
}, 16);  // 60Hz (권장: 10Hz ~ 200Hz)
```

---

#### Vehicle Speed 송신
```javascript
// GPS 데이터 기반 (1Hz)
setInterval(() => {
  let speed = getVehicleSpeed();  // GPS, OBD, 시뮬레이션 등
  
  agent.publish("P_NSTEL__C_VehicleSpeed", {
    A_sourceID: { A_resourceId: 99, A_instanceId: 1 },
    A_timeOfDataGeneration: getCurrentTimestamp(),
    A_speed: speed
  });
}, 1000);
```

---

### 타임스탬프 생성

```javascript
function getCurrentTimestamp() {
  let now = Date.now();
  let seconds = Math.floor(now / 1000);
  let nanoseconds = (now % 1000) * 1000000;
  
  return {
    A_second: seconds,
    A_nanoseconds: nanoseconds
  };
}
```

---

## 테스트 시나리오

### 시나리오 1: 기본 초기화

#### 절차
1. DemoApp 시작
2. AgentUI에서 PBIT 수신 대기 (5초 이내)
3. PBIT 검증: 12개 컴포넌트 모두 "L_BITResultType_NORMAL"
4. CBIT 수신 시작 (1Hz)

#### 검증
- ✅ PBIT 1개 수신
- ✅ CBIT 매초 1개씩 수신
- ✅ Actuator Signal 초당 200개 수신

---

### 시나리오 2: IBIT 요청/응답

#### 절차
1. AgentUI에서 "IBIT 시작" 클릭
2. runBIT 송신 (referenceNum=1234)
3. 3초 대기 (진행 표시)
4. resultBIT 수신 (referenceNum=1234 확인)
5. 결과 다이얼로그 표시

#### 검증
- ✅ referenceNum 일치
- ✅ 3초 ± 0.1초 이내 응답
- ✅ 12개 컴포넌트 상태 확인

---

### 시나리오 3: 위치 제어

#### 절차
1. AgentUI 슬라이더를 90도로 설정
2. Actuator Control 송신 (drivingPosition=90.0)
3. Actuator Signal 모니터링
4. `A_azAngle`이 0 → 90도로 수렴 확인

#### 검증
- ✅ 각도 수렴 (90도 ± 0.5도)
- ✅ 속도 감소 (오버슈트 없음)
- ✅ 응답 시간 < 5초

---

### 시나리오 4: Fault Injection

#### 절차
1. DemoApp에서 Fault 주입: `round` (회전 모터)
2. AgentUI CBIT 모니터링
3. `A_roundMotor`: `true` → `false` 변화 확인
4. `A_mainCannonFixStatus`: `NORMAL` → `FIX` 변화 확인
5. Fault 해제
6. 정상 복구 확인

#### 검증
- ✅ 1초 이내 고장 감지
- ✅ Enum 값 변경 확인
- ✅ 복구 후 정상 상태

---

## 부록

### Enum 값 전체 목록

#### T_BITType
```
L_BITType_P_BIT
L_BITType_C_BIT
L_BITType_I_BIT
```

#### T_OperationModeType
```
L_OperationModeType_NORMAL
L_OperationModeType_DEGRADED
L_OperationModeType_EMERGENCY
```

#### T_OnOffType
```
L_OnOffType_ON
L_OnOffType_OFF
```

#### T_ChangingStatusType
```
L_ChangingStatusType_NORMAL
L_ChangingStatusType_DISCHARGE
L_ChangingStatusType_CHARGE
```

#### T_MainCannonFixStatusType
```
L_MainCannonFixStatusType_NORMAL
L_MainCannonFixStatusType_FIX
```

#### T_DekClearanceType
```
L_DekClearanceType_INSIDE
L_DekClearanceType_OUTSIDE
L_DekClearanceType_ETC
```

#### T_ArmPositionType
```
L_ArmPositionType_NORMAL
L_ArmPositionType_ABNORMAL
```

#### T_MainCannonReturnStatusType
```
L_MainCannonReturnStatusType_STANDBY
L_MainCannonReturnStatusType_RUNNING
```

#### T_ArmSafetyMainCannonLock
```
L_ArmSafetyMainCannonLock_NORMAL
L_ArmSafetyMainCannonLock_ABNORMAL
```

#### T_CannonDrivingDeviceShutdownType
```
L_CannonDrivingDeviceShutdownType_NORMAL
L_CannonDrivingDeviceShutdownType_UNKNOWN
```

#### T_BoolType
```
L_BoolType_TRUE
L_BoolType_FALSE
```

---

### 메시지 크기 추정

| 메시지 | 필드 수 | JSON 크기 (대략) |
|--------|---------|------------------|
| PBIT | 14 | ~500 bytes |
| CBIT | 17 | ~600 bytes |
| resultBIT | 14 | ~500 bytes |
| Actuator Signal | 14 | ~700 bytes (Enum 문자열) |
| runBIT | 4 | ~200 bytes |
| Actuator Control | 16 | ~800 bytes |
| Vehicle Speed | 3 | ~150 bytes |

**대역폭 추정**:
- Actuator Signal (200Hz): ~140 KB/s
- CBIT (1Hz): ~0.6 KB/s
- 총 송신: ~141 KB/s
- 총 수신: ~100 KB/s (Control 200Hz 가정)

---

### 필드 표준화

문서와 코드 모두 RefDoc XML 스키마를 기준으로 표준화되어 있습니다. 주요 자이로 필드:

| 필드 | 사용처 |
|------|--------|
| `A_baseGyro` | PBIT, CBIT, IBIT |
| `A_topForwardGyro` | PBIT, CBIT, IBIT |
| `A_vehicleForwardGyro` | PBIT, CBIT, IBIT |

---

## 결론

본 규격서는 DemoApp과 AgentUI 간의 **7개 DDS 메시지** 송수신 인터페이스를 정의합니다.

**핵심 요약**:
1. **송신 메시지**: PBIT(1회), CBIT(1Hz), resultBIT(이벤트), Signal(200Hz)
2. **수신 메시지**: runBIT(이벤트), Control(60Hz 권장), Speed(1Hz)
3. **제어 모드**: 위치 제어 (비례), 속도 제어 (직접)
4. **Fault 매핑**: BIT 상태 → Signal Enum 값
5. **IBIT 프로토콜**: referenceNum 기반 요청-응답

AgentUI 개발 시 본 규격을 준수하면 DemoApp과 정상적으로 연동됩니다.
