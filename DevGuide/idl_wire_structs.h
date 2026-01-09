/**
 * @file idl_wire_structs.h
 * @brief 자동 생성된 wire struct 정의 (C 호환 POD)
 * @warning 이 파일은 자동 생성되었습니다. 직접 수정하지 마세요.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)

/** @brief Wire struct for C_AlarmMsg */
typedef struct Wire_C_AlarmMsg {
    int32_t level;
    char text[256];
} Wire_C_AlarmMsg;

/** @brief Wire struct for P_LDM_Common::T_IdentifierType */
typedef struct Wire_P_LDM_Common_T_IdentifierType {
    int32_t A_resourceId;
    int32_t A_instanceId;
} Wire_P_LDM_Common_T_IdentifierType;

/** @brief Wire struct for P_LDM_Common::T_DateTimeType */
typedef struct Wire_P_LDM_Common_T_DateTimeType {
    int64_t A_second;
    int32_t A_nanoseconds;
} Wire_P_LDM_Common_T_DateTimeType;

/** @brief Wire struct for P_Alarms_PSM::C_Crew_Role_In_Mission_State */
typedef struct Wire_P_Alarms_PSM_C_Crew_Role_In_Mission_State {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_crewRoleName[1];  // unresolved: nonBasic
    uint32_t A_relevantAlarmType_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_relevantAlarmType_sourceID[100];
    uint32_t A_missionState_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_missionState_sourceID[100];
} Wire_P_Alarms_PSM_C_Crew_Role_In_Mission_State;

/** @brief Wire struct for P_LDM_Common::T_DurationType */
typedef struct Wire_P_LDM_Common_T_DurationType {
    int32_t A_seconds;
    int32_t A_nanoseconds;
} Wire_P_LDM_Common_T_DurationType;

/** @brief Wire struct for P_Alarms_PSM::C_Alarm_Category_Specification */
typedef struct Wire_P_Alarms_PSM_C_Alarm_Category_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_alarmCategoryName;  // enum P_Alarms_PSM::T_AlarmCategoryType
    uint8_t A_alarmCategoryAbbreviation[1];  // unresolved: nonBasic
    bool A_isAutoAcknowledged;
    Wire_P_LDM_Common_T_DurationType A_automaticAcknowledgeTimeout;
    bool A_hideOnAcknowledge;
    bool A_isRepeated;
    Wire_P_LDM_Common_T_DurationType A_repeatTimeout;
    uint32_t A_categorisedConditionSpecification_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_categorisedConditionSpecification_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_notifyingToneSpecification_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_alarmCategory_sourceID;
} Wire_P_Alarms_PSM_C_Alarm_Category_Specification;

