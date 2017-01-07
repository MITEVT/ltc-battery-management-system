#include <string.h>
#include "board.h"

const uint32_t OscRateIn = 0;

#define UART_BUFFER_SIZE 42
static RINGBUFF_T uart_rx_ring;
static volatile uint8_t _uart_rx_ring[UART_BUFFER_SIZE];
static RINGBUFF_T uart_tx_ring;
static volatile uint8_t _uart_tx_ring[UART_BUFFER_SIZE];

// ------------------------------------------------
// Private Functions

void canBaudrateCalculate(uint32_t baud_rate, uint32_t *can_api_timing_cfg) {
	uint32_t pClk, div, quanta, segs, seg1, seg2, clk_per_bit, can_sjw;
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_CAN);
	pClk = Chip_Clock_GetMainClockRate();

	clk_per_bit = pClk / baud_rate;

	for (div = 0; div <= 15; div++) {
		for (quanta = 1; quanta <= 32; quanta++) {
			for (segs = 3; segs <= 17; segs++) {
				if (clk_per_bit == (segs * quanta * (div + 1))) {
					segs -= 3;
					seg1 = segs / 2;
					seg2 = segs - seg1;
					can_sjw = seg1 > 3 ? 3 : seg1;
					can_api_timing_cfg[0] = div;
					can_api_timing_cfg[1] =
						((quanta - 1) & 0x3F) | (can_sjw & 0x03) << 6 | (seg1 & 0x0F) << 8 | (seg2 & 0x07) << 12;
					return;
				}
			}
		}
	}
}




/**
 * @brief	CCAN Interrupt Handler
 * @return	Nothing
 * @note	The CCAN interrupt handler must be provided by the user application.
 *	It's function is to call the isr() API located in the ROM
 */
void CAN_IRQHandler(void) {
	LPC_CCAN_API->isr();
}

/**
 * @brief	UART Interrupt Handler
 * @return	Nothing
 * @note	stuff more stuff into the buffer
 */

void UART_IRQHandler(void) {
	Chip_UART_IRQRBHandler(LPC_USART, &uart_rx_ring, &uart_tx_ring);
}

// ------------------------------------------------
// Public Functions



void Board_Print(char* str){
	Chip_UART_SendRB(LPC_USART, &uart_tx_ring, str, strlen(str));
}

void Board_Println(char* str){
	Board_Print(str);
	Board_Print("\r\n");
}

void Board_Write(char* str, uint32_t count){

}

void Board_Read(){

}



void Board_UART_Init(uint32_t baudRateHz) {
	// Initialize UART Buffers
	RingBuffer_Init(&uart_rx_ring, _uart_rx_ring, sizeof(uint8_t), UART_BUFFER_SIZE);
	RingBuffer_Flush(&uart_rx_ring);
	RingBuffer_Init(&uart_tx_ring, _uart_tx_ring, sizeof(uint8_t), UART_BUFFER_SIZE);
	RingBuffer_Flush(&uart_tx_ring);


	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */
 
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaudFDR(LPC_USART, baudRateHz);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	Chip_UART_IntEnable(LPC_USART, UART_IER_RBRINT);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_EnableIRQ(UART0_IRQn);


}

void Board_SPI_Init(uint32_t baudRateHz) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MOSI0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SCK0 */
	Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO2_11);

	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_SetBitRate(LPC_SSP0, baudRateHz);

	Chip_SSP_SetFormat(LPC_SSP0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE0);
	Chip_SSP_SetMaster(LPC_SSP0, true);
	Chip_SSP_Enable(LPC_SSP0);
}

void Board_CCAN_Init(uint32_t baudRateHz, 
					void (*CAN_rx)(uint8_t), 
					void (*CAN_tx)(uint8_t), 
					void (*CAN_error)(uint32_t)) {
	uint32_t CanApiClkInitTable[2];
	CCAN_CALLBACKS_T callbacks = {
		CAN_rx,
		CAN_tx,
		CAN_error,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};
	canBaudrateCalculate(baudRateHz, CanApiClkInitTable);

	LPC_CCAN_API->init_can(CanApiClkInitTable, TRUE);
	LPC_CCAN_API->config_calb(&callbacks);
	NVIC_EnableIRQ(CAN_IRQn);
}

void Board_LED_Init(void) {
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED1_GPIO, LED1_PIN);
}

void Board_LED_On(void) {
	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LED1_GPIO, LED1_PIN);
}

void Board_LED_Off(void) {
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED1_GPIO, LED1_PIN);
}

void Board_Switch_Init(void) {
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SWITCH_GPIO, SWITCH_PIN);
}

bool Board_Switch_Read(void) {
	return Chip_GPIO_GetPinState(LPC_GPIO, SWITCH_GPIO, SWITCH_PIN);

}

