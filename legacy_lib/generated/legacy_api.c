#include "legacy_api.h"
#include <string.h>

int legacy_send_P_LDM_Common_T_Position2DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_Position2DType", json_data);
}

int legacy_send_P_LDM_Common_T_Size2DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_Size2DType", json_data);
}

int legacy_send_P_LDM_Common_T_IdentifierType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_IdentifierType", json_data);
}

int legacy_send_P_LDM_Common_T_DurationType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_DurationType", json_data);
}

int legacy_send_P_LDM_Common_T_DateTimeType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_DateTimeType", json_data);
}

int legacy_send_P_LDM_Common_T_LinearAcceleration3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_LinearAcceleration3DType", json_data);
}

int legacy_send_P_LDM_Common_T_AttitudeType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_AttitudeType", json_data);
}

int legacy_send_P_LDM_Common_T_PointPolar3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_PointPolar3DType", json_data);
}

int legacy_send_P_LDM_Common_T_CoordinatePolar3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_CoordinatePolar3DType", json_data);
}

int legacy_send_P_LDM_Common_T_CoordinatePolar2DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_CoordinatePolar2DType", json_data);
}

int legacy_send_P_LDM_Common_T_AngularAcceleration3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_AngularAcceleration3DType", json_data);
}

int legacy_send_P_LDM_Common_T_AngularVelocity3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_AngularVelocity3DType", json_data);
}

int legacy_send_P_LDM_Common_T_LinearVelocity3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_LinearVelocity3DType", json_data);
}

int legacy_send_P_LDM_Common_T_RotationalOffsetType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_RotationalOffsetType", json_data);
}

int legacy_send_P_LDM_Common_T_Coordinate2DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_Coordinate2DType", json_data);
}

int legacy_send_P_LDM_Common_T_LinearSpeed3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_LinearSpeed3DType", json_data);
}

int legacy_send_P_LDM_Common_T_LinearOffsetType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_LinearOffsetType", json_data);
}

int legacy_send_P_LDM_Common_T_LinearVelocity2DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_LinearVelocity2DType", json_data);
}

int legacy_send_P_LDM_Common_T_Coordinate3DType_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_LDM_Common__T_Coordinate3DType", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_Specification_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_Specification", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_commandDriving_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_commandDriving", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_Signal", json_data);
}

int legacy_send_P_NSTEL_C_VehicleSpeed_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_VehicleSpeed", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_PowerOnBIT", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_PBIT", json_data);
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_NSTEL__C_CannonDrivingDevice_IBIT", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Failure_Event", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Installed_Software", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic_Specification", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_Specification", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Preventive_Maintenance_Recommended_Event", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Exceedence_Event", json_data);
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification_json(void* agent, const char* json_data) {
    if (!agent || !json_data) return -1;
    return legacy_agent_publish_json(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Specification", json_data);
}

int legacy_send_P_LDM_Common_T_IdentifierType_struct(void* agent, const Wire_P_LDM_Common_T_IdentifierType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_IdentifierType", "P_LDM_Common::T_IdentifierType", data, sizeof(Wire_P_LDM_Common_T_IdentifierType));
}

int legacy_send_P_LDM_Common_T_DateTimeType_struct(void* agent, const Wire_P_LDM_Common_T_DateTimeType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_DateTimeType", "P_LDM_Common::T_DateTimeType", data, sizeof(Wire_P_LDM_Common_T_DateTimeType));
}

int legacy_send_P_LDM_Common_T_DurationType_struct(void* agent, const Wire_P_LDM_Common_T_DurationType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_DurationType", "P_LDM_Common::T_DurationType", data, sizeof(Wire_P_LDM_Common_T_DurationType));
}

int legacy_send_P_LDM_Common_T_Position2DType_struct(void* agent, const Wire_P_LDM_Common_T_Position2DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_Position2DType", "P_LDM_Common::T_Position2DType", data, sizeof(Wire_P_LDM_Common_T_Position2DType));
}

int legacy_send_P_LDM_Common_T_Size2DType_struct(void* agent, const Wire_P_LDM_Common_T_Size2DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_Size2DType", "P_LDM_Common::T_Size2DType", data, sizeof(Wire_P_LDM_Common_T_Size2DType));
}

int legacy_send_P_LDM_Common_T_LinearAcceleration3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearAcceleration3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_LinearAcceleration3DType", "P_LDM_Common::T_LinearAcceleration3DType", data, sizeof(Wire_P_LDM_Common_T_LinearAcceleration3DType));
}

int legacy_send_P_LDM_Common_T_AttitudeType_struct(void* agent, const Wire_P_LDM_Common_T_AttitudeType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_AttitudeType", "P_LDM_Common::T_AttitudeType", data, sizeof(Wire_P_LDM_Common_T_AttitudeType));
}

int legacy_send_P_LDM_Common_T_PointPolar3DType_struct(void* agent, const Wire_P_LDM_Common_T_PointPolar3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_PointPolar3DType", "P_LDM_Common::T_PointPolar3DType", data, sizeof(Wire_P_LDM_Common_T_PointPolar3DType));
}

int legacy_send_P_LDM_Common_T_CoordinatePolar3DType_struct(void* agent, const Wire_P_LDM_Common_T_CoordinatePolar3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_CoordinatePolar3DType", "P_LDM_Common::T_CoordinatePolar3DType", data, sizeof(Wire_P_LDM_Common_T_CoordinatePolar3DType));
}

