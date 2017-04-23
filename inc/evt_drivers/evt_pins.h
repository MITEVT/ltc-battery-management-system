#ifndef _EVT_PINS_H_
#define _EVT_PINS_H_

// LEDs
#define LED1 2, 10
#define IOCON_LED1 IOCON_PIO2_10
#define IOCON_LED1_FUNC IOCON_FUNC0

#define LED2 1, 3
#define IOCON_LED2 IOCON_PIO1_3
#define IOCON_LED2_FUNC IOCON_FUNC1

void Evt_GPIO_Init(void);
void Evt_Headroom_Toggle(void);
#endif // _EVT_PINS_H_
