#ifndef _SOLAR_CAN_H
#define _SOLAR_CAN_H

#include <stdint.h>

#include "state_types.h"

#define VCU_HEARTBEAT_TIMEOUT   10000

void Solar_Can_Init(uint32_t baud_rate);
void Solar_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output);
void Solar_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output);

#endif // _SOLAR_CAN_H
