# Phase 4.5 설계 문서: 실제 JSON 스키마 적용

## 목표

Phase 1-4에서 간단한 데모용 JSON 구조로 구현했던 부분을 실제 XML 스키마 및 JSON 샘플에 맞춰 완전히 재작성

## 현재 vs 실제 비교

### 1. 공통 필드

**현재** (없음):
```json
{
  "deviceId": "CANNON_DRIVE_01",
  "timestamp": 12345
}
```

**실제** (XML 기반):
```json
{
  "A_sourceID": {
    "A_resourceId": 1,
    "A_instanceId": 1
  },
  "A_timeOfDataGeneration": {
    "A_second": 1,
    "A_nanoseconds": 1
  }
}
```

### 2. PBIT/CBIT/resultBIT

**현재** (3개 컴포넌트):
```c
fault_azimuth, fault_updown, fault_sensor
```

**실제** (12-15개 컴포넌트):
```
PBIT: 12개 boolean 필드
- A_upDownMotor, A_roundMotor
- A_upDownAmp, A_roundAmp
- A_baseGyro, A_topForwardGryro, A_vehicleForwardGyroi
- A_powerController, A_energyStorage
- A_directPower, A_cableLoop

CBIT: 15개 (PBIT + 3개)
- 위 12개 + A_upDownPark, A_round_Park, A_mainCannon_Lock, A_commFault

resultBIT: 12개 (PBIT와 동일)
```

### 3. Actuator Control

**현재**:
```c
azimuthCommand, updownCommand
azimuthVelocity, updownVelocity
operationMode
```

**실제**:
```
A_drivingPosition       (float32)
A_upDownPosition        (float32)
A_roundAngleVelocity    (float32)
A_upDownAngleVelocity   (float32)
A_cannonUpDownAngle     (float32)
A_topRelativeAngle      (float32)
A_operationMode         (enum)
A_parm                  (enum)
A_targetDesingation     (enum)
A_autoArmPosition       (enum)
A_manualArmPosition     (enum)
A_mainCannonRestore     (enum)
A_manCannonFix          (enum)
A_closeEquipOpenStatus  (enum)
```

### 4. Actuator Signal

**현재**:
```c
azimuthPosition, updownPosition
azimuthVelocity, updownVelocity
sensorStatus
```

**실제**:
```
A_azAngle               (float32)
A_e1AngleVelocity       (float32)
A_roundGyro             (float32)
A_upDownGyro            (float32)
A_energyStorage         (enum)
A_mainCannonFixStatus   (enum)
A_deckClearance         (enum)
A_autoArmPositionComplement (enum)
A_manualArmPositionComple (enum)
A_mainCannonRestoreComplement (enum)
A_armSafetyMainCannonLock (enum)
A_shutdown              (enum)
```

## Enum 타입 정의

### Nstel_PSM.xml에서 추출

```c
typedef enum {
    L_OperationModeType_NORMAL = 0,
    L_OperationModeType_EMER_GENCY = 1,
    L_OperationModeType_MANUAL = 2
} T_OperationModeType;

typedef enum {
    L_OnOffType_ON = 0,
    L_OnOffType_OFF = 1
} T_OnOffType;

typedef enum {
    L_TargetAllotType_ALLOT = 0,
    L_TargetAllotType_ETC = 1
} T_TargetAllotType;

typedef enum {
    L_ArmPositionLockType_RELEASE = 0,
    L_ArmPositionLockType_LOCK = 1
} T_ArmPositionLockType;

typedef enum {
    L_MainCannonReturnType_RELEASE = 0,
    L_MainCannonReturnType_COMMAND = 1
} T_MainCannonReturnType;

typedef enum {
    L_MainCannonFixType_RELEASE = 0,
    L_MainCannonFixType_COMMAND = 1
} T_MainCannonFixType;

typedef enum {
    L_EquipOpenLockType_LOCK = 0,
    L_EquipOpenLockType_OPEN = 1
} T_EquipOpenLockType;

typedef enum {
    L_ChangingStatusType_NORMAL = 0,
    L_ChangingStatusType_DISCHARGE = 1
} T_ChangingStatusType;

typedef enum {
    L_DekClearanceType_OUTSIDE = 0,
    L_DekClearanceType_RUNNING = 1
} T_DekClearanceType;

typedef enum {
    L_ArmPositionType_NORMAL = 0,
    L_ArmPositionType_MANUAL = 1
} T_ArmPositionType;

typedef enum {
    L_MainCannonFixStatusType_NORMAL = 0,
    L_MainCannonFixStatusType_FIX = 1
} T_MainCannonFixStatusType;

typedef enum {
    L_MainCannonReturnStatusType_RUNNING = 0,
    L_MainCannonReturnStatusType_COMPLETE = 1
} T_MainCannonReturnStatusType;

typedef enum {
    L_ArmSafetyMainCannonLock_NORMAL = 0,
    L_ArmSafetyMainCannonLock_COMPLETE = 1
} T_ArmSafetyMainCannonLock;

typedef enum {
    L_CannonDrivingDeviceShutdownType_UNKNOWN = 0,
    L_CannonDrivingDeviceShutdownType_SHUTDOWN = 1
} T_CannonDrivingDeviceShutdownType;
```