/** @brief Wire struct for P_Alarms_PSM::C_Mission_State_setMissionState */
typedef struct Wire_P_Alarms_PSM_C_Mission_State_setMissionState {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_missionState;  // enum P_Alarms_PSM::T_MissionStateType
    uint8_t A_missionStateName[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Mission_State_setMissionState;

/** @brief Wire struct for P_Alarms_PSM::C_Mission_State */
typedef struct Wire_P_Alarms_PSM_C_Mission_State {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_missionState;  // enum P_Alarms_PSM::T_MissionStateType
    uint8_t A_missionStateName[1];  // unresolved: nonBasic
    uint32_t A_crewRoleInMissionState_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_crewRoleInMissionState_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_ownPlatform_sourceID;
} Wire_P_Alarms_PSM_C_Mission_State;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm_acknowledgeAlarm */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm_acknowledgeAlarm {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Actual_Alarm_acknowledgeAlarm;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    Wire_P_LDM_Common_T_DateTimeType A_dateTimeRaised;
    int32_t A_alarmState;  // enum P_Alarms_PSM::T_Actual_Alarm_StateType
    Wire_P_LDM_Common_T_IdentifierType A_raisingCondition_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_alarmCategory_sourceID;
} Wire_P_Alarms_PSM_C_Actual_Alarm;

/** @brief Wire struct for P_Alarms_PSM::C_Alarm_Condition_Specification_raiseAlarmCondition */
typedef struct Wire_P_Alarms_PSM_C_Alarm_Condition_Specification_raiseAlarmCondition {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Alarm_Condition_Specification_raiseAlarmCondition;

/** @brief Wire struct for P_Alarms_PSM::C_Alarm_Condition_Specification_isOfInterestToCrewRole */
typedef struct Wire_P_Alarms_PSM_C_Alarm_Condition_Specification_isOfInterestToCrewRole {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_crewRole[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Alarm_Condition_Specification_isOfInterestToCrewRole;

/** @brief Wire struct for P_Alarms_PSM::C_Alarm_Condition_Specification */
typedef struct Wire_P_Alarms_PSM_C_Alarm_Condition_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
    uint8_t A_alarmConditionCategory[1];  // unresolved: nonBasic
    uint8_t A_alarmConditionName[1];  // unresolved: nonBasic
    bool A_hasMultipleInstances;
    int32_t A_overrideState;  // enum P_Alarms_PSM::T_Alarm_Condition_Specification_StateType
    uint32_t A_actualAlarmCondition_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_actualAlarmCondition_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_alarmCategory_sourceID;
    uint32_t A_interestedRole_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_interestedRole_sourceID[100];
} Wire_P_Alarms_PSM_C_Alarm_Condition_Specification;

/** @brief Wire struct for P_Alarms_PSM::C_Tone_Specification */
typedef struct Wire_P_Alarms_PSM_C_Tone_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_toneFrequency[1];  // unresolved: nonBasic
    uint8_t A_toneModulationType[1];  // unresolved: nonBasic
    uint8_t A_toneRepetitionFrequency[1];  // unresolved: nonBasic
    uint8_t A_toneMaxVolume[1];  // unresolved: nonBasic
    Wire_P_LDM_Common_T_IdentifierType A_alarmCategorySpecification_sourceID;
} Wire_P_Alarms_PSM_C_Tone_Specification;

/** @brief Wire struct for P_Alarms_PSM::C_Own_Platform */
typedef struct Wire_P_Alarms_PSM_C_Own_Platform {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_activeAlarmsExist;
    uint32_t A_possibleMissionState_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_possibleMissionState_sourceID[100];
} Wire_P_Alarms_PSM_C_Own_Platform;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm_Condition_unoverrideAlarmCondition */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_unoverrideAlarmCondition {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_unoverrideAlarmCondition;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm_Condition_overrideAlarmCondition */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_overrideAlarmCondition {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_overrideAlarmCondition;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm_Condition_clearAlarmCondition */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_clearAlarmCondition {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_componentName[1];  // unresolved: nonBasic
    uint8_t A_subsystemName[1];  // unresolved: nonBasic
    uint8_t A_measure[1];  // unresolved: nonBasic
    uint8_t A_nature[1];  // unresolved: nonBasic
} Wire_P_Alarms_PSM_C_Actual_Alarm_Condition_clearAlarmCondition;

/** @brief Wire struct for P_Alarms_PSM::C_Actual_Alarm_Condition */
typedef struct Wire_P_Alarms_PSM_C_Actual_Alarm_Condition {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_alarmSourceID;
    Wire_P_LDM_Common_T_DateTimeType A_dateTimeRaised;
    bool A_isOverridden;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_raisedActualAlarm_sourceID;
} Wire_P_Alarms_PSM_C_Actual_Alarm_Condition;

/** @brief Wire struct for P_Alarms_PSM::C_Alarm_Category */
typedef struct Wire_P_Alarms_PSM_C_Alarm_Category {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int16_t A_activeAlarmCount;
    int16_t A_unacknowledgedAlarmCount;
    uint32_t A_categorisedActualAlarm_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_categorisedActualAlarm_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_alarmCategorySpecification_sourceID;
} Wire_P_Alarms_PSM_C_Alarm_Category;

/** @brief Wire struct for P_LDM_Common::T_Position2DType */
typedef struct Wire_P_LDM_Common_T_Position2DType {
    int32_t A_xPosition;
    int32_t A_yPosition;
} Wire_P_LDM_Common_T_Position2DType;

/** @brief Wire struct for P_LDM_Common::T_Size2DType */
typedef struct Wire_P_LDM_Common_T_Size2DType {
    int32_t A_xSize;
    int32_t A_ySize;
} Wire_P_LDM_Common_T_Size2DType;

/** @brief Wire struct for P_LDM_Common::T_LinearAcceleration3DType */
typedef struct Wire_P_LDM_Common_T_LinearAcceleration3DType {
    uint8_t A_xAcceleration[1];  // unresolved: nonBasic
    uint8_t A_yAcceleration[1];  // unresolved: nonBasic
    uint8_t A_zAcceleration[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearAcceleration3DType;

/** @brief Wire struct for P_LDM_Common::T_AttitudeType */
typedef struct Wire_P_LDM_Common_T_AttitudeType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AttitudeType;

/** @brief Wire struct for P_LDM_Common::T_PointPolar3DType */
typedef struct Wire_P_LDM_Common_T_PointPolar3DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_elevation[1];  // unresolved: nonBasic
    uint8_t A_radius[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_PointPolar3DType;

/** @brief Wire struct for P_LDM_Common::T_CoordinatePolar3DType */
typedef struct Wire_P_LDM_Common_T_CoordinatePolar3DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_elevation[1];  // unresolved: nonBasic
    uint8_t A_range[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_CoordinatePolar3DType;

/** @brief Wire struct for P_LDM_Common::T_CoordinatePolar2DType */
typedef struct Wire_P_LDM_Common_T_CoordinatePolar2DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_range[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_CoordinatePolar2DType;

/** @brief Wire struct for P_LDM_Common::T_AngularAcceleration3DType */
typedef struct Wire_P_LDM_Common_T_AngularAcceleration3DType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AngularAcceleration3DType;

/** @brief Wire struct for P_LDM_Common::T_AngularVelocity3DType */
typedef struct Wire_P_LDM_Common_T_AngularVelocity3DType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AngularVelocity3DType;

/** @brief Wire struct for P_LDM_Common::T_LinearVelocity3DType */
typedef struct Wire_P_LDM_Common_T_LinearVelocity3DType {
    uint8_t A_heading[1];  // unresolved: nonBasic
    uint8_t A_speed[1];  // unresolved: nonBasic
    uint8_t A_vrate[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearVelocity3DType;

/** @brief Wire struct for P_LDM_Common::T_RotationalOffsetType */
typedef struct Wire_P_LDM_Common_T_RotationalOffsetType {
    uint8_t A_pitchOffset[1];  // unresolved: nonBasic
    uint8_t A_rollOffset[1];  // unresolved: nonBasic
    uint8_t A_yawOffset[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_RotationalOffsetType;

/** @brief Wire struct for P_LDM_Common::T_Coordinate2DType */
typedef struct Wire_P_LDM_Common_T_Coordinate2DType {
    uint8_t A_latitude[1];  // unresolved: nonBasic
    uint8_t A_longitude[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_Coordinate2DType;

/** @brief Wire struct for P_LDM_Common::T_LinearSpeed3DType */
typedef struct Wire_P_LDM_Common_T_LinearSpeed3DType {
    uint8_t A_xSpeed[1];  // unresolved: nonBasic
    uint8_t A_ySpeed[1];  // unresolved: nonBasic
    uint8_t A_zSpeed[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearSpeed3DType;

/** @brief Wire struct for P_LDM_Common::T_LinearOffsetType */
typedef struct Wire_P_LDM_Common_T_LinearOffsetType {
    uint8_t A_xOffset[1];  // unresolved: nonBasic
    uint8_t A_yOffset[1];  // unresolved: nonBasic
    uint8_t A_zOffset[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearOffsetType;

/** @brief Wire struct for P_LDM_Common::T_LinearVelocity2DType */
typedef struct Wire_P_LDM_Common_T_LinearVelocity2DType {
    uint8_t A_heading[1];  // unresolved: nonBasic
    uint8_t A_speed[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearVelocity2DType;

/** @brief Wire struct for P_LDM_Common::T_Coordinate3DType */
typedef struct Wire_P_LDM_Common_T_Coordinate3DType {
    uint8_t A_altitude[1];  // unresolved: nonBasic
    uint8_t A_latitude[1];  // unresolved: nonBasic
    uint8_t A_longitude[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_Coordinate3DType;

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
} Wire_P_NSTEL_C_CannonDrivingDevice;

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_Specification */
typedef struct Wire_P_NSTEL_C_CannonDrivingDevice_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_powerOnBITSupported;
    bool A_continousBITSupported;
    bool A_initiatedBITSupported;
    int32_t A_continousBITInterval;
    uint32_t A_specifiedCannonDrivingDevices_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedCannonDrivingDevices_sourceID[100];
} Wire_P_NSTEL_C_CannonDrivingDevice_Specification;

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_commandDriving */
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

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_Signal */
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

/** @brief Wire struct for P_NSTEL::C_VehicleSpeed */
typedef struct Wire_P_NSTEL_C_VehicleSpeed {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    double A_value;
} Wire_P_NSTEL_C_VehicleSpeed;

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_PowerOnBIT */
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

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_PBIT */
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

/** @brief Wire struct for P_NSTEL::C_CannonDrivingDevice_IBIT */
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

/** @brief Wire struct for C_StringMsg */
typedef struct Wire_C_StringMsg {
    char text[256];
} Wire_C_StringMsg;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Failure_Event */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_DateTimeType A_eventOccurenceDate;
    double A_standardFaultCode;
    uint8_t A_proprietaryFaultCode[1];  // unresolved: nonBasic
    bool A_predictedFailure;
    uint8_t A_failureDescription[1];  // unresolved: nonBasic
    uint8_t A_causeDescription[1];  // unresolved: nonBasic
    uint8_t A_operationalConsequenceDescription[1];  // unresolved: nonBasic
    uint8_t A_operatorActionDescription[1];  // unresolved: nonBasic
    uint8_t A_maintenanceActionDescription[1];  // unresolved: nonBasic
    uint8_t A_complementaryInformation[1];  // unresolved: nonBasic
    uint8_t A_eventDocumentationURI[1];  // unresolved: nonBasic
    Wire_P_LDM_Common_T_IdentifierType A_monitoredEntity_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Installed_Software */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_serialNumber;
    int32_t A_version;
    int32_t A_configuration;
    Wire_P_LDM_Common_T_IdentifierType A_subSystem_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    double A_value;
    Wire_P_LDM_Common_T_IdentifierType A_monitoredEntity_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
    uint32_t A_thresholdExceedenceEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_thresholdExceedenceEvents_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic_Specification */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_unit[1];  // unresolved: nonBasic
    uint8_t A_descriptor[1];  // unresolved: nonBasic
    int32_t A_publishingIntervalInSeconds;
    int32_t A_characteristicKind;  // enum P_Usage_And_Condition_Monitoring_PSM::T_MonitoredCharacteristicKind
    uint32_t A_specifiedMonitoredCharacteristic_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedMonitoredCharacteristic_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_thresholds_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_type;  // enum P_Usage_And_Condition_Monitoring_PSM::T_BITType
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_healthStatus;  // enum P_Usage_And_Condition_Monitoring_PSM::T_HealthStatus
    int32_t A_currentState;  // enum P_Usage_And_Condition_Monitoring_PSM::T_Monitored_Entity_StateType
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
    uint32_t A_monitoredCharacteristics_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_monitoredCharacteristics_sourceID[100];
    uint32_t A_failureEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_failureEvents_sourceID[100];
    uint32_t A_installedSoftware_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_installedSoftware_sourceID[100];
    uint32_t A_preventativeMaintenanceRecomendedEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_preventativeMaintenanceRecomendedEvents_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_Specification */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_cumulativeMonitoredCharacteristicsOutputSupported;
    bool A_failureStandardFaultCodeSupported;
    bool A_failureProprietaryFaultCodeSupported;
    bool A_failureDescriptionSupported;
    bool A_failureCauseDescriptionSupported;
    bool A_failureOperationalConsequenceDescriptionSupported;
    bool A_failureOperatorActionDescriptionSupported;
    bool A_failureMaintenanceActionDescriptionSupported;
    bool A_failureComplementaryInformationSupported;
    bool A_failureDocumentationURISupported;
    Wire_P_LDM_Common_T_DateTimeType A_installationDateTime;
    uint32_t A_specifiedMonitoredEntities_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedMonitoredEntities_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Preventive_Maintenance_Recommended_Event */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_dataDescriptor[1];  // unresolved: nonBasic
    uint8_t A_dataUnit[1];  // unresolved: nonBasic
    double A_dataValue;
    double A_preventiveMaintenanceThreshold;
    Wire_P_LDM_Common_T_DateTimeType A_occurrenceDateTime;
    Wire_P_LDM_Common_T_IdentifierType A_subSystem_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    uint8_t A_name[1];  // unresolved: nonBasic
    double A_value;
    int32_t A_type;  // enum P_Usage_And_Condition_Monitoring_PSM::T_ThresholdType
    int32_t A_maxDuration;
    int32_t A_maxNumberOfRepetitions;
    uint32_t A_conditionedThresholds_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_conditionedThresholds_sourceID[100];
    uint32_t A_conditioningThreshold_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_conditioningThreshold_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_concernedCharacteristic_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
    uint32_t A_exceedenceEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_exceedenceEvents_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_conditionongThreshold_sourceID;
    uint32_t A_conditionedThreshold_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_conditionedThreshold_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Exceedence_Event */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    Wire_P_LDM_Common_T_DateTimeType A_eventOccurenceDate;
    double A_measuredValue;
    uint32_t A_inducedEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_inducedEvents_sourceID[100];
    Wire_P_LDM_Common_T_IdentifierType A_inducingEvent_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_concernedCharacteristic_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_exceededThreshold_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event;

/** @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Specification */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_valueThresholdSupported;
    bool A_durationThresholdSupported;
    bool A_repetitionsThresholdSupported;
    uint32_t A_specifiedThreshold_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedThreshold_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification;

#pragma pack(pop)
