/* msg_fields.h - Centralized JSON field name definitions and helper format macros
 * All field name macros and helper P_FMT / P_COLON macros are provided here.
 */

#ifndef MSG_FIELDS_H
#define MSG_FIELDS_H

/* Structural / common fields */
#define F_A_SOURCEID "A_sourceID"
#define F_A_RECIPIENTID "A_recipientID"
#define F_A_RESOURCEID "A_resourceId"
#define F_A_INSTANCEID "A_instanceId"
#define F_A_TIMEOFDATA "A_timeOfDataGeneration"
#define F_A_SECOND "A_second"
#define F_A_NANOSECONDS "A_nanoseconds"

/* Actuator signal fields */
#define F_A_AZANGLE "A_azAngle"
#define F_A_E1ANGLEVELOCITY "A_e1AngleVelocity"
#define F_A_ROUNDGYRO "A_roundGyro"
#define F_A_UPDOWNGYRO "A_upDownGyro"
#define F_A_DRIVINGPOSITION "A_drivingPosition"
#define F_A_UPDOWNPOSITION "A_upDownPosition"
#define F_A_ROUNDANGLEVELOCITY "A_roundAngleVelocity"
#define F_A_UPDOWNANGLEVELOCITY "A_upDownAngleVelocity"
#define F_A_CANNONUPDOWNANGLE "A_cannonUpDownAngle"
#define F_A_TOPRELATIVEANGLE "A_topRelativeAngle"

/* Control / command fields */
#define F_A_REFERENCE_NUM "A_referenceNum"
#define F_A_TYPE "A_type"
#define F_A_BITRUNNING "A_BITRunning"
#define F_A_SPEED "A_speed"
#define F_A_OPERATIONMODE "A_operationMode"
#define F_A_PARM "A_parm"
#define F_A_TARGET_DESINGATION "A_targetDesingation"
#define F_A_AUTO_ARM_POSITION "A_autoArmPosition"
#define F_A_MANUAL_ARM_POSITION "A_manualArmPosition"
#define F_A_MAIN_CANNON_RESTORE "A_mainCannonRestore"
#define F_A_MAN_CANNON_FIX "A_manCannonFix"
#define F_A_CLOSE_EQUIP_OPEN_STATUS "A_closeEquipOpenStatus"

/* Status / BIT fields */
#define F_A_ENEGERYSTORAGE "A_energyStorage"
#define F_A_MAINCANNONFIXSTATUS "A_mainCannonFixStatus"
#define F_A_DECKCLEARANCE "A_deckClearance"
#define F_A_AUTO_ARM_POSITION "A_autoArmPositionComplement"
#define F_A_MANUAL_ARM_POSITION_COMPLE "A_manualArmPositionComple"
#define F_A_MAIN_CANNON_RESTORE_COMPLEMENT "A_mainCannonRestoreComplement"
#define F_A_ARM_SAFETY_MAIN_CANNON_LOCK "A_armSafetyMainCannonLock"
#define F_A_SHUTDOWN "A_shutdown"
#define F_A_TOPFORWARDGRYRO "A_topForwardGryro"
#define F_A_VEHICLEFORWARDGYROI "A_vehicleForwardGyroi"
#define F_A_POWER_CONTROLLER "A_power_Controller"
#define F_A_ROUND_PARK "A_round_Park"
#define F_A_VEHICLEFORWARDGYRO "A_vehicleForwardGyro"
#define F_A_POWERCONTROLLER "A_powerController"
#define F_A_RUNBITENTITY_SOURCEID "A_runBITEntity_sourceID"
#define F_A_UPDOWNMOTOR "A_upDownMotor"
#define F_A_ROUNDMOTOR "A_roundMotor"
#define F_A_UPDOWNAMP "A_upDownAmp"
#define F_A_ROUNDAMP "A_roundAmp"
#define F_A_BASEGYRO "A_baseGyro"
#define F_A_DIRECTPOWER "A_directPower"
#define F_A_CABLELOOP "A_cableLoop"
#define F_A_UPDOWNPARK "A_upDownPark"
#define F_A_MAINCANNON_LOCK "A_mainCannon_Lock"
#define F_A_COMMFAULT "A_commFault"

/* PBIT/CBIT/RESULT fields */
#define F_P_RESULTBIT "P_resultBit"
#define F_P_PBIT "P_pbit"
#define F_P_CBIT "P_cbit"

/* Helper macros to build quoted JSON patterns and scanf formats */
#define P_QUOTE(name) "\"" name "\""
#define P_COLON(name) "\"" name "\":"
#define P_FMT_STR(name) "\"" name "\":\"%63[^\"]\""
#define P_FMT_FLOAT(name) "\"" name "\":%f"
#define P_FMT_UINT(name) "\"" name "\":%u"

#endif /* MSG_FIELDS_H */

