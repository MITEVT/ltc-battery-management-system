#ifndef _SOC_H_
#define _SOC_H_
void SOC_Init(void);
void SOC_Full(void);
void SOC_Write(void);
uint32_t SOC_Estimate(BMS_INPUT_T* bms_input);
#endif
