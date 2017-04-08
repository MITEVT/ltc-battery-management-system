#ifndef DISCHARGE_H
#define DISCHARGE_H

#include "state_types.h"
#include "bms_utils.h"


uint32_t Calculate_Max_Current(uint32_t cell_capacity_cAh, uint32_t discharge_rating_cC, uint32_t pack_cells_p, uint16_t cell_temp_dC);

void Discharge_Init(BMS_STATE_T *state);
void Discharge_Config(PACK_CONFIG_T *pack_config);
void Discharge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output);
uint32_t Read_Max_Current(void);

#endif
