#ifndef _SOLAR_CAN_H
#define _SOLAR_CAN_H

#include <stdint.h>

#include "state_types.h"

#define VCU_HEARTBEAT_TIMEOUT   10000

typedef struct {
  BMS_SSM_MODE state;
  uint16_t soc;
  bool fan_enable;
  bool dcdc_enable;
  bool dcdc_fault;
} Can_Bms_Heartbeat_T;

typedef struct {
  int16_t avg_cell_temp;
  int16_t min_cell_temp;
  int16_t max_cell_temp;
  uint16_t id_min_cell_temp;
  uint16_t id_max_cell_temp;
} Can_Bms_CellTemps_T;

typedef struct {
  uint16_t pack_voltage;
  int16_t pack_current;
  uint16_t avg_cell_voltage;
  uint16_t min_cell_voltage;
  uint8_t id_min_cell_voltage;
  uint16_t max_cell_voltage;
  uint8_t id_max_cell_voltage;
} Can_Bms_PackStatus_T;

typedef struct {
  Can_Bms_ErrorID_T type;
} Can_Bms_Error_T;

void Solar_Can_Init(uint32_t baud_rate);
void Solar_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output);
void Solar_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output);

#endif // _SOLAR_CAN_H
