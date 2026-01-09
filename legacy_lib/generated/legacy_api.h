/**
 * @file legacy_api.h
 * @brief 자동 생성된 API (JSON: XML 기반, Struct: 헤더 기반)
 */

#ifndef LEGACY_API_GENERATED_H
#define LEGACY_API_GENERATED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "legacy_agent.h"
#include "idl_wire_LDM_Common.h"
#include "idl_wire_Nstel_PSM.h"
#include "idl_wire_Usage_And_Condition_Monitoring_PSM.h"

/* --- JSON API (Driven by XML) --- */
/** @brief Publish JSON for topic P_LDM_Common__T_Position2DType (0x2000) */
int legacy_send_P_LDM_Common_T_Position2DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_Size2DType (0x2000) */
int legacy_send_P_LDM_Common_T_Size2DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_IdentifierType (0x2000) */
int legacy_send_P_LDM_Common_T_IdentifierType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_DurationType (0x2000) */
int legacy_send_P_LDM_Common_T_DurationType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_DateTimeType (0x2000) */
int legacy_send_P_LDM_Common_T_DateTimeType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_LinearAcceleration3DType (0x2000) */
int legacy_send_P_LDM_Common_T_LinearAcceleration3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_AttitudeType (0x2000) */
int legacy_send_P_LDM_Common_T_AttitudeType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_PointPolar3DType (0x2000) */
int legacy_send_P_LDM_Common_T_PointPolar3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_CoordinatePolar3DType (0x2000) */
int legacy_send_P_LDM_Common_T_CoordinatePolar3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_CoordinatePolar2DType (0x2000) */
int legacy_send_P_LDM_Common_T_CoordinatePolar2DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_AngularAcceleration3DType (0x2000) */
int legacy_send_P_LDM_Common_T_AngularAcceleration3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_AngularVelocity3DType (0x2000) */
int legacy_send_P_LDM_Common_T_AngularVelocity3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_LinearVelocity3DType (0x2000) */
int legacy_send_P_LDM_Common_T_LinearVelocity3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_RotationalOffsetType (0x2000) */
int legacy_send_P_LDM_Common_T_RotationalOffsetType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_Coordinate2DType (0x2000) */
int legacy_send_P_LDM_Common_T_Coordinate2DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_LinearSpeed3DType (0x2000) */
int legacy_send_P_LDM_Common_T_LinearSpeed3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_LinearOffsetType (0x2000) */
int legacy_send_P_LDM_Common_T_LinearOffsetType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_LinearVelocity2DType (0x2000) */
int legacy_send_P_LDM_Common_T_LinearVelocity2DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_LDM_Common__T_Coordinate3DType (0x2000) */
int legacy_send_P_LDM_Common_T_Coordinate3DType_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_Specification (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_Specification_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_commandDriving (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_commandDriving_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_Signal (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_VehicleSpeed (0x2000) */
int legacy_send_P_NSTEL_C_VehicleSpeed_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_PowerOnBIT (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_PBIT (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_NSTEL__C_CannonDrivingDevice_IBIT (0x2000) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Failure_Event (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Installed_Software (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic_Specification (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_Specification (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Preventive_Maintenance_Recommended_Event (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Threshold (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Exceedence_Event (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event_json(void* agent, const char* json_data);

/** @brief Publish JSON for topic P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Specification (0x2000) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification_json(void* agent, const char* json_data);

/* --- Struct API (Driven by Headers) --- */
/** @brief Publish Struct for Wire_P_LDM_Common_T_IdentifierType (0x2100) */
int legacy_send_P_LDM_Common_T_IdentifierType_struct(void* agent, const Wire_P_LDM_Common_T_IdentifierType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_DateTimeType (0x2100) */
int legacy_send_P_LDM_Common_T_DateTimeType_struct(void* agent, const Wire_P_LDM_Common_T_DateTimeType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_DurationType (0x2100) */
int legacy_send_P_LDM_Common_T_DurationType_struct(void* agent, const Wire_P_LDM_Common_T_DurationType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_Position2DType (0x2100) */
int legacy_send_P_LDM_Common_T_Position2DType_struct(void* agent, const Wire_P_LDM_Common_T_Position2DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_Size2DType (0x2100) */
int legacy_send_P_LDM_Common_T_Size2DType_struct(void* agent, const Wire_P_LDM_Common_T_Size2DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_LinearAcceleration3DType (0x2100) */
int legacy_send_P_LDM_Common_T_LinearAcceleration3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearAcceleration3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_AttitudeType (0x2100) */
int legacy_send_P_LDM_Common_T_AttitudeType_struct(void* agent, const Wire_P_LDM_Common_T_AttitudeType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_PointPolar3DType (0x2100) */
int legacy_send_P_LDM_Common_T_PointPolar3DType_struct(void* agent, const Wire_P_LDM_Common_T_PointPolar3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_CoordinatePolar3DType (0x2100) */
int legacy_send_P_LDM_Common_T_CoordinatePolar3DType_struct(void* agent, const Wire_P_LDM_Common_T_CoordinatePolar3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_CoordinatePolar2DType (0x2100) */
int legacy_send_P_LDM_Common_T_CoordinatePolar2DType_struct(void* agent, const Wire_P_LDM_Common_T_CoordinatePolar2DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_AngularAcceleration3DType (0x2100) */
int legacy_send_P_LDM_Common_T_AngularAcceleration3DType_struct(void* agent, const Wire_P_LDM_Common_T_AngularAcceleration3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_AngularVelocity3DType (0x2100) */
int legacy_send_P_LDM_Common_T_AngularVelocity3DType_struct(void* agent, const Wire_P_LDM_Common_T_AngularVelocity3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_LinearVelocity3DType (0x2100) */
int legacy_send_P_LDM_Common_T_LinearVelocity3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearVelocity3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_RotationalOffsetType (0x2100) */
int legacy_send_P_LDM_Common_T_RotationalOffsetType_struct(void* agent, const Wire_P_LDM_Common_T_RotationalOffsetType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_Coordinate2DType (0x2100) */
int legacy_send_P_LDM_Common_T_Coordinate2DType_struct(void* agent, const Wire_P_LDM_Common_T_Coordinate2DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_LinearSpeed3DType (0x2100) */
int legacy_send_P_LDM_Common_T_LinearSpeed3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearSpeed3DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_LinearOffsetType (0x2100) */
int legacy_send_P_LDM_Common_T_LinearOffsetType_struct(void* agent, const Wire_P_LDM_Common_T_LinearOffsetType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_LinearVelocity2DType (0x2100) */
int legacy_send_P_LDM_Common_T_LinearVelocity2DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearVelocity2DType* data);

/** @brief Publish Struct for Wire_P_LDM_Common_T_Coordinate3DType (0x2100) */
int legacy_send_P_LDM_Common_T_Coordinate3DType_struct(void* agent, const Wire_P_LDM_Common_T_Coordinate3DType* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_Specification (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_Specification_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_Specification* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_commandDriving_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_Signal (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_Signal* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_VehicleSpeed (0x2100) */
int legacy_send_P_NSTEL_C_VehicleSpeed_struct(void* agent, const Wire_P_NSTEL_C_VehicleSpeed* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_PBIT (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_PBIT* data);

/** @brief Publish Struct for Wire_P_NSTEL_C_CannonDrivingDevice_IBIT (0x2100) */
int legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_IBIT* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event* data);

/** @brief Publish Struct for Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification (0x2100) */
int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification* data);

#ifdef __cplusplus
}
#endif
#endif // LEGACY_API_GENERATED_H
