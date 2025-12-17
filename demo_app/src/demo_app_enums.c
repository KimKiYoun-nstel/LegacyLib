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
    
    if (strstr(str, "EMERGENCY")) return L_OperationModeType_EMERGENCY;
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

/* Format BIT result fields (T_BITResultType -> enumerator string)
 * Convention: L_BITResultType_NORMAL -> "L_BITResultType_NORMAL"
 */
const char* format_bit_result(T_BITResultType ok) {
    switch (ok) {
        case L_BITResultType_NORMAL: return "L_BITResultType_NORMAL";
        case L_BITResultType_ABNORMAL: return "L_BITResultType_ABNORMAL";
        default: return "L_BITResultType_NORMAL";
    }
}

const char* format_operation_mode(T_OperationModeType mode) {
    switch(mode) {
        case L_OperationModeType_NORMAL: return "L_OperationModeType_NORMAL";
        case L_OperationModeType_EMERGENCY: return "L_OperationModeType_EMERGENCY";
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

/* The XML schema uses a different enum name for energy storage status; format
 * functions for A_energyStorage must produce the schema enumerator names.
 */
const char* format_energy_storage(T_ChangingStatusType status) {
    switch(status) {
        case L_ChangingStatusType_NORMAL: return "L_EnergyStorageStatusType_NORMAL";
        case L_ChangingStatusType_DISCHARGE: return "L_EnergyStorageStatusType_DISCHARGE";
        default: return "L_EnergyStorageStatusType_NORMAL";
    }
}

const char* format_dek_clearance(T_DekClearanceType type) {
    /* Map internal names to schema enumerators */
    switch(type) {
        case L_DekClearanceType_OUTSIDE: return "L_DeckClearanceType_OUT_OF_DECK";
        case L_DekClearanceType_RUNNING: return "L_DeckClearanceType_IN_DECK";
        default: return "L_DeckClearanceType_OUT_OF_DECK";
    }
}

const char* format_arm_position(T_ArmPositionType type) {
    /* Align with XML: ArmPositionType uses RELEASE / DRIVING */
    switch(type) {
        case L_ArmPositionType_NORMAL: return "L_ArmPositionType_RELEASE";
        case L_ArmPositionType_MANUAL: return "L_ArmPositionType_DRIVING";
        default: return "L_ArmPositionType_RELEASE";
    }
}

const char* format_main_cannon_fix_status(T_MainCannonFixStatusType status) {
    /* Signal schema expects P_NSTEL::T_CannonFixType enumerators */
    switch(status) {
        case L_MainCannonFixStatusType_NORMAL: return "L_CannonFixType_RELEASE";
        case L_MainCannonFixStatusType_FIX: return "L_CannonFixType_FIX";
        default: return "L_CannonFixType_RELEASE";
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
    /* Schema expects P_NSTEL::T_CannonLockType enumerators */
    switch(lock) {
        case L_ArmSafetyMainCannonLock_NORMAL: return "L_CannonLockType_NORMAL";
        case L_ArmSafetyMainCannonLock_COMPLETE: return "L_CannonLockType_LOCKED";
        default: return "L_CannonLockType_NORMAL";
    }
}

const char* format_shutdown_type(T_CannonDrivingDeviceShutdownType type) {
    /* Map internal shutdown enum to schema's T_ShutdownType enumerator strings */
    switch(type) {
        case L_CannonDrivingDeviceShutdownType_UNKNOWN: return "L_ShutdownType_UNKNOWN";
        case L_CannonDrivingDeviceShutdownType_SHUTDOWN: return "L_ShutdownType_SHUTDOWN";
        default: return "L_ShutdownType_UNKNOWN";
    }
}

/* Map internal ArmPosition / ReturnStatus types to schema::T_CannonDrivingType */
const char* format_cannon_driving_from_arm(T_ArmPositionType type) {
    switch(type) {
        case L_ArmPositionType_NORMAL: return "L_CannonDrivingType_DRIVING";
        case L_ArmPositionType_MANUAL: return "L_CannonDrivingType_DONE";
        default: return "L_CannonDrivingType_DRIVING";
    }
}

const char* format_cannon_driving_from_return(T_MainCannonReturnStatusType status) {
    switch(status) {
        case L_MainCannonReturnStatusType_RUNNING: return "L_CannonDrivingType_DRIVING";
        case L_MainCannonReturnStatusType_COMPLETE: return "L_CannonDrivingType_DONE";
        default: return "L_CannonDrivingType_DRIVING";
    }
}
