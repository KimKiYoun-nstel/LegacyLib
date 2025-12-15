/*
 * DemoApp - VxWorks Demo Application for Track 1
 * 
 * Purpose:
 *   Demonstrate 7 DDS messages with proper QoS and state machine
 *   for Cannon Driving Device simulation
 */

#ifndef DEMO_APP_H
#define DEMO_APP_H

#include <stdint.h>
#include <stdbool.h>
#include "legacy_agent.h"
#include "demo_app_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Common Type Definitions (from LDM_Common.xml)
 * ======================================================================== */

// P_LDM_Common::T_IdentifierType
typedef struct {
    int32_t A_resourceId;
    int32_t A_instanceId;
} T_IdentifierType;

// P_LDM_Common::T_DateTimeType
typedef struct {
    int64_t A_second;
    int32_t A_nanoseconds;
} T_DateTimeType;

/* ========================================================================
 * State Machine Definitions
 * ======================================================================== */

typedef enum {
    DEMO_STATE_IDLE,           // Module loaded, demo not started
    DEMO_STATE_INIT,           // DDS initialization in progress
    DEMO_STATE_POWERON_BIT,    // PowerOn BIT executing, publish PBIT
    DEMO_STATE_RUN,            // Normal operation (periodic messages)
    DEMO_STATE_IBIT_RUNNING,   // IBIT execution in progress
    DEMO_STATE_PEND            // Scenario stopped/paused (timers stopped)
} DemoState;

const char* demo_state_name(DemoState state);

/* ========================================================================
 * Message Type Definitions (7 DDS Topics)
 * ======================================================================== */

// Receive Messages (AgentUI -> DemoApp)
#define TOPIC_RUNBIT           "P_UCMS__C_Monitored_Entity_runBIT"
#define TOPIC_ACTUATOR_CONTROL "P_NSTEL__C_Cannon_Actuator_Control"
#define TOPIC_VEHICLE_SPEED    "P_NSTEL__C_Vehicle_Speed"

#define TYPE_RUNBIT            "P_UCMS::C_Monitored_Entity_runBIT"
#define TYPE_ACTUATOR_CONTROL  "P_NSTEL::C_Cannon_Actuator_Control"
#define TYPE_VEHICLE_SPEED     "P_NSTEL::C_Vehicle_Speed"

// Send Messages (DemoApp -> AgentUI)
#define TOPIC_RESULT_BIT       "P_NSTEL__C_Cannon_Driving_Device_resultBIT"
#define TOPIC_CBIT             "P_NSTEL__C_Cannon_Driving_Device_CBIT"
#define TOPIC_PBIT             "P_NSTEL__C_Cannon_Driving_Device_PBIT"
#define TOPIC_ACTUATOR_SIGNAL  "P_NSTEL__C_Cannon_Actuator_Signal"

#define TYPE_RESULT_BIT        "P_NSTEL::C_Cannon_Driving_Device_resultBIT"
#define TYPE_CBIT              "P_NSTEL::C_Cannon_Driving_Device_CBIT"
#define TYPE_PBIT              "P_NSTEL::C_Cannon_Driving_Device_PBIT"
#define TYPE_ACTUATOR_SIGNAL   "P_NSTEL::C_Cannon_Actuator_Signal"

/* ========================================================================
 * Internal State Structures
 * ======================================================================== */

// Actuator Control State (from received control messages)
// Based on P_NSTEL::C_Cannon_Actuator_Control
typedef struct {
    // Common fields
    T_IdentifierType recipientID;
    T_IdentifierType sourceID;
    T_DateTimeType timeOfDataGeneration;
    
    // Position/Velocity fields
    float drivingPosition;       // A_drivingPosition (azimuth)
    float upDownPosition;        // A_upDownPosition
    float roundAngleVelocity;    // A_roundAngleVelocity
    float upDownAngleVelocity;   // A_upDownAngleVelocity
    float cannonUpDownAngle;     // A_cannonUpDownAngle
    float topRelativeAngle;      // A_topRelativeAngle
    
    // Enum fields
    T_OperationModeType operationMode;          // A_operationMode
    T_OnOffType parm;                           // A_parm
    T_TargetAllotType targetDesingation;        // A_targetDesingation
    T_ArmPositionLockType autoArmPosition;      // A_autoArmPosition
    T_ArmPositionLockType manualArmPosition;    // A_manualArmPosition
    T_MainCannonReturnType mainCannonRestore;   // A_mainCannonRestore
    T_MainCannonFixType manCannonFix;           // A_manCannonFix
    T_EquipOpenLockType closeEquipOpenStatus;   // A_closeEquipOpenStatus
    
    uint64_t last_update_time;   // Internal timestamp
} ActuatorControlState;

