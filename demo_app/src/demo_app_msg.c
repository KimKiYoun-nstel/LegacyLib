/*
 * demo_app_msg.c - Message Handlers and Publishers
 */

#include "../include/demo_app.h"
#include "../include/demo_app_log.h"
#include "../include/msg_fields.h"
#include "../../legacy_lib/generated/legacy_api.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <tickLib.h>
#endif

/* ========================================================================
 * Callbacks
 * ======================================================================== */

static int demo_is_little_endian(void) {
    const uint16_t v = 1;
    return *((const uint8_t*)&v) == 1;
}

static uint32_t demo_bswap32(uint32_t v) {
    return ((v & 0x000000FFU) << 24) |
           ((v & 0x0000FF00U) << 8) |
           ((v & 0x00FF0000U) >> 8) |
           ((v & 0xFF000000U) >> 24);
}

static uint64_t demo_bswap64(uint64_t v) {
    return ((v & 0x00000000000000FFULL) << 56) |
           ((v & 0x000000000000FF00ULL) << 40) |
           ((v & 0x0000000000FF0000ULL) << 24) |
           ((v & 0x00000000FF000000ULL) << 8) |
           ((v & 0x000000FF00000000ULL) >> 8) |
           ((v & 0x0000FF0000000000ULL) >> 24) |
           ((v & 0x00FF000000000000ULL) >> 40) |
           ((v & 0xFF00000000000000ULL) >> 56);
}

static uint32_t demo_be32(uint32_t v) {
    return demo_is_little_endian() ? demo_bswap32(v) : v;
}

static uint64_t demo_be64(uint64_t v) {
    return demo_is_little_endian() ? demo_bswap64(v) : v;
}

static int32_t demo_be32s(int32_t v) {
    return (int32_t)demo_be32((uint32_t)v);
}

static int64_t demo_be64s(int64_t v) {
    return (int64_t)demo_be64((uint64_t)v);
}

static double demo_be_double(double v) {
    union { double d; uint64_t u; } u;
    u.d = v;
    u.u = demo_be64(u.u);
    return u.d;
}

static void demo_wire_id_be(Wire_P_LDM_Common_T_IdentifierType* v) {
    v->A_resourceId = demo_be32s(v->A_resourceId);
    v->A_instanceId = demo_be32s(v->A_instanceId);
}

static void demo_wire_time_be(Wire_P_LDM_Common_T_DateTimeType* v) {
    v->A_second = demo_be64s(v->A_second);
    v->A_nanoseconds = demo_be32s(v->A_nanoseconds);
}

static void demo_wire_runbit_be(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT* v) {
    demo_wire_id_be(&v->A_recipientID);
    demo_wire_id_be(&v->A_sourceID);
    v->A_referenceNum = demo_be32s(v->A_referenceNum);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    v->A_type = demo_be32s(v->A_type);
}

