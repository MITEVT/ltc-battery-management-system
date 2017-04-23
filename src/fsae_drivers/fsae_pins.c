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

// Fan 1 is timer 3
#define MATCH_REGISTER_FAN_1 3
#define ENABLE_PWM_FAN_1 (1 << MATCH_REGISTER_FAN_1)

// Fan 2 Pin
#define FSAE_FAN_2_PIN 1, 3
#define FSAE_IOCON_FAN_2_PIN IOCON_PIO1_3

// Fan 2 is timer 2
#define MATCH_REGISTER_FAN_2 2
#define ENABLE_PWM_FAN_2 (1 << MATCH_REGISTER_FAN_2)

// Mystery magic numbers, talk to Jorge if curious
#define FAN_TIMER_PRESCALE 18
#define MATCH_REGISTER_FAN_PWM_CYCLE 0

// Cycle time for PWM
#define FAN_PWM_CYCLE 100

// Fan off means it runs at (Cycle + 1)ms, which never happens
#define FAN_OFF_DUTY_RATIO -1
#define FAN_TIMER_OFF (FAN_PWM_CYCLE - FAN_OFF_DUTY_RATIO)

// Fan on means it runs at (Cycle - 10)ms, which happens here 10% of the time
#define FAN_ON_DUTY_RATIO 10
#define FAN_TIMER_ON (FAN_PWM_CYCLE - FAN_ON_DUTY_RATIO)

// Unused pins
#define PIN_37 3, 1
#define PIN_37_IOCON IOCON_PIO3_1
#define PIN_37_PIO_FUNC IOCON_FUNC0
#define PIN_35 1, 2
#define PIN_35_IOCON IOCON_PIO1_2
#define PIN_35_PIO_FUNC IOCON_FUNC1
#define PIN_34 1, 1
#define PIN_34_IOCON IOCON_PIO1_1
#define PIN_34_PIO_FUNC IOCON_FUNC1
#define PIN_33 1, 0
#define PIN_33_IOCON IOCON_PIO1_0
#define PIN_33_PIO_FUNC IOCON_FUNC1
#define PIN_32 0, 11
#define PIN_32_IOCON IOCON_PIO0_11
#define PIN_32_PIO_FUNC IOCON_FUNC1
#define PIN_31 2, 11
#define PIN_31_IOCON IOCON_PIO2_11
#define PIN_31_PIO_FUNC IOCON_FUNC0
#define PIN_30 1, 10
#define PIN_30_IOCON IOCON_PIO1_10
#define PIN_30_PIO_FUNC IOCON_FUNC0
#define PIN_29 0, 10
#define PIN_29_IOCON IOCON_PIO0_10
#define PIN_29_PIO_FUNC IOCON_FUNC1

void Fsae_GPIO_Init(void) {

    // Fault Pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAULT_GPIO);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAULT_GPIO,
            (IOCON_FUNC0 | IOCON_MODE_INACT));

    // Fault starts in off state (so shutdown loop is open)
    Fsae_Fault_Pin_Set(false);

    // Charge Enable Pin
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_CHARGE_ENABLE_GPIO);
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_CHARGE_ENABLE_GPIO,
            (IOCON_FUNC0 | IOCON_MODE_INACT));

    // Charge Enable starts in off state
    Fsae_Charge_Enable_Set(false);

    // Fan pin config
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAN_1_PIN,
            (IOCON_FUNC2 | IOCON_MODE_INACT));
    Chip_IOCON_PinMuxSet(LPC_IOCON, FSAE_IOCON_FAN_2_PIN,
            (IOCON_FUNC3 | IOCON_MODE_INACT));

    // Fan Timer
    Chip_TIMER_Init(LPC_TIMER32_1);
    Chip_TIMER_Reset(LPC_TIMER32_1);
    Chip_TIMER_PrescaleSet(LPC_TIMER32_1, FAN_TIMER_PRESCALE);
    Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_PWM_CYCLE, FAN_PWM_CYCLE); 
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, MATCH_REGISTER_FAN_PWM_CYCLE);

    // Fans start in off state
    Fsae_Fan_Set(false);

    const uint8_t pwmControlRegister_TIMER32_1 = (ENABLE_PWM_FAN_1 | ENABLE_PWM_FAN_2);
    LPC_TIMER32_1->PWMC |= pwmControlRegister_TIMER32_1;
    Chip_TIMER_ExtMatchControlSet(LPC_TIMER32_1, 0, TIMER_EXTMATCH_TOGGLE, 
            MATCH_REGISTER_FAN_1);  
    Chip_TIMER_ExtMatchControlSet(LPC_TIMER32_1, 0, TIMER_EXTMATCH_TOGGLE, 
            MATCH_REGISTER_FAN_2);  
    Chip_TIMER_Enable(LPC_TIMER32_1);

    // Enable pull down resistors on unused pins
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_37);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_37_IOCON, 
        (PIN_37_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_37, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_35);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_35_IOCON, 
        (PIN_35_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_35, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_34);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_34_IOCON, 
        (PIN_34_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_34, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_33);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_33_IOCON, 
        (PIN_33_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_33, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_32);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_32_IOCON, 
        (PIN_32_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_32, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_31);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_31_IOCON, 
        (PIN_31_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_31, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_30);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_30_IOCON, 
        (PIN_30_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_30, false);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, PIN_29);
    Chip_IOCON_PinMuxSet(LPC_IOCON, PIN_29_IOCON, 
        (PIN_29_PIO_FUNC | IOCON_MODE_PULLDOWN) );
    Chip_GPIO_SetPinState(LPC_GPIO, PIN_29, false);

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
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_1, FAN_TIMER_ON);
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_2, FAN_TIMER_ON);
    } else {
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_1, FAN_TIMER_OFF);
        Chip_TIMER_SetMatch(LPC_TIMER32_1, MATCH_REGISTER_FAN_2, FAN_TIMER_OFF);
    }
}
