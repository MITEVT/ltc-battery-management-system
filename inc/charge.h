#ifndef _CHARGE_H
#define _CHARGE_H
#include "state_types.h"


typedef struct {
	uint32_t balanceOnCellThreshold_mV;
	uint32_t balanceOffCellThreshold_mV;
	uint32_t chargeCRating_cC;
	uint32_t ccCellThreshold_mV;
} CHARGE_CONFIG_T;

void Charge_Init(BMS_STATE_T *state);
void Charge_Config(CHARGE_CONFIG_T *charge_config);
void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
void Charge_Reset(BMS_STATE_T *state);

#endif