static void demo_wire_commanddriving_be(Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_id_be(&v->A_recipientID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    v->A_referenceNum = demo_be32s(v->A_referenceNum);
    v->A_roundPosition = demo_be_double(v->A_roundPosition);
    v->A_upDownPosition = demo_be_double(v->A_upDownPosition);
    v->A_roundAngleVelocity = demo_be_double(v->A_roundAngleVelocity);
    v->A_upDownAngleVelocity = demo_be_double(v->A_upDownAngleVelocity);
    v->A_cannonUpDownAngle = demo_be_double(v->A_cannonUpDownAngle);
    v->A_topRelativeAngle = demo_be_double(v->A_topRelativeAngle);
    v->A_operationMode = demo_be32s(v->A_operationMode);
    v->A_parm = demo_be32s(v->A_parm);
    v->A_targetFix = demo_be32s(v->A_targetFix);
    v->A_autoArmPosition = demo_be32s(v->A_autoArmPosition);
    v->A_manualArmPosition = demo_be32s(v->A_manualArmPosition);
    v->A_mainCannonRestore = demo_be32s(v->A_mainCannonRestore);
    v->A_mainCannonFix = demo_be32s(v->A_mainCannonFix);
    v->A_closureEquipOpenStatus = demo_be32s(v->A_closureEquipOpenStatus);
}

static void demo_wire_vehicle_speed_be(Wire_P_NSTEL_C_VehicleSpeed* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    v->A_value = demo_be_double(v->A_value);
}

static void demo_wire_poweronbit_be(Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    demo_wire_id_be(&v->A_cannonDrivingDevice_sourceID);
    v->A_upDownMotor = demo_be32s(v->A_upDownMotor);
    v->A_roundMotor = demo_be32s(v->A_roundMotor);
    v->A_upDownAmp = demo_be32s(v->A_upDownAmp);
    v->A_roundAmp = demo_be32s(v->A_roundAmp);
    v->A_baseGyro = demo_be32s(v->A_baseGyro);
    v->A_topForwardGyro = demo_be32s(v->A_topForwardGyro);
    v->A_vehicleForwardGyro = demo_be32s(v->A_vehicleForwardGyro);
    v->A_powerController = demo_be32s(v->A_powerController);
    v->A_energyStorage = demo_be32s(v->A_energyStorage);
    v->A_directPower = demo_be32s(v->A_directPower);
    v->A_cableLoop = demo_be32s(v->A_cableLoop);
}

static void demo_wire_pbit_be(Wire_P_NSTEL_C_CannonDrivingDevice_PBIT* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    demo_wire_id_be(&v->A_cannonDrivingDevice_sourceID);
    v->A_upDownMotor = demo_be32s(v->A_upDownMotor);
    v->A_roundMotor = demo_be32s(v->A_roundMotor);
    v->A_upDownAmp = demo_be32s(v->A_upDownAmp);
    v->A_roundAmp = demo_be32s(v->A_roundAmp);
    v->A_baseGyro = demo_be32s(v->A_baseGyro);
    v->A_topForwardGyro = demo_be32s(v->A_topForwardGyro);
    v->A_vehicleForwardGyro = demo_be32s(v->A_vehicleForwardGyro);
    v->A_powerController = demo_be32s(v->A_powerController);
    v->A_energyStorage = demo_be32s(v->A_energyStorage);
    v->A_directPower = demo_be32s(v->A_directPower);
    v->A_cableLoop = demo_be32s(v->A_cableLoop);
    v->A_upDownPark = demo_be32s(v->A_upDownPark);
    v->A_roundPark = demo_be32s(v->A_roundPark);
    v->A_mainCannonLock = demo_be32s(v->A_mainCannonLock);
    v->A_controllerNetwork = demo_be32s(v->A_controllerNetwork);
}

static void demo_wire_ibit_be(Wire_P_NSTEL_C_CannonDrivingDevice_IBIT* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    demo_wire_id_be(&v->A_cannonDrivingDevice_sourceID);
    v->A_referenceNum = demo_be32s(v->A_referenceNum);
    v->A_upDownMotor = demo_be32s(v->A_upDownMotor);
    v->A_roundMotor = demo_be32s(v->A_roundMotor);
    v->A_upDownAmp = demo_be32s(v->A_upDownAmp);
    v->A_roundAmp = demo_be32s(v->A_roundAmp);
    v->A_baseGyro = demo_be32s(v->A_baseGyro);
    v->A_topForwardGyro = demo_be32s(v->A_topForwardGyro);
    v->A_vehicleForwardGyro = demo_be32s(v->A_vehicleForwardGyro);
    v->A_powerController = demo_be32s(v->A_powerController);
    v->A_energyStorage = demo_be32s(v->A_energyStorage);
    v->A_directPower = demo_be32s(v->A_directPower);
    v->A_cableLoop = demo_be32s(v->A_cableLoop);
}

static void demo_wire_signal_be(Wire_P_NSTEL_C_CannonDrivingDevice_Signal* v) {
    demo_wire_id_be(&v->A_sourceID);
    demo_wire_id_be(&v->A_recipientID);
    demo_wire_time_be(&v->A_timeOfDataGeneration);
    v->A_azAngleVelocity = demo_be_double(v->A_azAngleVelocity);
    v->A_e1AngleVelocity = demo_be_double(v->A_e1AngleVelocity);
    v->A_energyStorage = demo_be32s(v->A_energyStorage);
    v->A_mainCannonFixStatus = demo_be32s(v->A_mainCannonFixStatus);
    v->A_deckCleance = demo_be32s(v->A_deckCleance);
    v->A_autoArmPositionComplement = demo_be32s(v->A_autoArmPositionComplement);
    v->A_manualArmPositionComplement = demo_be32s(v->A_manualArmPositionComplement);
    v->A_mainCannonRestoreComplement = demo_be32s(v->A_mainCannonRestoreComplement);
    v->A_armSafetyMainCannonLock = demo_be32s(v->A_armSafetyMainCannonLock);
    v->A_shutdown = demo_be32s(v->A_shutdown);
    v->A_roundGyro = demo_be_double(v->A_roundGyro);
    v->A_upDownGyro = demo_be_double(v->A_upDownGyro);
}

static void demo_log_hex(const char* label, const void* data, size_t len) {
    if (!data || len == 0) return;
    if (demo_log_get_level() < LOG_LEVEL_DEBUG) return;

    const uint8_t* bytes = (const uint8_t*)data;
    LOG_DEBUG("[HEX] %s (len=%zu)\n", label, len);

    for (size_t offset = 0; offset < len; offset += 16) {
        char line[128];
        size_t chunk = len - offset;
        if (chunk > 16) chunk = 16;

        int pos = snprintf(line, sizeof(line), "[HEX] +%04zu:", offset);
        for (size_t i = 0; i < chunk && pos > 0 && (size_t)pos < sizeof(line); i++) {
            pos += snprintf(line + pos, sizeof(line) - (size_t)pos, " %02X", bytes[offset + i]);
        }
        snprintf(line + pos, sizeof(line) - (size_t)pos, "\n");
        LOG_DEBUG("%s", line);
    }
}

static void on_writer_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    (void)h; (void)req_id;
    const char* topic = (const char*)user;
    if (res->ok) {
        LOG_INFO("Writer created: %s\n", topic);
    } else {
        LOG_INFO("ERROR: Failed to create writer for %s: %s\n",
               topic, res->msg ? res->msg : "Unknown");
    }
}

static void on_reader_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    (void)h; (void)req_id;
    const char* topic = (const char*)user;
    if (res->ok) {
        LOG_INFO("Reader created: %s\n", topic);
    } else {
        LOG_INFO("ERROR: Failed to create reader for %s: %s\n",
               topic, res->msg ? res->msg : "Unknown");
    }
}

/* ========================================================================
 * Initialization
 * ======================================================================== */

