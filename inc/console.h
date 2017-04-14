#include <string.h>
#include <stdlib.h>
#include "microrl.h"
#include "state_types.h"
#include "util.h"
#include "console_types.h"
#include "eeprom_config.h"

#ifndef _CONSOLE_H
#define _CONSOLE_H

// [TODO] Add command to reset to factory default

static const char * const commands[] = { "get",
                            "set",
                            "help",
                            "config",
                            "bal",
                            "chrg",
                            "dis",
                            "config_def",
                            "measure"
                                    };

static const char nargs[ARRAY_SIZE(commands)] = {  1 ,
                        2 ,
                        1 ,
                        0 ,
                        1 ,
                        0 ,
                        0 ,
                        0 ,
                        1};

static const char * const helpstring[NUMCOMMANDS] = {"Get a value. Possible options:", 
                            "Set a value. Possible options:", "Get help!", 
                            "go through BMS initialization procedure again, load from eeprom", 
                            "set balance current: bal [off|NUMBER]",
                            "go into charge mode: chrg [on|off]",
                            "go into discharge mode: dis [on|off]",
                            "configure pack config defaults",
                            "start measurement printout mode, four flags (pcurrent/pvoltage/cell temps/voltages): measure [print_flags|temps|voltages|packcurrent|packvoltage|on|off]"};

static const char * const locstring[] =  {
                            "cell_min_mV",
                            "cell_max_mV",
                            "cell_capacity_cAh",
                            "num_modules",
                            "module_cell_count",
                            "cell_charge_c_rating_cC",
                            "bal_on_thresh_mV",
                            "bal_off_thresh_mV",
                            "pack_cells_p",
                            "cv_min_current_mA",
                            "cv_min_current_ms",
                            "cc_cell_voltage_mV",
                            "cell_discharge_c_rating_cC",
                            "max_cell_temp_param",
                            //can't write to the follwing
                            "state",
                            "cvm",
                            "temps",
                            "pack_cell_max_mV",
                            "pack_cell_min_mV",
                            "pack_current_mA",
                            "pack_voltage_mV",
                            "max_temp",
                            "error"
};

static const uint32_t locparam[ARRAY_SIZE(locstring)][3] = { 
                            {1, 0,UINT32_MAX},//"cell_min_mV",
                            {1, 0,UINT32_MAX},//"cell_max_mV",
                            {1, 0,UINT32_MAX},//"cell_capacity_cAh",
                            {1, 0,UINT8_MAX},//"num_modules",
                            {20, 0,UINT8_MAX},//"module_cell_count",
                            {1, 0,UINT16_MAX},//"cell_charge_c_rating_cC",
                            {1, 0,UINT32_MAX},//"bal_on_thresh_mV",
                            {1, 0,UINT32_MAX},//"bal_off_thresh_mV",
                            {1, 0,UINT8_MAX},//"pack_cells_p",
                            {1, 0,UINT32_MAX},//"cv_min_current_mA",
                            {1, 0,UINT32_MAX},//"cv_min_current_ms",
                            {1, 0,UINT32_MAX},//"cc_cell_voltage_mV",
                            {1, 0,UINT32_MAX},//"cell_discharge_c_rating_cC",
                            {1, 0,UINT32_MAX},//"max_cell_temp_dC",
                            //can't write to the follwing
                            {0,0,0},//"state",
                            {0,0,0},//"*cell_voltages_mV",
                            {0,0,0},//"temps",
                            {0,0,0},//"pack_cell_max_mV",
                            {0,0,0},//"pack_cell_min_mV",
                            {0,0,0},//"pack_current_mA",
                            {0,0,0},//"pack_voltage_mV",
                            {0,0,0},//"max_temp",
                            {0,0,0}//"error"
};

typedef void (* const EXECUTE_HANDLER)(const char * const *);

typedef struct {
    bool valid_mode_request;
    bool config_default;
    BMS_SSM_MODE_T mode_request;
    uint32_t balance_mV;
    bool measure_on;
    bool measure_temp;
    bool measure_voltage;
    bool measure_packcurrent;
    bool measure_packvoltage;
} CONSOLE_OUTPUT_T;

typedef struct console_t {
    BMS_INPUT_T     *bms_input;
    BMS_STATE_T     *bms_state;
    CONSOLE_OUTPUT_T *console_output;
} console_t;

// static console_t console;

void console_init(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, CONSOLE_OUTPUT_T *console_output);
void executerl(int32_t argc, const char * const * argv);  


#endif
