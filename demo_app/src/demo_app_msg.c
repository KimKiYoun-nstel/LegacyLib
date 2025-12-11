/*
 * demo_app_msg.c - Message Handlers and Publishers
 */

#include "../include/demo_app.h"
#include <stdio.h>
#include <string.h>

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#endif

/* ========================================================================
 * Callbacks
 * ======================================================================== */

static void on_writer_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    const char* topic = (const char*)user;
    if (res->ok) {
        printf("[DemoApp Msg] Writer created: %s\n", topic);
    } else {
        printf("[DemoApp Msg] ERROR: Failed to create writer for %s: %s\n",
               topic, res->msg ? res->msg : "Unknown");
    }
}

static void on_reader_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    const char* topic = (const char*)user;
    if (res->ok) {
        printf("[DemoApp Msg] Reader created: %s\n", topic);
    } else {
        printf("[DemoApp Msg] ERROR: Failed to create reader for %s: %s\n",
               topic, res->msg ? res->msg : "Unknown");
    }
}

/* ========================================================================
 * Initialization
 * ======================================================================== */

int demo_msg_init(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Msg] Initializing message handlers...\n");
    
    LegacyStatus status;
    
    // ===== Create Writers (4 send topics) =====
    
    // 1. PBIT Writer (Initial State QoS)
    LegacyWriterConfig pbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_PBIT,
        TYPE_PBIT,
        "NstelCustomQosLib::InitialStateProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &pbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_PBIT);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create PBIT writer\n");
        return -1;
    }
    
    // 2. CBIT Writer (Low Frequency Status QoS - 1Hz)
    LegacyWriterConfig cbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_CBIT,
        TYPE_CBIT,
        "NstelCustomQosLib::LowFreqStatusProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &cbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_CBIT);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create CBIT writer\n");
        return -1;
    }
    
    // 3. ResultBIT Writer (Non-Periodic Event QoS)
    LegacyWriterConfig rbit_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_RESULT_BIT,
        TYPE_RESULT_BIT,
        "NstelCustomQosLib::NonPeriodicEventProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &rbit_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_RESULT_BIT);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create ResultBIT writer\n");
        return -1;
    }
    
    // 4. Actuator Signal Writer (High Frequency Periodic QoS - 200Hz)
    LegacyWriterConfig signal_wcfg = {
        ctx->domain_id,
        "pub1",
        TOPIC_ACTUATOR_SIGNAL,
        TYPE_ACTUATOR_SIGNAL,
        "NstelCustomQosLib::HighFreqPeriodicProfile"
    };
    status = legacy_agent_create_writer(ctx->agent, &signal_wcfg, 2000,
                                       on_writer_created, (void*)TOPIC_ACTUATOR_SIGNAL);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create Actuator Signal writer\n");
        return -1;
    }
    
    // ===== Create Readers (3 receive topics) =====
    
    // 1. runBIT Reader (Non-Periodic Event QoS)
    LegacyReaderConfig runbit_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_RUNBIT,
        TYPE_RUNBIT,
        "NstelCustomQosLib::NonPeriodicEventProfile"
    };
    status = legacy_agent_create_reader(ctx->agent, &runbit_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_RUNBIT);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create runBIT reader\n");
        return -1;
    }
    
    // Subscribe to runBIT events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_RUNBIT, TYPE_RUNBIT,
                                         demo_msg_on_runbit, ctx);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to subscribe to runBIT\n");
        return -1;
    }
    
    // 2. Actuator Control Reader (High Frequency Periodic QoS - 200Hz)
    LegacyReaderConfig control_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_ACTUATOR_CONTROL,
        TYPE_ACTUATOR_CONTROL,
        "NstelCustomQosLib::HighFreqPeriodicProfile"
    };
    status = legacy_agent_create_reader(ctx->agent, &control_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_ACTUATOR_CONTROL);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create Actuator Control reader\n");
        return -1;
    }
    
    // Subscribe to control events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_ACTUATOR_CONTROL,
                                         TYPE_ACTUATOR_CONTROL,
                                         demo_msg_on_actuator_control, ctx);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to subscribe to Actuator Control\n");
        return -1;
    }
    
    // 3. Vehicle Speed Reader (Low Frequency Vehicle QoS - 1Hz)
    LegacyReaderConfig speed_rcfg = {
        ctx->domain_id,
        "sub1",
        TOPIC_VEHICLE_SPEED,
        TYPE_VEHICLE_SPEED,
        "NstelCustomQosLib::LowFreqVehicleProfile"
    };
    status = legacy_agent_create_reader(ctx->agent, &speed_rcfg, 2000,
                                       on_reader_created, (void*)TOPIC_VEHICLE_SPEED);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to create Vehicle Speed reader\n");
        return -1;
    }
    
    // Subscribe to speed events
    status = legacy_agent_subscribe_event(ctx->agent, TOPIC_VEHICLE_SPEED,
                                         TYPE_VEHICLE_SPEED,
                                         demo_msg_on_vehicle_speed, ctx);
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to subscribe to Vehicle Speed\n");
        return -1;
    }
    
    // Give time for all entities to be created
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet());  // 1 second
    #endif
    
    printf("[DemoApp Msg] All message handlers initialized (7 topics)\n");
    return 0;
}

