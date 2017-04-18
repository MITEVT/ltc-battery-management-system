#include "ssm.h"

#include "error_handler.h"
#include "bms_utils.h"
#include "board.h"

volatile uint32_t msTicks;

void SSM_Init(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    // Initialize BMS state variables
    state->curr_mode = BMS_SSM_MODE_INIT;
    state->init_state = BMS_INIT_OFF;

    output->read_eeprom_packconfig = false;
    output->check_packconfig_with_ltc = false;

    input->ltc_packconfig_check_done = false;
    input->eeprom_packconfig_read_done = false;

    input->hard_reset_line = false;
    input->pack_status->max_cell_temp_dC = 0;

    input->eeprom_read_error = false;

    Charge_Init(state);
    Discharge_Init(state);
}

void Init_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    switch(state->init_state) {
        case(BMS_INIT_OFF):
            output->read_eeprom_packconfig = true;
            state->init_state = BMS_INIT_READ_PACKCONFIG;
            input->eeprom_packconfig_read_done = false;
            break;
        case(BMS_INIT_READ_PACKCONFIG):
            if(input->eeprom_packconfig_read_done) {
                output->read_eeprom_packconfig = false;
                output->check_packconfig_with_ltc = true;
                state->init_state = BMS_INIT_CHECK_PACKCONFIG;
                input->ltc_packconfig_check_done = false;
                input->eeprom_packconfig_read_done = false;
            }
            break;
        case(BMS_INIT_CHECK_PACKCONFIG):
            if(input->ltc_packconfig_check_done) {
                output->check_packconfig_with_ltc = false;
                state->init_state = BMS_INIT_DONE;
                state->curr_mode = BMS_SSM_MODE_STANDBY;
                input->ltc_packconfig_check_done = false;
            }
            break;
        case(BMS_INIT_DONE):
            state->curr_mode = BMS_SSM_MODE_STANDBY;
            break;
    }
}

bool Is_Valid_Jump(BMS_SSM_MODE_T mode1, BMS_SSM_MODE_T mode2) {

    if(mode1 == BMS_SSM_MODE_STANDBY && mode2 == BMS_SSM_MODE_CHARGE) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_STANDBY && mode2 == BMS_SSM_MODE_BALANCE) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_STANDBY && mode2 == BMS_SSM_MODE_DISCHARGE) {
        return true;

    } else if(mode1 == BMS_SSM_MODE_CHARGE && mode2 == BMS_SSM_MODE_STANDBY) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_BALANCE && mode2 == BMS_SSM_MODE_STANDBY) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_DISCHARGE && mode2 == BMS_SSM_MODE_STANDBY) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_BALANCE && mode2 == BMS_SSM_MODE_STANDBY) {
        return true;

    } else if(mode1 == BMS_SSM_MODE_BALANCE && mode2 == BMS_SSM_MODE_CHARGE) {
        return true;
    } else if(mode1 == BMS_SSM_MODE_CHARGE && mode2 == BMS_SSM_MODE_BALANCE) {
        return true;
    }
    return false;
}

bool Is_Charge_Balance_Switch(BMS_SSM_MODE_T mode1, BMS_SSM_MODE_T mode2) {
    return (mode1 == BMS_SSM_MODE_BALANCE && mode2 == BMS_SSM_MODE_CHARGE)
        || (mode1 == BMS_SSM_MODE_CHARGE && mode2 == BMS_SSM_MODE_BALANCE);
}

bool Is_State_Done(BMS_STATE_T *state) {
    switch(state->curr_mode) {
        case BMS_SSM_MODE_CHARGE:
        case BMS_SSM_MODE_BALANCE:
            return state->charge_state == BMS_CHARGE_OFF;
        case BMS_SSM_MODE_DISCHARGE:
            return state->discharge_state == BMS_DISCHARGE_OFF; 
        case BMS_SSM_MODE_INIT:
            return state->init_state == BMS_INIT_DONE;
        case BMS_SSM_MODE_STANDBY:
            return true;
    }
    return false;
}

static void Check_Error(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    (void)(output);

    // checks if there is a reported error
    //  communicating with the eeprom or ltc    
    // if (input->eeprom_read_error) {
    //   state->error_code = BMS_EEPROM_ERROR;
    //   return;
    // }
    //[TODO] change to case statement
    if (state->curr_mode != BMS_SSM_MODE_INIT) {
        int16_t max_cell_temp_thres_dC = state->pack_config->max_cell_temp_dC;

        if (input->pack_status->pack_cell_min_mV < state->pack_config->cell_min_mV) {
            Error_Assert(ERROR_CELL_UNDER_VOLTAGE, input->msTicks);
        } else {
            Error_Pass(ERROR_CELL_UNDER_VOLTAGE);
        }

        if (input->pack_status->pack_cell_max_mV > state->pack_config->cell_max_mV + 5) {
            Error_Assert(ERROR_CELL_OVER_VOLTAGE, input->msTicks);
        } else {
            Error_Pass(ERROR_CELL_OVER_VOLTAGE);
        }

        if (input->pack_status->max_cell_temp_dC > max_cell_temp_thres_dC) {
            Error_Assert(ERROR_CELL_OVER_TEMP, input->msTicks);
        } else {
            Error_Pass(ERROR_CELL_OVER_TEMP);
        }

#ifdef FSAE_DRIVERS

        int16_t min_cell_temp_thres_dC = state->pack_config->min_cell_temp_dC;
        if (input->pack_status->min_cell_temp_dC < min_cell_temp_thres_dC) {
            Error_Assert(ERROR_CELL_UNDER_TEMP, input->msTicks);
        } else {
            Error_Pass(ERROR_CELL_UNDER_TEMP);
        }

#endif
    }
}


void SSM_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    // OUTLINE:
    // If change state request made and possible, change state
    // Check if in standby:
    //   if in standby:
    //        if mode request change valid, switch over
    //   else dispatch step to appropriate SM step
    Check_Error(input, state, output);

    if((Is_Valid_Jump(state->curr_mode, input->mode_request)
            && Is_State_Done(state))
            || Is_Charge_Balance_Switch(state->curr_mode, input->mode_request)) {
        state->curr_mode = input->mode_request;
        output->close_contactors = false;
        output->charge_req->charger_on = false;
        memset(output->balance_req, 0, sizeof(output->balance_req[0])*Get_Total_Cell_Count(state->pack_config));
    }

    switch(state->curr_mode) {
        case BMS_SSM_MODE_STANDBY:
            break;
        case BMS_SSM_MODE_INIT:
            Init_Step(input, state, output);
            break;
        case BMS_SSM_MODE_CHARGE:
            Charge_Step(input, state, output);
            break;
        case BMS_SSM_MODE_DISCHARGE:
            Discharge_Step(input, state, output);
            break;
        case BMS_SSM_MODE_BALANCE:
            Charge_Step(input, state, output);
            break;
    }
}
