/*
 * demo_app_msg.cpp - Message Handlers and Publishers (C++ port)
 * Converted from demo_app_msg.c to use nlohmann::json for robustness.
 */

#include "../include/demo_app.h"
#include "../include/demo_app_log.h"
#include "../include/msg_fields.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <sstream>
#include "json.hpp"

using json = nlohmann::json;

extern "C" {

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
        ctx->speed_rx_count++;
        ctx->runbit_rx_count++;
    
    const char* json_c = evt->data_json;
    if (!json_c) return;

    // Use nlohmann::json to parse safely
    try {
        json j = json::parse(json_c);

        uint32_t reference_num = j.value(F_A_REFERENCE_NUM, 0u);
        std::string type_str = j.value(F_A_TYPE, std::string());
        T_BITType type = parse_bit_type(type_str.c_str());

        printf("[DemoApp Msg] runBIT parsed: A_referenceNum=%u, A_type=%d\n", reference_num, (int)type);

        // Trigger IBIT
        if (demo_app_trigger_ibit(ctx, reference_num, type) != 0) {
            printf("[DemoApp Msg] WARNING: Failed to trigger IBIT\n");
        }
    } catch (...) {
        printf("[DemoApp Msg] WARNING: Failed to parse runBIT JSON\n");
    }
}

void demo_msg_on_actuator_control(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    ctx->control_rx_count++;
    
    const char* json_c = evt->data_json;
    if (!json_c) return;
    
    ActuatorControlState* ctrl = &ctx->control_state;
    
    try {
        json j = json::parse(json_c);

        // Extract float/double fields
        ctrl->drivingPosition = j.value(F_A_DRIVINGPOSITION, ctrl->drivingPosition);
        ctrl->upDownPosition = j.value(F_A_UPDOWNPOSITION, ctrl->upDownPosition);
        ctrl->roundAngleVelocity = j.value(F_A_ROUNDANGLEVELOCITY, ctrl->roundAngleVelocity);
        ctrl->upDownAngleVelocity = j.value(F_A_UPDOWNANGLEVELOCITY, ctrl->upDownAngleVelocity);
        ctrl->cannonUpDownAngle = j.value(F_A_CANNONUPDOWNANGLE, ctrl->cannonUpDownAngle);
        ctrl->topRelativeAngle = j.value(F_A_TOPRELATIVEANGLE, ctrl->topRelativeAngle);

        // Extract enum fields (strings)
        if (j.contains(F_A_OPERATIONMODE)) {
            std::string enum_val = j[F_A_OPERATIONMODE];
            ctrl->operationMode = parse_operation_mode(enum_val.c_str());
        }
        if (j.contains(F_A_PARM)) {
            std::string enum_val = j[F_A_PARM];
            ctrl->parm = parse_onoff_type(enum_val.c_str());
        }
        if (j.contains(F_A_TARGET_DESIGNATION)) {
            std::string enum_val = j[F_A_TARGET_DESIGNATION];
            ctrl->targetDesingation = parse_target_allot(enum_val.c_str());
        }
        if (j.contains(F_A_AUTO_ARM_POSITION)) {
            std::string enum_val = j[F_A_AUTO_ARM_POSITION];
            ctrl->autoArmPosition = parse_arm_position_lock(enum_val.c_str());
        }
        if (j.contains(F_A_MANUAL_ARM_POSITION)) {
            std::string enum_val = j[F_A_MANUAL_ARM_POSITION];
            ctrl->manualArmPosition = parse_arm_position_lock(enum_val.c_str());
        }
        if (j.contains(F_A_MAIN_CANNON_RESTORE)) {
            std::string enum_val = j[F_A_MAIN_CANNON_RESTORE];
            ctrl->mainCannonRestore = parse_main_cannon_return(enum_val.c_str());
        }
        if (j.contains(F_A_MAIN_CANNON_FIX)) {
            std::string enum_val = j[F_A_MAIN_CANNON_FIX];
            ctrl->manCannonFix = parse_main_cannon_fix(enum_val.c_str());
        }
        if (j.contains(F_A_CLOSE_EQUIP_OPEN_STATUS)) {
            std::string enum_val = j[F_A_CLOSE_EQUIP_OPEN_STATUS];
            ctrl->closeEquipOpenStatus = parse_equip_open_lock(enum_val.c_str());
        }

        ctrl->last_update_time = ctx->tick_count;
        ctx->control_rx_count++;

        if ((ctx->control_rx_count % 100) == 0) {
            LOG_RX("Actuator Control: driving=%.2f, updown=%.2f, mode=%d (rx=%u)\n",
                   ctrl->drivingPosition,
                   ctrl->upDownPosition,
                   (int)ctrl->operationMode,
                   ctx->control_rx_count);
        }
    } catch (...) {
        LOG_INFO("ERROR: Failed to parse actuator control JSON\n");
    }
}

