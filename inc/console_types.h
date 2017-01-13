#ifndef _CONSOLE_TYPES_H
#define _CONSOLE_TYPES_H


typedef enum command_label{
    C_GET,
    C_SET,
    C_HELP,
    C_CONFIG,
    C_BAL,
    NUMCOMMANDS
} command_label_t;

typedef enum rw_loc_label{
    RWL_cell_min_mV,
    RWL_cell_max_mV,
    RWL_cell_capacity_cAh,
    RWL_num_modules,
    RWL_num_cells_in_modules, //need to think through how this will work
    RWL_cell_charge_c_rating_cC,
    RWL_bal_on_thresh_mV,
    RWL_bal_off_thresh_mV,
    RWL_pack_cells_p,
    RWL_cv_min_current_mA,
    RWL_cv_min_current_ms,
    RWL_cc_cell_voltage_mV,
    RWL_LENGTH
} rw_loc_lable_t;

#define ROL_FIRST RWL_LENGTH

typedef enum ro_loc_label{
    ROL_state = ROL_FIRST,
    ROL_cell_voltage_mV,
    ROL_pack_cell_max_mV,
    ROL_pack_cell_min_mV,
    ROL_pack_current_mA,
    ROL_pack_voltage_mV,
    ROL_precharge_voltage,
    ROL_LENGTH
} ro_loc_lable_t;




#endif