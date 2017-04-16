#include "fsae_pins.h"

#include "chip.h"

// Low Side Contactor
#define FSAE_CTR_LOW_SWTCH 2, 8
#define FSAE_IOCON_CTR_LOW_SWTCH IOCON_PIO2_8

// High Side Contactor
#define FSAE_CTR_HIGH_SWTCH 2, 7
#define FSAE_IOCON_CTR_HIGH_SWTCH IOCON_PIO2_7

// Fault Pin
#define FSAE_FAULT_GPIO 3, 0
#define FSAE_IOCON_FAULT_GPIO IOCON_PIO3_0

// Charge Enable Pin
#define FSAE_CHARGE_ENABLE_GPIO 3, 2
#define FSAE_IOCON_CHARGE_ENABLE_GPIO IOCON_PIO3_2

// Fan 1 Pin
#define FSAE_FAN_1_PIN 1, 4
#define FSAE_IOCON_FAN_1_PIN IOCON_PIO1_4

// Fan 2 Pin
#define FSAE_FAN_2_PIN 1, 3
#define FSAE_IOCON_FAN_2_PIN IOCON_PIO1_3

#define FAN_TIMER_PRESCALE 18
#define MATCH_REGISTER_FAN_1 3
#define MATCH_REGISTER_FAN_2 2
#define ENABLE_PWM_FAN_1 0b1000
#define ENABLE_PWM_FAN_2 0b0100
#define MATCH_REGISTER_FAN_PWM_CYCLE 0
#define FAN_OFF_DUTY_RATIO -100
#define FAN_ON_DUTY_RATIO 10
#define FAN_PWM_CYCLE 100

void Fsae_GPIO_Init(void) {
    // LEDs
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED2);

    // Fault Pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAULT_GPIO);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAULT_GPIO,
            (IOCON_FUNC0 | IOCON_MODE_INACT));

    // Charge Enable Pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_CHARGE_ENABLE_GPIO);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_CHARGE_ENABLE_GPIO,
            (IOCON_FUNC0 | IOCON_MODE_INACT));

    // Fan 1 pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAN_1_PIN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAN_1_PIN,
            (IOCON_FUNC2 | IOCON_MODE_INACT));
    Chip_GPIO_SetPinState(LPC_GPIO, FSAE_FAN_1_PIN, false);

    // Fan 2 pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAN_2_PIN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAN_2_PIN, 
            (IOCON_FUNC3 | IOCON_MODE_INACT));
    Chip_GPIO_SetPinState(LPC_GPIO, FSAE_FAN_2_PIN, false); 

    // Fan PWM
    Chip_TIMER_Init(LPC_TIMER32_1);
    Chip_TIMER_Reset(LPC_TIMER32_1);
    Chip_TIMER_PrescaleSet(LPC_TIMER32_1, FAN_TIMER_PRESCALE);  /* Set the prescaler */
    Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_1, 
            FAN_PWM_CYCLE - FAN_OFF_DUTY_RATIO);
    Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_2, 
            FAN_PWM_CYCLE - FAN_OFF_DUTY_RATIO);
    Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_PWM_CYCLE, FAN_PWM_CYCLE); 
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, MATCH_REGISTER_FAN_PWM_CYCLE);
    const uint8_t pwmControlRegister_TIMER32_1 = (ENABLE_PWM_FAN_1 | ENABLE_PWM_FAN_2);
    LPC_TIMER32_1->PWMC |= pwmControlRegister_TIMER32_1;
    Chip_TIMER_ExtMatchControlSet(LPC_TIMER32_1, 0, TIMER_EXTMATCH_TOGGLE, 
            MATCH_REGISTER_FAN_1);  
    Chip_TIMER_ExtMatchControlSet(LPC_TIMER32_1, 0, TIMER_EXTMATCH_TOGGLE, 
            MATCH_REGISTER_FAN_2);  
    Chip_TIMER_Enable(LPC_TIMER32_1);
}

void Fsae_Fault_Pin_Set(bool state) {
    Chip_GPIO_SetPinState(LPC_GPIO, FSAE_FAULT_GPIO, state);
}

bool Fsae_Fault_Pin_Get(void) {
    return Chip_GPIO_GetPinState(LPC_GPIO, FSAE_FAULT_GPIO);
}

void Fsae_Charge_Enable_Set(bool state) {
    Chip_GPIO_SetPinState(LPC_GPIO, FSAE_CHARGE_ENABLE_GPIO, state);
}

bool Fsae_Charge_Enable_Get(void) {
    return Chip_GPIO_GetPinState(LPC_GPIO, FSAE_CHARGE_ENABLE_GPIO);
}

void Fsae_Fan_Set(bool state) {
    if (state) {
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_1, FAN_ON_DUTY_RATIO);
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_2, FAN_ON_DUTY_RATIO);
    } else {
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_1, FAN_OFF_DUTY_RATIO);
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_2, FAN_OFF_DUTY_RATIO);
    }
}