void demo_msg_on_vehicle_speed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    if (!ctx || !evt) return;
    
    const char* json_c = evt->data_json;
    if (!json_c) return;
    
    try {
        json j = json::parse(json_c);
        ctx->speed_state.speed = j.value(F_A_SPEED, ctx->speed_state.speed);
        ctx->speed_state.last_update_time = ctx->tick_count;
        ctx->speed_rx_count++;

        LOG_RX("Vehicle Speed: A_value=%.2f m/s (rx=%u)\n",
            ctx->speed_state.speed, ctx->speed_rx_count);
    } catch (...) {
        LOG_INFO("ERROR: Failed to parse vehicle speed JSON\n");
    }
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
}

/* ========================================================================
 * Publish Functions (use nlohmann::json builders)
 * ======================================================================== */

int demo_msg_publish_pbit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Msg] Publishing PBIT...\n");
    
    BITComponentState* comp = &ctx->bit_state.pbit_components;

    json j;
    // SourceID
    j[F_A_SOURCEID] = {
        {F_A_RESOURCEID, 1},
        {F_A_INSTANCEID, 1}
    };
    // Time
    j[F_A_TIMEOFDATA] = {
        {F_A_SECOND, (long long)(ctx->tick_count/1000)},
        {F_A_NANOSECONDS, (int)((ctx->tick_count%1000)*1000000)}
    };
    // Cannon source id
    j[F_A_CANNON_SOURCEID] = {
        {F_A_RESOURCEID, 1},
        {F_A_INSTANCEID, 1}
    };
    j[F_A_BITRUNNING] = comp->bitRunning ? true : false;
    j[F_A_UPDOWNMOTOR] = format_bit_result(comp->upDownMotor);
    j[F_A_ROUNDMOTOR] = format_bit_result(comp->roundMotor);
    j[F_A_UPDOWNAMP] = format_bit_result(comp->upDownAmp);
    j[F_A_ROUNDAMP] = format_bit_result(comp->roundAmp);
    j[F_A_BASEGIRO] = format_bit_result(comp->baseGiro);
    j[F_A_TOPFORWARDGIRO] = format_bit_result(comp->topForwardGiro);
    j[F_A_VEHICLEFORWARDGIRO] = format_bit_result(comp->vehicleForwardGiro);
    j[F_A_POWER_CONTROLLER] = format_bit_result(comp->powerController);
    j[F_A_ENERGY_STORAGE] = format_bit_result(comp->energyStorage);
    j[F_A_DIRECTPOWER] = format_bit_result(comp->directPower);
    j[F_A_CABLELOOP] = format_bit_result(comp->cableLoop);

    std::string s = j.dump();

    LegacyWriteJsonOptions wopt = {
        TOPIC_PowerOnBIT,
        TYPE_PowerOnBIT,
        s.c_str(),
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
    
    CBITComponentState* cbit = &ctx->bit_state.cbit_components;
    json j;
    j[F_A_SOURCEID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_TIMEOFDATA] = {{F_A_SECOND,(long long)(ctx->tick_count/1000)},{F_A_NANOSECONDS,(int)((ctx->tick_count%1000)*1000000)}};
    j[F_A_CANNON_SOURCEID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_CONTROLLER_NETWORK] = format_bit_result(cbit->controllerNetwork);
    j[F_A_UPDOWNMOTOR] = format_bit_result(cbit->base.upDownMotor);
    j[F_A_ROUNDMOTOR] = format_bit_result(cbit->base.roundMotor);
    j[F_A_UPDOWNAMP] = format_bit_result(cbit->base.upDownAmp);
    j[F_A_ROUNDAMP] = format_bit_result(cbit->base.roundAmp);
    j[F_A_BASEGIRO] = format_bit_result(cbit->base.baseGiro);
    j[F_A_TOPFORWARDGIRO] = format_bit_result(cbit->base.topForwardGiro);
    j[F_A_VEHICLEFORWARDGIRO] = format_bit_result(cbit->base.vehicleForwardGiro);
    j[F_A_POWER_CONTROLLER] = format_bit_result(cbit->base.powerController);
    j[F_A_ENERGY_STORAGE] = format_bit_result(cbit->base.energyStorage);
    j[F_A_DIRECTPOWER] = format_bit_result(cbit->base.directPower);
    j[F_A_CABLELOOP] = format_bit_result(cbit->base.cableLoop);
    j[F_A_UPDOWNPARK] = format_bit_result(cbit->upDownPark);
    j[F_A_ROUND_PARK] = format_bit_result(cbit->round_Park);
    j[F_A_MAINCANNON_LOCK] = format_bit_result(cbit->mainCannon_Lock);
    j[F_A_COMMFAULT] = format_bit_result(cbit->commFault);

    std::string s = j.dump();

    LegacyWriteJsonOptions wopt = {
        TOPIC_PBIT,
        TYPE_PBIT,
        s.c_str(),
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::LowFrequencyStatusProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 1000,
                                                  on_write_complete, (void*)"CBIT");
    if (status != LEGACY_OK) return -1;
    
    ctx->cbit_pub_count++;
    LOG_TX("CBIT published (count=%u)\n", ctx->cbit_pub_count);
    return 0;
}

