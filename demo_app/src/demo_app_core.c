/*
 * demo_app_core.c - State Machine and Core Logic
 */

#include "../include/demo_app.h"
#include "../include/demo_app_enums.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _VXWORKS_
#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#endif

/* ========================================================================
 * State Name Helper
 * ======================================================================== */

const char* demo_state_name(DemoState state) {
    switch (state) {
        case DEMO_STATE_IDLE:         return "Idle";
        case DEMO_STATE_INIT:         return "Init";
        case DEMO_STATE_POWERON_BIT:  return "PowerOnBit";
        case DEMO_STATE_RUN:          return "Run";
        case DEMO_STATE_IBIT_RUNNING: return "IBitRunning";
        default:                      return "Unknown";
    }
}

/* ========================================================================
 * Context Initialization
 * ======================================================================== */

void demo_app_context_init(DemoAppContext* ctx) {
    if (!ctx) return;
    
    memset(ctx, 0, sizeof(DemoAppContext));
    
    ctx->current_state = DEMO_STATE_IDLE;
    ctx->domain_id = 0;  // Default domain
    ctx->agent = NULL;
    
    // Initialize all BIT components to true (healthy)
    ctx->bit_state.pbit_components.upDownMotor = true;
    ctx->bit_state.pbit_components.roundMotor = true;
    ctx->bit_state.pbit_components.upDownAmp = true;
    ctx->bit_state.pbit_components.roundAmp = true;
    ctx->bit_state.pbit_components.baseGyro = true;
    ctx->bit_state.pbit_components.topForwardGryro = true;
    ctx->bit_state.pbit_components.vehicleForwardGyro = true;
    ctx->bit_state.pbit_components.powerController = true;
    ctx->bit_state.pbit_components.energyStorage = true;
    ctx->bit_state.pbit_components.directPower = true;
    ctx->bit_state.pbit_components.cableLoop = true;
    ctx->bit_state.pbit_components.bitRunning = true;
    
    // Initialize CBIT components (copy PBIT + 4 additional)
    ctx->bit_state.cbit_components.base = ctx->bit_state.pbit_components;
    ctx->bit_state.cbit_components.upDownPark = true;
    ctx->bit_state.cbit_components.round_Park = true;
    ctx->bit_state.cbit_components.mainCannon_Lock = true;
    ctx->bit_state.cbit_components.commFault = true;
    
    // Initialize control state with default enum values
    ctx->control_state.operationMode = L_OperationModeType_NORMAL;
    ctx->control_state.parm = L_OnOffType_OFF;
    ctx->control_state.targetDesingation = L_TargetAllotType_ETC;
    
    // Initialize signal state with default enum values
    ctx->signal_state.energyStorage = L_ChangingStatusType_NORMAL;
    ctx->signal_state.mainCannonFixStatus = L_MainCannonFixStatusType_NORMAL;
    ctx->signal_state.deckClearance = L_DekClearanceType_OUTSIDE;
    ctx->signal_state.autoArmPositionComplement = L_ArmPositionType_NORMAL;
    ctx->signal_state.manualArmPositionComple = L_ArmPositionType_NORMAL;
    ctx->signal_state.mainCannonRestoreComplement = L_MainCannonReturnStatusType_RUNNING;
    ctx->signal_state.armSafetyMainCannonLock = L_ArmSafetyMainCannonLock_NORMAL;
    ctx->signal_state.shutdown = L_CannonDrivingDeviceShutdownType_UNKNOWN;
    
    // Initialize statistics
    ctx->tick_count = 0;
    ctx->stats_last_tick = 0;
    ctx->signal_pub_count = 0;
    ctx->cbit_pub_count = 0;
    ctx->pbit_pub_count = 0;
    ctx->result_pub_count = 0;
    ctx->control_rx_count = 0;
    ctx->speed_rx_count = 0;
    ctx->runbit_rx_count = 0;
    
    ctx->signal_pub_hz = 0;
    ctx->cbit_pub_hz = 0;
    ctx->pbit_pub_hz = 0;
    ctx->result_pub_hz = 0;
    ctx->control_rx_hz = 0;
    ctx->speed_rx_hz = 0;
    ctx->runbit_rx_hz = 0;
    
    ctx->signal_pub_prev = 0;
    ctx->cbit_pub_prev = 0;
    ctx->pbit_pub_prev = 0;
    ctx->result_pub_prev = 0;
    ctx->control_rx_prev = 0;
    ctx->speed_rx_prev = 0;
    ctx->runbit_rx_prev = 0;
    
    printf("[DemoApp Core] Context initialized with default values\n");
}

