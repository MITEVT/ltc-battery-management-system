#include "charge.h"
#include "bms_utils.h"

static uint16_t total_num_cells;
static uint32_t cc_charge_voltage_mV;
static uint32_t cc_charge_current_mA;
static uint32_t cv_charge_voltage_mV;
static uint32_t cv_charge_current_mA;
static uint32_t last_time_above_cv_min_curr;

bool _calc_balance(bool *balance_req, uint32_t *cell_voltages_mV, uint32_t balance_mV, PACK_CONFIG_T *config);
void _set_output(bool close_contactors, bool charger_on, uint32_t charge_voltage_mV, uint32_t charge_current_mA, BMS_OUTPUT_T *output);

void Charge_Init(BMS_STATE_T *state) {
    state->charge_state = BMS_CHARGE_OFF;
    last_time_above_cv_min_curr = 0;
}

void Charge_Config(PACK_CONFIG_T *pack_config) {
    total_num_cells = Get_Total_Cell_Count(pack_config);

    cc_charge_voltage_mV = pack_config->cc_cell_voltage_mV * total_num_cells;
    cc_charge_current_mA = pack_config->cell_capacity_cAh * pack_config->cell_charge_c_rating_cC * pack_config->pack_cells_p / 10;

    cv_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
    cv_charge_current_mA = cc_charge_current_mA;
}

void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
#ifdef FSAE_DRIVERS
    if (input->pack_status->max_cell_temp_dC > state->pack_config->fan_on_threshold_dC) {
        output->fans_on = true;
    } else {
        output->fans_on = false;
    }
