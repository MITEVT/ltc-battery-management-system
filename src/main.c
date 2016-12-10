#include "chip.h"
#include "util.h"
#include "config.h"
#include "lc1024.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler

#define LED0 2, 10
volatile uint32_t msTicks;

static char str[100];

static uint8_t Rx_Buf[SPI_BUFFER_SIZE];

static void PrintRxBuffer(void);
static void ZeroRxBuf(void);

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

static void PrintRxBuffer(void) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < SPI_BUFFER_SIZE; i++) {
        itoa(Rx_Buf[i], str, 16);
        if(Rx_Buf[i] < 16) {
            Chip_UART_SendBlocking(LPC_USART, "0", 1);
        }
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);
}

static void uart_init(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 57600);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
}

static void ZeroRxBuf(void) {
	uint8_t i;
	for (i = 0; i < SPI_BUFFER_SIZE; i++) {
		Rx_Buf[i] = 0;
	}
}

int main(void) {
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	GPIO_Config();
	LED_Config();

    uart_init();
    LC1024_Init(600000, 0, 7);
    
    ZeroRxBuf();

    LC1024_WriteDisable();

    LC1024_ReadStatusReg(&Rx_Buf[0]);
    Chip_UART_SendBlocking(LPC_USART, "ST reg before wenb:\n", 20);
    PrintRxBuffer();

    LC1024_WriteEnable();

    LC1024_ReadStatusReg(&Rx_Buf[0]);
    Chip_UART_SendBlocking(LPC_USART, "ST reg after wenb:\n", 19);
    PrintRxBuffer();

    LC1024_ReadMem(&Rx_Buf[0], 2);
    Chip_UART_SendBlocking(LPC_USART, "ze mem before wmem:\n", 20);
    PrintRxBuffer();

    LC1024_WriteMem();
    Chip_UART_SendBlocking(LPC_USART, "buf after write:\n", 14);
    PrintRxBuffer();

    LC1024_ReadMem(&Rx_Buf[0], 2);
    Chip_UART_SendBlocking(LPC_USART, "mem read after write:\n", 22);
    PrintRxBuffer();

	while(1) {
		delay(5);
	}

	return 0;
}