int demo_msg_init(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    LOG_INFO("Initializing message handlers...\n");
    
    LegacyStatus status;
    
    // ===== Create Writers (4 send topics) =====
    
    // 1. PBIT Writer (Initial State QoS)
    LegacyWriterConfig pbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_PowerOnBIT,
        TYPE_PowerOnBIT,
        "NstelQosLib::InitialStateProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &pbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_PowerOnBIT);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create PBIT writer\n");
        return -1;
    }
    
    // 2. CBIT Writer (Low Frequency Status QoS - 1Hz)
    LegacyWriterConfig cbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_PBIT,
        TYPE_PBIT,
        "NstelQosLib::LowFrequencyStatusProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &cbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_PBIT);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create CBIT writer\n");
        return -1;
    }
    
    // 3. ResultBIT Writer (Non-Periodic Event QoS)
    LegacyWriterConfig rbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_IBIT,
        TYPE_IBIT,
        "NstelQosLib::NonPeriodicEventProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &rbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_IBIT);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create ResultBIT writer\n");
        return -1;
    }
    
    // 4. Actuator Signal Writer (High Frequency Periodic QoS - 200Hz)
    LegacyWriterConfig signal_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_Signal,
        TYPE_Signal,
        "NstelQosLib::HighFrequencyPeriodicProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &signal_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_Signal);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create Actuator Signal writer\n");
        return -1;
    }
    
    // ===== Create Readers (3 receive topics) =====
    
    // 1. runBIT Reader (Non-Periodic Event QoS)
    LegacyReaderConfig runbit_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_runBIT,
        TYPE_runBIT,
        "NstelQosLib::NonPeriodicEventProfile"
    };
    status = legacy_agent_create_reader(ctx->agent, &runbit_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_runBIT);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create runBIT reader\n");
        return -1;
    }
    
    // Subscribe to runBIT events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_runBIT, TYPE_runBIT,
                                         demo_msg_on_runbit, ctx);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to subscribe to runBIT\n");
        return -1;
    }
    
    // 2. Actuator Control Reader (High Frequency Periodic QoS - 200Hz)
    LegacyReaderConfig control_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_commandDriving,
        TYPE_commandDriving,
        "NstelQosLib::HighFrequencyPeriodicProfile"
    };

    // Register Type Adapters for all 7 topics (4 Write, 3 Read)
    LegacyTypeAdapter adapters[] = {
        {{ TOPIC_PowerOnBIT, TYPE_PowerOnBIT }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT), NULL},
        {{ TOPIC_PBIT, TYPE_PBIT }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_PBIT), NULL},
        {{ TOPIC_IBIT, TYPE_IBIT }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_IBIT), NULL},
        {{ TOPIC_Signal, TYPE_Signal }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_Signal), NULL},
        {{ TOPIC_runBIT, TYPE_runBIT }, NULL, NULL, NULL, sizeof(Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT), NULL},
        {{ TOPIC_commandDriving, TYPE_commandDriving }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving), NULL},
        {{ TOPIC_VehicleSpeed, TYPE_VehicleSpeed }, NULL, NULL, NULL, sizeof(Wire_P_NSTEL_C_VehicleSpeed), NULL}
    };
    for (int i = 0; i < 7; i++) {
        legacy_agent_register_type_adapter(ctx->agent, &adapters[i]);
    }

    status = legacy_agent_create_reader(ctx->agent, &control_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_commandDriving);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create Actuator Control reader\n");
        return -1;
    }
    
    // Subscribe to control events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_commandDriving,
                                         TYPE_commandDriving,
                                         demo_msg_on_actuator_control, ctx);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to subscribe to Actuator Control\n");
        return -1;
    }
    
    // 3. Vehicle Speed Reader (Low Frequency Vehicle QoS - 1Hz)
    LegacyReaderConfig speed_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_VehicleSpeed,
        TYPE_VehicleSpeed,
        "NstelQosLib::LowFrequencyVehicleProfile"
    };
    status = legacy_agent_create_reader(ctx->agent, &speed_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_VehicleSpeed);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to create Vehicle Speed reader\n");
        return -1;
    }
    
    // Subscribe to speed events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_VehicleSpeed,
                                         TYPE_VehicleSpeed,
                                         demo_msg_on_vehicle_speed, ctx);
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to subscribe to Vehicle Speed\n");
        return -1;
    }
    
    // Give time for all entities to be created
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet());  // 1 second
    #endif
    
    LOG_INFO("All message handlers initialized (7 topics)\n");
    return 0;
}

void demo_msg_cleanup(DemoAppContext* ctx) {
    if (!ctx) return;
    
    LOG_INFO("Cleaning up message handlers...\n");
    
    // TODO Phase 3: Unsubscribe and delete writers
}

/* ========================================================================
 * Receive Callbacks
 * ======================================================================== */

void demo_msg_on_runbit(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    (void)h;
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    ctx->runbit_rx_count++;
    
    uint32_t reference_num = 0;
    T_BITType type = L_BITType_I_BIT;  // Default to I_BIT

    if (evt->data_json == NULL && evt->raw_json != NULL) {
        // Binary struct event
        Wire_P_Usage_And_Condition_Monitoring_PSM_C_Monitored_Entity_runBIT run;
        memcpy(&run, evt->raw_json, sizeof(run));
        demo_log_hex("runBIT RX", &run, sizeof(run));
        demo_wire_runbit_be(&run);
        reference_num = (uint32_t)run.A_referenceNum;
        type = (T_BITType)run.A_type;
        LOG_RX("Received runBIT (BINARY): ref=%u, type=%d\n", reference_num, (int)type);
    } else {
        const char* json = evt->data_json;
        if (!json) return;
        
        LOG_RX("Received runBIT (JSON): %s\n", json);
        
        // Parse A_referenceNum
        const char* ref_str = strstr(json, P_COLON(F_A_REFERENCE_NUM));
        if (ref_str) {
            sscanf(ref_str, P_FMT_UINT(F_A_REFERENCE_NUM), &reference_num);
        }
        
        // Parse A_type (enum string)
        const char* type_str = strstr(json, P_COLON(F_A_TYPE));
        if (type_str) {
            char type_value[64];
            if (sscanf(type_str, P_FMT_STR(F_A_TYPE), type_value) == 1) {
                type = parse_bit_type(type_value);
            }
        }
    }
    
    LOG_INFO("runBIT parsed: A_referenceNum=%u, A_type=%d\n", reference_num, (int)type);
    
    // Trigger IBIT
    if (demo_app_trigger_ibit(ctx, reference_num, type) != 0) {
        LOG_INFO("WARNING: Failed to trigger IBIT\n");
    }
}

