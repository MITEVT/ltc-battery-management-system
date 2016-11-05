
#include "chip.h"
#include "util.h"
#include "ltc6804.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler

#define CS 0, 7

#define LED0 2, 10
static CELL_INFO_T readings;
volatile uint32_t msTicks;

static char str[100];

void PrintCellGroupsHelper(uint16_t num);
void PrintCellGroups(void);

void SysTick_Handler(void) {
	msTicks++;
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);

}

void PrintCellGroupsHelper(uint16_t num) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    if(num == 0) {
        Chip_UART_SendBlocking(LPC_USART, str, 1);
    } else {
        Chip_UART_SendBlocking(LPC_USART, str, 4);
    }
    Chip_UART_SendBlocking(LPC_USART, " ", 2);
}

void PrintCellGroups(void) {
    uint8_t i;

    Chip_UART_SendBlocking(LPC_USART, "Group A: ", 9);
    for(i = 0; i < 3; i++) {
        itoa(readings.groupA[i], str, 16);
        PrintCellGroupsHelper(readings.groupA[i]);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);

    Chip_UART_SendBlocking(LPC_USART, "Group B: ", 9);
    for(i = 0; i < 3; i++) {
        itoa(readings.groupB[i], str, 16);
        PrintCellGroupsHelper(readings.groupB[i]);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);

    Chip_UART_SendBlocking(LPC_USART, "Group C: ", 9);
    for(i = 0; i < 3; i++) {
        itoa(readings.groupC[i], str, 16);
        PrintCellGroupsHelper(readings.groupC[i]);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);

    Chip_UART_SendBlocking(LPC_USART, "Group D: ", 9);
    for(i = 0; i < 3; i++) {
        itoa(readings.groupD[i], str, 16);
        PrintCellGroupsHelper(readings.groupD[i]);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);
}

/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
int main(void)
{
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	/* LED Initialization */
	GPIO_Config();
	LED_Config();

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

    uint8_t Rx_Buf[12];
    CELL_INFO_T readings;

    LTC6804_Init(600000, 0, 7, msTicks);
    LTC6804_StartADC(msTicks);
    delay(5);
    LTC6804_ReadVoltageGroup(Rx_Buf, &readings, CELL_GROUP_A, msTicks);
    delay(5);
    PrintCellGroups();
    
/*
    LTC6804_ReadCFG(Rx_Buf, msTicks);

    int i;
    for (i = 4; i < 12; i++) {
        Chip_UART_SendBlocking(LPC_USART, "0x", 2);
        itoa(Rx_Buf[i], str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 2);
        Chip_UART_SendBlocking(LPC_USART,", ",2);
    }
*/

	while(1) {
		delay(5);
	}

	return 0;
}

