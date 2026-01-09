/**
 * @file idl_wire_LDM_Common.h
 * @brief Wire struct 정의 - LDM_Common.idl
 * @warning 이 파일은 자동 생성되었습니다. 직접 수정하지 마세요.
 */
#ifndef IDL_WIRE_LDM_COMMON_H
#define IDL_WIRE_LDM_COMMON_H

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)

/** @brief Wire struct for P_LDM_Common::T_IdentifierType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_IdentifierType {
    int32_t A_resourceId;
    int32_t A_instanceId;
} Wire_P_LDM_Common_T_IdentifierType;

/** @brief Wire struct for P_LDM_Common::T_DateTimeType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_DateTimeType {
    int64_t A_second;
    int32_t A_nanoseconds;
} Wire_P_LDM_Common_T_DateTimeType;

/** @brief Wire struct for P_LDM_Common::T_DurationType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_DurationType {
    int32_t A_seconds;
    int32_t A_nanoseconds;
} Wire_P_LDM_Common_T_DurationType;

/** @brief Wire struct for P_LDM_Common::T_Position2DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_Position2DType {
    int32_t A_xPosition;
    int32_t A_yPosition;
} Wire_P_LDM_Common_T_Position2DType;

/** @brief Wire struct for P_LDM_Common::T_Size2DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_Size2DType {
    int32_t A_xSize;
    int32_t A_ySize;
} Wire_P_LDM_Common_T_Size2DType;

/** @brief Wire struct for P_LDM_Common::T_LinearAcceleration3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_LinearAcceleration3DType {
    uint8_t A_xAcceleration[1];  // unresolved: nonBasic
    uint8_t A_yAcceleration[1];  // unresolved: nonBasic
    uint8_t A_zAcceleration[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearAcceleration3DType;

/** @brief Wire struct for P_LDM_Common::T_AttitudeType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_AttitudeType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AttitudeType;

/** @brief Wire struct for P_LDM_Common::T_PointPolar3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_PointPolar3DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_elevation[1];  // unresolved: nonBasic
    uint8_t A_radius[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_PointPolar3DType;

/** @brief Wire struct for P_LDM_Common::T_CoordinatePolar3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_CoordinatePolar3DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_elevation[1];  // unresolved: nonBasic
    uint8_t A_range[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_CoordinatePolar3DType;

/** @brief Wire struct for P_LDM_Common::T_CoordinatePolar2DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_CoordinatePolar2DType {
    uint8_t A_angle[1];  // unresolved: nonBasic
    uint8_t A_range[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_CoordinatePolar2DType;

/** @brief Wire struct for P_LDM_Common::T_AngularAcceleration3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_AngularAcceleration3DType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AngularAcceleration3DType;

/** @brief Wire struct for P_LDM_Common::T_AngularVelocity3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_AngularVelocity3DType {
    uint8_t A_pitch[1];  // unresolved: nonBasic
    uint8_t A_roll[1];  // unresolved: nonBasic
    uint8_t A_yaw[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_AngularVelocity3DType;

/** @brief Wire struct for P_LDM_Common::T_LinearVelocity3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_LinearVelocity3DType {
    uint8_t A_heading[1];  // unresolved: nonBasic
    uint8_t A_speed[1];  // unresolved: nonBasic
    uint8_t A_vrate[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearVelocity3DType;

/** @brief Wire struct for P_LDM_Common::T_RotationalOffsetType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_RotationalOffsetType {
    uint8_t A_pitchOffset[1];  // unresolved: nonBasic
    uint8_t A_rollOffset[1];  // unresolved: nonBasic
    uint8_t A_yawOffset[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_RotationalOffsetType;

/** @brief Wire struct for P_LDM_Common::T_Coordinate2DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_Coordinate2DType {
    uint8_t A_latitude[1];  // unresolved: nonBasic
    uint8_t A_longitude[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_Coordinate2DType;

/** @brief Wire struct for P_LDM_Common::T_LinearSpeed3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_LinearSpeed3DType {
    uint8_t A_xSpeed[1];  // unresolved: nonBasic
    uint8_t A_ySpeed[1];  // unresolved: nonBasic
    uint8_t A_zSpeed[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearSpeed3DType;

/** @brief Wire struct for P_LDM_Common::T_LinearOffsetType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_LinearOffsetType {
    uint8_t A_xOffset[1];  // unresolved: nonBasic
    uint8_t A_yOffset[1];  // unresolved: nonBasic
    uint8_t A_zOffset[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearOffsetType;

/** @brief Wire struct for P_LDM_Common::T_LinearVelocity2DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_LinearVelocity2DType {
    uint8_t A_heading[1];  // unresolved: nonBasic
    uint8_t A_speed[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_LinearVelocity2DType;

/** @brief Wire struct for P_LDM_Common::T_Coordinate3DType (내부 타입) */
typedef struct Wire_P_LDM_Common_T_Coordinate3DType {
    uint8_t A_altitude[1];  // unresolved: nonBasic
    uint8_t A_latitude[1];  // unresolved: nonBasic
    uint8_t A_longitude[1];  // unresolved: nonBasic
} Wire_P_LDM_Common_T_Coordinate3DType;

#pragma pack(pop)

#endif /* IDL_WIRE_LDM_COMMON_H */
