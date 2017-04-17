#ifndef _STATE_TYPES_H
#define _STATE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t cell_min_mV;               // 1
    uint32_t cell_max_mV;
    uint32_t cell_capacity_cAh;
    uint32_t num_modules;
    uint32_t cell_charge_c_rating_cC;   // 5
    uint32_t bal_on_thresh_mV;
    uint32_t bal_off_thresh_mV;
    uint32_t pack_cells_p;
    uint32_t cv_min_current_mA;
    uint32_t cv_min_current_ms;         // 10
    uint32_t cc_cell_voltage_mV;        // 11
    // Size = 4*11 = 44 bytes
    
    uint32_t cell_discharge_c_rating_cC; // at 27 degrees C
    uint32_t max_cell_temp_dC;
    // FSAE specific configurations
#ifdef FSAE_DRIVERS
    int16_t min_cell_temp_dC;
    int16_t fan_on_threshold_dC;
#endif //FSAE_DRIVERS

    uint8_t *module_cell_count;
    // Total Size = 44 + 4 + 4 + 1= 53 bytes (not including fan_on_threshold)
    
} PACK_CONFIG_T;

typedef struct BMS_PACK_STATUS {
    uint32_t *cell_voltages_mV; // array size = #modules * cells/module
    int16_t *cell_temperatures_dC; // array size = #modules * thermistors/module
    uint32_t pack_cell_max_mV;
    uint32_t pack_cell_min_mV;
    uint32_t pack_current_mA;
    uint32_t pack_voltage_mV;
    int16_t max_cell_temp_dC;

    //FSAE specific pack status variables
#ifdef FSAE_DRIVERS
    int16_t min_cell_temp_dC;
    int16_t avg_cell_temp_dC;
    uint16_t max_cell_temp_position; //range: 0-MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE
    uint16_t min_cell_temp_position; //range: 0-MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE
#endif //FSAE_DRIVERS

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
} BMS_SSM_MODE_T;

static const char * const BMS_SSM_MODE_NAMES[] = {
    "BMS_SSM_MODE_INIT",
    "BMS_SSM_MODE_STANDBY",
    "BMS_SSM_MODE_CHARGE",
    "BMS_SSM_MODE_BALANCE",
    "BMS_SSM_MODE_DISCHARGE",
};

typedef enum {
    BMS_CHARGE_OFF,
    BMS_CHARGE_INIT,
    BMS_CHARGE_CC,
    BMS_CHARGE_CV,
    BMS_CHARGE_BAL,
    BMS_CHARGE_DONE
} BMS_CHARGE_MODE_T;

static const char * const BMS_CHARGE_MODE_NAMES[] = {
    "BMS_CHARGE_OFF",
    "BMS_CHARGE_INIT",
    "BMS_CHARGE_CC",
    "BMS_CHARGE_CV",
    "BMS_CHARGE_BAL",
    "BMS_CHARGE_DONE"
};

typedef enum {
    BMS_INIT_OFF,
    BMS_INIT_READ_PACKCONFIG,
    BMS_INIT_CHECK_PACKCONFIG,
    BMS_INIT_DONE
} BMS_INIT_MODE_T;

static const char * const BMS_INIT_MODE_NAMES[] = {
    "BMS_INIT_OFF",
    "BMS_INIT_READ_PACKCONFIG",
    "BMS_INIT_CHECK_PACKCONFIG",
    "BMS_INIT_DONE"
};

typedef enum {
    BMS_DISCHARGE_OFF,
    BMS_DISCHARGE_INIT,
    BMS_DISCHARGE_RUN,
    BMS_DISCHARGE_DONE
} BMS_DISCHARGE_MODE_T;

static const char * const BMS_DISCHARGE_MODE_NAMES[] = {
    "BMS_DISCHARGE_OFF",
    "BMS_DISCHARGE_INIT",
    "BMS_DISCHARGE_RUN",
    "BMS_DISCHARGE_DONE"
};

typedef struct BMS_STATE {
    BMS_CHARGER_STATUS_T *charger_status;
    PACK_CONFIG_T *pack_config;
    BMS_SSM_MODE_T curr_mode;

    // sub state machine state
    BMS_INIT_MODE_T init_state;
    BMS_CHARGE_MODE_T charge_state;
    BMS_DISCHARGE_MODE_T discharge_state;

} BMS_STATE_T;

typedef struct BMS_INPUT {
    bool hard_reset_line; // example hw input
    BMS_SSM_MODE_T mode_request;
    uint32_t balance_mV; // console request balance to mV
    bool contactors_closed;
    uint32_t msTicks;
    BMS_PACK_STATUS_T *pack_status;
    bool charger_on;
    // [TODO] Add input charge connected

    // for bms initialization
    bool eeprom_packconfig_read_done;
    bool ltc_packconfig_check_done;

    bool eeprom_read_error;

#ifdef FSAE_DRIVERS
    uint32_t last_vcu_msg_ms;
#endif
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

    // for bms initialization
    bool read_eeprom_packconfig;
    bool check_packconfig_with_ltc;

    // FSAE specific output variables
#ifdef FSAE_DRIVERS
    bool fans_on;
#endif //FSAE_DRIVERS
} BMS_OUTPUT_T;

#endif