void demo_msg_on_actuator_control(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    (void)h;
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    ActuatorControlState* ctrl = &ctx->control_state;

    // Check if binary codec is being used
    if (evt->data_json == NULL && evt->raw_json != NULL) {
        // This is a binary struct event (passed via handleStructResponse)
        Wire_P_NSTEL_C_CannonDrivingDevice_commandDriving cmd;
        memcpy(&cmd, evt->raw_json, sizeof(cmd));
        demo_log_hex("commandDriving RX", &cmd, sizeof(cmd));
        demo_wire_commanddriving_be(&cmd);
        
        ctrl->drivingPosition = (double)cmd.A_roundPosition;
        ctrl->upDownPosition = (double)cmd.A_upDownPosition;
        ctrl->roundAngleVelocity = (double)cmd.A_roundAngleVelocity;
        ctrl->upDownAngleVelocity = (double)cmd.A_upDownAngleVelocity;
        ctrl->cannonUpDownAngle = (double)cmd.A_cannonUpDownAngle;
        ctrl->topRelativeAngle = (double)cmd.A_topRelativeAngle;
        ctrl->operationMode = (T_OperationModeType)cmd.A_operationMode;
        ctrl->parm = (T_OnOffType)cmd.A_parm;
        ctrl->targetDesingation = (T_TargetAllotType)cmd.A_targetFix;
        ctrl->autoArmPosition = (T_ArmPositionLockType)cmd.A_autoArmPosition;
        ctrl->manualArmPosition = (T_ArmPositionLockType)cmd.A_manualArmPosition;
        ctrl->mainCannonRestore = (T_MainCannonReturnType)cmd.A_mainCannonRestore;
        ctrl->manCannonFix = (T_MainCannonFixType)cmd.A_mainCannonFix;
        ctrl->closeEquipOpenStatus = (T_EquipOpenLockType)cmd.A_closureEquipOpenStatus;
        
        ctx->control_rx_count++;
        ctrl->last_update_time = ctx->tick_count;
        return;
    }
    
    const char* json = evt->data_json;
    if (!json) return;
    
    // Extract float fields
    const char* ptr;
    if ((ptr = strstr(json, P_COLON(F_A_DRIVINGPOSITION)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_DRIVINGPOSITION), &ctrl->drivingPosition);
    }
    if ((ptr = strstr(json, P_COLON(F_A_UPDOWNPOSITION)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_UPDOWNPOSITION), &ctrl->upDownPosition);
    }
    if ((ptr = strstr(json, P_COLON(F_A_ROUNDANGLEVELOCITY)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_ROUNDANGLEVELOCITY), &ctrl->roundAngleVelocity);
    }
    if ((ptr = strstr(json, P_COLON(F_A_UPDOWNANGLEVELOCITY)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_UPDOWNANGLEVELOCITY), &ctrl->upDownAngleVelocity);
    }
    if ((ptr = strstr(json, P_COLON(F_A_CANNONUPDOWNANGLE)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_CANNONUPDOWNANGLE), &ctrl->cannonUpDownAngle);
    }
    if ((ptr = strstr(json, P_COLON(F_A_TOPRELATIVEANGLE)))) {
        sscanf(ptr, P_FMT_FLOAT(F_A_TOPRELATIVEANGLE), &ctrl->topRelativeAngle);
    }
    
    // Extract enum fields
    char enum_val[64];
    if ((ptr = strstr(json, P_COLON(F_A_OPERATIONMODE))) && 
        sscanf(ptr, P_FMT_STR(F_A_OPERATIONMODE), enum_val) == 1) {
        ctrl->operationMode = parse_operation_mode(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_PARM))) && 
        sscanf(ptr, P_FMT_STR(F_A_PARM), enum_val) == 1) {
        ctrl->parm = parse_onoff_type(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_TARGET_DESIGNATION))) && 
        sscanf(ptr, P_FMT_STR(F_A_TARGET_DESIGNATION), enum_val) == 1) {
        ctrl->targetDesingation = parse_target_allot(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_AUTO_ARM_POSITION))) && 
        sscanf(ptr, P_FMT_STR(F_A_AUTO_ARM_POSITION), enum_val) == 1) {
        ctrl->autoArmPosition = parse_arm_position_lock(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_MANUAL_ARM_POSITION))) && 
        sscanf(ptr, P_FMT_STR(F_A_MANUAL_ARM_POSITION), enum_val) == 1) {
        ctrl->manualArmPosition = parse_arm_position_lock(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_MAIN_CANNON_RESTORE))) && 
        sscanf(ptr, P_FMT_STR(F_A_MAIN_CANNON_RESTORE), enum_val) == 1) {
        ctrl->mainCannonRestore = parse_main_cannon_return(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_MAIN_CANNON_FIX))) && 
        sscanf(ptr, P_FMT_STR(F_A_MAIN_CANNON_FIX), enum_val) == 1) {
        ctrl->manCannonFix = parse_main_cannon_fix(enum_val);
    }
    if ((ptr = strstr(json, P_COLON(F_A_CLOSE_EQUIP_OPEN_STATUS))) && 
        sscanf(ptr, P_FMT_STR(F_A_CLOSE_EQUIP_OPEN_STATUS), enum_val) == 1) {
        ctrl->closeEquipOpenStatus = parse_equip_open_lock(enum_val);
    }
    
    ctrl->last_update_time = ctx->tick_count;
    ctx->control_rx_count++;
    
    // Log only every 100 messages (reduce verbosity)
    if ((ctx->control_rx_count % 100) == 0) {
        LOG_RX("Actuator Control: driving=%.2f, updown=%.2f, mode=%d (rx=%u)\n",
               ctrl->drivingPosition,
               ctrl->upDownPosition,
               (int)ctrl->operationMode,
               ctx->control_rx_count);
    }
}

void demo_msg_on_vehicle_speed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    (void)h;
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    if (evt->data_json == NULL && evt->raw_json != NULL) {
        // Binary struct event
        Wire_P_NSTEL_C_VehicleSpeed speed;
        memcpy(&speed, evt->raw_json, sizeof(speed));
        demo_log_hex("VehicleSpeed RX", &speed, sizeof(speed));
        demo_wire_vehicle_speed_be(&speed);
        ctx->speed_state.speed = speed.A_value;
        LOG_RX("Received VehicleSpeed (BINARY): %.2f m/s\n", speed.A_value);
    } else {
        const char* json = evt->data_json;
        if (!json) return;
        
        // Extract A_value (vehicle speed) using field macro
        const char* speed_ptr = strstr(json, P_COLON(F_A_SPEED));
        if (speed_ptr) {
            sscanf(speed_ptr, P_FMT_FLOAT(F_A_SPEED), &ctx->speed_state.speed);
        }
    }
    
    ctx->speed_state.last_update_time = ctx->tick_count;
    ctx->speed_rx_count++;
    
    if ((ctx->speed_rx_count % 100) == 0) {
        LOG_RX("Vehicle Speed: A_value=%.2f m/s (rx=%u)\n",
            ctx->speed_state.speed, ctx->speed_rx_count);
    }
}

/* ========================================================================
 * Publish Functions
 * ======================================================================== */

/* ========================================================================
 * Publish Functions
 * ======================================================================== */