/* ========================================================================
 * State Transitions
 * ======================================================================== */

void enter_state(DemoAppContext* ctx, DemoState new_state) {
    DemoState old_state = ctx->current_state;
    ctx->current_state = new_state;
    
    printf("[DemoApp Core] State transition: %s -> %s\n",
           demo_state_name(old_state), demo_state_name(new_state));
}

/* ========================================================================
 * Callbacks
 * ======================================================================== */

static void on_hello(LEGACY_HANDLE h, LegacyRequestId req_id,
                     const LegacySimpleResult* res,
                     const LegacyHelloInfo* info, void* user) {
    DemoAppContext* ctx = (DemoAppContext*)user;
    printf("[DemoApp Core] Hello response: ok=%d, proto=%d\n",
           res->ok, info ? info->proto : -1);
    
    if (res->ok) {
        printf("[DemoApp Core] Agent connection established\n");
    }
}

static void on_entity_created(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    const char* entity_name = (const char*)user;
    if (res->ok) {
        printf("[DemoApp Core] Entity created: %s\n", 
               entity_name ? entity_name : "Unknown");
    } else {
        printf("[DemoApp Core] ERROR: Failed to create %s: %s\n",
               entity_name ? entity_name : "Unknown",
               res->msg ? res->msg : "Unknown error");
    }
}

static void on_entity_cleared(LEGACY_HANDLE h, LegacyRequestId req_id,
                              const LegacySimpleResult* res, void* user) {
    (void)h;
    (void)req_id;
    (void)user;
    if (res->ok) {
        printf("[DemoApp Core] DDS entities cleared\n");
    } else {
        printf("[DemoApp Core] WARNING: Failed to clear entities: %s\n",
               res->msg ? res->msg : "Unknown error");
    }
}

/* ========================================================================
 * DDS Initialization Helper
 * ======================================================================== */

static int create_dds_entities(DemoAppContext* ctx) {
    if (!ctx || !ctx->agent) return -1;
    
    printf("[DemoApp Core] Creating DDS entities...\n");
    
    LegacyStatus status;
    
    // 1. Create Participant
    LegacyParticipantConfig pcfg = { ctx->domain_id, "TriadQosLib::DefaultReliable" };
    status = legacy_agent_create_participant(ctx->agent, &pcfg, 2000,
                                            on_entity_created, (void*)"Participant");
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] ERROR: Failed to create participant\n");
        return -1;
    }
    
    // 2. Create Publisher
    LegacyPublisherConfig pubcfg = { ctx->domain_id, "pub1", "TriadQosLib::DefaultReliable" };
    status = legacy_agent_create_publisher(ctx->agent, &pubcfg, 2000,
                                          on_entity_created, (void*)"Publisher");
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] ERROR: Failed to create publisher\n");
        return -1;
    }
    
    // 3. Create Subscriber
    LegacySubscriberConfig subcfg = { ctx->domain_id, "sub1", "TriadQosLib::DefaultReliable" };
    status = legacy_agent_create_subscriber(ctx->agent, &subcfg, 2000,
                                           on_entity_created, (void*)"Subscriber");
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] ERROR: Failed to create subscriber\n");
        return -1;
    }
    
    // Give time for entities to be created
    // TODO: Better synchronization mechanism
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet() / 2);  // 500ms
    #else
    // Linux: sleep not available in this context
    #endif
    
    printf("[DemoApp Core] DDS entities created successfully\n");
    return 0;
}

