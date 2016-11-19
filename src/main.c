
#include "chip.h"
#include "util.h"
#include "config.h"
#include "ltc6804.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler

#define CS 0, 7

#define LED0 2, 10
volatile uint32_t msTicks;

static char str[100];

static uint8_t Tx_Buf[SPI_BUFFER_SIZE];
static uint8_t Rx_Buf[SPI_BUFFER_SIZE];
static Chip_SSP_DATA_SETUP_T xf_setup;

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

static void PrintRxBuffer() {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < SPI_BUFFER_SIZE; i++) {
        itoa(Rx_Buf[i], str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);
}

static void ZeroRxBuf() {
	uint8_t i;
	for (i = 0; i < SPI_BUFFER_SIZE; i++) {
		Rx_Buf[i] = 0;
	}
}

static void ZeroTxBuf(uint8_t start) {
	uint8_t i;
	for (i = start; i < SPI_BUFFER_SIZE; i++) {
		Tx_Buf[i] = 0;
	}
}

static void spi_init(void) {
    // SPI SETUP
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MOSI1 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* SCK1 */
	Chip_GPIO_WriteDirBit(LPC_GPIO, CS, true);							/* Chip Select */
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

	Chip_SSP_Init(LPC_SSP1);
	Chip_SSP_SetBitRate(LPC_SSP1, 600000);
	Chip_SSP_SetFormat(LPC_SSP1, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE3);
	Chip_SSP_SetMaster(LPC_SSP1, true);
	Chip_SSP_Enable(LPC_SSP1);

    // For communicating with EEPROM
	xf_setup.tx_data = Tx_Buf;
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

static void WriteEnable(void) {
    // write enable command
    Chip_UART_SendBlocking(LPC_USART, "Sending write enable cmd...\n", 28);
    Tx_Buf[0] = 0x6;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
    Chip_UART_SendBlocking(LPC_USART, "Done write enable cmd...\n", 25);
    // end write enable command

}

static void WriteDisable(void) {
    // write disable command
    Chip_UART_SendBlocking(LPC_USART, "Sending write disable cmd...\n", 29);
    Tx_Buf[0] = 0x4;
    ZeroTxBuf(1);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 1);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
    Chip_UART_SendBlocking(LPC_USART, "Done write disable cmd...\n", 29);
    // end write disable command
}

static void ReadStatusReg(void) {
    // read status register
    Chip_UART_SendBlocking(LPC_USART, "Sending read reg  cmd...\n", 25);

    Tx_Buf[0] = 0x5;
    ZeroTxBuf(1);
	xf_setup.length = 2; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
	xf_setup.rx_data = Rx_Buf;

	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP1, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

    Chip_UART_SendBlocking(LPC_USART, "Done read reg  cmd...\n", 22);
}

static void ReadMem(void) {
    // read status register
    Chip_UART_SendBlocking(LPC_USART, "Reading mem byte cmd...\n", 24);

    Tx_Buf[0] = 0x3;
    Tx_Buf[1] = 0x0;
    Tx_Buf[2] = 0xF;
    ZeroTxBuf(3);
    xf_setup.length = 4; xf_setup.rx_cnt = 0; xf_setup.tx_cnt = 0;
    xf_setup.rx_data = Rx_Buf;

    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
    Chip_SSP_RWFrames_Blocking(LPC_SSP1, &xf_setup);
    Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

    Chip_UART_SendBlocking(LPC_USART, "Done reading mem byte cmd...\n", 29);
}

static void WriteMem(void) {
    Chip_UART_SendBlocking(LPC_USART, "Writing mem byte cmd...\n", 24);

    Tx_Buf[0] = 0x2;
    Tx_Buf[1] = 0x0;
    Tx_Buf[2] = 0xF;
    Tx_Buf[3] = 0xA;
    ZeroTxBuf(4);

    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_WriteFrames_Blocking(LPC_SSP1, Tx_Buf, 1);
    Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

    Chip_UART_SendBlocking(LPC_USART, "Done writing mem byte cmd...\n", 29);
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

    uart_init();
    spi_init();
    
    ZeroRxBuf();
    ZeroTxBuf(0);

    ReadStatusReg();
    Chip_UART_SendBlocking(LPC_USART, "ST reg before wenb:\n", 20);
    PrintRxBuffer();

    WriteEnable();

    ReadStatusReg();
    Chip_UART_SendBlocking(LPC_USART, "ST reg after wenb:\n", 19);
    PrintRxBuffer();

    ReadMem();
    Chip_UART_SendBlocking(LPC_USART, "ze mem before wmem:\n", 20);
    PrintRxBuffer();

    WriteMem();

    ReadMem();
    Chip_UART_SendBlocking(LPC_USART, "ze mem after wmem:\n", 19);
    PrintRxBuffer();

	while(1) {
		delay(5);
	}

	return 0;
}

