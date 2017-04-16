#ifndef _FSAE_PINS_H_
#define _FSAE_PINS_H_

#include <stdbool.h>

// LED1
#define LED1 2, 9

// LED2
#define LED2 2, 10

void Fsae_GPIO_Init(void);

void Fsae_Fault_Pin_Set(bool state);
bool Fsae_Fault_Pin_Get(void);

void Fsae_Charge_Enable_Set(bool state);
bool Fsae_Charge_Enable_Get(void);

void Fsae_Fan_Set(bool state);

#endif //_FSAE_PINS_H_
