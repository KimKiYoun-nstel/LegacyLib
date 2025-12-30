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

/* ------------------------------------------------------------------------
 * XML-derived struct types starting with C_ (from RefDoc/Nstel_PSM.xml)
 * These are added to provide a 1:1 representation of the schema structs
 * used by the DemoApp JSON builders and parsers.
 * NOTE: sequenceMaxLength fields are implemented as fixed-size arrays
 *       to match schema constraints.
 * ------------------------------------------------------------------------ */

// P_NSTEL::C_CannonDrivingDevice
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    T_IdentifierType A_specification_sourceID;
} C_CannonDrivingDevice;

// P_NSTEL::C_CannonDrivingDevice_Specification
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    /* Boolean type from LDM_Common */
    bool             A_powerOnBITSupported;
    bool             A_continousBITSupported;
    bool             A_initiatedBITSupported;
    int32_t          A_continousBITInterval;
    /* sequenceMaxLength=100 */
    T_IdentifierType A_specifiedCannonDrivingDevices_sourceID[100];
} C_CannonDrivingDevice_Specification;

// P_NSTEL::C_CannonDrivingDevice_commandDriving
typedef struct {
    T_IdentifierType        A_sourceID;
    T_IdentifierType        A_recipientID;
    T_DateTimeType          A_timeOfDataGeneration;
    int32_t                 A_referenceNum;
    double                  A_roundPosition;
    double                  A_upDownPosition;
    double                  A_roundAngleVelocity;
    double                  A_upDownAngleVelocity;
    double                  A_cannonUpDownAngle;
    double                  A_topRelativeAngle;
    T_OperationModeType     A_operationMode;
    T_PalmModeType          A_parm;
    T_TargetFixType         A_targetFix;
    T_ArmPositionType       A_autoArmPosition;
    T_ArmPositionType       A_manualArmPosition;
    T_CannonRestoreType     A_mainCannonRestore;
    T_CannonFixType         A_mainCannonFix;
    T_EquipOpenStatusType   A_closureEquipOpenStatus;
} C_CannonDrivingDevice_commandDriving;

// P_NSTEL::C_CannonDrivingDevice_Signal
typedef struct {
    T_IdentifierType            A_sourceID;
    T_IdentifierType            A_recipientID;
    T_DateTimeType              A_timeOfDataGeneration;
    double                      A_azAngleVelocity;
    double                      A_e1AngleVelocity;
    T_ChangingStatusType        A_energyStorage;          // maps to T_EnergyStorageStatusType in XML
    T_CannonFixType             A_mainCannonFixStatus;
    T_DekClearanceType         A_deckCleance;            // note: name preserved from XML (typo)
    T_CannonDrivingType         A_autoArmPositionComplement;
    T_CannonDrivingType         A_manualArmPositionComplement;
    T_CannonDrivingType         A_mainCannonRestoreComplement;
    T_CannonLockType            A_armSafetyMainCannonLock;
    T_CannonDrivingDeviceShutdownType A_shutdown;
    double                      A_roundGiro;
    double                      A_upDownGiro;
} C_CannonDrivingDevice_Signal;

// P_NSTEL::C_VehicleSpeed
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    double           A_value;
} C_VehicleSpeed;

// P_NSTEL::C_CannonDrivingDevice_PowerOnBIT
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    T_IdentifierType A_cannonDrivingDevice_sourceID;
    bool             A_BITRunning; /* boolean */
    /* BIT result fields: represented in XML as T_BITResultType enumerators */
    /* In code these are typically represented as booleans; keep XML semantics */
    int32_t          A_upDownMotor;    /* use integer for internal representation */
    int32_t          A_roundMotor;
    int32_t          A_upDownAmp;
    int32_t          A_roundAmp;
    int32_t          A_baseGiro;
    int32_t          A_topForwardGiro;
    int32_t          A_vehicleForwardGiro;
    int32_t          A_powerController;
    int32_t          A_energyStorage;
    int32_t          A_directPower;
    int32_t          A_cableLoop;
} C_CannonDrivingDevice_PowerOnBIT;

// P_NSTEL::C_CannonDrivingDevice_PBIT
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    T_IdentifierType A_cannonDrivingDevice_sourceID;
    int32_t          A_upDownMotor;
    int32_t          A_roundMotor;
    int32_t          A_upDownAmp;
    int32_t          A_roundAmp;
    int32_t          A_baseGiro;
    int32_t          A_topForwardGiro;
    int32_t          A_vehicleForwardGiro;
    int32_t          A_powerController;
    int32_t          A_energyStorage;
    int32_t          A_directPower;
    int32_t          A_cableLoop;
    int32_t          A_upDownPark;
    int32_t          A_roundPark;
    int32_t          A_mainCannonLock;
    int32_t          A_controllerNetwork;
} C_CannonDrivingDevice_PBIT;

// P_NSTEL::C_CannonDrivingDevice_IBIT
typedef struct {
    T_IdentifierType A_sourceID;
    T_DateTimeType   A_timeOfDataGeneration;
    T_IdentifierType A_cannonDrivingDevice_sourceID;
    int32_t          A_referenceNum;
    bool             A_BITRunning;
    int32_t          A_upDownMotor;
    int32_t          A_roundMotor;
    int32_t          A_upDownAmp;
    int32_t          A_roundAmp;
    int32_t          A_baseGiro;
    int32_t          A_topForwardGiro;
    int32_t          A_vehicleForwardGiro;
    int32_t          A_powerController;
    int32_t          A_energyStorage;
    int32_t          A_directPower;
    int32_t          A_cableLoop;
} C_CannonDrivingDevice_IBIT;

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

