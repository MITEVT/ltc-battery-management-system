#ifndef _SOC_H_
#define _SOC_H_
void SOC_Init(uint32_t ms_ticks);
uint32_t SOC_Estimate(BMS_INPUT_T* bms_input, uint32_t ms_ticks);
#endif
