#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "microrl.h"
#include "state_types.h"
#include "util.h"




// microrl object
static microrl_t rl;

typedef void (* const EXECUTE_HANDLER)(const char * const *);



static void get(const char * const *);
static void set(const char * const *);
static void help(const char * const *);
static void config(const char * const *);
static void bal(const char * const *);

typedef struct console_t
{
	BMS_INPUT_T 	* bms_input;
	BMS_STATE_T 	* bms_state;
	BMS_OUTPUT_T 	* bms_output;
} console_t;

typedef enum command_label{
    C_GET,
    C_SET,
    C_HELP,
    C_CONFIG,
    C_BAL,
    NUMCOMMANDS
} command_label_t;

static const char * const commands[NUMCOMMANDS] = { "get",
                            "set",
                            "help",
                            "config",
                            "bal"};



static const char nargs[NUMCOMMANDS] = {  1 ,
                        2 ,
                        1 ,
                        0 ,
                        1};

static const char * const helpstring[NUMCOMMANDS] = {"Get a value. Possible options:", 
							"Set a value. Possible options:", "This", 
							"flash that sucker", 
							"set ballence current"};

typedef enum rw_loc_label{
    RWL_cell_min_mV,
    RWL_cell_max_mV,
    RWL_cell_capacity_cAh,
    RWL_num_modules,
    RWL_num_cells_in_modules,
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
    ROL_precharge_voltag,
    ROL_LENGTH
} ro_loc_lable_t;

static const char * const locstring[] =  {
                            "cell_min_mV",
                            "cell_max_mV",
                            "cell_capacity_cAh",
                            "num_modules",
                            "num_cells_in_modules",
                            "cell_charge_c_rating_cC",
                            "bal_on_thresh_mV",
                            "bal_off_thresh_mV",
                            "pack_cells_p",
                            "cv_min_current_mA",
                            "cv_min_current_ms",
                            "cc_cell_voltage_mV",
                            //can't write to the follwing
                            "state",
                            "cell_voltage_mV",
                            "pack_cell_max_mV",
                            "pack_cell_min_mV",
                            "pack_current_mA",
                            "pack_voltage_mV",
                            "precharge_voltage"
};

static const uint32_t locparam[ARRAY_SIZE(locstring)][3] = {
                            {1, 0,UINT32_MAX},//"cell_min_mV",
                            {1, 0,UINT32_MAX},//"cell_max_mV",
                            {1, 0,UINT32_MAX},//"cell_capacity_cAh",
                            {1, 0,UINT8_MAX},//"num_modules",
                            {20, 0,UINT8_MAX},//"num_cells_in_modules",
                            {1, 0,UINT16_MAX},//"cell_charge_c_rating_cC",
                            {1, 0,UINT32_MAX},//"bal_on_thresh_mV",
                            {1, 0,UINT32_MAX},//"bal_off_thresh_mV",
                            {1, 0,UINT8_MAX},//"pack_cells_p",
                            {1, 0,UINT32_MAX},//"cv_min_current_mA",
                            {1, 0,UINT32_MAX},//"cv_min_current_ms",
                            {1, 0,UINT32_MAX},//"cc_cell_voltage_mV",
                            //can't write to the follwing
                            {0,0},//"state",
                            {0,0},//"*cell_voltage_mV",
                            {0,0},//"pack_cell_max_mV",
                            {0,0},//"pack_cell_min_mV",
                            {0,0},//"pack_current_mA",
                            {0,0},//"pack_voltage_mV",
                            {0,0}//"precharge_voltage"
};





void console_init(void);
void executerl(uint32_t argc, const char * const * argv);                    