void demo_msg_cleanup(DemoAppContext* ctx) {
    if (!ctx) return;
    
    printf("[DemoApp Msg] Cleaning up message handlers...\n");
    
    // TODO Phase 3: Unsubscribe and delete writers
}

/* ========================================================================
 * Receive Callbacks
 * ======================================================================== */

void demo_msg_on_runbit(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    const char* json = evt->data_json;
    if (!json) return;
    
    printf("[DemoApp Msg] Received runBIT: %s\n", json);
    
    // Parse A_referenceNum
    uint32_t reference_num = 0;
    const char* ref_str = strstr(json, "\"A_referenceNum\":");
    if (ref_str) {
        sscanf(ref_str, "\"A_referenceNum\":%u", &reference_num);
    }
    
    // Parse A_type (enum string)
    T_BITType type = L_BITType_I_BIT;  // Default to I_BIT
    const char* type_str = strstr(json, "\"A_type\":");
    if (type_str) {
        // Extract enum string value
        char type_value[64];
        if (sscanf(type_str, "\"A_type\":\"%63[^\"]\"", type_value) == 1) {
            type = parse_bit_type(type_value);
        }
    }
    
    printf("[DemoApp Msg] runBIT parsed: A_referenceNum=%u, A_type=%d\n", reference_num, (int)type);
    
    // Trigger IBIT
    if (demo_app_trigger_ibit(ctx, reference_num, type) != 0) {
        printf("[DemoApp Msg] WARNING: Failed to trigger IBIT\n");
    }
}

