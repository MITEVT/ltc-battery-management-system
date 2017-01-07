#ifndef STATE_TYPES_H
#define STATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef bool PACK_ERROR_T;
typedef bool CHARGER_ERROR_T;

typedef struct BMS_PACK_STATUS{
	uint32_t 	numCells;
	uint32_t * 	cellVoltage_mV;
	uint32_t	packCurrent_mv;
	uint32_t	packVoltage_mv;
	uint32_t	prechargeVoltage;
	PACK_ERROR_T error;
} BMS_PACK_STATUS_T;

typedef struct BMS_CHARGE_STATUS{
	CHARGER_ERROR_T error;
} BMS_CHARGE_STATUS_T;

/*
 * REQ structs
*/
typedef struct BMS_BALLANCE_REQ{
	uint32_t 	numCells;
	uint32_t * 	ballenceStatus;
} BMS_BALLANCE_REQ_T;

typedef struct BMS_CHARGE_REQ{
	bool charging;
	uint32_t maxCurrent_mA;
	uint32_t maxVoltage_mV;
	uint32_t errorStatus;
} BMS_CHARGE_REQ_T;

typedef enum BMS_CONTACTOR_REQ
{
	CONTACTOR_OFF,
	CONTACTOR_PRECHARGE,
	CONTACTOR_ON_W_PRECHARGE,
	CONTACTOR_ON
} BMS_CONTACTOR_REQ_T;

/*
 * Struct for the SSM	
*/

 typedef enum {
	BMS_SSM_MODE_INIT,
	BMS_SSM_MODE_CHARGE,
	BMS_SSM_MODE_BALLANCE,
	BMS_SSM_MODE_DISCHARGE,
	BMS_SSM_MODE_ERROR
} BMS_SSM_MODE_T;

typedef enum{
	BMS_DISCHARGE_MODE_OFF,
	BMS_DISCHARGE_MODE_INIT,
	BMS_DISCHARGE_MODE_RUN,
	BMS_DISCHARGE_MODE_DONE
} BMS_DISCHARGE_MODE_T;

typedef enum{
	BMS_CHARGE_MODE_OFF,
	BMS_CHARGE_MODE_INIT,
	BMS_CHARGE_MODE_RUN,
	BMS_CHARGE_MODE_DONE
} BMS_CHARGE_MODE_T;

typedef struct BMS_SM_OUTPUT{
	BMS_BALLANCE_REQ_T ballence_Req;
	BMS_CHARGE_REQ_T charge_Req;
	BMS_CONTACTOR_REQ_T contactor_Req;
} BMS_SM_OUTPUT_T;

typedef struct BMS_SM_INPUT{
	BMS_SSM_MODE_T mode_request;
	BMS_CHARGE_STATUS_T charger_status;
	BMS_PACK_STATUS_T pack_status;
} BMS_SM_INPUT_T;

#endif
//STATE_TYPES_H