int legacy_send_P_LDM_Common_T_CoordinatePolar2DType_struct(void* agent, const Wire_P_LDM_Common_T_CoordinatePolar2DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_CoordinatePolar2DType", "P_LDM_Common::T_CoordinatePolar2DType", data, sizeof(Wire_P_LDM_Common_T_CoordinatePolar2DType));
}

int legacy_send_P_LDM_Common_T_AngularAcceleration3DType_struct(void* agent, const Wire_P_LDM_Common_T_AngularAcceleration3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_AngularAcceleration3DType", "P_LDM_Common::T_AngularAcceleration3DType", data, sizeof(Wire_P_LDM_Common_T_AngularAcceleration3DType));
}

int legacy_send_P_LDM_Common_T_AngularVelocity3DType_struct(void* agent, const Wire_P_LDM_Common_T_AngularVelocity3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_AngularVelocity3DType", "P_LDM_Common::T_AngularVelocity3DType", data, sizeof(Wire_P_LDM_Common_T_AngularVelocity3DType));
}

int legacy_send_P_LDM_Common_T_LinearVelocity3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearVelocity3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_LinearVelocity3DType", "P_LDM_Common::T_LinearVelocity3DType", data, sizeof(Wire_P_LDM_Common_T_LinearVelocity3DType));
}

int legacy_send_P_LDM_Common_T_RotationalOffsetType_struct(void* agent, const Wire_P_LDM_Common_T_RotationalOffsetType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_RotationalOffsetType", "P_LDM_Common::T_RotationalOffsetType", data, sizeof(Wire_P_LDM_Common_T_RotationalOffsetType));
}

int legacy_send_P_LDM_Common_T_Coordinate2DType_struct(void* agent, const Wire_P_LDM_Common_T_Coordinate2DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_Coordinate2DType", "P_LDM_Common::T_Coordinate2DType", data, sizeof(Wire_P_LDM_Common_T_Coordinate2DType));
}

int legacy_send_P_LDM_Common_T_LinearSpeed3DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearSpeed3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_LinearSpeed3DType", "P_LDM_Common::T_LinearSpeed3DType", data, sizeof(Wire_P_LDM_Common_T_LinearSpeed3DType));
}

int legacy_send_P_LDM_Common_T_LinearOffsetType_struct(void* agent, const Wire_P_LDM_Common_T_LinearOffsetType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_LinearOffsetType", "P_LDM_Common::T_LinearOffsetType", data, sizeof(Wire_P_LDM_Common_T_LinearOffsetType));
}

int legacy_send_P_LDM_Common_T_LinearVelocity2DType_struct(void* agent, const Wire_P_LDM_Common_T_LinearVelocity2DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_LinearVelocity2DType", "P_LDM_Common::T_LinearVelocity2DType", data, sizeof(Wire_P_LDM_Common_T_LinearVelocity2DType));
}

int legacy_send_P_LDM_Common_T_Coordinate3DType_struct(void* agent, const Wire_P_LDM_Common_T_Coordinate3DType* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_LDM_Common__T_Coordinate3DType", "P_LDM_Common::T_Coordinate3DType", data, sizeof(Wire_P_LDM_Common_T_Coordinate3DType));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice", "P_NSTEL::C_CannonDrivingDevice", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_Specification_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_Specification* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_Specification", "P_NSTEL::C_CannonDrivingDevice_Specification", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_Specification));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_commandDriving_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_commandDriving", "P_NSTEL::C_CannonDrivingDevice_commandDriving", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_Signal* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_Signal", "P_NSTEL::C_CannonDrivingDevice_Signal", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_Signal));
}

int legacy_send_P_NSTEL_C_VehicleSpeed_struct(void* agent, const Wire_P_NSTEL_C_VehicleSpeed* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_VehicleSpeed", "P_NSTEL::C_VehicleSpeed", data, sizeof(Wire_P_NSTEL_C_VehicleSpeed));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_PowerOnBIT", "P_NSTEL::C_CannonDrivingDevice_PowerOnBIT", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_PBIT* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_PBIT", "P_NSTEL::C_CannonDrivingDevice_PBIT", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_PBIT));
}

int legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_struct(void* agent, const Wire_P_NSTEL_C_CannonDrivingDevice_IBIT* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_NSTEL__C_CannonDrivingDevice_IBIT", "P_NSTEL::C_CannonDrivingDevice_IBIT", data, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_IBIT));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Failure_Event", "P_Usage_And_Condition_Monitoring_PSM::C_Failure_Event", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Failure_Event));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Installed_Software", "P_Usage_And_Condition_Monitoring_PSM::C_Installed_Software", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic", "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Characteristic_Specification", "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic_Specification", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic_Specification));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT", "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity", "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_Specification", "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_Specification", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_Specification));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Preventive_Maintenance_Recommended_Event", "P_Usage_And_Condition_Monitoring_PSM::C_Preventive_Maintenance_Recommended_Event", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Preventive_Maintenance_Recommended_Event));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold", "P_Usage_And_Condition_Monitoring_PSM::C_Threshold", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Exceedence_Event", "P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Exceedence_Event", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Exceedence_Event));
}

int legacy_send_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification_struct(void* agent, const Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification* data) {
    if (!agent || !data) return -1;
    return legacy_agent_publish_struct(agent, "P_Usage_And_Condition_Monitoring_PSM__C_Threshold_Specification", "P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Specification", data, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification));
}

