#ifndef SSM_H
#define SSM_H

#include "state_types.h"

void SSM_Init(BMS_STATE_T *state);
void SSM_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);

#endif
