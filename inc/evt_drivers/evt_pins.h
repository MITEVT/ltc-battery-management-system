#ifndef _EVT_PINS_H_
#define _EVT_PINS_H_

// LEDs
#define LED0 2, 8
#define LED1 2, 10
#define LED2 1, 3
#define IOCON_LED2 IOCON_PIO1_3

// CTR Pin
#define EVT_CTR_SWTCH 1, 4 // pin 40 on lpc24, used as input 
#define EVT_IOCON_CTR_SWTCH IOCON_PIO1_4
#define EVT_IOCON_CTR_SWTCH_FUNC IOCON_FUNC0

// Headroom Pin
#define HEADROOM 3, 2
#define IOCON_HEADROOM IOCON_PIO3_2

void Evt_GPIO_Init(void);
#endif // _EVT_PINS_H_