// Receive Messages (AgentUI -> DemoApp) - canonical names use XML struct name (last underscore part)
#define TOPIC_runBIT           "P_Usage_And_Condition_Monitoring_PSM__C_Monitored_Entity_runBIT"
#define TOPIC_commandDriving   "P_NSTEL__C_CannonDrivingDevice_commandDriving"
#define TOPIC_VehicleSpeed     "P_NSTEL__C_VehicleSpeed"

#define TYPE_runBIT            "P_Usage_And_Condition_Monitoring_PSM::C_Monitored_Entity_runBIT"
#define TYPE_commandDriving    "P_NSTEL::C_CannonDrivingDevice_commandDriving"
#define TYPE_VehicleSpeed      "P_NSTEL::C_VehicleSpeed"

// Send Messages (DemoApp -> AgentUI) - canonical names: IBIT, PBIT, PowerOnBIT, Signal
#define TOPIC_IBIT             "P_NSTEL__C_CannonDrivingDevice_IBIT"
#define TOPIC_PBIT             "P_NSTEL__C_CannonDrivingDevice_PBIT"
#define TOPIC_PowerOnBIT       "P_NSTEL__C_CannonDrivingDevice_PowerOnBIT"
#define TOPIC_Signal           "P_NSTEL__C_CannonDrivingDevice_Signal"

#define TYPE_IBIT              "P_NSTEL::C_CannonDrivingDevice_IBIT"
#define TYPE_PBIT              "P_NSTEL::C_CannonDrivingDevice_PBIT"
#define TYPE_PowerOnBIT        "P_NSTEL::C_CannonDrivingDevice_PowerOnBIT"
#define TYPE_Signal            "P_NSTEL::C_CannonDrivingDevice_Signal"

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
    double drivingPosition;       // A_drivingPosition (azimuth)
    double upDownPosition;        // A_upDownPosition
    double roundAngleVelocity;    // A_roundAngleVelocity
    double upDownAngleVelocity;   // A_upDownAngleVelocity
    double cannonUpDownAngle;     // A_cannonUpDownAngle
    double topRelativeAngle;      // A_topRelativeAngle
    
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
    double azAngle;               // A_azAngle (azimuth angle)
    double e1AngleVelocity;       // A_e1AngleVelocity
    double roundGiro;             // A_roundGiro
    double upDownGiro;            // A_upDownGiro
    
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
    double speed;                 // A_speed (m/s)
    
    uint64_t last_update_time;   // Internal timestamp
} VehicleSpeedState;

// BIT Component State (12 components from XML schema)
// Internal representation now uses T_BITResultType to match schema semantics
typedef struct {
    T_BITResultType upDownMotor;
    T_BITResultType roundMotor;
    T_BITResultType upDownAmp;
    T_BITResultType roundAmp;
    T_BITResultType baseGiro;
    T_BITResultType topForwardGiro;         // A_topForwardGiro
    T_BITResultType vehicleForwardGiro;     // A_vehicleForwardGiro
    T_BITResultType powerController;
    T_BITResultType energyStorage;
    T_BITResultType directPower;
    T_BITResultType cableLoop;
    T_BITResultType bitRunning;             // PBIT/resultBIT only
} BITComponentState;

// CBIT has 3 additional components
typedef struct {
    BITComponentState base;      // 12 base components
    T_BITResultType upDownPark;
    T_BITResultType round_Park;
    T_BITResultType mainCannon_Lock;
    T_BITResultType commFault;
    T_BITResultType controllerNetwork; /* corresponds to A_controllerNetwork (T_BITResultType) */
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

    /* Performance instrumentation accumulators (nanoseconds / counts) */
    uint64_t timer_tick_ns_total;    // total time spent in timer tick (ns)
    uint32_t timer_tick_ns_count;    // samples

    uint64_t pub_signal_ns_total;    // total time spent in signal publish (ns)
    uint32_t pub_signal_count;

    uint64_t pub_cbit_ns_total;      // total time spent in cbit publish (ns)
    uint32_t pub_cbit_count;

    uint64_t pub_pbit_ns_total;      // one-shot pbit publish
    uint32_t pub_pbit_count;

    uint64_t pub_result_ns_total;    // resultBIT publish
    uint32_t pub_result_count;

    uint64_t json_dump_ns_total;     // time spent in j.dump()
    uint32_t json_dump_count;

    uint64_t legacy_write_ns_total;  // time spent in legacy_agent_write_json call
    uint32_t legacy_write_count;

    uint64_t pub_worker_write_ns_total; // time spent in publisher worker (dequeue->write)
    uint32_t pub_worker_write_count;

    // Scenario start time (wall-clock) recorded when scenario starts
    T_DateTimeType scenario_start_time;
    int scenario_started; /* boolean: 0 = not started, 1 = started */

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

#include <stddef.h>

// Print status to either console or TCP CLI (use to_tcp=1 for TCP client)
void demo_app_print_status(int to_tcp);
#ifdef __cplusplus
}
#endif

#endif /* DEMO_APP_H */
