#include <string.h>
#include <stdlib.h>
#include "console.h"
#include "board.h"
#include "microrl.h"






void get(const char * const * argv) {
    rw_loc_lable_t rwloc;
    //loop over r/w entries
    bool foundloc = false;
    for (rwloc = 0; rwloc < RWL_LENGTH; ++rwloc){
        if (strcmp(argv[1],locstring[rwloc]) == 0){
            foundloc = true;
            break; 
        }
    }

    if (foundloc) {

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
                    break;
                case ROL_cell_voltage_mV:
                    Board_Println("Unimplimented!");
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
                case ROL_LENGTH:
                    break;
            }
        }
        else{
            Board_Println("invalid location");
        }
    }
    

}
void set(const char * const * argv) {
    rw_loc_lable_t rwloc;
    //loop over r/w entries
    bool foundloc = false;
    for (rwloc = 0; rwloc < RWL_LENGTH; ++rwloc){
        if (strcmp(argv[1],locstring[rwloc]) == 0){
            foundloc = true;
            break; 
        }
    }
    if (!foundloc) {
        //loop over r/o entries
        ro_loc_lable_t roloc;
        for (roloc = ROL_FIRST; roloc< ROL_LENGTH; ++roloc){
            if (strcmp(argv[1],locstring[roloc]) == 0){
                foundloc = true;
                Board_Println("this location is read only");
                break; 
            }
        }
    }
    if (foundloc)
    {
        /* code */
    }
    else{
        Board_Println("invalid location");
    }
}
void help(const char * const * argv) {
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
void config(const char * const * argv) {
    Board_Println("config");
    // [TODO]
}
void bal(const char * const * argv) {
    Board_Println("bal");
    // [TODO]
}                       

void console_init(BMS_INPUT_T * bms_input, BMS_STATE_T * bms_state, BMS_OUTPUT_T *bms_output){
    console.bms_input = bms_input;
    console.bms_state = bms_state;
    console.bms_output = bms_output;
}

void executerl(uint32_t argc, const char * const * argv){
    uint32_t command_i = 0;
    bool found_command = false;
    for (command_i = 0; command_i < NUMCOMMANDS; ++command_i)
    {
        if (memcmp(argv[0],commands[command_i],3) == 0){
            found_command = true;  
            break;
        }
    }
    if (found_command) {
        if (nargs[command_i] == argc-1)
        {
            handlers[command_i](argv);
        }
        else {
            Board_Println("inccorrect number of args");
        }
    }
    else{
        Board_Println("Unrecognized command");
    }
}