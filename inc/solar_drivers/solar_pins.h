#ifndef _SOLAR_PINS_H_
#define _SOLAR_PINS_H_

#include <stdbool.h>

// LED1
#define LED1 2, 9
#define IOCON_LED1 IOCON_PIO2_9
#define IOCON_LED1_FUNC IOCON_FUNC0

// LED2
#define LED2 2, 10
#define IOCON_LED2 IOCON_PIO2_10
#define IOCON_LED2_FUNC IOCON_FUNC0

#define OFF_IN 3,3
#define IOCON_OFF_IN IOCON_PIO3_3
#define IOCON_OFF_IN_FUNC IOCON_FUNC0

void Solar_GPIO_Init(void);

void Solar_Fault_Pin_Set(bool state);
bool Solar_Fault_Pin_Get(void);

bool Solar_Contactor_Pin_Get(void);

void Solar_Charge_Enable_Set(bool state);
bool Solar_Charge_Enable_Get(void);

void Solar_DC_DC_Enable_Set(bool enabled);

bool Solar_DC_DC_Fault_Get(void);

void Solar_Fan_Set(bool state);

#endif //_SOLAR_PINS_H_
