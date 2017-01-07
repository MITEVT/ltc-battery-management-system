#ifndef ERROR_H
#define ERROR_H
#include "state_types.h"

void Error_Init(BMS_STATE_T *state);
void Error_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
void Error_Reset(BMS_STATE_T *state);

#endif