int demo_msg_publish_pbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    LOG_TX("Publishing PBIT...\n");
    
    // Build PBIT JSON payload (12 components from XML schema)
    // Field names use 'Gyro' spelling per XML canonicalization
    char pbit_json[2048];
    BITComponentState* comp = &ctx->bit_state.pbit_components;
    
    int ppos = 0;
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "{");
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":{\"%s\":1,\"%s\":1},", F_A_SOURCEID, F_A_RESOURCEID, F_A_INSTANCEID);
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":{\"%s\":%lld,\"%s\":%d},", F_A_TIMEOFDATA, F_A_SECOND, (long long)(ctx->tick_count/1000), F_A_NANOSECONDS, (int)((ctx->tick_count%1000)*1000000));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":{\"%s\":1,\"%s\":1},", F_A_CANNON_SOURCEID, F_A_RESOURCEID, F_A_INSTANCEID);
    /* A_type is not part of PowerOnBIT schema — removed */
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":%s,", F_A_BITRUNNING, comp->bitRunning == L_BITResultType_NORMAL ? "true" : "false");
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_UPDOWNMOTOR, format_bit_result(comp->upDownMotor));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_ROUNDMOTOR, format_bit_result(comp->roundMotor));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_UPDOWNAMP, format_bit_result(comp->upDownAmp));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_ROUNDAMP, format_bit_result(comp->roundAmp));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_BASEGYRO, format_bit_result(comp->baseGyro));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_TOPFORWARDGYRO, format_bit_result(comp->topForwardGyro));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_VEHICLEFORWARDGYRO, format_bit_result(comp->vehicleForwardGyro));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_POWER_CONTROLLER, format_bit_result(comp->powerController));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_ENERGY_STORAGE, format_bit_result(comp->energyStorage));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\",", F_A_DIRECTPOWER, format_bit_result(comp->directPower));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "\"%s\":\"%s\"", F_A_CABLELOOP, format_bit_result(comp->cableLoop));
    ppos += snprintf(pbit_json + ppos, sizeof(pbit_json) - ppos, "}");
    
    // Write to DDS
    LegacyStatus status;
    if (ctx->data_codec == LEGACY_CODEC_STRUCT) {
        Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT wire;
        memset(&wire, 0, sizeof(wire));
        wire.A_sourceID.A_resourceId = 1;
        wire.A_sourceID.A_instanceId = 1;
        wire.A_timeOfDataGeneration.A_second = (int64_t)(ctx->tick_count/1000);
        wire.A_timeOfDataGeneration.A_nanoseconds = (int32_t)((ctx->tick_count%1000)*1000000);
        wire.A_cannonDrivingDevice_sourceID.A_resourceId = 1;
        wire.A_cannonDrivingDevice_sourceID.A_instanceId = 1;
        wire.A_BITRunning = (comp->bitRunning == L_BITResultType_NORMAL);
        wire.A_upDownMotor = (int32_t)comp->upDownMotor;
        wire.A_roundMotor = (int32_t)comp->roundMotor;
        wire.A_upDownAmp = (int32_t)comp->upDownAmp;
        wire.A_roundAmp = (int32_t)comp->roundAmp;
        wire.A_baseGyro = (int32_t)comp->baseGyro;
        wire.A_topForwardGyro = (int32_t)comp->topForwardGyro;
        wire.A_vehicleForwardGyro = (int32_t)comp->vehicleForwardGyro;
        wire.A_powerController = (int32_t)comp->powerController;
        wire.A_energyStorage = (int32_t)comp->energyStorage;
        wire.A_directPower = (int32_t)comp->directPower;
        wire.A_cableLoop = (int32_t)comp->cableLoop;

        Wire_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT wire_be = wire;
        demo_wire_poweronbit_be(&wire_be);
        demo_log_hex("PowerOnBIT TX", &wire_be, sizeof(wire_be));
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_struct(ctx->agent, &wire_be);
    } else {
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_PowerOnBIT_json(ctx->agent, pbit_json);
    }
    
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to send PBIT write request\n");
        return -1;
    }
    
    ctx->bit_state.pbit_completed = true;
    ctx->pbit_pub_count++;
    LOG_TX("PBIT published\n");
    return 0;
}

int demo_msg_publish_cbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    // Build CBIT JSON payload (15 components from XML schema)
    char cbit_json[2048];
    CBITComponentState* cbit = &ctx->bit_state.cbit_components;

#ifdef DEMO_PERF_INSTRUMENTATION
    uint64_t t_build0 = 0, t_build1 = 0;
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet();
        int tr = sysClkRateGet();
        t_build0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tbs; clock_gettime(CLOCK_MONOTONIC, &tbs); t_build0 = (uint64_t)tbs.tv_sec*1000000000ULL + tbs.tv_nsec;
#endif
#endif

    snprintf(cbit_json, sizeof(cbit_json),
        "{"
        "\"" F_A_SOURCEID "\":{"
            "\"" F_A_RESOURCEID "\":1,"
            "\"" F_A_INSTANCEID "\":1"
        "},"
        "\"" F_A_TIMEOFDATA "\":{"
            "\"" F_A_SECOND "\":%lld,"
            "\"" F_A_NANOSECONDS "\":%d"
        "},"
        "\"" F_A_CANNON_SOURCEID "\":{"
            "\"" F_A_RESOURCEID "\":1,"
            "\"" F_A_INSTANCEID "\":1"
        "},"
        /* A_type removed per schema; include controller network result instead */
        "\"" F_A_CONTROLLER_NETWORK "\":\"%s\","
        "\"" F_A_UPDOWNMOTOR "\":\"%s\"," 
        "\"" F_A_ROUNDMOTOR "\":\"%s\"," 
        "\"" F_A_UPDOWNAMP "\":\"%s\"," 
        "\"" F_A_ROUNDAMP "\":\"%s\"," 
        "\"" F_A_BASEGYRO "\":\"%s\"," 
        "\"" F_A_TOPFORWARDGYRO "\":\"%s\"," 
        "\"" F_A_VEHICLEFORWARDGYRO "\":\"%s\"," 
        "\"" F_A_POWER_CONTROLLER "\":\"%s\"," 
        "\"" F_A_ENERGY_STORAGE "\":\"%s\"," 
        "\"" F_A_DIRECTPOWER "\":\"%s\"," 
        "\"" F_A_CABLELOOP "\":\"%s\"," 
        "\"" F_A_UPDOWNPARK "\":\"%s\"," 
        "\"" F_A_ROUND_PARK "\":\"%s\"," 
        "\"" F_A_MAINCANNON_LOCK "\":\"%s\"," 
        "\"" F_A_COMMFAULT "\":\"%s\""
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        format_bit_result(cbit->base.upDownMotor),
        format_bit_result(cbit->base.roundMotor),
        format_bit_result(cbit->base.upDownAmp),
        format_bit_result(cbit->base.roundAmp),
        format_bit_result(cbit->base.baseGyro),
        format_bit_result(cbit->base.topForwardGyro),
        format_bit_result(cbit->base.vehicleForwardGyro),
        format_bit_result(cbit->base.powerController),
        format_bit_result(cbit->base.energyStorage),
        format_bit_result(cbit->base.directPower),
        format_bit_result(cbit->base.cableLoop),
        format_bit_result(cbit->upDownPark),
        format_bit_result(cbit->round_Park),
        format_bit_result(cbit->mainCannon_Lock),
        format_bit_result(cbit->controllerNetwork),
        format_bit_result(cbit->commFault)
    );
    
    LegacyStatus status;
    if (ctx->data_codec == LEGACY_CODEC_STRUCT) {
        Wire_P_NSTEL_C_CannonDrivingDevice_PBIT wire;
        memset(&wire, 0, sizeof(wire));
        wire.A_sourceID.A_resourceId = 1;
        wire.A_sourceID.A_instanceId = 1;
        wire.A_timeOfDataGeneration.A_second = (int64_t)(ctx->tick_count/1000);
        wire.A_timeOfDataGeneration.A_nanoseconds = (int32_t)((ctx->tick_count%1000)*1000000);
        wire.A_cannonDrivingDevice_sourceID.A_resourceId = 1;
        wire.A_cannonDrivingDevice_sourceID.A_instanceId = 1;
        
        wire.A_upDownMotor = (int32_t)cbit->base.upDownMotor;
        wire.A_roundMotor = (int32_t)cbit->base.roundMotor;
        wire.A_upDownAmp = (int32_t)cbit->base.upDownAmp;
        wire.A_roundAmp = (int32_t)cbit->base.roundAmp;
        wire.A_baseGyro = (int32_t)cbit->base.baseGyro;
        wire.A_topForwardGyro = (int32_t)cbit->base.topForwardGyro;
        wire.A_vehicleForwardGyro = (int32_t)cbit->base.vehicleForwardGyro;
        wire.A_powerController = (int32_t)cbit->base.powerController;
        wire.A_energyStorage = (int32_t)cbit->base.energyStorage;
        wire.A_directPower = (int32_t)cbit->base.directPower;
        wire.A_cableLoop = (int32_t)cbit->base.cableLoop;
        wire.A_upDownPark = (int32_t)cbit->upDownPark;
        wire.A_roundPark = (int32_t)cbit->round_Park;
        wire.A_mainCannonLock = (int32_t)cbit->mainCannon_Lock;
        wire.A_controllerNetwork = (int32_t)cbit->controllerNetwork;

        Wire_P_NSTEL_C_CannonDrivingDevice_PBIT wire_be = wire;
        demo_wire_pbit_be(&wire_be);
        demo_log_hex("PBIT TX", &wire_be, sizeof(wire_be));
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_struct(ctx->agent, &wire_be);
    } else {
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_PBIT_json(ctx->agent, cbit_json);
    }

