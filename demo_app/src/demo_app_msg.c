/*
 * demo_app_msg.c - Message Handlers and Publishers
 */

#include "../include/demo_app.h"
#include "../include/demo_app_log.h"
#include "../include/msg_fields.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <tickLib.h>
#endif

/* ========================================================================
 * Callbacks
 * ======================================================================== */

static void on_writer_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
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
        "NstelCustomQosLib::InitialStateProfile"
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
        "NstelCustomQosLib::LowFrequencyStatusProfile"
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
        "NstelCustomQosLib::NonPeriodicEventProfile"
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
        "NstelCustomQosLib::HighFrequencyPeriodicProfile"
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
        "NstelCustomQosLib::NonPeriodicEventProfile"
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
        "NstelCustomQosLib::HighFrequencyPeriodicProfile"
    };
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
        "NstelCustomQosLib::LowFrequencyVehicleProfile"
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
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    ctx->runbit_rx_count++;
    
    const char* json = evt->data_json;
    if (!json) return;
    
    printf("[DemoApp Msg] Received runBIT: %s\n", json);
    
    // Parse A_referenceNum
    uint32_t reference_num = 0;
    const char* ref_str = strstr(json, P_COLON(F_A_REFERENCE_NUM));
    if (ref_str) {
        sscanf(ref_str, P_FMT_UINT(F_A_REFERENCE_NUM), &reference_num);
    }
    
    // Parse A_type (enum string)
    T_BITType type = L_BITType_I_BIT;  // Default to I_BIT
    const char* type_str = strstr(json, P_COLON(F_A_TYPE));
    if (type_str) {
        char type_value[64];
        if (sscanf(type_str, P_FMT_STR(F_A_TYPE), type_value) == 1) {
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
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    // Parse JSON for vehicle speed
    const char* json = evt->data_json;
    if (!json) return;
    
    // Extract A_value (vehicle speed) using field macro
    const char* speed_ptr = strstr(json, P_COLON(F_A_SPEED));
    if (speed_ptr) {
        sscanf(speed_ptr, P_FMT_FLOAT(F_A_SPEED), &ctx->speed_state.speed);
    }
    
    ctx->speed_state.last_update_time = ctx->tick_count;
    ctx->speed_rx_count++;
    
        LOG_RX("Vehicle Speed: A_value=%.2f m/s (rx=%u)\n",
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
    if (!res->ok) {
        LOG_INFO("ERROR: Failed to publish %s: %s\n",
               msg_type, res->msg ? res->msg : "Unknown");
    }
    // Success is silent - actual logs are in publish functions with LOG_TX
}

/* ========================================================================
 * Publish Functions
 * ======================================================================== */

int demo_msg_publish_pbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Msg] Publishing PBIT...\n");
    
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
    LegacyWriteJsonOptions wopt = {
        TOPIC_PowerOnBIT,
        TYPE_PowerOnBIT,
        pbit_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::InitialStateProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 2000,
                                                  on_write_complete, (void*)"PBIT");
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
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_PBIT,
        TYPE_PBIT,
        cbit_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::LowFrequencyStatusProfile"
    };
    
    
#ifdef DEMO_PERF_INSTRUMENTATION
    uint64_t t_write0 = 0, t_write1 = 0;
#if defined(_VXWORKS_)
    {
        unsigned long tk = tickGet();
        int tr = sysClkRateGet();
        t_write0 = (uint64_t)tk * (1000000000ULL / (tr > 0 ? tr : 1));
    }
#else
    struct timespec tws; clock_gettime(CLOCK_MONOTONIC, &tws); t_write0 = (uint64_t)tws.tv_sec*1000000000ULL + tws.tv_nsec;
#endif
#endif

    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 1000,
                                                  on_write_complete, (void*)"CBIT");

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
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_IBIT,
        TYPE_IBIT,
        json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::NonPeriodicEventProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 2000,
                                                  on_write_complete, (void*)"ResultBIT");
    
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
    
    LegacyWriteJsonOptions wopt = {
        TOPIC_Signal,
        TYPE_Signal,
        signal_json,
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::HighFrequencyPeriodicProfile"
    };
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

    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 500,
                                                  on_write_complete, (void*)"Signal");

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
    demo_log(LOG_DIR_INFO, "[TIMING] publish_signal JSON+send approx %llu ms (ticks delta=%llu)\n",
             (unsigned long long)(t_publish_end - t_publish_start),
             (unsigned long long)(t_publish_end - t_publish_start));
#else
    struct timespec tpe; clock_gettime(CLOCK_MONOTONIC, &tpe); t_publish_end = (uint64_t)tpe.tv_sec*1000ULL + (uint64_t)(tpe.tv_nsec/1000000ULL);
    demo_log(LOG_DIR_INFO, "[TIMING] publish_signal JSON+send took %llu ms\n", (unsigned long long)(t_publish_end - t_publish_start));
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
    
    printf("[DemoApp Test] Sending PBIT test message...\n");
    return demo_msg_publish_pbit(ctx);
}

int demo_msg_test_write_cbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Test] Sending CBIT test message...\n");
    return demo_msg_publish_cbit(ctx);
}

int demo_msg_test_write_signal(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    printf("[DemoApp Test] Sending Actuator Signal test message...\n");
    return demo_msg_publish_actuator_signal(ctx);
}

int demo_msg_test_write_result_bit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;

    // Set dummy reference number for test
    ctx->bit_state.ibit_reference_num = 999;

    printf("[DemoApp Test] Sending resultBIT test message (ref=999)...\n");
    return demo_msg_publish_result_bit(ctx);
}
