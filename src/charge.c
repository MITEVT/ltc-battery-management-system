#include "charge.h"
#include "bms_utils.h"
#include "board.h"

static uint16_t total_num_cells;
static uint32_t cc_charge_voltage_mV;
static uint32_t cc_charge_current_mA;
static uint32_t cv_charge_voltage_mV;
static uint32_t cv_charge_current_mA;
static uint32_t last_time_above_cv_min_curr;

void Charge_Init(BMS_STATE_T *state) {
	state->charge_state = BMS_CHARGE_OFF;
	last_time_above_cv_min_curr = 0;
}

void Charge_Config(PACK_CONFIG_T *pack_config) {
	total_num_cells = Get_Total_Cell_Count(pack_config);

	cc_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
	cc_charge_current_mA = pack_config->cell_capacity_cAh * pack_config->cell_charge_c_rating_cC * pack_config->pack_cells_p / 10;

	cv_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
	cv_charge_current_mA = cc_charge_current_mA;
}

BMS_ERROR_T Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
	switch (input->mode_request) {
		case BMS_SSM_MODE_INIT:
            // Invalid, shouldn't be called from init
			return BMS_INVALID_SSM_STATE_ERROR;
            
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
			output->close_contactors = false;
			output->charge_req->charger_on = false;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);
			break;

		case BMS_CHARGE_INIT:
			output->close_contactors = (input->mode_request == BMS_SSM_MODE_CHARGE);
			output->charge_req->charger_on = false;
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
				// Need to go to CV Mode
				state->charge_state = BMS_CHARGE_CV;
			} else {
				// Charge in CC Mode
				output->charge_req->charge_voltage_mV = cc_charge_voltage_mV;
				output->charge_req->charge_current_mA = cc_charge_current_mA;
				output->charge_req->charger_on = true;
			}
			
			int i;
            // checks that each cell is within some threshold of the minimum cell
            //  voltage. uses two different thresholds based on whether 
            //  we were just balancing or not (account for hysteresis)
			for (i = 0; i < total_num_cells; i++) {
				if (output->balance_req[i]) {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->pack_status->pack_cell_min_mV + state->pack_config->bal_off_thresh_mV);
				} else {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->pack_status->pack_cell_min_mV + state->pack_config->bal_on_thresh_mV);
				}
			}

            if(!input->contactors_closed) {
            	// [TODO] Consider setting outputs to zero
                return BMS_CONTACTORS_ERRONEOUS_STATE;
            }

			break;
		case BMS_CHARGE_CV:

			if (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) {
				// Need to go back to CC Mode
				state->charge_state = BMS_CHARGE_CC;
			} else {
				output->charge_req->charge_voltage_mV = cv_charge_voltage_mV;
				output->charge_req->charge_current_mA = cv_charge_current_mA;
				output->charge_req->charger_on = true;
				// [TODO] change to cv_min_current times num p
				if (input->pack_status->pack_current_mA < state->pack_config->cv_min_current_mA) {
					if ((input->msTicks - last_time_above_cv_min_curr) >= state->pack_config->cv_min_current_ms) {
						output->charge_req->charger_on = false;
						state->charge_state = BMS_CHARGE_DONE;
						memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);
						break;
					}
				} else {
					last_time_above_cv_min_curr = input->msTicks;
				}
			}

			for (i = 0; i < total_num_cells; i++) {
				if (output->balance_req[i]) {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->pack_status->pack_cell_min_mV + state->pack_config->bal_off_thresh_mV);
				} else {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->pack_status->pack_cell_min_mV + state->pack_config->bal_on_thresh_mV);
				}
			}

            if(!input->contactors_closed) {
            	// [TODO] Consider setting outputs to zero
                return BMS_CONTACTORS_ERRONEOUS_STATE;
            }

			break;

		case BMS_CHARGE_BAL:
			output->close_contactors = false;
			output->charge_req->charger_on = false;

			bool balancing = false;
			for (i = 0; i < total_num_cells; i++) {
				if (output->balance_req[i]) {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->balance_mV + state->pack_config->bal_off_thresh_mV);
				} else {
					output->balance_req[i] = (input->pack_status->cell_voltage_mV[i] > input->balance_mV + state->pack_config->bal_on_thresh_mV);
				}
				if (output->balance_req[i]) balancing = true;
			}

			// Done balancing
			if (!balancing) {
				state->charge_state = BMS_CHARGE_DONE;
			}

			// [TODO] add errors such as contactors opening
			break;
		case BMS_CHARGE_DONE:
			output->close_contactors = false;
			output->charge_req->charger_on = false;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);

            // if not in Charge or Balance, that means SSM is trying to switch to another mode so wait for contactors to close
            // if in charge or balance, make sure we don't need to go back to charge or balance
            //		if we do, go back to init
			if (input->mode_request != BMS_SSM_MODE_CHARGE && input->mode_request != BMS_SSM_MODE_BALANCE) {
				if (!input->contactors_closed) {
					state->charge_state = BMS_CHARGE_OFF;
				}
			} else {
				if(input->mode_request == BMS_SSM_MODE_CHARGE) {
					if (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) {
						state->charge_state = BMS_CHARGE_INIT;
					}
				} else if (input->mode_request == BMS_SSM_MODE_BALANCE) {
					for (i = 0; i < total_num_cells; i++) {
						if (input->pack_status->cell_voltage_mV[i] > input->balance_mV + state->pack_config->bal_on_thresh_mV) {
							state->charge_state = BMS_CHARGE_INIT;
						}
					}
				}
			}
	}
    return BMS_NO_ERROR;
}