#ifdef DEMO_PERF_INSTRUMENTATION
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet();
        int tr = sysClkRateGet();
        t_write1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec twe; clock_gettime(CLOCK_MONOTONIC, &twe); t_write1 = (uint64_t)twe.tv_sec*1000000000ULL + twe.tv_nsec;
#endif
    /* accumulate build and write times */
    if (t_build1 == 0) {
#if defined(_VXWORKS_)
        unsigned long tk = tickGet(); int tr = sysClkRateGet(); t_build1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
#else
        struct timespec tbe; clock_gettime(CLOCK_MONOTONIC, &tbe); t_build1 = (uint64_t)tbe.tv_sec*1000000000ULL + tbe.tv_nsec;
#endif
    }
    ctx->json_dump_ns_total += (t_build1 > t_build0) ? (t_build1 - t_build0) : 0ULL;
    ctx->json_dump_count++;
    ctx->legacy_write_ns_total += (t_write1 > t_write0) ? (t_write1 - t_write0) : 0ULL;
    ctx->legacy_write_count++;
#endif
    if (status != LEGACY_OK) {
        return -1;
    }
    
    ctx->cbit_pub_count++;
    LOG_TX("CBIT published (count=%u)\n", ctx->cbit_pub_count);
    return 0;
}

int demo_msg_publish_result_bit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    // Build resultBIT JSON payload (12 components + referenceNum)
    // Using canonical field name: A_powerController (no underscore)
    char json[2048];
    BITComponentState* result = &ctx->bit_state.result_components;
    
    int rpos = 0;
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "{");
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":{\"%s\":1,\"%s\":1},", F_A_SOURCEID, F_A_RESOURCEID, F_A_INSTANCEID);
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":{\"%s\":%lld,\"%s\":%d},", F_A_TIMEOFDATA, F_A_SECOND, (long long)(ctx->tick_count/1000), F_A_NANOSECONDS, (int)((ctx->tick_count%1000)*1000000));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":%u,", F_A_REFERENCE_NUM, ctx->bit_state.ibit_reference_num);
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":{\"%s\":1,\"%s\":1},", F_A_CANNON_SOURCEID, F_A_RESOURCEID, F_A_INSTANCEID);
    /* A_type is not part of IBIT schema — removed */
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":%s,", F_A_BITRUNNING, result->bitRunning == L_BITResultType_NORMAL ? "true" : "false");
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_UPDOWNMOTOR, format_bit_result(result->upDownMotor));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_ROUNDMOTOR, format_bit_result(result->roundMotor));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_UPDOWNAMP, format_bit_result(result->upDownAmp));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_ROUNDAMP, format_bit_result(result->roundAmp));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_BASEGYRO, format_bit_result(result->baseGyro));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_TOPFORWARDGYRO, format_bit_result(result->topForwardGyro));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_VEHICLEFORWARDGYRO, format_bit_result(result->vehicleForwardGyro));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_POWER_CONTROLLER, format_bit_result(result->powerController));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_ENERGY_STORAGE, format_bit_result(result->energyStorage));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\",", F_A_DIRECTPOWER, format_bit_result(result->directPower));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "\"%s\":\"%s\"", F_A_CABLELOOP, format_bit_result(result->cableLoop));
    rpos += snprintf(json + rpos, sizeof(json) - rpos, "}");
    
    LegacyStatus status;
    if (ctx->data_codec == LEGACY_CODEC_STRUCT) {
        Wire_P_NSTEL_C_CannonDrivingDevice_IBIT wire;
        memset(&wire, 0, sizeof(wire));
        wire.A_sourceID.A_resourceId = 1;
        wire.A_sourceID.A_instanceId = 1;
        wire.A_timeOfDataGeneration.A_second = (int64_t)(ctx->tick_count/1000);
        wire.A_timeOfDataGeneration.A_nanoseconds = (int32_t)((ctx->tick_count%1000)*1000000);
        wire.A_cannonDrivingDevice_sourceID.A_resourceId = 1;
        wire.A_cannonDrivingDevice_sourceID.A_instanceId = 1;
        wire.A_referenceNum = (int32_t)ctx->bit_state.ibit_reference_num;
        wire.A_BITRunning = (result->bitRunning == L_BITResultType_NORMAL);
        
        wire.A_upDownMotor = (int32_t)result->upDownMotor;
        wire.A_roundMotor = (int32_t)result->roundMotor;
        wire.A_upDownAmp = (int32_t)result->upDownAmp;
        wire.A_roundAmp = (int32_t)result->roundAmp;
        wire.A_baseGyro = (int32_t)result->baseGyro;
        wire.A_topForwardGyro = (int32_t)result->topForwardGyro;
        wire.A_vehicleForwardGyro = (int32_t)result->vehicleForwardGyro;
        wire.A_powerController = (int32_t)result->powerController;
        wire.A_energyStorage = (int32_t)result->energyStorage;
        wire.A_directPower = (int32_t)result->directPower;
        wire.A_cableLoop = (int32_t)result->cableLoop;

        Wire_P_NSTEL_C_CannonDrivingDevice_IBIT wire_be = wire;
        demo_wire_ibit_be(&wire_be);
        demo_log_hex("IBIT TX", &wire_be, sizeof(wire_be));
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_struct(ctx->agent, &wire_be);
    } else {
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_IBIT_json(ctx->agent, json);
    }
    
    if (status != LEGACY_OK) {
        LOG_INFO("ERROR: Failed to publish resultBIT\n");
        return -1;
    }
    
    ctx->result_pub_count++;
    
    // Calculate overall result
    bool has_fault = !(
        result->upDownMotor == L_BITResultType_NORMAL &&
        result->roundMotor == L_BITResultType_NORMAL &&
        result->upDownAmp == L_BITResultType_NORMAL &&
        result->roundAmp == L_BITResultType_NORMAL &&
        result->baseGyro == L_BITResultType_NORMAL &&
        result->topForwardGyro == L_BITResultType_NORMAL &&
        result->vehicleForwardGyro == L_BITResultType_NORMAL &&
        result->powerController == L_BITResultType_NORMAL &&
        result->energyStorage == L_BITResultType_NORMAL &&
        result->directPower == L_BITResultType_NORMAL &&
        result->cableLoop == L_BITResultType_NORMAL
    );
    
    LOG_TX("resultBIT published: ref=%u, result=%s\n",
           ctx->bit_state.ibit_reference_num,
           has_fault ? "FAIL" : "PASS");
    
    return 0;
}