### UCMS에서 추출

```c
typedef enum {
    L_BITType_C_BIT = 0,
    L_BITType_P_BIT = 1,
    L_BITType_I_BIT = 2
} T_BITType;
```

## 구조체 재정의

### BITComponentState (12개 컴포넌트)

```c
typedef struct {
    bool upDownMotor;
    bool roundMotor;
    bool upDownAmp;
    bool roundAmp;
    bool baseGyro;
    bool topForwardGryro;      // 오타 주의: Gyro가 아닌 Gryro
    bool vehicleForwardGyro;   // PBIT는 Gyroi (오타), resultBIT는 Gyro
    bool powerController;      // CBIT는 powerController, resultBIT는 power_Controller
    bool energyStorage;
    bool directPower;
    bool cableLoop;
    bool bitRunning;           // PBIT/resultBIT에만 존재
} BITComponentState;
```

### CBITComponentState (15개 컴포넌트)

```c
typedef struct {
    BITComponentState base;    // 위 12개 포함
    // CBIT 추가 필드
    bool upDownPark;
    bool round_Park;
    bool mainCannon_Lock;
    bool commFault;
} CBITComponentState;
```

### ActuatorControlState

```c
typedef struct {
    // ID 필드
    T_IdentifierType recipientID;
    T_IdentifierType sourceID;
    T_DateTimeType timeOfDataGeneration;
    
    // Position/Velocity
    float drivingPosition;        // A_drivingPosition
    float upDownPosition;         // A_upDownPosition
    float roundAngleVelocity;     // A_roundAngleVelocity
    float upDownAngleVelocity;    // A_upDownAngleVelocity
    float cannonUpDownAngle;      // A_cannonUpDownAngle
    float topRelativeAngle;       // A_topRelativeAngle
    
    // Enum 필드
    T_OperationModeType operationMode;
    T_OnOffType parm;
    T_TargetAllotType targetDesingation;
    T_ArmPositionLockType autoArmPosition;
    T_ArmPositionLockType manualArmPosition;
    T_MainCannonReturnType mainCannonRestore;
    T_MainCannonFixType manCannonFix;
    T_EquipOpenLockType closeEquipOpenStatus;
    
    uint64_t last_update_time;
} ActuatorControlState;
```

### ActuatorSignalState

```c
typedef struct {
    // ID 필드
    T_IdentifierType recipientID;
    T_IdentifierType sourceID;
    T_DateTimeType timeOfDataGeneration;
    
    // Sensor values
    float azAngle;                // A_azAngle
    float e1AngleVelocity;        // A_e1AngleVelocity
    float roundGyro;              // A_roundGyro
    float upDownGyro;             // A_upDownGyro
    
    // Enum 상태
    T_ChangingStatusType energyStorage;
    T_MainCannonFixStatusType mainCannonFixStatus;
    T_DekClearanceType deckClearance;
    T_ArmPositionType autoArmPositionComplement;
    T_ArmPositionType manualArmPositionComple;
    T_MainCannonReturnStatusType mainCannonRestoreComplement;
    T_ArmSafetyMainCannonLock armSafetyMainCannonLock;
    T_CannonDrivingDeviceShutdownType shutdown;
} ActuatorSignalState;
```

