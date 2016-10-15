
#include "chip.h"
#include "ltc6804.h"

const uint32_t OscRateIn = 0;

#define SSP_MODE_TEST       1	/*1: Master, 0: Slave */
#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)
#define SSP_DATA_BIT_NUM(databits)          (databits + 1)
#define SSP_DATA_BYTES(databits)            (((databits) > SSP_BITS_8) ? 2 : 1)
#define SSP_LO_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? 0xFF : (0xFF >> \
																					  (8 - SSP_DATA_BIT_NUM(databits))))
#define SSP_HI_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? (0xFF >> \
																			   (16 - SSP_DATA_BIT_NUM(databits))) : 0)
#define LPC_SSP           LPC_SSP0
#define SSP_IRQ           SSP0_IRQn
#define SSPIRQHANDLER     SSP0_IRQHandler

#define _cs_pin 	2
#define _cs_port 	3


#define LED_PIN 8
/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];

/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

static uint8_t test_pec_data[2];

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;

static void Init_SSP_PinMux(void) {
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MISO0 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* MOSI0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_0, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SSEL0 */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC1 | IOCON_MODE_INACT));	/* SCK0 */
	Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO2_11);
}

static void Buffer_Init(void) {
	uint16_t i;
	uint8_t ch = 0;

	for (i = 0; i < BUFFER_SIZE; i++) {
		Tx_Buf[i] = 0x01;
		Rx_Buf[i] = 0xAA;
	}
} 

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, 2, LED_PIN, true);

}

static void LED_On(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 2, LED_PIN, true);
	Chip_UART_SendBlocking(LPC_USART, "LED_ON", 6);
}

static void LED_Off(void) {
	Chip_GPIO_SetPinState(LPC_GPIO, 2, LED_PIN, false);
	Chip_UART_SendBlocking(LPC_USART, "LED_OFF", 6);
}

static void LTC8604_RDCFG(void){
	Chip_GPIO_SetPinState(LPC_GPIO, _cs_port, _cs_pin, false);
	Tx_Buf[0] = (RDCFG & 0xFF00)>>8;
	Tx_Buf[1] = (RDCFG & 0x00FF);
	uint16_t pec_val = ltc6804_calculate_pec(&test_pec_data, 2);	
	Tx_Buf[2] = (pec_val & 0xFF00)>>8;
	Tx_Buf[3] = (pec_val & 0x00FF);
	
	xf_setup.length = 6;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO,_cs_port, _cs_pin, true);
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
	LED_On();


    Init_SSP_PinMux();
	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 30000);
	/* SSP initialization */
	

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE0; 
	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_SetMaster(LPC_SSP, SSP_MODE_TEST);
	Chip_SSP_Enable(LPC_SSP);


	Buffer_Init();

	LED_On();

	int i;

	while (1) {
		Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, BUFFER_SIZE);

		test_pec[0] = 0x01;
		test_pec[1] = 0x00;
        uint16_t pec = ltc6804_calculate_pec(&test_pec_data, 2);
	    Chip_UART_SendBlocking(LPC_USART, "LED_ON", 6);
        LED_ON();
		for(i=0; i< 0xFFFF; i++);
        LED_OFF();
	}

	return 0;
}
