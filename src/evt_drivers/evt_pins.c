#include "evt_pins.h"

#include "chip.h"
void Evt_GPIO_Init(void) {
    // LEDs
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED0);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED2);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_LED2, IOCON_FUNC1);	
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED2, true);

    // CTR_SWTCH
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, EVT_CTR_SWTCH);

    // Headroom
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_HEADROOM, IOCON_FUNC1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, HEADROOM);
}