/* ========================================================================
 * Start/Stop
 * ======================================================================== */

int demo_app_start(DemoAppContext* ctx, const char* agent_ip, uint16_t agent_port) {
    if (!ctx) return -1;
    
    if (ctx->current_state != DEMO_STATE_IDLE) {
        printf("[DemoApp Core] ERROR: Cannot start from state %s\n",
               demo_state_name(ctx->current_state));
        return -1;
    }
    
    printf("[DemoApp Core] Starting demo application...\n");
    printf("[DemoApp Core] Agent: %s:%d\n", agent_ip, agent_port);
    
    // Transition: Idle -> Init
    enter_state(ctx, DEMO_STATE_INIT);
    
    // Initialize LegacyLib
    LegacyConfig cfg = {
        agent_ip,
        agent_port,
        100,      // recv_task_priority
        64*1024,  // recv_task_stack
        100,      // send_task_priority
        64*1024,  // send_task_stack
        NULL,     // log_cb
        NULL      // log_user
    };
    
    LegacyStatus status = legacy_agent_init(&cfg, &ctx->agent);
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] ERROR: Failed to initialize LegacyLib\n");
        enter_state(ctx, DEMO_STATE_IDLE);
        return -1;
    }
    
    printf("[DemoApp Core] LegacyLib initialized\n");
    
    // Send Hello
    status = legacy_agent_hello(ctx->agent, 2000, on_hello, ctx);
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] ERROR: Failed to send hello\n");
        legacy_agent_close(ctx->agent);
        ctx->agent = NULL;
        enter_state(ctx, DEMO_STATE_IDLE);
        return -1;
    }
    
    // Wait for hello response
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet() / 2);  // 500ms
    #endif
    
    // Clear existing DDS entities (clean slate)
    printf("[DemoApp Core] Clearing existing DDS entities...\n");
    status = legacy_agent_clear_dds_entities(ctx->agent, 2000,
                                             on_entity_cleared, (void*)"ClearEntities");
    if (status != LEGACY_OK) {
        printf("[DemoApp Core] WARNING: Failed to clear DDS entities\n");
    }
    
    // Wait for clear response
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet() / 2);  // 500ms
    #endif
    
    printf("[DemoApp Core] Agent connection established\n");
    printf("[DemoApp Core] Ready to create DDS entities\n");
    printf("[DemoApp Core] Use 'demoAppCreateEntities()' to proceed\n");
    return 0;
}

int demo_app_create_entities(DemoAppContext* ctx) {
    if (!ctx) return -1;
    
    if (ctx->current_state != DEMO_STATE_INIT) {
        printf("[DemoApp Core] ERROR: Cannot create entities from state %s\n",
               demo_state_name(ctx->current_state));
        printf("[DemoApp Core] Run 'demoAppStart()' first\n");
        return -1;
    }
    
    printf("[DemoApp Core] Creating DDS entities...\n");
    
    // Create DDS entities (Participant, Publisher, Subscriber)
    if (create_dds_entities(ctx) != 0) {
        printf("[DemoApp Core] ERROR: Failed to create DDS entities\n");
        return -1;
    }
    
    // Initialize message handlers (Writers/Readers)
    if (demo_msg_init(ctx) != 0) {
        printf("[DemoApp Core] ERROR: Failed to initialize message handlers\n");
        return -1;
    }
    
    printf("[DemoApp Core] DDS entities created successfully\n");
    printf("[DemoApp Core] Use 'demoAppStartScenario()' to begin simulation\n");
    return 0;
}