// Actuator Signal State (for signal publishing)
// Based on P_NSTEL::C_Cannon_Actuator_Signal
typedef struct {
    // Common fields
    T_IdentifierType recipientID;
    T_IdentifierType sourceID;
    T_DateTimeType timeOfDataGeneration;
    
    // Sensor values
    float azAngle;               // A_azAngle (azimuth angle)
    float e1AngleVelocity;       // A_e1AngleVelocity
    float roundGyro;             // A_roundGyro
    float upDownGyro;            // A_upDownGyro
    
    // Enum status fields
    T_ChangingStatusType energyStorage;                      // A_energyStorage
    T_MainCannonFixStatusType mainCannonFixStatus;           // A_mainCannonFixStatus
    T_DekClearanceType deckClearance;                        // A_deckClearance
    T_ArmPositionType autoArmPositionComplement;             // A_autoArmPositionComplement
    T_ArmPositionType manualArmPositionComple;               // A_manualArmPositionComple (typo in XML)
    T_MainCannonReturnStatusType mainCannonRestoreComplement;// A_mainCannonRestoreComplement
    T_ArmSafetyMainCannonLock armSafetyMainCannonLock;       // A_armSafetyMainCannonLock
    T_CannonDrivingDeviceShutdownType shutdown;              // A_shutdown
} ActuatorSignalState;

// Vehicle Speed State
// Based on P_NSTEL::C_Vehicle_Speed
typedef struct {
    // Common fields
    T_IdentifierType sourceID;
    T_DateTimeType timeOfDataGeneration;
    
    // Speed value
    float speed;                 // A_speed (m/s)
    
    uint64_t last_update_time;   // Internal timestamp
} VehicleSpeedState;

// BIT Component State (12 components from XML schema)
typedef struct {
    bool upDownMotor;
    bool roundMotor;
    bool upDownAmp;
    bool roundAmp;
    bool baseGyro;
    bool topForwardGryro;        // Note: XML has 'Gryro' typo
    bool vehicleForwardGyro;     // PBIT: Gyroi, CBIT/resultBIT: Gyro
    bool powerController;
    bool energyStorage;
    bool directPower;
    bool cableLoop;
    bool bitRunning;             // PBIT/resultBIT only
} BITComponentState;

// CBIT has 3 additional components
typedef struct {
    BITComponentState base;      // 12 base components
    bool upDownPark;
    bool round_Park;
    bool mainCannon_Lock;
    bool commFault;
} CBITComponentState;

// BIT (Built-In Test) State
typedef struct {
    bool     pbit_completed;     // PowerOn BIT completed flag
    bool     cbit_active;        // Continuous BIT active flag
    bool     ibit_running;       // IBIT in progress flag
    uint32_t ibit_reference_num; // IBIT request reference number
    T_BITType ibit_type;         // IBIT type (from request)
    uint64_t ibit_start_time;    // IBIT start timestamp
    
    // Component states (real schema)
    BITComponentState pbit_components;
    CBITComponentState cbit_components;
    BITComponentState result_components;
} BITState;

/* ========================================================================
 * Main Context Structure
 * ======================================================================== */

typedef struct {
    // State
    DemoState current_state;
    
    // LegacyLib handle
    LEGACY_HANDLE agent;
    
    // DDS Configuration
    int domain_id;
    
    // Internal state
    ActuatorControlState control_state;
    ActuatorSignalState  signal_state;
    VehicleSpeedState    speed_state;
    BITState             bit_state;
    
    // Timing
    uint64_t tick_count;         // 1ms tick counter
    uint64_t last_200hz_tick;    // Last 200Hz update
    uint64_t last_1hz_tick;      // Last 1Hz update
    // Configurable publish periods (ms)
    uint32_t signal_period_ms;   // default 5 (200Hz)
    uint32_t cbit_period_ms;     // default 1000 (1Hz)
    uint32_t pbit_period_ms;     // default 0 (only at start)
    
    // Statistics (counters)
    uint32_t signal_pub_count;   // Actuator signal publish count
    uint32_t cbit_pub_count;     // CBIT publish count
    uint32_t pbit_pub_count;     // PBIT publish count
    uint32_t result_pub_count;   // ResultBIT publish count
    uint32_t control_rx_count;   // Control message received count
    uint32_t speed_rx_count;     // Speed message received count
    uint32_t runbit_rx_count;    // RunBIT message received count
    
    // Hz calculation (1-second window)
    uint64_t stats_last_tick;    // Last statistics update tick
    uint32_t signal_pub_hz;      // Current Signal publish Hz
    uint32_t cbit_pub_hz;        // Current CBIT publish Hz
    uint32_t pbit_pub_hz;        // Current PBIT publish Hz
    uint32_t result_pub_hz;      // Current ResultBIT publish Hz
    uint32_t control_rx_hz;      // Current Control receive Hz
    uint32_t speed_rx_hz;        // Current Speed receive Hz
    uint32_t runbit_rx_hz;       // Current RunBIT receive Hz
    
    // Hz calculation (previous counters)
    uint32_t signal_pub_prev;
    uint32_t cbit_pub_prev;
    uint32_t pbit_pub_prev;
    uint32_t result_pub_prev;
    uint32_t control_rx_prev;
    uint32_t speed_rx_prev;
    uint32_t runbit_rx_prev;
    
} DemoAppContext;

