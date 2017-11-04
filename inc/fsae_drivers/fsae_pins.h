#ifndef _FSAE_PINS_H_
#define _FSAE_PINS_H_

#include <stdbool.h>

// LED1
#define LED1 2, 9
#define IOCON_LED1 IOCON_PIO2_9
#define IOCON_LED1_FUNC IOCON_FUNC0

// LED2
#define LED2 2, 10
#define IOCON_LED2 IOCON_PIO2_10
#define IOCON_LED2_FUNC IOCON_FUNC0

#define FAULT 1, 5
#define IOCON_FAULT IOCON_PIO1_5
#define IOCON_FAULT_FUNC IOCON_FUNC0


#define OFF_IN 3,3
#define IOCON_OFF_IN IOCON_PIO3_3
#define IOCON_OFF_IN_FUNC IOCON_FUNC0

void Fsae_GPIO_Init(void);

void Fsae_Fault_Pin_Set(bool state);
bool Fsae_Fault_Pin_Get(void);

bool Fsae_Contactor_Pin_Get(void);

void Fsae_Charge_Enable_Set(bool state);
bool Fsae_Charge_Enable_Get(void);

void Fsae_DC_DC_Enable_Set(bool enabled);

bool Fsae_DC_DC_Fault_Get(void);

void Fsae_Fan_Set(bool state);

#endif //_FSAE_PINS_H_