#endif //FSAE_DRIVERS

    switch (input->mode_request) {
        case BMS_SSM_MODE_CHARGE:
            if (state->charge_state == BMS_CHARGE_OFF 
                    || state->charge_state == BMS_CHARGE_BAL) {
                state->charge_state = BMS_CHARGE_INIT;
            }
            break;

        case BMS_SSM_MODE_BALANCE:
            if (state->charge_state == BMS_CHARGE_OFF 
                    || state->charge_state == BMS_CHARGE_CC 
                    || state->charge_state == BMS_CHARGE_CV) {
                state->charge_state = BMS_CHARGE_INIT;
            }
            break;

        // we want to switch states (either to STANDBY/DISCHARGE/ERROR)
        default:
            if(state->charge_state == BMS_CHARGE_OFF) {
                state->charge_state = BMS_CHARGE_OFF;
            } else {
                state->charge_state = BMS_CHARGE_DONE;
            }
            break;
    }

    switch (state->charge_state) {
        case BMS_CHARGE_OFF:
            _set_output(false, false, 0, 0, output);
            memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);
            break;
        case BMS_CHARGE_INIT:
            _set_output((input->mode_request == BMS_SSM_MODE_CHARGE), false, 0, 0, output);
            memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);
            
            if (input->contactors_closed == output->close_contactors) {
                if(input->mode_request == BMS_SSM_MODE_CHARGE) {
                    state->charge_state = 
                        (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) ? BMS_CHARGE_CC : BMS_CHARGE_CV;
                } else if (input->mode_request == BMS_SSM_MODE_BALANCE) {
                    state->charge_state = BMS_CHARGE_BAL;
                }
            }
            break;
        case BMS_CHARGE_CC:
            if (input->pack_status->pack_cell_max_mV >= state->pack_config->cell_max_mV) {
                state->charge_state = BMS_CHARGE_CV; // Need to go to CV Mode
                _set_output(true, true, cv_charge_voltage_mV, cv_charge_current_mA, output);
            } else {
                // Charge in CC Mode
                _set_output(true, true, cc_charge_voltage_mV, cc_charge_current_mA, output);
            }

            _calc_balance(output->balance_req, input->pack_status->cell_voltages_mV, input->pack_status->pack_cell_min_mV, state->pack_config);

            // if(!input->contactors_closed || !input->charger_on) { // [TODO] Think about this
            if(!input->contactors_closed) {
                _set_output(true, false, 0, 0, output);
                state->charge_state = BMS_CHARGE_INIT;
            }
            break;
        case BMS_CHARGE_CV:

            if (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) {
                // Need to go back to CC Mode
                state->charge_state = BMS_CHARGE_CC;
                _set_output(true, true, cc_charge_voltage_mV, cc_charge_current_mA, output);
            } else {
                _set_output(true, true, cv_charge_voltage_mV, cv_charge_current_mA, output);

                if (input->pack_status->pack_current_mA < state->pack_config->cv_min_current_mA*state->pack_config->pack_cells_p) {
                    if ((input->msTicks - last_time_above_cv_min_curr) >= state->pack_config->cv_min_current_ms) {
                        _set_output(false, false, 0, 0, output);
                        memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);
                        state->charge_state = BMS_CHARGE_DONE;
                        break;
                    }
                } else {
                    last_time_above_cv_min_curr = input->msTicks;
                }
            }

            _calc_balance(output->balance_req, input->pack_status->cell_voltages_mV, input->pack_status->pack_cell_min_mV, state->pack_config);

            if(!input->contactors_closed) {
                _set_output(true, false, 0, 0, output);
                state->charge_state = BMS_CHARGE_INIT;
            }
            break;

        case BMS_CHARGE_BAL:
            _set_output(false, false, 0, 0, output);
            bool balancing = _calc_balance(output->balance_req, input->pack_status->cell_voltages_mV, input->balance_mV, state->pack_config);

            // Done balancing
            if (!balancing) {
                state->charge_state = BMS_CHARGE_DONE;
            }

            if(input->contactors_closed) {
                _set_output(false, false, 0, 0, output);
                state->charge_state = BMS_CHARGE_INIT;
            }

            break;
        case BMS_CHARGE_DONE:
            _set_output(false, false, 0, 0, output);
            memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);

            // if not in Charge or Balance, that means SSM is trying to switch to another mode so wait for contactors to close
            // if in charge or balance, make sure we don't need to go back to charge or balance
            //    if we do, go back to init
            if (input->mode_request != BMS_SSM_MODE_CHARGE && input->mode_request != BMS_SSM_MODE_BALANCE) {
                if (!input->contactors_closed && !input->charger_on) {
                    state->charge_state = BMS_CHARGE_OFF;
                }
            } else {
                if(input->mode_request == BMS_SSM_MODE_CHARGE) {
                    if (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) {
                        state->charge_state = BMS_CHARGE_INIT;
                    }
                } else if (input->mode_request == BMS_SSM_MODE_BALANCE) {
                    int i;
                    for (i = 0; i < total_num_cells; i++) {
                        if (input->pack_status->cell_voltages_mV[i] > input->balance_mV + state->pack_config->bal_on_thresh_mV) {
                            state->charge_state = BMS_CHARGE_INIT;
                        }
                    }
                }
            }
    }
}


// checks that each cell is within some threshold of the minimum cell
            //  voltage. uses two different thresholds based on whether 
            //  we were just balancing or not (account for hysteresis)
bool _calc_balance(bool *balance_req, uint32_t *cell_voltages_mV, uint32_t balance_mV, PACK_CONFIG_T *config) {
    bool balancing = false;
    int i;
    for (i = 0; i < total_num_cells; i++) {
        if (balance_req[i]) {
            balance_req[i] = (cell_voltages_mV[i] > balance_mV + config->bal_off_thresh_mV);
        } else {
            balance_req[i] = (cell_voltages_mV[i] > balance_mV + config->bal_on_thresh_mV);
        }
        if (balance_req[i]) balancing = true;
    }
    return balancing;
}

void _set_output(bool close_contactors, bool charger_on, uint32_t charge_voltage_mV, uint32_t charge_current_mA, BMS_OUTPUT_T *output) {
    output->close_contactors = close_contactors;
    output->charge_req->charger_on = charger_on;
    output->charge_req->charge_voltage_mV = charge_voltage_mV;
    output->charge_req->charge_current_mA = charge_current_mA;
}


