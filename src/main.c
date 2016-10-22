
#include "chip.h"
#include "util.h"
#include "ltc6804.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define TX_BUFFER_SIZE                         (0x100)
#define RX_BUFFER_SIZE                         (0x100)

#define LPC_SSP           LPC_SSP1
#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler

#define CS 0, 7


#define LED0 2, 10
/* Tx buffer */
static uint8_t Tx_Buf[TX_BUFFER_SIZE];

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

/* Rx buffer */
static uint8_t Rx_Buf[RX_BUFFER_SIZE];

static char str[100];

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);

}

static void LTC8604_RDCFG(void){
	// Chip_GPIO_SetPinState(LPC_GPIO, _cs_port, _cs_pin, false);

	Chip_UART_SendBlocking(LPC_USART, "sending frames..\n",17);	
	Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, TX_BUFFER_SIZE);
	Chip_UART_SendBlocking(LPC_USART, "done sending frames..\n",23);	
	
    /*
	xf_setup.length = 5;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_UART_SendBlocking(LPC_USART, "done reading frames..\n",23);	
    */

	// Chip_GPIO_SetPinState(LPC_GPIO,_cs_port, _cs_pin, true);
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
	Chip_UART_SetBaud(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 30000);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_BITS_8;
	ssp_format.clockMode = SSP_CLOCK_MODE3;  // may need to change
	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_SetMaster(LPC_SSP, true);
	Chip_SSP_Enable(LPC_SSP);


	Chip_GPIO_SetPinState(LPC_GPIO, LED0, true);

	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* MOSI1 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));	/* SCK1 */
	// Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_9, (IOCON_FUNC0 | IOCON_MODE_INACT));
	Chip_GPIO_WriteDirBit(LPC_GPIO, CS, true);

	uint16_t i;

    Tx_Buf[0] = 0x00;
    Tx_Buf[1] = 0x01;
    // Hard coding PEC, use erpo's PEC code for later
    Tx_Buf[2] = 0x3D; // 0011 1101
    Tx_Buf[3] = 0x6E;

	while(1) {
		Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	    Chip_UART_SendBlocking(LPC_USART, "sending frames..\n",17);	
		Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 4);
		Chip_UART_SendBlocking(LPC_USART, "done sending frames..\n",23);
		Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
    }

	return 0;
}