void demo_msg_on_actuator_control(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    const char* json = evt->data_json;
    if (!json) return;
    
    ActuatorControlState* ctrl = &ctx->control_state;
    
    // Extract float fields
    const char* ptr;
    if ((ptr = strstr(json, "\"A_drivingPosition\":"))) {
        sscanf(ptr, "\"A_drivingPosition\":%f", &ctrl->drivingPosition);
    }
    if ((ptr = strstr(json, "\"A_upDownPosition\":"))) {
        sscanf(ptr, "\"A_upDownPosition\":%f", &ctrl->upDownPosition);
    }
    if ((ptr = strstr(json, "\"A_roundAngleVelocity\":"))) {
        sscanf(ptr, "\"A_roundAngleVelocity\":%f", &ctrl->roundAngleVelocity);
    }
    if ((ptr = strstr(json, "\"A_upDownAngleVelocity\":"))) {
        sscanf(ptr, "\"A_upDownAngleVelocity\":%f", &ctrl->upDownAngleVelocity);
    }
    if ((ptr = strstr(json, "\"A_cannonUpDownAngle\":"))) {
        sscanf(ptr, "\"A_cannonUpDownAngle\":%f", &ctrl->cannonUpDownAngle);
    }
    if ((ptr = strstr(json, "\"A_topRelativeAngle\":"))) {
        sscanf(ptr, "\"A_topRelativeAngle\":%f", &ctrl->topRelativeAngle);
    }
    
    // Extract enum fields
    char enum_val[64];
    if ((ptr = strstr(json, "\"A_operationMode\":")) && 
        sscanf(ptr, "\"A_operationMode\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->operationMode = parse_operation_mode(enum_val);
    }
    if ((ptr = strstr(json, "\"A_parm\":")) && 
        sscanf(ptr, "\"A_parm\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->parm = parse_onoff_type(enum_val);
    }
    if ((ptr = strstr(json, "\"A_targetDesingation\":")) && 
        sscanf(ptr, "\"A_targetDesingation\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->targetDesingation = parse_target_allot(enum_val);
    }
    if ((ptr = strstr(json, "\"A_autoArmPosition\":")) && 
        sscanf(ptr, "\"A_autoArmPosition\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->autoArmPosition = parse_arm_position_lock(enum_val);
    }
    if ((ptr = strstr(json, "\"A_manualArmPosition\":")) && 
        sscanf(ptr, "\"A_manualArmPosition\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->manualArmPosition = parse_arm_position_lock(enum_val);
    }
    if ((ptr = strstr(json, "\"A_mainCannonRestore\":")) && 
        sscanf(ptr, "\"A_mainCannonRestore\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->mainCannonRestore = parse_main_cannon_return(enum_val);
    }
    if ((ptr = strstr(json, "\"A_manCannonFix\":")) && 
        sscanf(ptr, "\"A_manCannonFix\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->manCannonFix = parse_main_cannon_fix(enum_val);
    }
    if ((ptr = strstr(json, "\"A_closeEquipOpenStatus\":")) && 
        sscanf(ptr, "\"A_closeEquipOpenStatus\":\"%63[^\"]\"", enum_val) == 1) {
        ctrl->closeEquipOpenStatus = parse_equip_open_lock(enum_val);
    }
    
    ctrl->last_update_time = ctx->tick_count;
    ctx->control_rx_count++;
    
    // Log only every 100 messages (reduce verbosity)
    if ((ctx->control_rx_count % 100) == 0) {
        printf("[DemoApp Msg] Actuator Control: driving=%.2f, updown=%.2f, mode=%d (rx=%u)\n",
               ctrl->drivingPosition,
               ctrl->upDownPosition,
               (int)ctrl->operationMode,
               ctx->control_rx_count);
    }
}

void demo_msg_on_vehicle_speed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    // Parse JSON for vehicle speed
    const char* json = evt->data_json;
    if (!json) return;
    
    // Extract A_speed value
    const char* speed_ptr = strstr(json, "\"A_speed\":");
    if (speed_ptr) {
        sscanf(speed_ptr, "\"A_speed\":%f", &ctx->speed_state.speed);
    }
    
    ctx->speed_state.last_update_time = ctx->tick_count;
    ctx->speed_rx_count++;
    
    printf("[DemoApp Msg] Vehicle Speed: A_speed=%.2f m/s (rx=%u)\n",
           ctx->speed_state.speed, ctx->speed_rx_count);
}

/* ========================================================================
 * Publish Functions
 * ======================================================================== */

/* ========================================================================
 * Write Callbacks
 * ======================================================================== */

static void on_write_complete(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    const char* msg_type = (const char*)user;
    if (res->ok) {
        // Suppress verbose logging for high-frequency messages
        if (strcmp(msg_type, "CBIT") == 0 || strcmp(msg_type, "Signal") == 0) {
            // Silent success for periodic messages
        } else {
            printf("[DemoApp Msg] Published %s successfully\n", msg_type);
        }
    } else {
        printf("[DemoApp Msg] ERROR: Failed to publish %s: %s\n",
               msg_type, res->msg ? res->msg : "Unknown");
    }
}

/* ========================================================================
 * Publish Functions
 * ======================================================================== */

int demo_msg_publish_pbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Msg] Publishing PBIT...\n");
    
    // Build PBIT JSON payload (12 components from XML schema)
    // Note: A_vehicleForwardGyroi has 'i' typo in PBIT schema
    char pbit_json[2048];
    BITComponentState* comp = &ctx->bit_state.pbit_components;
    
    snprintf(pbit_json, sizeof(pbit_json),
        "{"
        "\"A_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_timeOfDataGeneration\":{"
            "\"A_second\":%lld,"
            "\"A_nanoseconds\":%d"
        "},"
        "\"A_runBITEntity_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_type\":\"%s\","
        "\"A_BITRunning\":%s,"
        "\"A_upDownMotor\":%s,"
        "\"A_roundMotor\":%s,"
        "\"A_upDownAmp\":%s,"
        "\"A_roundAmp\":%s,"
        "\"A_baseGyro\":%s,"
        "\"A_topForwardGryro\":%s,"
        "\"A_vehicleForwardGyroi\":%s,"
        "\"A_powerController\":%s,"
        "\"A_energyStorage\":%s,"
        "\"A_directPower\":%s,"
        "\"A_cableLoop\":%s"
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        format_bit_type(L_BITType_P_BIT),
        comp->bitRunning ? "true" : "false",
        comp->upDownMotor ? "true" : "false",
        comp->roundMotor ? "true" : "false",
        comp->upDownAmp ? "true" : "false",
        comp->roundAmp ? "true" : "false",
        comp->baseGyro ? "true" : "false",
        comp->topForwardGryro ? "true" : "false",
        comp->vehicleForwardGyro ? "true" : "false",
        comp->powerController ? "true" : "false",
        comp->energyStorage ? "true" : "false",
        comp->directPower ? "true" : "false",
        comp->cableLoop ? "true" : "false"
    );
    
    // Write to DDS
    LegacyWriteJsonOptions wopt = {
        TOPIC_PBIT,
        TYPE_PBIT,
        pbit_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::InitialStateProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 2000,
                                                  on_write_complete, (void*)"PBIT");
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to send PBIT write request\n");
        return -1;
    }
    
    ctx->bit_state.pbit_completed = true;
    printf("[DemoApp Msg] PBIT published\n");
    return 0;
}

int demo_msg_publish_cbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    // Build CBIT JSON payload (15 components from XML schema)
    char cbit_json[2048];
    CBITComponentState* cbit = &ctx->bit_state.cbit_components;
    
    snprintf(cbit_json, sizeof(cbit_json),
        "{"
        "\"A_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_timeOfDataGeneration\":{"
            "\"A_second\":%lld,"
            "\"A_nanoseconds\":%d"
        "},"
        "\"A_runBITEntity_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_type\":\"%s\","
        "\"A_upDownMotor\":%s,"
        "\"A_roundMotor\":%s,"
        "\"A_upDownAmp\":%s,"
        "\"A_roundAmp\":%s,"
        "\"A_baseGyro\":%s,"
        "\"A_topForwardGryro\":%s,"
        "\"A_vehicleForwardGyro\":%s,"
        "\"A_powerController\":%s,"
        "\"A_energyStorage\":%s,"
        "\"A_directPower\":%s,"
        "\"A_cableLoop\":%s,"
        "\"A_upDownPark\":%s,"
        "\"A_round_Park\":%s,"
        "\"A_mainCannon_Lock\":%s,"
        "\"A_commFault\":%s"
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        format_bit_type(L_BITType_C_BIT),
        cbit->base.upDownMotor ? "true" : "false",
        cbit->base.roundMotor ? "true" : "false",
        cbit->base.upDownAmp ? "true" : "false",
        cbit->base.roundAmp ? "true" : "false",
        cbit->base.baseGyro ? "true" : "false",
        cbit->base.topForwardGryro ? "true" : "false",
        cbit->base.vehicleForwardGyro ? "true" : "false",
        cbit->base.powerController ? "true" : "false",
        cbit->base.energyStorage ? "true" : "false",
        cbit->base.directPower ? "true" : "false",
        cbit->base.cableLoop ? "true" : "false",
        cbit->upDownPark ? "true" : "false",
        cbit->round_Park ? "true" : "false",
        cbit->mainCannon_Lock ? "true" : "false",
        cbit->commFault ? "true" : "false"
    );
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_CBIT,
        TYPE_CBIT,
        cbit_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::LowFreqStatusProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 1000,
                                                  on_write_complete, (void*)"CBIT");
    if (status != LEGACY_OK) {
        return -1;
    }
    
    ctx->cbit_pub_count++;
    return 0;
}

int demo_msg_publish_result_bit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    // Build resultBIT JSON payload (12 components + referenceNum)
    // Note: A_power_Controller has underscore in different position
    char json[2048];
    BITComponentState* result = &ctx->bit_state.result_components;
    
    snprintf(json, sizeof(json),
        "{"
        "\"A_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_timeOfDataGeneration\":{"
            "\"A_second\":%lld,"
            "\"A_nanoseconds\":%d"
        "},"
        "\"A_referenceNum\":%u,"
        "\"A_runBITEntity_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_type\":\"%s\","
        "\"A_BITRunning\":%s,"
        "\"A_upDownMotor\":%s,"
        "\"A_roundMotor\":%s,"
        "\"A_upDownAmp\":%s,"
        "\"A_roundAmp\":%s,"
        "\"A_baseGyro\":%s,"
        "\"A_topForwardGryro\":%s,"
        "\"A_vehicleForwardGyro\":%s,"
        "\"A_power_Controller\":%s,"
        "\"A_energyStorage\":%s,"
        "\"A_directPower\":%s,"
        "\"A_cableLoop\":%s"
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        ctx->bit_state.ibit_reference_num,
        format_bit_type(L_BITType_I_BIT),
        result->bitRunning ? "true" : "false",
        result->upDownMotor ? "true" : "false",
        result->roundMotor ? "true" : "false",
        result->upDownAmp ? "true" : "false",
        result->roundAmp ? "true" : "false",
        result->baseGyro ? "true" : "false",
        result->topForwardGryro ? "true" : "false",
        result->vehicleForwardGyro ? "true" : "false",
        result->powerController ? "true" : "false",
        result->energyStorage ? "true" : "false",
        result->directPower ? "true" : "false",
        result->cableLoop ? "true" : "false"
    );
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_RESULT_BIT,
        TYPE_RESULT_BIT,
        json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::NonPeriodicEventProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 2000,
                                                  on_write_complete, (void*)"ResultBIT");
    
    if (status != LEGACY_OK) {
        printf("[DemoApp Msg] ERROR: Failed to publish resultBIT\n");
        return -1;
    }
    
    // Calculate overall result
    bool has_fault = result->upDownMotor || result->roundMotor || result->upDownAmp ||
                     result->roundAmp || result->baseGyro || result->topForwardGryro ||
                     result->vehicleForwardGyro || result->powerController ||
                     result->energyStorage || result->directPower || result->cableLoop;
    
    printf("[DemoApp Msg] Published resultBIT: ref=%u, result=%s\n",
           ctx->bit_state.ibit_reference_num,
           has_fault ? "FAIL" : "PASS");
    
    return 0;
}

