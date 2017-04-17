#ifndef _FSAE_CAN_H
#define _FSAE_CAN_H

#include <stdint.h>

#include "state_types.h"

#define BMS_HEARTBEAT_PERIOD    1000
#define BMS_ERRORS_PERIOD       10000
#define BMS_CELL_TEMPS_PERIOD   10000
#define BMS_PACK_STATUS_PERIOD  100
#define VCU_HEARTBEAT_TIMEOUT   10000

void Fsae_Can_Init(uint32_t baud_rate);
void Fsae_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output);
void Fsae_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output);
#endif // _FSAE_CAN_H
