/**
 * @file idl_wire_Usage_And_Condition_Monitoring_PSM.h
 * @brief Wire struct 정의 - Usage_And_Condition_Monitoring_PSM.idl
 * @warning 이 파일은 자동 생성되었습니다. 직접 수정하지 마세요.
 */
#ifndef IDL_WIRE_USAGE_AND_CONDITION_MONITORING_PSM_H
#define IDL_WIRE_USAGE_AND_CONDITION_MONITORING_PSM_H

#include <stdint.h>
#include <stdbool.h>

#include "idl_wire_LDM_Common.h"

#pragma pack(push, 1)

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Failure_Event
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Installed_Software
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_serialNumber;
    int32_t A_version;
    int32_t A_configuration;
    Wire_P_LDM_Common_T_IdentifierType A_subSystem_sourceID;
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Installed_Software;

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    double A_value;
    Wire_P_LDM_Common_T_IdentifierType A_monitoredEntity_sourceID;
    Wire_P_LDM_Common_T_IdentifierType A_specification_sourceID;
    uint32_t A_thresholdExceedenceEvents_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_thresholdExceedenceEvents_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Characteristic;

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Characteristic_Specification
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT {
    Wire_P_LDM_Common_T_IdentifierType A_recipientID;
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    int32_t A_referenceNum;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    int32_t A_type;  // enum P_Usage_And_Condition_Monitoring_PSM::T_BITType
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT;

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_Specification
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Preventive_Maintenance_Recommended_Event
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Exceedence_Event
 * @topic DDS Topic 전송용 구조체
 */
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

/**
 * @brief Wire struct for P_Usage_And_Condition_Monitoring_PSM::C_Threshold_Specification
 * @topic DDS Topic 전송용 구조체
 */
typedef struct Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification {
    Wire_P_LDM_Common_T_IdentifierType A_sourceID;
    Wire_P_LDM_Common_T_DateTimeType A_timeOfDataGeneration;
    bool A_valueThresholdSupported;
    bool A_durationThresholdSupported;
    bool A_repetitionsThresholdSupported;
    uint32_t A_specifiedThreshold_sourceID_len; Wire_P_LDM_Common_T_IdentifierType A_specifiedThreshold_sourceID[100];
} Wire_P_Usage_And_Condition_Monitoring_PSM_C_Threshold_Specification;

#pragma pack(pop)

#endif /* IDL_WIRE_USAGE_AND_CONDITION_MONITORING_PSM_H */
