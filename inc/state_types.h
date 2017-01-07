#ifndef STATE_TYPES_H
#define STATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t cell_min_mV;
	uint32_t cell_max_mV;
	uint32_t cell_capacity_cAh;
	uint8_t num_modules;
	uint8_t *num_cells_in_modules;
	uint8_t num_packcells_in_parallel;
} PACK_CONFIG_T;

typedef struct BMS_PACK_STATUS {
	uint32_t *cell_voltage_mV; // array size = #modules * cells/module
	uint32_t pack_current_mV;
	uint32_t pack_voltage_mV;
	uint32_t precharge_voltage;
	bool error;
} BMS_PACK_STATUS_T;

typedef struct BMS_CHARGER_STATUS {
    bool connected; // example charger status
	bool error;
} BMS_CHARGER_STATUS_T;

typedef enum BMS_SSM_MODE {
	BMS_SSM_MODE_INIT,
	BMS_SSM_MODE_STANDBY,
	BMS_SSM_MODE_CHARGE,
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
    PACK_CONFIG_T *pack_config;
	BMS_SSM_MODE_T curr_mode;
	BMS_CHARGE_MODE_T charge_state;
	BMS_DISCHARGE_MODE_T discharge_state;
	BMS_ERROR_T error;
} BMS_STATE_T;

typedef struct BMS_INPUT {
    bool hard_reset_line; // example hw input
	BMS_SSM_MODE_T mode_request;
} BMS_INPUT_T;

/*
 * HW request structs (this should be in output)
*/
typedef struct BMS_BALANCE_REQ {
	uint32_t *balance_status;
} BMS_BALANCE_REQ_T;

typedef struct BMS_CHARGE_REQ {
	bool charging;
	uint32_t max_current_mA;
	uint32_t max_voltage_mV;
	uint32_t error_status;
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

