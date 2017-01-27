#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "board.h"
#include "bms_utils.h"
#include "microrl.h"
#include "console_types.h"



uint32_t my_atou(const char *str)
{
    uint32_t res = 0; // Initialize result
  
    // Iterate through all characters of input string and
    // update result
    uint32_t i;
    for ( i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';
  
    // return result.
    return res;
}

static void get(const char * const * argv) {
    rw_loc_lable_t rwloc;
    uint8_t i;

    //loop over r/w entries
    bool foundloc = false;
    for (rwloc = 0; rwloc < RWL_LENGTH; ++rwloc){
        if (strcmp(argv[1],locstring[rwloc]) == 0){
            foundloc = true;
            break; 
        }
    }

    if (foundloc) {
        char tempstr[20];
        switch (rwloc) {
            case RWL_cell_min_mV:
                utoa(console.bms_state->pack_config->cell_min_mV, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cell_max_mV:
                utoa(console.bms_state->pack_config->cell_max_mV, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cell_capacity_cAh:
                utoa(console.bms_state->pack_config->cell_capacity_cAh, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_num_modules:
                utoa(console.bms_state->pack_config->num_modules, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_num_cells_in_modules:
                for(i = 0; i < console.bms_state->pack_config->num_modules; i++) {
                    utoa(console.bms_state->pack_config->num_cells_in_modules[i],
                            tempstr, 10);
                    Board_Println(tempstr);
                }
                break;
            case RWL_cell_charge_c_rating_cC:
                utoa(console.bms_state->pack_config->cell_charge_c_rating_cC, tempstr,10);
                Board_Println(tempstr);
                break;  
            case RWL_bal_on_thresh_mV:
                utoa(console.bms_state->pack_config->bal_on_thresh_mV, tempstr,10);
                Board_Println(tempstr);
                break;  
            case RWL_bal_off_thresh_mV:
                utoa(console.bms_state->pack_config->bal_off_thresh_mV, tempstr,10);
                Board_Println(tempstr);
                break; 
            case RWL_pack_cells_p:
                utoa(console.bms_state->pack_config->pack_cells_p, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cv_min_current_mA:
                utoa(console.bms_state->pack_config->cv_min_current_mA, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cv_min_current_ms:
                utoa(console.bms_state->pack_config->cv_min_current_ms, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cc_cell_voltage_mV:
                utoa(console.bms_state->pack_config->cc_cell_voltage_mV, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_cell_discharge_c_rating_cC:
                utoa(console.bms_state->pack_config->cell_discharge_c_rating_cC, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_max_cell_temp_C:
                utoa(console.bms_state->pack_config->max_cell_temp_C, tempstr,10);
                Board_Println(tempstr);
                break;
            case RWL_LENGTH:
                break;
        }

    }
    else {
        //loop over r/o entries
        ro_loc_lable_t roloc;
        for (roloc = ROL_FIRST; roloc< ROL_LENGTH; ++roloc){
            if (strcmp(argv[1],locstring[roloc]) == 0){
                foundloc = true;
                break; 
            }
        }
        if (foundloc) {
            char tempstr[20];
            switch (roloc) {
                case ROL_state:
                    Board_Println(BMS_SSM_MODE_NAMES[console.bms_state->curr_mode]);
                    Board_Println(BMS_INIT_MODE_NAMES[console.bms_state->init_state]);
                    Board_Println(BMS_CHARGE_MODE_NAMES[console.bms_state->charge_state]);
                    Board_Println(BMS_DISCHARGE_MODE_NAMES[console.bms_state->discharge_state]);
                    Board_Println(BMS_ERROR_NAMES[console.bms_state->error_code]);
                    break;
                case ROL_cell_voltage_mV:
                    for(i = 0; i < Get_Total_Cell_Count(console.bms_state->pack_config); i++) {
                       utoa(console.bms_input->pack_status->cell_voltage_mV[i], tempstr, 10);
                       Board_Println(tempstr);
                    }
                    break;
                case ROL_pack_cell_max_mV:
                    utoa(console.bms_input->pack_status->pack_cell_max_mV, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_pack_cell_min_mV:
                    utoa(console.bms_input->pack_status->pack_cell_min_mV, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_pack_current_mA:
                    utoa(console.bms_input->pack_status->pack_current_mA, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_pack_voltage_mV:
                    utoa(console.bms_input->pack_status->pack_voltage_mV, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_precharge_voltage:
                    utoa(console.bms_input->pack_status->precharge_voltage, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_max_cell_temp_C:
                    utoa(console.bms_input->pack_status->max_cell_temp_C, tempstr,10);
                    Board_Println(tempstr);
                    break;
                case ROL_error:
                    if(console.bms_input->pack_status->error) {
                        Board_Println("Pack has error!");
                    } else {
                        Board_Println("Pack has no error!");
                    } 
                    break;
                case ROL_LENGTH:
                    break;
            }
        }
        else{
            Board_Println("invalid get location");
        }
    }
}

// [TODO] Check max/min bounds and max > min
static void set(const char * const * argv) {
    if (console.bms_state->curr_mode != BMS_SSM_MODE_STANDBY)
    {
        Board_Println("Set failed (not in standby mode)!");
        return;
    }
    rw_loc_lable_t rwloc;
    //loop over r/w entries
    bool foundloc = false;
    for (rwloc = 0; rwloc < RWL_LENGTH; ++rwloc){
        if (strcmp(argv[1],locstring[rwloc]) == 0){
            foundloc = true;
            break; 
        }
    }
    if(foundloc){
        uint8_t ret;
        ret = EEPROM_Change_Config(rwloc,my_atou(argv[2]));
        if(ret != 0) {
            Board_Println("Set failed (command not yet implemented?)!");
        }
    } else {
        //loop over r/o entries
        ro_loc_lable_t roloc;
        for (roloc = ROL_FIRST; roloc< ROL_LENGTH; ++roloc){
            if (strcmp(argv[1],locstring[roloc]) == 0){
                foundloc = true;
                Board_Println("this location is read only");
                return; 
            }
        }
        Board_Println("invalid location");
    }
}

static void help(const char * const * argv) {
    command_label_t command_i = 0;
    for (command_i = 0; command_i < NUMCOMMANDS; ++command_i)
    {
        if (strcmp(argv[1],commands[command_i]) == 0){
            Board_Println_BLOCKING(helpstring[command_i]); //blocking print

            break; 
        }
    }
    Board_Print("");

    if (command_i == C_GET || command_i == C_SET)
    {
        rw_loc_lable_t i;
        Board_Println_BLOCKING("------r/w entries------");
        for (i = 0; i < RWL_LENGTH; ++i){
            Board_Println_BLOCKING(locstring[i]); //blocking print.
        }

        Board_Println_BLOCKING("------r/o entries------");
        for (i = ROL_FIRST; i < ROL_LENGTH; ++i){
            Board_Println_BLOCKING(locstring[i]); //blocking print.
        }
    }

}

// [TODO] This might not be safe
static void config(const char * const * argv) {
    UNUSED(argv);
    if (console.bms_state->curr_mode == BMS_SSM_MODE_STANDBY)
    {
        console.bms_state->curr_mode = BMS_SSM_MODE_INIT;
        console.bms_state->init_state = BMS_INIT_OFF;
    }
}

// [TODO] This should set a flag to be handled in Process_Input
static void bal(const char * const * argv) {
    UNUSED(argv);
    if (console.bms_state->curr_mode == BMS_SSM_MODE_STANDBY ||
            console.bms_state->curr_mode == BMS_SSM_MODE_BALANCE) {    
        
        if (strcmp(argv[1],"off") == 0) {
            console.console_output->valid_mode_request = false;
            console.console_output->balance_mV = UINT32_MAX;
            Board_Println("bal off");
        } else {
            console.console_output->valid_mode_request = true;
            console.console_output->mode_request = BMS_SSM_MODE_BALANCE;
            console.console_output->balance_mV = my_atou(argv[1]);
            Board_Println("bal on");
        }
    } else {
        Board_Println("Must be in standby");
    }
}                       

void console_init(BMS_INPUT_T * bms_input, BMS_STATE_T * bms_state, CONSOLE_OUTPUT_T *console_output){
    console.bms_input = bms_input;
    console.bms_state = bms_state;
    console.console_output = console_output;
    console.console_output->valid_mode_request = false;
    console.console_output->mode_request = BMS_SSM_MODE_STANDBY;
}

static const EXECUTE_HANDLER handlers[] = {get, set, help, config, bal};

void executerl(uint32_t argc, const char * const * argv){
    uint32_t command_i = 0;
    bool found_command = false;
    for (command_i = 0; command_i < NUMCOMMANDS; ++command_i)
    {
        if (strcmp(argv[0],commands[command_i]) == 0){
            found_command = true;  
            break;
        }
    }
    if (found_command) {
        if (nargs[command_i] == (uint32_t)(argc-1))
        {
            handlers[command_i](argv);
        }
        else {
            Board_Println("incorrect number of args");
        }
    }
    else{
        Board_Println("Unrecognized command");
    }
}