int demo_msg_publish_result_bit(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    BITComponentState* result = &ctx->bit_state.result_components;
    json j;
    j[F_A_SOURCEID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_TIMEOFDATA] = {{F_A_SECOND,(long long)(ctx->tick_count/1000)},{F_A_NANOSECONDS,(int)((ctx->tick_count%1000)*1000000)}};
    j[F_A_REFERENCE_NUM] = ctx->bit_state.ibit_reference_num;
    j[F_A_CANNON_SOURCEID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_BITRUNNING] = result->bitRunning ? true : false;
    j[F_A_UPDOWNMOTOR] = format_bit_result(result->upDownMotor);
    j[F_A_ROUNDMOTOR] = format_bit_result(result->roundMotor);
    j[F_A_UPDOWNAMP] = format_bit_result(result->upDownAmp);
    j[F_A_ROUNDAMP] = format_bit_result(result->roundAmp);
    j[F_A_BASEGIRO] = format_bit_result(result->baseGiro);
    j[F_A_TOPFORWARDGIRO] = format_bit_result(result->topForwardGiro);
    j[F_A_VEHICLEFORWARDGIRO] = format_bit_result(result->vehicleForwardGiro);
    j[F_A_POWER_CONTROLLER] = format_bit_result(result->powerController);
    j[F_A_ENERGY_STORAGE] = format_bit_result(result->energyStorage);
    j[F_A_DIRECTPOWER] = format_bit_result(result->directPower);
    j[F_A_CABLELOOP] = format_bit_result(result->cableLoop);

    std::string s = j.dump();

    LegacyWriteJsonOptions wopt = {
        TOPIC_IBIT,
        TYPE_IBIT,
        s.c_str(),
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
    bool has_fault = !(result->upDownMotor && result->roundMotor && result->upDownAmp &&
                     result->roundAmp && result->baseGiro && result->topForwardGiro &&
                     result->vehicleForwardGiro && result->powerController &&
                     result->energyStorage && result->directPower && result->cableLoop);
    
    LOG_TX("resultBIT published: ref=%u, result=%s\n",
           ctx->bit_state.ibit_reference_num,
           has_fault ? "FAIL" : "PASS");
    
    return 0;
}

int demo_msg_publish_actuator_signal(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    ActuatorSignalState* sig = &ctx->signal_state;
    
    // Quantize/clamp values (use double-aware math)
    double az_v = sig->e1AngleVelocity;
    if (az_v > 800.0) az_v = 800.0;
    if (az_v < -800.0) az_v = -800.0;
    az_v = round(az_v / 0.01) * 0.01;

    double e1_v = sig->e1AngleVelocity;
    if (e1_v > 450.0) e1_v = 450.0;
    if (e1_v < -450.0) e1_v = -450.0;
    e1_v = round(e1_v / 0.01) * 0.01;

    double round_v = sig->roundGiro;
    if (round_v > 655.0) round_v = 655.0;
    if (round_v < -655.0) round_v = -655.0;
    round_v = round(round_v / 0.02) * 0.02;

    double updown_v = sig->upDownGiro;
    if (updown_v > 655.0) updown_v = 655.0;
    if (updown_v < -655.0) updown_v = -655.0;
    updown_v = round(updown_v / 0.02) * 0.02;

    json j;
    j[F_A_RECIPIENTID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_SOURCEID] = {{F_A_RESOURCEID,1},{F_A_INSTANCEID,1}};
    j[F_A_TIMEOFDATA] = {{F_A_SECOND,(long long)(ctx->tick_count/1000)},{F_A_NANOSECONDS,(int)((ctx->tick_count%1000)*1000000)}};
    j[F_A_AZANGLEVELOCITY] = az_v;
    j[F_A_E1ANGLEVELOCITY] = e1_v;
    j[F_A_ENERGY_STORAGE] = format_energy_storage(sig->energyStorage);
    j[F_A_MAINCANNONFIXSTATUS] = format_main_cannon_fix_status(sig->mainCannonFixStatus);
    j[F_A_DECKCLEARANCE] = format_dek_clearance(sig->deckClearance);
    j[F_A_AUTO_ARM_POSITION_COMPLEMENT] = format_cannon_driving_from_arm(sig->autoArmPositionComplement);
    j[F_A_MANUAL_ARM_POSITION_COMPLEMENT] = format_cannon_driving_from_arm(sig->manualArmPositionComple);
    j[F_A_MAIN_CANNON_RESTORE_COMPLEMENT] = format_cannon_driving_from_return(sig->mainCannonRestoreComplement);
    j[F_A_ARM_SAFETY_MAIN_CANNON_LOCK] = format_arm_safety_lock(sig->armSafetyMainCannonLock);
    j[F_A_SHUTDOWN] = format_shutdown_type(sig->shutdown);
    j[F_A_ROUNDGIRO] = round_v;
    j[F_A_UPDOWNGIRO] = updown_v;

    std::string s = j.dump();

    LegacyWriteJsonOptions wopt = {
        TOPIC_Signal,
        TYPE_Signal,
        s.c_str(),
        ctx->domain_id,
        "pub1",
        "NstelCustomQosLib::HighFrequencyPeriodicProfile"
    };
    
    LegacyStatus status = legacy_agent_write_json(ctx->agent, &wopt, 500,
                                                  on_write_complete, (void*)"Signal");
    if (status != LEGACY_OK) {
        return -1;
    }
    
    ctx->signal_pub_count++;
    if ((ctx->signal_pub_count % 200) == 0) {
        LOG_TX("Actuator Signal published (count=%u, 200Hz)\n", ctx->signal_pub_count);
    }
    return 0;
}

/* ========================================================================
 * Test Write Functions
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

} // extern "C"