int demo_msg_publish_actuator_signal(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    // Build Actuator Signal JSON payload (200Hz feedback with 14 fields)
    // Note: A_manualArmPositionComple has typo (Comple not Complement)
    char signal_json[2048];
    ActuatorSignalState* sig = &ctx->signal_state;
    
    snprintf(signal_json, sizeof(signal_json),
        "{"
        "\"A_recipientID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_sourceID\":{"
            "\"A_resourceId\":1,"
            "\"A_instanceId\":1"
        "},"
        "\"A_timeOfDataGeneration\":{"
            "\"A_second\":%lld,"
            "\"A_nanoseconds\":%d"
        "},"
        "\"A_azAngle\":%.3f,"
        "\"A_e1AngleVelocity\":%.3f,"
        "\"A_energyStorage\":\"%s\","
        "\"A_mainCannonFixStatus\":\"%s\","
        "\"A_deckClearance\":\"%s\","
        "\"A_autoArmPositionComplement\":\"%s\","
        "\"A_manualArmPositionComple\":\"%s\","
        "\"A_mainCannonRestoreComplement\":\"%s\","
        "\"A_armSafetyMainCannonLock\":\"%s\","
        "\"A_shutdown\":\"%s\","
        "\"A_roundGyro\":%.3f,"
        "\"A_upDownGyro\":%.3f"
        "}",
        (long long)(ctx->tick_count / 1000),
        (int)((ctx->tick_count % 1000) * 1000000),
        sig->azAngle,
        sig->e1AngleVelocity,
        format_changing_status(sig->energyStorage),
        format_main_cannon_fix_status(sig->mainCannonFixStatus),
        format_dek_clearance(sig->deckClearance),
        format_arm_position(sig->autoArmPositionComplement),
        format_arm_position(sig->manualArmPositionComple),
        format_main_cannon_return_status(sig->mainCannonRestoreComplement),
        format_arm_safety_lock(sig->armSafetyMainCannonLock),
        format_shutdown_type(sig->shutdown),
        sig->roundGyro,
        sig->upDownGyro
    );
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_ACTUATOR_SIGNAL,
        TYPE_ACTUATOR_SIGNAL,
        signal_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::HighFreqPeriodicProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 500,
                                                  on_write_complete, (void*)"Signal");
    if (status != LEGACY_OK) {
        return -1;
    }
    
    ctx->signal_pub_count++;
    return 0;
}