int demo_app_start_scenario(DemoAppContext* ctx) {
    if (!ctx) return -1;
    
    if (ctx->current_state != DEMO_STATE_INIT) {
        printf("[DemoApp Core] ERROR: Cannot start scenario from state %s\n",
               demo_state_name(ctx->current_state));
        return -1;
    }
    
    printf("[DemoApp Core] Starting scenario...\n");
    
    // Transition: Init -> PowerOnBit
    enter_state(ctx, DEMO_STATE_POWERON_BIT);
    
    // Perform PowerOn BIT
    printf("[DemoApp Core] Performing PowerOn BIT...\n");
    
    // Simulate PBIT execution
    #ifdef _VXWORKS_
    taskDelay(sysClkRateGet());  // 1 second
    #endif
    
    // Publish PBIT result
    if (demo_msg_publish_pbit(ctx) != 0) {
        printf("[DemoApp Core] WARNING: Failed to publish PBIT\n");
    }
    
    // Transition: PowerOnBit -> Run
    enter_state(ctx, DEMO_STATE_RUN);
    
    // Initialize timer subsystem (start periodic tasks)
    if (demo_timer_init(ctx) != 0) {
        printf("[DemoApp Core] ERROR: Failed to initialize timer\n");
        return -1;
    }
    
    printf("[DemoApp Core] Scenario started successfully\n");
    printf("[DemoApp Core] Periodic publishing: 200Hz Signal, 1Hz CBIT\n");
    return 0;
}

void demo_app_stop(DemoAppContext* ctx) {
    if (!ctx) return;
    
    printf("[DemoApp Core] Stopping demo application...\n");
    
    // Stop timer first
    demo_timer_cleanup(ctx);
    
    // Cleanup message handlers
    demo_msg_cleanup(ctx);
    
    // Clear DDS entities
    if (ctx->agent) {
        printf("[DemoApp Core] Clearing DDS entities...\n");
        legacy_agent_clear_dds_entities(ctx->agent, 2000,
                                        on_entity_created, (void*)"ClearEntities");
        
        // Give time for cleanup
        #ifdef _VXWORKS_
        taskDelay(sysClkRateGet() / 2);  // 500ms
        #endif
        
        // Close LegacyLib
        legacy_agent_close(ctx->agent);
        ctx->agent = NULL;
    }
    
    enter_state(ctx, DEMO_STATE_IDLE);
    printf("[DemoApp Core] Stopped\n");
}

/* ========================================================================
 * State Query
 * ======================================================================== */

DemoState demo_app_get_state(const DemoAppContext* ctx) {
    return ctx ? ctx->current_state : DEMO_STATE_IDLE;
}

/* ========================================================================
 * Manual State Transition (for CLI)
 * ======================================================================== */

int demo_app_transition_to(DemoAppContext* ctx, DemoState new_state) {
    if (!ctx) return -1;
    
    // TODO Phase 2: Add state transition validation
    enter_state(ctx, new_state);
    return 0;
}

/* ========================================================================
 * IBIT Trigger
 * ======================================================================== */

int demo_app_trigger_ibit(DemoAppContext* ctx, uint32_t reference_num, int type) {
    if (!ctx) return -1;
    
    if (ctx->current_state != DEMO_STATE_RUN) {
        printf("[DemoApp Core] ERROR: IBIT can only run from Run state\n");
        return -1;
    }
    
    printf("[DemoApp Core] Triggering IBIT: ref=%u, type=%d\n", reference_num, type);
    
    ctx->bit_state.ibit_running = true;
    ctx->bit_state.ibit_reference_num = reference_num;
    ctx->bit_state.ibit_type = (T_BITType)type;
    
    // Get current tick count as timestamp (milliseconds)
    ctx->bit_state.ibit_start_time = ctx->tick_count;
    
    enter_state(ctx, DEMO_STATE_IBIT_RUNNING);
    
    printf("[DemoApp Core] IBIT started at tick %llu (will complete in 3 seconds)\n",
           ctx->bit_state.ibit_start_time);
    
    return 0;
}

/* ========================================================================
 * Fault Injection (for Testing)
 * ======================================================================== */

