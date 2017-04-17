#include "evt_pins.h"

#include "chip.h"

// CTR Pin
#define EVT_CTR_SWTCH 1, 4 // pin 40 on lpc24, used as input 
#define EVT_IOCON_CTR_SWTCH IOCON_PIO1_4
#define EVT_IOCON_CTR_SWTCH_FUNC IOCON_FUNC0

// Headroom Pin
#define EVT_HEADROOM 3, 2
#define EVT_IOCON_HEADROOM IOCON_PIO3_2
#define EVT_IOCON_HEADROOM_FUNC IOCON_FUNC1

void Evt_GPIO_Init(void) {
    // CTR_SWTCH
    Chip_IOCON_PinMuxSet(
        LPC_IOCON, EVT_IOCON_CTR_SWTCH, EVT_IOCON_CTR_SWTCH_FUNC);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, EVT_CTR_SWTCH);

    // Headroom
    Chip_IOCON_PinMuxSet(LPC_IOCON, EVT_IOCON_HEADROOM, EVT_IOCON_HEADROOM_FUNC);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, EVT_HEADROOM);

    // Headroom initialized to off
    Chip_GPIO_SetPinState(LPC_GPIO, EVT_HEADROOM, false);
}

void Evt_Headroom_Toggle(void) {
    Chip_GPIO_SetPinState(LPC_GPIO, EVT_HEADROOM, 1 - Chip_GPIO_GetPinState(LPC_GPIO, EVT_HEADROOM));
}
