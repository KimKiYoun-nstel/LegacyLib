/**
 * @file idl_wire_Nstel_PSM.h
 * @brief Wire struct 정의 - Nstel_PSM.idl
 * @warning 이 파일은 자동 생성되었습니다. 직접 수정하지 마세요.
 */
#ifndef IDL_WIRE_NSTEL_PSM_H
#define IDL_WIRE_NSTEL_PSM_H

#include <stdint.h>
#include <stdbool.h>

#include "idl_wire_LDM_Common.h"

#pragma pack(push, 1)

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
} Wire_P_NSTEL_C_CannonDrivingDevice;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_Specification
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_powerOnBITSupported;
    bool A_continousBITSupported;
    bool A_initiatedBITSupported;
    int32_t A_continousBITInterval;
    uint32_t A_specifiedCannonDrivingDevices_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedCannonDrivingDevices_sourceID[100];
} Wire_P_NSTEL_C_CannonDrivingDevice_Specification;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_commandDriving
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_referenceNum;
    double A_roundPosition;
    double A_upDownPosition;
    double A_roundAngleVelocity;
    double A_upDownAngleVelocity;
    double A_cannonUpDownAngle;
    double A_topRelativeAngle;
    int32_t A_operationMode;  // enum P_NSTEL::T_OperationModeType
    int32_t A_parm;  // enum P_NSTEL::T_PalmModeType
    int32_t A_targetFix;  // enum P_NSTEL::T_TargetFixType
    int32_t A_autoArmPosition;  // enum P_NSTEL::T_ArmPositionType
    int32_t A_manualArmPosition;  // enum P_NSTEL::T_ArmPositionType
    int32_t A_mainCannonRestore;  // enum P_NSTEL::T_CannonRestoreType
    int32_t A_mainCannonFix;  // enum P_NSTEL::T_CannonFixType
    int32_t A_closureEquipOpenStatus;  // enum P_NSTEL::T_EquipOpenStatusType
} Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_Signal
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_Signal {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    double A_azAngleVelocity;
    double A_e1AngleVelocity;
    int32_t A_energyStorage;  // enum P_NSTEL::T_EnergyStorageStatusType
    int32_t A_mainCannonFixStatus;  // enum P_NSTEL::T_CannonFixType
    int32_t A_deckCleance;  // enum P_NSTEL::T_DeckClearanceType
    int32_t A_autoArmPositionComplement;  // enum P_NSTEL::T_CannonDrivingType
    int32_t A_manualArmPositionComplement;  // enum P_NSTEL::T_CannonDrivingType
    int32_t A_mainCannonRestoreComplement;  // enum P_NSTEL::T_CannonDrivingType
    int32_t A_armSafetyMainCannonLock;  // enum P_NSTEL::T_CannonLockType
    int32_t A_shutdown;  // enum P_NSTEL::T_ShutdownType
    double A_roundGyro;
    double A_upDownGyro;
} Wire_P_NSTEL_C_CannonDrivingDevice_Signal;

/**
 * @brief Wire struct for P_NSTEL::C_VehicleSpeed
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_VehicleSpeed {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    double A_value;
} Wire_P_NSTEL_C_VehicleSpeed;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_PowerOnBIT
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_cannonDrivingDevice_sourceID;
    bool A_BITRunning;
    int32_t A_upDownMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_upDownAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_baseGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_topForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_vehicleForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_powerController;  // enum P_NSTEL::T_BITResultType
    int32_t A_energyStorage;  // enum P_NSTEL::T_BITResultType
    int32_t A_directPower;  // enum P_NSTEL::T_BITResultType
    int32_t A_cableLoop;  // enum P_NSTEL::T_BITResultType
} Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_PBIT
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_PBIT {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_cannonDrivingDevice_sourceID;
    int32_t A_upDownMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_upDownAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_baseGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_topForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_vehicleForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_powerController;  // enum P_NSTEL::T_BITResultType
    int32_t A_energyStorage;  // enum P_NSTEL::T_BITResultType
    int32_t A_directPower;  // enum P_NSTEL::T_BITResultType
    int32_t A_cableLoop;  // enum P_NSTEL::T_BITResultType
    int32_t A_upDownPark;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundPark;  // enum P_NSTEL::T_BITResultType
    int32_t A_mainCannonLock;  // enum P_NSTEL::T_BITResultType
    int32_t A_controllerNetwork;  // enum P_NSTEL::T_BITResultType
} Wire_P_NSTEL_C_CannonDrivingDevice_PBIT;

/**
 * @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_IBIT
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_IBIT {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_cannonDrivingDevice_sourceID;
    int32_t A_referenceNum;
    bool A_BITRunning;
    int32_t A_upDownMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundMotor;  // enum P_NSTEL::T_BITResultType
    int32_t A_upDownAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_roundAmp;  // enum P_NSTEL::T_BITResultType
    int32_t A_baseGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_topForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_vehicleForwardGyro;  // enum P_NSTEL::T_BITResultType
    int32_t A_powerController;  // enum P_NSTEL::T_BITResultType
    int32_t A_energyStorage;  // enum P_NSTEL::T_BITResultType
    int32_t A_directPower;  // enum P_NSTEL::T_BITResultType
    int32_t A_cableLoop;  // enum P_NSTEL::T_BITResultType
} Wire_P_NSTEL_C_CannonDrivingDevice_IBIT;

#pragma pack(pop)

#endif /* IDL_WIRE_NSTEL_PSM_H */
