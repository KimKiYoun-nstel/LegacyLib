/* msg_fields.h - Centralized JSON field name definitions and helper format macros
 * All field name macros and helper P_FMT / P_COLON macros are provided here.
 */

#ifndef MSG_FIELDS_H
#define MSG_FIELDS_H

#define F_A_SOURCEID "A_sourceID"
#define F_A_RECIPIENTID "A_recipientID"
#define F_A_RESOURCEID "A_resourceId"
#define F_A_INSTANCEID "A_instanceId"
#define F_A_TIMEOFDATA "A_timeOfDataGeneration"
#define F_A_SECOND "A_second"
#define F_A_NANOSECONDS "A_nanoseconds"

/* Actuator signal/control fields (align with attachments)
 * - Some messages use 'A_roundPosition' for azimuth position
 * - Signal uses velocity-named azimuth field 'A_azAngleVelocity'
 */
#define F_A_AZANGLEVELOCITY "A_azAngleVelocity"
#define F_A_E1ANGLEVELOCITY "A_e1AngleVelocity"
#define F_A_ROUNDGIRO "A_roundGiro"
#define F_A_UPDOWNGIRO "A_upDownGiro"
/* Control/command position naming (attachment uses A_roundPosition)
 * keep alias name for compatibility
 */
#define F_A_DRIVINGPOSITION "A_roundPosition"
#define F_A_UPDOWNPOSITION "A_upDownPosition"
#define F_A_ROUNDANGLEVELOCITY "A_roundAngleVelocity"
#define F_A_UPDOWNANGLEVELOCITY "A_upDownAngleVelocity"
#define F_A_CANNONUPDOWNANGLE "A_cannonUpDownAngle"
#define F_A_TOPRELATIVEANGLE "A_topRelativeAngle"

/* Control / command fields */
#define F_A_REFERENCE_NUM "A_referenceNum"
#define F_A_TYPE "A_type"
#define F_A_BITRUNNING "A_BITRunning"
/* Vehicle speed in current attachments uses 'A_value' */
#define F_A_SPEED "A_value"
#define F_A_OPERATIONMODE "A_operationMode"
#define F_A_PARM "A_parm"
/* Attachment changed 'A_targetDesingation' -> 'A_targetFix' */
#define F_A_TARGET_DESIGNATION "A_targetFix"
#define F_A_AUTO_ARM_POSITION "A_autoArmPosition"
#define F_A_MANUAL_ARM_POSITION "A_manualArmPosition"
#define F_A_MAIN_CANNON_RESTORE "A_mainCannonRestore"
/* Attachment uses 'A_mainCannonFix' spelling */
#define F_A_MAIN_CANNON_FIX "A_mainCannonFix"
/* Attachment uses 'A_closureEquipOpenStatus' */
#define F_A_CLOSE_EQUIP_OPEN_STATUS "A_closureEquipOpenStatus"

/* Status / BIT fields (updated to actual incoming names)
 * Note: attachments include several typos; definitions mirror attachments
 */
#define F_A_ENERGY_STORAGE "A_energyStorage"
#define F_A_MAINCANNONFIXSTATUS "A_mainCannonFixStatus"
/* attachment uses 'A_deckCleance' (typo preserved) */
#define F_A_DECKCLEARANCE "A_deckCleance"
#define F_A_AUTO_ARM_POSITION_COMPLEMENT "A_autoArmPositionComplement"
#define F_A_MANUAL_ARM_POSITION_COMPLEMENT "A_manualArmPositionComplement"
#define F_A_MAIN_CANNON_RESTORE_COMPLEMENT "A_mainCannonRestoreComplement"
#define F_A_ARM_SAFETY_MAIN_CANNON_LOCK "A_armSafetyMainCannonLock"
#define F_A_SHUTDOWN "A_shutdown"
#define F_A_TOPFORWARDGIRO "A_topForwardGiro"
#define F_A_VEHICLEFORWARDGIRO "A_vehicleForwardGiro"
#define F_A_POWER_CONTROLLER "A_powerController"
#define F_A_ROUND_PARK "A_roundPark"
#define F_A_RUNBITENTITY_SOURCEID "A_runBITEntity_sourceID"
#define F_A_UPDOWNMOTOR "A_upDownMotor"
#define F_A_ROUNDMOTOR "A_roundMotor"
#define F_A_UPDOWNAMP "A_upDownAmp"
#define F_A_ROUNDAMP "A_roundAmp"
#define F_A_BASEGIRO "A_baseGiro"
#define F_A_DIRECTPOWER "A_directPower"
#define F_A_CABLELOOP "A_cableLoop"
#define F_A_UPDOWNPARK "A_upDownPark"
#define F_A_MAINCANNON_LOCK "A_mainCannonLock"
#define F_A_COMMFAULT "A_commFault"

/* Specific source id used in some attachment files */
#define F_A_CANNON_SOURCEID "A_cannonDrivingDevice_sourceID"

/* NOTE: Removed compatibility aliases — only current/actual field names are defined. */

/* Controller network field used by PBIT/CBIT per schema */
#define F_A_CONTROLLER_NETWORK "A_controllerNetwork"

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

