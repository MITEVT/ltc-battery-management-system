#include "charge.h"

static uint16_t total_num_cells;
static uint32_t cc_charge_voltage_mV;
static uint32_t cc_charge_current_mA;

void Charge_Init(BMS_STATE_T *state) {
	state->charge_state = BMS_CHARGE_OFF;
}

void Charge_Config(PACK_CONFIG_T *pack_config) {
	total_num_cells = 0;
	int i;
	for (i = 0; i < pack_config->num_modules; i++) {
		total_num_cells += pack_config->num_cells_in_modules[i];
	}

	cc_charge_voltage_mV = pack_config->cell_max_mV * total_num_cells;
	cc_charge_current_mA = pack_config->cell_capacity_cAh * pack_config->cell_charge_c_rating_cC * pack_config->pack_cells_p / 10;
}

void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
	switch (input->mode_request) {

		case BMS_SSM_MODE_INIT:
			break; // Invalid, Charge_Step shouldn't be called by SSM
		case BMS_SSM_MODE_STANDBY:
			state->charge_state = (state->charge_state == BMS_CHARGE_OFF) ? BMS_CHARGE_OFF : BMS_CHARGE_DONE;
			break;
		case BMS_SSM_MODE_CHARGE:
			if (state->charge_state == BMS_CHARGE_OFF || state->charge_state == BMS_CHARGE_BAL) {
				state->charge_state = BMS_CHARGE_INIT;
			}
			break;
		case BMS_SSM_MODE_BALANCE:
			if (state->charge_state == BMS_CHARGE_OFF || state->charge_state == BMS_CHARGE_CC || state->charge_state == BMS_CHARGE_CV) {
				state->charge_state = BMS_CHARGE_INIT;
			}
			break;
		case BMS_SSM_MODE_DISCHARGE:
			state->charge_state = (state->charge_state == BMS_CHARGE_OFF) ? BMS_CHARGE_OFF : BMS_CHARGE_DONE;
			break; 
		case BMS_SSM_MODE_ERROR:
			state->charge_state = (state->charge_state == BMS_CHARGE_OFF) ? BMS_CHARGE_OFF : BMS_CHARGE_DONE;
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
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*state->pack_config->num_modules);
			break;
		case BMS_CHARGE_INIT:
			output->close_contactors = (input->mode_request == BMS_SSM_MODE_CHARGE);
			output->charge_req->charger_on = false;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*state->pack_config->num_modules);

			if (input->contactors_closed == output->close_contactors) {
				if(input->mode_request == BMS_SSM_MODE_CHARGE) {
					state->charge_state = 
						(state->pack_status->pack_cell_max_mV < state->pack_config->cell_max_mV) ? BMS_CHARGE_CC : BMS_CHARGE_CV;
				} else if (input->mode_request == BMS_SSM_MODE_BALANCE) {
					state->charge_state = BMS_CHARGE_BAL;
				}
				goto handler;
			}
			break;
		case BMS_CHARGE_CC:
			if (state->pack_status->pack_cell_max_mV >= state->pack_config->cell_max_mV) {
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
			for (i = 0; i < total_num_cells; i++) {
				if (output->balance_req[i]) {
					output->balance_req[i] = (state->pack_status->cell_voltage_mV[i] > state->pack_status->pack_cell_min_mV + state->pack_config->bal_off_thresh_mV);
				} else {
					output->balance_req[i] = (state->pack_status->cell_voltage_mV[i] > state->pack_status->pack_cell_min_mV + state->pack_config->bal_on_thresh_mV);
				}
			}
			break;
		case BMS_CHARGE_CV:
			break;
		case BMS_CHARGE_BAL:
			break;
		case BMS_CHARGE_DONE:
			output->close_contactors = false;
			output->charge_req->charger_on = false;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*state->pack_config->num_modules);
			if (input->mode_request != BMS_SSM_MODE_CHARGE && input->mode_request != BMS_SSM_MODE_BALANCE) {
				if (!input->contactors_closed) {
					state->charge_state = BMS_CHARGE_OFF;
				}
			}
			break;

	}
}

void Charge_Reset(BMS_STATE_T *state){

}

