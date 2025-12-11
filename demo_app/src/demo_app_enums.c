/*
 * demo_app_enums.c - Enum Conversion Implementation
 */

#include "../include/demo_app_enums.h"
#include <string.h>

/* ========================================================================
 * Enum Parsing Functions (JSON String -> Enum Value)
 * ======================================================================== */

T_BITType parse_bit_type(const char* str) {
    if (!str) return L_BITType_C_BIT;
    
    if (strstr(str, "P_BIT")) return L_BITType_P_BIT;
    if (strstr(str, "I_BIT")) return L_BITType_I_BIT;
    if (strstr(str, "C_BIT")) return L_BITType_C_BIT;
    
    return L_BITType_C_BIT;  // default
}

T_OperationModeType parse_operation_mode(const char* str) {
    if (!str) return L_OperationModeType_NORMAL;
    
    if (strstr(str, "EMER_GENCY")) return L_OperationModeType_EMER_GENCY;
    if (strstr(str, "MANUAL")) return L_OperationModeType_MANUAL;
    if (strstr(str, "NORMAL")) return L_OperationModeType_NORMAL;
    
    return L_OperationModeType_NORMAL;
}

T_OnOffType parse_onoff_type(const char* str) {
    if (!str) return L_OnOffType_OFF;
    
    if (strstr(str, "ON")) return L_OnOffType_ON;
    if (strstr(str, "OFF")) return L_OnOffType_OFF;
    
    return L_OnOffType_OFF;
}

T_TargetAllotType parse_target_allot(const char* str) {
    if (!str) return L_TargetAllotType_ALLOT;
    
    if (strstr(str, "ALLOT")) return L_TargetAllotType_ALLOT;
    if (strstr(str, "ETC")) return L_TargetAllotType_ETC;
    
    return L_TargetAllotType_ALLOT;
}

T_ArmPositionLockType parse_arm_position_lock(const char* str) {
    if (!str) return L_ArmPositionLockType_RELEASE;
    
    if (strstr(str, "LOCK")) return L_ArmPositionLockType_LOCK;
    if (strstr(str, "RELEASE")) return L_ArmPositionLockType_RELEASE;
    
    return L_ArmPositionLockType_RELEASE;
}

T_MainCannonReturnType parse_main_cannon_return(const char* str) {
    if (!str) return L_MainCannonReturnType_RELEASE;
    
    if (strstr(str, "COMMAND")) return L_MainCannonReturnType_COMMAND;
    if (strstr(str, "RELEASE")) return L_MainCannonReturnType_RELEASE;
    
    return L_MainCannonReturnType_RELEASE;
}

T_MainCannonFixType parse_main_cannon_fix(const char* str) {
    if (!str) return L_MainCannonFixType_RELEASE;
    
    if (strstr(str, "COMMAND")) return L_MainCannonFixType_COMMAND;
    if (strstr(str, "RELEASE")) return L_MainCannonFixType_RELEASE;
    
    return L_MainCannonFixType_RELEASE;
}

T_EquipOpenLockType parse_equip_open_lock(const char* str) {
    if (!str) return L_EquipOpenLockType_LOCK;
    
    if (strstr(str, "OPEN")) return L_EquipOpenLockType_OPEN;
    if (strstr(str, "LOCK")) return L_EquipOpenLockType_LOCK;
    
    return L_EquipOpenLockType_LOCK;
}

/* ========================================================================
 * Enum Formatting Functions (Enum Value -> JSON String)
 * ======================================================================== */

const char* format_bit_type(T_BITType type) {
    switch(type) {
        case L_BITType_C_BIT: return "L_BITType_C_BIT";
        case L_BITType_P_BIT: return "L_BITType_P_BIT";
        case L_BITType_I_BIT: return "L_BITType_I_BIT";
        default: return "L_BITType_C_BIT";
    }
}

const char* format_operation_mode(T_OperationModeType mode) {
    switch(mode) {
        case L_OperationModeType_NORMAL: return "L_OperationModeType_NORMAL";
        case L_OperationModeType_EMER_GENCY: return "L_OperationModeType_EMER_GENCY";
        case L_OperationModeType_MANUAL: return "L_OperationModeType_MANUAL";
        default: return "L_OperationModeType_NORMAL";
    }
}

const char* format_onoff_type(T_OnOffType type) {
    switch(type) {
        case L_OnOffType_ON: return "L_OnOffType_ON";
        case L_OnOffType_OFF: return "L_OnOffType_OFF";
        default: return "L_OnOffType_OFF";
    }
}

