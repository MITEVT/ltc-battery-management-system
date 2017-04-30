#ifndef _EVT_CAN_H
#define _EVT_CAN_H

#include <stdint.h>

#include "state_types.h"

void Evt_Can_Init(uint32_t baudRateHz, volatile uint32_t* msTicksPtrArg);
void Evt_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output);
void Evt_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output);

#endif
