/*
 * demo_app_enums.h - Enum Type Definitions and Conversion Functions
 * 
 * Based on Nstel_PSM.xml and Usage_And_Condition_Monitoring_PSM.xml
 */

#ifndef DEMO_APP_ENUMS_H
#define DEMO_APP_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Enum Type Definitions from XML Schema
 * ======================================================================== */

// P_UCMS::T_BITType
typedef enum {
    L_BITType_C_BIT = 0,
    L_BITType_P_BIT = 1,
    L_BITType_I_BIT = 2
} T_BITType;

// P_NSTEL::T_OperationModeType
typedef enum {
    L_OperationModeType_NORMAL = 0,
    L_OperationModeType_EMERGENCY = 1,
    L_OperationModeType_MANUAL = 2
} T_OperationModeType;

// P_NSTEL::T_PalmModeType
typedef enum {
    L_PalmModeType_ON = 0,
    L_PalmModeType_OFF = 1
} T_PalmModeType;

// P_NSTEL::T_TargetFixType
typedef enum {
    L_TargetFixType_FIXED = 0,
    L_TargetFixType_ETC = 1
} T_TargetFixType;

// P_NSTEL::T_OnOffType
typedef enum {
    L_OnOffType_ON = 0,
    L_OnOffType_OFF = 1
} T_OnOffType;

// P_NSTEL::T_TargetAllotType
typedef enum {
    L_TargetAllotType_ALLOT = 0,
    L_TargetAllotType_ETC = 1
} T_TargetAllotType;

// P_NSTEL::T_ArmPositionLockType
typedef enum {
    L_ArmPositionLockType_RELEASE = 0,
    L_ArmPositionLockType_LOCK = 1
} T_ArmPositionLockType;

// P_NSTEL::T_MainCannonReturnType
typedef enum {
    L_MainCannonReturnType_RELEASE = 0,
    L_MainCannonReturnType_COMMAND = 1
} T_MainCannonReturnType;

// P_NSTEL::T_MainCannonFixType
typedef enum {
    L_MainCannonFixType_RELEASE = 0,
    L_MainCannonFixType_COMMAND = 1
} T_MainCannonFixType;

// P_NSTEL::T_EquipOpenLockType
typedef enum {
    L_EquipOpenLockType_LOCK = 0,
    L_EquipOpenLockType_OPEN = 1
} T_EquipOpenLockType;

// P_NSTEL::T_ChangingStatusType
typedef enum {
    L_ChangingStatusType_NORMAL = 0,
    L_ChangingStatusType_DISCHARGE = 1
} T_ChangingStatusType;

// P_NSTEL::T_DekClearanceType
typedef enum {
    L_DekClearanceType_OUTSIDE = 0,
    L_DekClearanceType_RUNNING = 1
} T_DekClearanceType;

// P_NSTEL::T_ArmPositionType
typedef enum {
    L_ArmPositionType_NORMAL = 0,
    L_ArmPositionType_MANUAL = 1
} T_ArmPositionType;

// P_NSTEL::T_CannonRestoreType
typedef enum {
    L_CannonRestoreType_RELEASE = 0,
    L_CannonRestoreType_RESTORE = 1
} T_CannonRestoreType;

// P_NSTEL::T_CannonFixType
typedef enum {
    L_CannonFixType_RELEASE = 0,
    L_CannonFixType_FIX = 1
} T_CannonFixType;

// P_NSTEL::T_EquipOpenStatusType
typedef enum {
    L_EquipOpenStatusType_CLOSE = 0,
    L_EquipOpenStatusType_OPEN = 1
} T_EquipOpenStatusType;


// P_NSTEL::T_MainCannonFixStatusType
typedef enum {
    L_MainCannonFixStatusType_NORMAL = 0,
    L_MainCannonFixStatusType_FIX = 1
} T_MainCannonFixStatusType;

// P_NSTEL::T_MainCannonReturnStatusType
typedef enum {
    L_MainCannonReturnStatusType_RUNNING = 0,
    L_MainCannonReturnStatusType_COMPLETE = 1
} T_MainCannonReturnStatusType;

// P_NSTEL::T_ArmSafetyMainCannonLock
typedef enum {
    L_ArmSafetyMainCannonLock_NORMAL = 0,
    L_ArmSafetyMainCannonLock_COMPLETE = 1
} T_ArmSafetyMainCannonLock;

// P_NSTEL::T_CannonDrivingDeviceShutdownType
typedef enum {
    L_CannonDrivingDeviceShutdownType_UNKNOWN = 0,
    L_CannonDrivingDeviceShutdownType_SHUTDOWN = 1
} T_CannonDrivingDeviceShutdownType;

// P_NSTEL::T_CannonDrivingType
typedef enum {
    L_CannonDrivingType_DRIVING = 0,
    L_CannonDrivingType_DONE = 1
} T_CannonDrivingType;

// P_NSTEL::T_CannonLockType
typedef enum {
    L_CannonLockType_NORMAL = 0,
    L_CannonLockType_LOCKED = 1
} T_CannonLockType;

// P_NSTEL::T_DeckClearanceType
typedef enum {
    L_DeckClearanceType_OUT_OF_DECK = 0,
    L_DeckClearanceType_IN_DECK = 1
} T_DeckClearanceType;

/* ========================================================================
 * Enum Parsing Functions (JSON String -> Enum Value)
 * ======================================================================== */

T_BITType parse_bit_type(const char* str);
T_OperationModeType parse_operation_mode(const char* str);
T_OnOffType parse_onoff_type(const char* str);
T_TargetAllotType parse_target_allot(const char* str);
T_ArmPositionLockType parse_arm_position_lock(const char* str);
T_MainCannonReturnType parse_main_cannon_return(const char* str);
T_MainCannonFixType parse_main_cannon_fix(const char* str);
T_EquipOpenLockType parse_equip_open_lock(const char* str);

/* ========================================================================
 * Enum Formatting Functions (Enum Value -> JSON String)
 * ======================================================================== */

const char* format_bit_type(T_BITType type);
const char* format_bit_result(int ok);
const char* format_operation_mode(T_OperationModeType mode);
const char* format_onoff_type(T_OnOffType type);
const char* format_target_allot(T_TargetAllotType type);
const char* format_arm_position_lock(T_ArmPositionLockType type);
const char* format_main_cannon_return(T_MainCannonReturnType type);
const char* format_main_cannon_fix(T_MainCannonFixType type);
const char* format_equip_open_lock(T_EquipOpenLockType type);
const char* format_changing_status(T_ChangingStatusType status);
const char* format_energy_storage(T_ChangingStatusType status);
const char* format_dek_clearance(T_DekClearanceType type);
const char* format_arm_position(T_ArmPositionType type);
const char* format_main_cannon_fix_status(T_MainCannonFixStatusType status);
const char* format_main_cannon_return_status(T_MainCannonReturnStatusType status);
const char* format_arm_safety_lock(T_ArmSafetyMainCannonLock lock);
const char* format_shutdown_type(T_CannonDrivingDeviceShutdownType type);

/* Map internal status types to schema T_CannonDrivingType enumerator strings */
const char* format_cannon_driving_from_arm(T_ArmPositionType type);
const char* format_cannon_driving_from_return(T_MainCannonReturnStatusType status);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_APP_ENUMS_H */