int demo_msg_publish_actuator_signal(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;

#ifdef DEMO_PERF_INSTRUMENTATION
    uint64_t t_publish_start = 0;
#ifdef _VXWORKS_
    t_publish_start = ctx ? ctx->tick_count : 0;
#else
    struct timespec tps0; clock_gettime(CLOCK_MONOTONIC, &tps0); t_publish_start = (uint64_t)tps0.tv_sec*1000ULL + (uint64_t)(tps0.tv_nsec/1000000ULL);
#endif
#endif
    
    // Build Actuator Signal JSON payload (200Hz feedback with 14 fields)
    // Note: A_manualArmPositionComple has typo (Comple not Complement)
    char signal_json[2048];
    ActuatorSignalState* sig = &ctx->signal_state;
    
    // --- Quantize / Clamp outputs according to spec ---
#ifdef DEMO_PERF_INSTRUMENTATION
    uint64_t t_build0 = 0, t_build1 = 0;
    uint64_t t_write0 = 0, t_write1 = 0;
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet();
        int tr = sysClkRateGet();
        t_build0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tps; clock_gettime(CLOCK_MONOTONIC, &tps); t_build0 = (uint64_t)tps.tv_sec*1000000000ULL + tps.tv_nsec;
#endif
#endif
    // A_azAngle: send current azimuth velocity (sig->e1AngleVelocity) but keep field name
    float az_v = sig->e1AngleVelocity; // treat A_azAngle as velocity
    if (az_v > 800.0f) az_v = 800.0f;
    if (az_v < -800.0f) az_v = -800.0f;
    az_v = roundf(az_v / 0.01f) * 0.01f;

    // A_e1AngleVelocity
    float e1_v = sig->e1AngleVelocity;
    if (e1_v > 450.0f) e1_v = 450.0f;
    if (e1_v < -450.0f) e1_v = -450.0f;
    e1_v = roundf(e1_v / 0.01f) * 0.01f;

    // A_roundGyro
    float round_v = sig->roundGyro;
    if (round_v > 655.0f) round_v = 655.0f;
    if (round_v < -655.0f) round_v = -655.0f;
    round_v = roundf(round_v / 0.02f) * 0.02f;

    // A_upDownGyro
    float updown_v = sig->upDownGyro;
    if (updown_v > 655.0f) updown_v = 655.0f;
    if (updown_v < -655.0f) updown_v = -655.0f;
    updown_v = roundf(updown_v / 0.02f) * 0.02f;

    snprintf(signal_json, sizeof(signal_json),
        "{"
        "\"" F_A_RECIPIENTID "\":{"
            "\"" F_A_RESOURCEID "\":1,"
            "\"" F_A_INSTANCEID "\":1"
        "},"
        "\"" F_A_SOURCEID "\":{"
            "\"" F_A_RESOURCEID "\":1,"
            "\"" F_A_INSTANCEID "\":1"
        "},"
        "\"" F_A_TIMEOFDATA "\":{"
            "\"" F_A_SECOND "\":%lld,"
            "\"" F_A_NANOSECONDS "\":%d"
        "},"
        "\"" F_A_AZANGLEVELOCITY "\":%.3f,"
        "\"" F_A_E1ANGLEVELOCITY "\":%.3f,"
        "\"" F_A_ENERGY_STORAGE "\":\"%s\","
        "\"" F_A_MAINCANNONFIXSTATUS "\":\"%s\","
        "\"" F_A_DECKCLEARANCE "\":\"%s\","
        "\"" F_A_AUTO_ARM_POSITION_COMPLEMENT "\":\"%s\","
        "\"" F_A_MANUAL_ARM_POSITION_COMPLEMENT "\":\"%s\","
        "\"" F_A_MAIN_CANNON_RESTORE_COMPLEMENT "\":\"%s\","
        "\"" F_A_ARM_SAFETY_MAIN_CANNON_LOCK "\":\"%s\","
        "\"" F_A_SHUTDOWN "\":\"%s\","
        "\"" F_A_ROUNDGYRO "\":%.3f,"
        "\"" F_A_UPDOWNGYRO "\":%.3f"
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        az_v,
        e1_v,
        format_energy_storage(sig->energyStorage),
        format_main_cannon_fix_status(sig->mainCannonFixStatus),
        format_dek_clearance(sig->deckClearance),
        format_cannon_driving_from_arm(sig->autoArmPositionComplement),
        format_cannon_driving_from_arm(sig->manualArmPositionComple),
        format_cannon_driving_from_return(sig->mainCannonRestoreComplement),
        format_arm_safety_lock(sig->armSafetyMainCannonLock),
        format_shutdown_type(sig->shutdown),
        round_v,
        updown_v
    );
    
#ifdef DEMO_PERF_INSTRUMENTATION
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet();
        int tr = sysClkRateGet();
        t_build1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tbend; clock_gettime(CLOCK_MONOTONIC, &tbend); t_build1 = (uint64_t)tbend.tv_sec*1000000000ULL + tbend.tv_nsec;
#endif
#endif

    /* measure write/send time */
#ifdef DEMO_PERF_INSTRUMENTATION
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet(); int tr = sysClkRateGet(); t_write0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tw0; clock_gettime(CLOCK_MONOTONIC, &tw0); t_write0 = (uint64_t)tw0.tv_sec*1000000000ULL + tw0.tv_nsec;
#endif
#endif

    LegacyStatus status;
    if (ctx->data_codec == LEGACY_CODEC_STRUCT) {
        // Data Plane STRUCT
        Wire_P_NSTEL_C_CannonDrivingDevice_Signal wire_sig;
        memset(&wire_sig, 0, sizeof(wire_sig));
        
        wire_sig.A_sourceID.A_resourceId = 1;
        wire_sig.A_sourceID.A_instanceId = 1;
        wire_sig.A_recipientID.A_resourceId = 1;
        wire_sig.A_recipientID.A_instanceId = 1;
        wire_sig.A_timeOfDataGeneration.A_second = (int64_t)(ctx->tick_count / 1000);
        wire_sig.A_timeOfDataGeneration.A_nanoseconds = (int32_t)((ctx->tick_count % 1000) * 1000000);

        wire_sig.A_azAngleVelocity = (double)az_v;
        wire_sig.A_e1AngleVelocity = (double)e1_v;
        wire_sig.A_energyStorage = (int32_t)sig->energyStorage;
        wire_sig.A_mainCannonFixStatus = (int32_t)sig->mainCannonFixStatus;
        wire_sig.A_deckCleance = (int32_t)sig->deckClearance;
        wire_sig.A_autoArmPositionComplement = (int32_t)sig->autoArmPositionComplement;
        wire_sig.A_manualArmPositionComplement = (int32_t)sig->manualArmPositionComple;
        wire_sig.A_mainCannonRestoreComplement = (int32_t)sig->mainCannonRestoreComplement;
        wire_sig.A_armSafetyMainCannonLock = (int32_t)sig->armSafetyMainCannonLock;
        wire_sig.A_shutdown = (int32_t)sig->shutdown;
        wire_sig.A_roundGyro = (double)round_v;
        wire_sig.A_upDownGyro = (double)updown_v;

        Wire_P_NSTEL_C_CannonDrivingDevice_Signal wire_sig_be = wire_sig;
        demo_wire_signal_be(&wire_sig_be);
        demo_log_hex("Signal TX", &wire_sig_be, sizeof(wire_sig_be));
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_struct(ctx->agent, &wire_sig_be);
    } else {
        status = legacy_send_P_NSTEL_C_CannonDrivingDevice_Signal_json(ctx->agent, signal_json);
    }

#ifdef DEMO_PERF_INSTRUMENTATION
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet(); int tr = sysClkRateGet(); t_write1 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tw1; clock_gettime(CLOCK_MONOTONIC, &tw1); t_write1 = (uint64_t)tw1.tv_sec*1000000000ULL + tw1.tv_nsec;
#endif
    /* accumulate */
    ctx->json_dump_ns_total += (t_build1 > t_build0) ? (t_build1 - t_build0) : 0ULL;
    ctx->json_dump_count++;
    ctx->legacy_write_ns_total += (t_write1 > t_write0) ? (t_write1 - t_write0) : 0ULL;
    ctx->legacy_write_count++;
#endif
#ifdef DEMO_PERF_INSTRUMENTATION
    uint64_t t_publish_end = 0;
#ifdef _VXWORKS_
    t_publish_end = ctx ? ctx->tick_count : 0;
    demo_log(LOG_LEVEL_INFO, "[TIMING] publish_signal JSON+send approx %llu ms (ticks delta=%llu)\n",
             (unsigned long long)(t_publish_end - t_publish_start),
             (unsigned long long)(t_publish_end - t_publish_start));
#else
    struct timespec tpe; clock_gettime(CLOCK_MONOTONIC, &tpe); t_publish_end = (uint64_t)tpe.tv_sec*1000ULL + (uint64_t)(tpe.tv_nsec/1000000ULL);
    demo_log(LOG_LEVEL_INFO, "[TIMING] publish_signal JSON+send took %llu ms\n", (unsigned long long)(t_publish_end - t_publish_start));
#endif
#endif
    if (status != LEGACY_OK) {
        return -1;
    }
    
    ctx->signal_pub_count++;
    if ((ctx->signal_pub_count % 200) == 0) {  // Log every 200 messages (1 second)
        LOG_TX("Actuator Signal published (count=%u, 200Hz)\n", ctx->signal_pub_count);
    }
    return 0;
}

/* ========================================================================
 * Test Write Functions (1회성 전송)
 * ======================================================================== */

int demo_msg_test_write_pbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    LOG_INFO("Sending PBIT test message...\n");
    return demo_msg_publish_pbit(ctx);
}

int demo_msg_test_write_cbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    LOG_INFO("Sending CBIT test message...\n");
    return demo_msg_publish_cbit(ctx);
}

int demo_msg_test_write_signal(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    LOG_INFO("Sending Actuator Signal test message...\n");
    return demo_msg_publish_actuator_signal(ctx);
}

int demo_msg_test_write_result_bit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;

    // Set dummy reference number for test
    ctx->bit_state.ibit_reference_num = 999;

    LOG_INFO("Sending resultBIT test message (ref=999)...\n");
    return demo_msg_publish_result_bit(ctx);
}
