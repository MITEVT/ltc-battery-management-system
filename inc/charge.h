#ifndef CHARGE_H
#define CHARGE_H
#include "state_types.h"

void Charge_Init(BMS_STATE_T *state);
void Charge_Config(BMS_STATE_T *state);
void Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
void Charge_Reset(BMS_STATE_T *state);

#endif