const char* format_target_allot(T_TargetAllotType type) {
    switch(type) {
        case L_TargetAllotType_ALLOT: return "L_TargetAllotType_ALLOT";
        case L_TargetAllotType_ETC: return "L_TargetAllotType_ETC";
        default: return "L_TargetAllotType_ALLOT";
    }
}

const char* format_arm_position_lock(T_ArmPositionLockType type) {
    switch(type) {
        case L_ArmPositionLockType_RELEASE: return "L_ArmPositionLockType_RELEASE";
        case L_ArmPositionLockType_LOCK: return "L_ArmPositionLockType_LOCK";
        default: return "L_ArmPositionLockType_RELEASE";
    }
}

const char* format_main_cannon_return(T_MainCannonReturnType type) {
    switch(type) {
        case L_MainCannonReturnType_RELEASE: return "L_MainCannonReturnType_RELEASE";
        case L_MainCannonReturnType_COMMAND: return "L_MainCannonReturnType_COMMAND";
        default: return "L_MainCannonReturnType_RELEASE";
    }
}

const char* format_main_cannon_fix(T_MainCannonFixType type) {
    switch(type) {
        case L_MainCannonFixType_RELEASE: return "L_MainCannonFixType_RELEASE";
        case L_MainCannonFixType_COMMAND: return "L_MainCannonFixType_COMMAND";
        default: return "L_MainCannonFixType_RELEASE";
    }
}

const char* format_equip_open_lock(T_EquipOpenLockType type) {
    switch(type) {
        case L_EquipOpenLockType_LOCK: return "L_EquipOpenLockType_LOCK";
        case L_EquipOpenLockType_OPEN: return "L_EquipOpenLockType_OPEN";
        default: return "L_EquipOpenLockType_LOCK";
    }
}

const char* format_changing_status(T_ChangingStatusType status) {
    switch(status) {
        case L_ChangingStatusType_NORMAL: return "L_ChangingStatusType_NORMAL";
        case L_ChangingStatusType_DISCHARGE: return "L_ChangingStatusType_DISCHARGE";
        default: return "L_ChangingStatusType_NORMAL";
    }
}

const char* format_dek_clearance(T_DekClearanceType type) {
    switch(type) {
        case L_DekClearanceType_OUTSIDE: return "L_DekClearanceType_OUTSIDE";
        case L_DekClearanceType_RUNNING: return "L_DekClearanceType_RUNNING";
        default: return "L_DekClearanceType_OUTSIDE";
    }
}

const char* format_arm_position(T_ArmPositionType type) {
    switch(type) {
        case L_ArmPositionType_NORMAL: return "L_ArmPositionType_NORMAL";
        case L_ArmPositionType_MANUAL: return "L_ArmPositionType_MANUAL";
        default: return "L_ArmPositionType_NORMAL";
    }
}

const char* format_main_cannon_fix_status(T_MainCannonFixStatusType status) {
    switch(status) {
        case L_MainCannonFixStatusType_NORMAL: return "L_MainCannonFixStatusType_NORMAL";
        case L_MainCannonFixStatusType_FIX: return "L_MainCannonFixStatusType_FIX";
        default: return "L_MainCannonFixStatusType_NORMAL";
    }
}

const char* format_main_cannon_return_status(T_MainCannonReturnStatusType status) {
    switch(status) {
        case L_MainCannonReturnStatusType_RUNNING: return "L_MainCannonReturnStatusType_RUNNING";
        case L_MainCannonReturnStatusType_COMPLETE: return "L_MainCannonReturnStatusType_COMPLETE";
        default: return "L_MainCannonReturnStatusType_RUNNING";
    }
}

const char* format_arm_safety_lock(T_ArmSafetyMainCannonLock lock) {
    switch(lock) {
        case L_ArmSafetyMainCannonLock_NORMAL: return "L_ArmSafetyMainCannonLock_NORMAL";
        case L_ArmSafetyMainCannonLock_COMPLETE: return "L_ArmSafetyMainCannonLock_COMPLETE";
        default: return "L_ArmSafetyMainCannonLock_NORMAL";
    }
}

const char* format_shutdown_type(T_CannonDrivingDeviceShutdownType type) {
    switch(type) {
        case L_CannonDrivingDeviceShutdownType_UNKNOWN: return "L_CannonDrivingDeviceShutdownType_UNKNOWN";
        case L_CannonDrivingDeviceShutdownType_SHUTDOWN: return "L_CannonDrivingDeviceShutdownType_SHUTDOWN";
        default: return "L_CannonDrivingDeviceShutdownType_UNKNOWN";
    }
}
