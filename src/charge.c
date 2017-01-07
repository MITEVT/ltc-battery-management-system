#include "charge.h"

void Charge_Init(BMS_STATE_T *state) {
	state->charge_state = BMS_CHARGE_MODE_OFF;
}

void Charge_Config(CHARGE_CONFIG_T *charge_config) {
}

void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
}

void Charge_Reset(BMS_STATE_T *state){
}