/* Publish period control APIs */
int demo_app_set_publish_hz(DemoAppContext* ctx, const char* topic, uint32_t hz);
void demo_app_reset_publish_periods(DemoAppContext* ctx);

/* ========================================================================
 * Core API (demo_app_core.c)
 * ======================================================================== */

// Initialize context structure
void demo_app_context_init(DemoAppContext* ctx);

// Start demo application (Idle -> Init, connect to Agent, send hello+clear)
// After this, use demo_app_create_entities() to proceed
int demo_app_start(DemoAppContext* ctx, const char* agent_ip, uint16_t agent_port);

// Create DDS entities (Participant, Publisher, Subscriber, Writers, Readers)
// Must call demo_app_start() first
int demo_app_create_entities(DemoAppContext* ctx);

// Start scenario (PowerOn BIT, periodic publishing: Init -> PowerOnBit -> Run)
// Must call demo_app_create_entities() first
int demo_app_start_scenario(DemoAppContext* ctx);

// Pause scenario: stop timers only and set state to PEND
void demo_app_stop(DemoAppContext* ctx);

// Full reset: cleanup messages/entities, close agent, reinitialize and set Idle
int demo_app_reset(DemoAppContext* ctx);

// Get current state
DemoState demo_app_get_state(const DemoAppContext* ctx);

// Transition to specific state (for CLI commands)
int demo_app_transition_to(DemoAppContext* ctx, DemoState new_state);

// Trigger IBIT manually
int demo_app_trigger_ibit(DemoAppContext* ctx, uint32_t reference_num, int type);

// Inject fault for testing
void demo_app_inject_fault(DemoAppContext* ctx, const char* component);

// Clear fault
void demo_app_clear_fault(DemoAppContext* ctx, const char* component);

// State transition (internal use, exposed for timer)
void enter_state(DemoAppContext* ctx, DemoState new_state);

/* ========================================================================
 * Message Handler API (demo_app_msg.c)
 * ======================================================================== */

// Initialize message handlers (subscribe to topics)
int demo_msg_init(DemoAppContext* ctx);

// Cleanup message handlers
void demo_msg_cleanup(DemoAppContext* ctx);

// Callbacks (called by LegacyLib when messages arrive)
void demo_msg_on_runbit(LEGACY_HANDLE h, const LegacyEvent* evt, void* user);
void demo_msg_on_actuator_control(LEGACY_HANDLE h, const LegacyEvent* evt, void* user);
void demo_msg_on_vehicle_speed(LEGACY_HANDLE h, const LegacyEvent* evt, void* user);

// Publish functions
int demo_msg_publish_pbit(DemoAppContext* ctx);
int demo_msg_publish_cbit(DemoAppContext* ctx);
int demo_msg_publish_result_bit(DemoAppContext* ctx);
int demo_msg_publish_actuator_signal(DemoAppContext* ctx);

// Test write functions (1회성 전송)
int demo_msg_test_write_pbit(DemoAppContext* ctx);
int demo_msg_test_write_cbit(DemoAppContext* ctx);
int demo_msg_test_write_result_bit(DemoAppContext* ctx);
int demo_msg_test_write_signal(DemoAppContext* ctx);

/* ========================================================================
 * Timer API (demo_app_timer.c)
 * ======================================================================== */

// Initialize timer subsystem
int demo_timer_init(DemoAppContext* ctx);

// Cleanup timer subsystem
void demo_timer_cleanup(DemoAppContext* ctx);

// 1ms tick handler (call from VxWorks timer)
void demo_timer_tick(DemoAppContext* ctx);

// Check if timer is running
int demo_timer_is_running(void);

// Simulation update (position/velocity calculation)
void demo_timer_update_simulation(DemoAppContext* ctx);

/* ========================================================================
 * VxWorks Integration (demo_app_dkm.c)
 * ======================================================================== */

// Start DemoApp (VxWorks shell command)
// Usage: demoAppStart(25000, "127.0.0.1")
int demoAppStart(int cli_port, const char* agent_ip);

// Stop DemoApp (VxWorks shell command)
// Usage: demoAppStop()
int demoAppStop(void);

// Get status (VxWorks shell command)
// Usage: demoAppStatus()
void demoAppStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_APP_H */
