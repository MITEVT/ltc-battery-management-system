#ifndef STATE_TYPES_H
#define STATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BMS_PACK_STATUS {
	uint32_t numCells;
	uint32_t *cellVoltage_mV;
	uint32_t packCurrent_mv;
	uint32_t packVoltage_mv;
	uint32_t prechargeVoltage;
	bool error;
} BMS_PACK_STATUS_T;

typedef struct BMS_CHARGER_STATUS {
    bool connected;
	bool error;
} BMS_CHARGER_STATUS_T;

typedef enum BMS_SSM_MODE {
	BMS_SSM_MODE_INIT,
	BMS_SSM_MODE_CHARGE,
	BMS_SSM_MODE_BALANCE,
	BMS_SSM_MODE_DISCHARGE,
	BMS_SSM_MODE_ERROR
} BMS_SSM_MODE_T;

typedef enum {
	BMS_CHARGE_MODE_OFF,
	BMS_CHARGE_MODE_INIT,
	BMS_CHARGE_MODE_RUN,
	BMS_CHARGE_MODE_DONE
} BMS_CHARGE_MODE_T;

typedef enum {
	BMS_DISCHARGE_MODE_OFF,
	BMS_DISCHARGE_MODE_INIT,
	BMS_DISCHARGE_MODE_RUN,
	BMS_DISCHARGE_MODE_DONE
} BMS_DISCHARGE_MODE_T;

typedef enum BMS_ERROR {
    BMS_BLEW_UP_ERROR
} BMS_ERROR_T;

typedef struct BMS_STATE {
    BMS_CHARGER_STATUS_T *charger_status;
    BMS_PACK_STATUS_T *pack_status;
	BMS_SSM_MODE_T curr_mode;
	BMS_CHARGE_MODE_T charge_state;
	BMS_DISCHARGE_MODE_T discharge_state;
	BMS_ERROR_T error;
} BMS_STATE_T;

typedef struct BMS_INPUT {
    bool hard_reset_line;
	BMS_SSM_MODE_T mode_request;
} BMS_INPUT_T;

/*
 * HW request structs (this should be in output)
*/
typedef struct BMS_BALANCE_REQ {
	uint32_t numCells;
	uint32_t *balanceStatus;
} BMS_BALANCE_REQ_T;

typedef struct BMS_CHARGE_REQ {
	bool charging;
	uint32_t maxCurrent_mA;
	uint32_t maxVoltage_mV;
	uint32_t errorStatus;
} BMS_CHARGE_REQ_T;

typedef enum BMS_CONTACTOR_REQ {
	CONTACTOR_OFF,
	CONTACTOR_PRECHARGE,
	CONTACTOR_ON_W_PRECHARGE,
	CONTACTOR_ON
} BMS_CONTACTOR_REQ_T;

typedef struct BMS_OUTPUT{
	BMS_BALANCE_REQ_T *balance_req;
	BMS_CHARGE_REQ_T *charge_req;
	BMS_CONTACTOR_REQ_T *contactor_req;
    BMS_ERROR_T *error;
} BMS_OUTPUT_T;

#endif

