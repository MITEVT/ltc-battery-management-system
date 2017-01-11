#ifndef _STATE_TYPES_H
#define _STATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t cell_min_mV;
	uint32_t cell_max_mV;
	uint32_t cell_capacity_cAh;
	uint8_t num_modules;
	uint8_t *num_cells_in_modules; // [TODO] refactor to module_cell_count
	uint16_t cell_charge_c_rating_cC;
	uint32_t bal_on_thresh_mV;
	uint32_t bal_off_thresh_mV;
	uint8_t pack_cells_p;
} PACK_CONFIG_T;

typedef struct BMS_PACK_STATUS {
	uint32_t *cell_voltage_mV; // array size = #modules * cells/module
	uint32_t pack_cell_max_mV;
	uint32_t pack_cell_min_mV;
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
	BMS_SSM_MODE_BALANCE,
	BMS_SSM_MODE_DISCHARGE,
	BMS_SSM_MODE_ERROR
} BMS_SSM_MODE_T;

typedef enum {
	BMS_INIT_MODE_OFF,
	BMS_INIT_MODE_INIT,
	BMS_INIT_MODE_RUN,
	BMS_INIT_MODE_DONE
} BMS_INIT_MODE_T;

typedef enum {
	BMS_CHARGE_OFF,
	BMS_CHARGE_INIT,
	BMS_CHARGE_CC,
	BMS_CHARGE_CV,
	BMS_CHARGE_BAL,
	BMS_CHARGE_DONE
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
    PACK_CONFIG_T *pack_config;
	BMS_SSM_MODE_T curr_mode;

    // sub state machine state
	BMS_INIT_MODE_T init_state;
	BMS_CHARGE_MODE_T charge_state;
	BMS_DISCHARGE_MODE_T discharge_state;

	BMS_ERROR_T error;
} BMS_STATE_T;

typedef struct BMS_INPUT {
    bool hard_reset_line; // example hw input
	BMS_SSM_MODE_T mode_request;
	uint32_t balance_mV;
	bool contactors_closed;
	BMS_PACK_STATUS_T *pack_status;
} BMS_INPUT_T;

/*
 * HW request structs (this should be in output)
*/
typedef struct BMS_CHARGE_REQ {
	bool charger_on;
	uint32_t charge_current_mA;
	uint32_t charge_voltage_mV;
} BMS_CHARGE_REQ_T;

typedef struct BMS_OUTPUT {
	BMS_CHARGE_REQ_T *charge_req;
	bool close_contactors;
	bool *balance_req;
    BMS_ERROR_T *error;
} BMS_OUTPUT_T;

#endif

