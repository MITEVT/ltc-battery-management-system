#include "charge.h"

void Charge_Init(BMS_STATE_T *state) {
	state->charge_state = BMS_CHARGE_MODE_OFF;
}

void Charge_Config(PACK_CONFIG_T *pack_config, CHARGE_CONFIG_T *charge_config) {

}

void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
	switch (input->mode_request) {

		case BMS_SSM_MODE_INIT:
			break; // Invalid, Charge_Step shouldn't be called by SSM
		case BMS_SSM_MODE_STANDBY:
			state->charge_state = BMS_CHARGE_MODE_OFF;
			break;
		case BMS_SSM_MODE_CHARGE:
		case BMS_SSM_MODE_BALANCE:
			state->charge_state = (state->charge_state == BMS_CHARGE_MODE_OFF) ? BMS_CHARGE_MODE_INIT : state->charge_state;
			break;
		case BMS_SSM_MODE_DISCHARGE:
			state->charge_state = BMS_CHARGE_MODE_OFF;
			break; 
		case BMS_SSM_MODE_ERROR:
			state->charge_state = BMS_CHARGE_MODE_OFF;
			break;
	}

	int i;
	switch (state->charge_state) {

		case BMS_CHARGE_MODE_OFF:
			output->close_contactors = false; 						// Open Contactors
			output->charge_req->charge_on = false;					// Turn off charger
			// for (i = 0; i < state->pack_config->num_modules; i++) { // Turn off balancing
			// 	output->balance_req[i] = 0;	
			// }
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*state->pack_config->num_modules);
			break;
		case BMS_CHARGE_MODE_INIT:
			output->close_contactors = true;
			output->charge_req->charge_on = false;
			memset(output->balance_req, 0, sizeof(output->balance_req[0])*state->pack_config->num_modules);
			break;
		case BMS_CHARGE_MODE_CC:
			break;
		case BMS_CHARGE_MODE_CV:
			break;
		case BMS_CHARGE_MODE_BALANCE:
			break;
		case BMS_CHARGE_MODE_DONE:
			break;

	}
}

void Charge_Reset(BMS_STATE_T *state){

}

