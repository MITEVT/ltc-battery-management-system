#include "charge.h"
#include "bms_utils.h"

static uint16_t total_num_cells;
static uint32_t cc_charge_voltage_mV;
static uint32_t cc_charge_current_mA;
static uint32_t cv_charge_voltage_mV;
static uint32_t cv_charge_current_mA;
static uint32_t last_time_above_cv_min_curr;

void Charge_Init(BMS_STATE_T *state) {
	state->charge_state = BMS_CHARGE_OFF;
}

void Charge_Config(PACK_CONFIG_T *pack_config) {
	total_num_cells = Get_Total_Cell_Count(pack_config);

	cc_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
	cc_charge_current_mA = pack_config->cell_capacity_cAh * pack_config->cell_charge_c_rating_cC * pack_config->pack_cells_p / 10;

	cv_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
	cv_charge_current_mA = cc_charge_current_mA;
	last_time_above_cv_min_curr = 0;
}

uint8_t Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
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

handler:

	switch (state->charge_state) {

		case BMS_CHARGE_OFF:
			output->close_contactors = false; 						// Open Contactors
			output->charge_req->charger_on = false;					// Turn off charger
			// for (i = 0; i < state->pack_config->num_modules; i++) { // Turn off balancing
			// 	output->balance_req[i] = 0;	
			// }
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
				goto handler;
			}
			break;
		case BMS_CHARGE_CC:
            // WUT WHERE IS TEMPERATURE MONITORING
            
			if (input->pack_status->pack_cell_max_mV >= state->pack_config->cell_max_mV) {
				// Need to go to CV Mode
				state->charge_state = BMS_CHARGE_CV;
				goto handler;
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

			// [TODO] add errors such as contactors opening
			break;
		case BMS_CHARGE_CV:
			if (input->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) {
				// Need to go back to CC Mode
				state->charge_state = BMS_CHARGE_CC;
				goto handler;
			} else {
				output->charge_req->charge_voltage_mV = cv_charge_voltage_mV;
				output->charge_req->charge_current_mA = cv_charge_voltage_mV;
				output->charge_req->charger_on = true;
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

			break;
		case BMS_CHARGE_BAL:
            // WUT WHAT DOES THIS DO (ANSWER: NOT FINISHED YET !!)
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

			if (!balancing) {
				state->charge_state = BMS_CHARGE_DONE;
				goto handler;
			}

			break;
		case BMS_CHARGE_DONE:
			output->close_contactors = false;
			output->charge_req->charger_on = false;
			// output->charge_req->charge_current_mA = 0;
			// output->charge_req->charge_voltage_mV = 0;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*total_num_cells);

            // WUT
            //      this outer if-check is incorrect i believe (should be removed)
            //      it's possible to get stuck in BMS_CHARGE_DONE if you are
            //      requesting BMS mode charge, because the CHARGE_DONE doesn't 
            //      get switched to init unless currently in BMS_CHARGE_OFF

            // this is for looping in charge done when we have 
            // finished charging to max battery capacity...
            // but probably a little better
			if (input->mode_request != BMS_SSM_MODE_CHARGE && input->mode_request != BMS_SSM_MODE_BALANCE) {
				if (!input->contactors_closed) {
					state->charge_state = BMS_CHARGE_OFF;
				}
			}
			break;
	}
    return 0;
}