### 공통 타입

```c
typedef struct {
    int32_t A_resourceId;
    int32_t A_instanceId;
} T_IdentifierType;

typedef struct {
    int64_t A_second;
    int32_t A_nanoseconds;
} T_DateTimeType;
```

## JSON 파싱/생성 전략

### 1. Enum 문자열 매핑

**파싱 (수신)**:
```c
// "L_OperationModeType_NORMAL" -> 0
int parse_operation_mode(const char* str) {
    if (strstr(str, "NORMAL")) return 0;
    if (strstr(str, "EMER_GENCY")) return 1;
    if (strstr(str, "MANUAL")) return 2;
    return 0;  // default
}
```

**생성 (송신)**:
```c
// 0 -> "L_OperationModeType_NORMAL"
const char* format_operation_mode(int mode) {
    switch(mode) {
        case 0: return "L_OperationModeType_NORMAL";
        case 1: return "L_OperationModeType_EMER_GENCY";
        case 2: return "L_OperationModeType_MANUAL";
        default: return "L_OperationModeType_NORMAL";
    }
}
```

### 2. 공통 필드 헬퍼

```c
// 모든 송신 메시지에 공통 필드 추가
void add_common_fields(char* json, size_t size, DemoAppContext* ctx) {
    snprintf(json, size,
        "\"A_sourceID\": {"
            "\"A_resourceId\": 1,"
            "\"A_instanceId\": 1"
        "},"
        "\"A_timeOfDataGeneration\": {"
            "\"A_second\": %lld,"
            "\"A_nanoseconds\": %d"
        "},",
        (long long)(ctx->tick_count / 1000),  // seconds
        (int)((ctx->tick_count % 1000) * 1000000)  // nanoseconds
    );
}
```

## 구현 순서

1. **demo_app.h**: Enum 타입 및 구조체 재정의
2. **demo_app_enums.h**: Enum 매핑 함수들 (새 파일)
3. **demo_app_msg.c**: 송신 메시지 재작성 (PBIT/CBIT/resultBIT/Signal)
4. **demo_app_msg.c**: 수신 파싱 재작성 (Control/Speed/runBIT)
5. **demo_app_core.c**: 고장 플래그 확장 (3개 -> 12개)

## 주의사항

### 오타/불일치

1. **PBIT**: `A_vehicleForwardGyroi` (Gyroi 오타)
2. **CBIT/resultBIT**: `A_vehicleForwardGyro` (Gyro 정상)
3. **CBIT**: `A_powerController`
4. **resultBIT**: `A_power_Controller` (언더스코어 위치 다름)
5. **Signal**: `A_manualArmPositionComple` (Complement가 아닌 Comple)

→ 샘플 JSON 그대로 따라야 함 (오타 포함)

### 필드 누락 방지

- PBIT: 12개 boolean + 3개 공통
- CBIT: 15개 boolean + 3개 공통
- resultBIT: 12개 boolean + 4개 공통 (referenceNum 추가)
- Control: 6개 float + 8개 enum + 3개 공통
- Signal: 4개 float + 8개 enum + 3개 공통

### 시뮬레이션 매핑

**기존 내부 변수** → **실제 필드**:
- `azimuth_position` → `A_azAngle`
- `updown_position` → (삭제, Signal에 없음)
- `azimuth_velocity` → `A_e1AngleVelocity`
- `azimuth_command` → `A_drivingPosition`
- `updown_command` → `A_upDownPosition`

## 테스트 계획

1. JSON 샘플 파일로 파싱 테스트
2. 생성된 JSON을 AgentUI로 검증
3. 모든 Enum 값 순회 테스트
4. 12개 컴포넌트 고장 주입 테스트

---

다음: Phase 4.5 구현 시작