void demo_app_inject_fault(DemoAppContext* ctx, const char* component) {
    if (!ctx || !component) return;
    
    // Map legacy fault names to new BIT components
    if (strcmp(component, "azimuth") == 0 || strcmp(component, "round") == 0) {
        ctx->bit_state.pbit_components.roundMotor = false;
        ctx->bit_state.pbit_components.roundAmp = false;
        printf("[DemoApp Core] Fault injected: Round Motor/Amp\n");
    }
    else if (strcmp(component, "updown") == 0) {
        ctx->bit_state.pbit_components.upDownMotor = false;
        ctx->bit_state.pbit_components.upDownAmp = false;
        printf("[DemoApp Core] Fault injected: UpDown Motor/Amp\n");
    }
    else if (strcmp(component, "sensor") == 0 || strcmp(component, "gyro") == 0) {
        ctx->bit_state.pbit_components.baseGyro = false;
        ctx->bit_state.pbit_components.vehicleForwardGyro = false;
        printf("[DemoApp Core] Fault injected: Base/Vehicle Gyro\n");
    }
    else if (strcmp(component, "power") == 0) {
        ctx->bit_state.pbit_components.powerController = false;
        ctx->bit_state.pbit_components.energyStorage = false;
        ctx->bit_state.pbit_components.directPower = false;
        printf("[DemoApp Core] Fault injected: Power/Energy\n");
    }
    else if (strcmp(component, "motor") == 0) {
        ctx->bit_state.pbit_components.roundMotor = false;
        ctx->bit_state.pbit_components.upDownMotor = false;
        printf("[DemoApp Core] Fault injected: All Motors\n");
    }
    else {
        printf("[DemoApp Core] Unknown component: %s\n", component);
        printf("  Available: azimuth, updown, sensor, power, motor\n");
    }
}

void demo_app_clear_fault(DemoAppContext* ctx, const char* component) {
    if (!ctx || !component) return;
    
    if (strcmp(component, "azimuth") == 0 || strcmp(component, "round") == 0) {
        ctx->bit_state.pbit_components.roundMotor = true;
        ctx->bit_state.pbit_components.roundAmp = true;
        printf("[DemoApp Core] Fault cleared: Round Motor/Amp\n");
    }
    else if (strcmp(component, "updown") == 0) {
        ctx->bit_state.pbit_components.upDownMotor = true;
        ctx->bit_state.pbit_components.upDownAmp = true;
        printf("[DemoApp Core] Fault cleared: UpDown Motor/Amp\n");
    }
    else if (strcmp(component, "sensor") == 0 || strcmp(component, "gyro") == 0) {
        ctx->bit_state.pbit_components.baseGyro = true;
        ctx->bit_state.pbit_components.vehicleForwardGyro = true;
        printf("[DemoApp Core] Fault cleared: Base/Vehicle Gyro\n");
    }
    else if (strcmp(component, "power") == 0) {
        ctx->bit_state.pbit_components.powerController = true;
        ctx->bit_state.pbit_components.energyStorage = true;
        ctx->bit_state.pbit_components.directPower = true;
        printf("[DemoApp Core] Fault cleared: Power/Energy\n");
    }
    else if (strcmp(component, "all") == 0) {
        // Clear all PBIT components (12 fields)
        ctx->bit_state.pbit_components.upDownMotor = true;
        ctx->bit_state.pbit_components.roundMotor = true;
        ctx->bit_state.pbit_components.upDownAmp = true;
        ctx->bit_state.pbit_components.roundAmp = true;
        ctx->bit_state.pbit_components.baseGyro = true;
        ctx->bit_state.pbit_components.topForwardGryro = true;
        ctx->bit_state.pbit_components.vehicleForwardGyro = true;
        ctx->bit_state.pbit_components.powerController = true;
        ctx->bit_state.pbit_components.energyStorage = true;
        ctx->bit_state.pbit_components.directPower = true;
        ctx->bit_state.pbit_components.cableLoop = true;
        ctx->bit_state.pbit_components.bitRunning = true;
        printf("[DemoApp Core] All faults cleared\n");
    }
    else {
        printf("[DemoApp Core] Unknown component: %s\n", component);
    }
}

