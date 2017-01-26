#ifndef SSM_H
#define SSM_H

#include "state_types.h"
#include "charge.h"
#include "discharge.h"
#include "error.h"
#include "config.h"
#include "bms_utils.h"

void SSM_Init(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
BMS_ERROR_T Init_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
bool Is_Valid_Jump(BMS_SSM_MODE_T mode1, BMS_SSM_MODE_T mode2);
bool Is_State_Done(BMS_STATE_T *state);
void SSM_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
BMS_ERROR_T Error_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);


#endif
