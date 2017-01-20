#ifndef _CHARGE_H
#define _CHARGE_H
#include "state_types.h"
#include <string.h>

void Charge_Init(BMS_STATE_T *state);
void Charge_Config(PACK_CONFIG_T *pack_config);
BMS_ERROR_T Charge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);

#endif
