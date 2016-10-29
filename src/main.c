
#include "chip.h"
#include "util.h"
#include "ltc6804.h"
#include <string.h>

const uint32_t OscRateIn = 0;

#define BUFFER_SIZE                         (14)

#define LPC_SSP           LPC_SSP1
#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler

#define CS 0, 7


#define LED0 2, 10
/* Tx buffer */
static uint8_t Tx_Buf[BUFFER_SIZE];
/* Rx buffer */
static uint8_t Rx_Buf[BUFFER_SIZE];

volatile uint32_t msTicks;

void SysTick_Handler(void) {
	msTicks++;
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

static char str[100];

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

static void LED_Config(void) {
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);

}

static void LTC6804_Wake(void) {
	Tx_Buf[0] = 0x00;
	Tx_Buf[1] = 0x00;


}

static void SendCommand(uint8_t CMD1, uint8_t CMD2, uint8_t *data, uint8_t read) {
	Tx_Buf[1] = CMD1;
	Tx_Buf[2] = CMD2;
	uint16_t pec = ltc6804_calculate_pec(Tx_Buf+1, 2);

}

static void ConfigureChip(void) {

	// For now send junk to chip to wake up
	xf_setup.length = 7;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
    Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
    Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

    Tx_Buf[0] = 0x00;
    Tx_Buf[1] = 0x01;
    uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
    Tx_Buf[2] = pec >> 8;
    Tx_Buf[3] = pec & 0x00FF;
    Tx_Buf[4] = 0xF8; // GPIO OFF with ADC Option 0
    Tx_Buf[5] = 0x00;
    Tx_Buf[6] = 0x00;
    Tx_Buf[7] = 0x00;
    Tx_Buf[8] = 0x00;
    Tx_Buf[9] = 0x00;
    pec = ltc6804_calculate_pec(Tx_Buf+4, 6);
    Tx_Buf[10] = pec >> 8;
    Tx_Buf[11] = pec & 0xFF;

    xf_setup.length = 12;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
}

static void OpenWireTest(uint8_t pup_bit) {
	Tx_Buf[0] = 0x03;
    if(pup_bit == 0) {
	    Tx_Buf[1] = 0x28;
    } else {
	    Tx_Buf[1] = 0x68;
    }
	uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

	xf_setup.length = 4;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
}

static void StartADC(void) {
	Tx_Buf[0] = 0x03;
	Tx_Buf[1] = 0x60;
	uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;

	xf_setup.length = 4;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
}

//read voltages of cells 1-2
static uint32_t ReadVoltageGroupA(void) {
	Tx_Buf[0] = 0x00;
	Tx_Buf[1] = 0x04;
	uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;
	int i;
	for(i = 5; i < 12; i++){
		Tx_Buf[i] = 0;
	}

	xf_setup.length = 12;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

	return (Rx_Buf[4])|(Rx_Buf[5]<<8)|(Rx_Buf[6]<<16)|(Rx_Buf[7]<<24);
}

//read voltages of cells 7-8
static uint32_t ReadVoltageGroupC(void) {
	Tx_Buf[0] = 0x00;
	Tx_Buf[1] = 0x08;
	uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
	Tx_Buf[2] = pec >> 8;
	Tx_Buf[3] = pec & 0xFF;
	int i;
	for(i = 5; i < 12; i++){
		Tx_Buf[i] = 0;
	}

	xf_setup.length = 12;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

	Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

	return (Rx_Buf[4])|(Rx_Buf[5]<<8)|(Rx_Buf[6]<<16)|(Rx_Buf[7]<<24);
}

static void CVSelfTest(void) {
    Tx_Buf[0] = 0x03;
    Tx_Buf[1] = 0x27;
    uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
    Tx_Buf[2] = pec >> 8;
    Tx_Buf[3] = pec & 0xFF;

    xf_setup.length = 4;
    xf_setup.rx_cnt = 0;
    xf_setup.tx_cnt = 0;
    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
    Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
    Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
}

static void ReadConfig(void) {

	uint16_t i;
    Tx_Buf[0] = 0x00;
    Tx_Buf[1] = 0x02;
    // Hard coding PEC, use erpo's PEC code for later
    uint16_t pec = ltc6804_calculate_pec(Tx_Buf, 2);
    Tx_Buf[2] = pec >> 8;
    Tx_Buf[3] = pec & 0x00FF;

    for(i = 5; i < 12; i++) {
    	Tx_Buf[i] = 0;	
    }

    xf_setup.length = 12;
	xf_setup.rx_cnt = 0;
	xf_setup.tx_cnt = 0;

    Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	Chip_GPIO_SetPinState(LPC_GPIO, CS, true);
}

void PrintTwoCellsTwoGroups(uint32_t a, uint32_t c) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    itoa(a >> 16, str, 16);
    Chip_UART_SendBlocking(LPC_USART, str, 4);
    Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
    itoa(a & 0xFFFF, str, 16);
    Chip_UART_SendBlocking(LPC_USART, str, 4);
    Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
    itoa(c >> 16, str, 16);
    Chip_UART_SendBlocking(LPC_USART, str, 4);
    Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
    itoa(c & 0xFFFF, str, 16);
    Chip_UART_SendBlocking(LPC_USART, str, 4);
    Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
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

	Chip_SSP_Init(LPC_SSP);
	Chip_SSP_SetBitRate(LPC_SSP, 500000);

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

    xf_setup.rx_data = Rx_Buf;
	xf_setup.tx_data = Tx_Buf;
    
	ConfigureChip();
	delay(1);
	StartADC();
    //CVSelfTest();
    
    int cellGroupA[3];
    int cellGroupA2[3];
    int cellGroupC[3];
    int cellGroupC2[3];

    OpenWireTest(1);
    OpenWireTest(1);
	delay(5);
	uint32_t a = ReadVoltageGroupA();
    cellGroupA[0] = a & 0xFFFF;
    cellGroupA[1] = a >> 16;
    cellGroupA[2] = 0;

	// delay(10);
	uint32_t c = ReadVoltageGroupC();
    cellGroupC[0] = c & 0xFFFF;
    cellGroupC[1] = c >> 16;
    cellGroupC[2] = 0;

    OpenWireTest(0);
    OpenWireTest(0);
	delay(5);
	uint32_t a2 = ReadVoltageGroupA(); // PUP down
    cellGroupA2[0] = a2 & 0xFFFF;
    cellGroupA2[1] = a2 >> 16;
    cellGroupA2[2] = 0;
	// delay(1);
	uint32_t c2 = ReadVoltageGroupC();
    cellGroupC2[0] = c2 & 0xFFFF;
    cellGroupC2[1] = c2 >> 16;
    cellGroupC2[2] = 0;

    PrintTwoCellsTwoGroups(a, c);
    PrintTwoCellsTwoGroups(a2, c2);

    uint8_t i;
    for(i = 0; i < 3; i++) {
        if(cellGroupA[i] - cellGroupA2[i] < -400) {
            itoa(i, str, 10);
            Chip_UART_SendBlocking(LPC_USART, "Cell Group A, Cell ", 18);
            Chip_UART_SendBlocking(LPC_USART, str, 1);
            Chip_UART_SendBlocking(LPC_USART, "  is probably open!\n", 20);
        } else {
            itoa(i, str, 10);
            Chip_UART_SendBlocking(LPC_USART, "Cell Group A, Cell ", 18);
            Chip_UART_SendBlocking(LPC_USART, str, 1);
            Chip_UART_SendBlocking(LPC_USART, "  is probably not open!\n", 24);
        }

        if(cellGroupC[i] - cellGroupC2[i] < -400) {
            itoa(i, str, 10);
            Chip_UART_SendBlocking(LPC_USART, "Cell Group B, Cell ", 18);
            Chip_UART_SendBlocking(LPC_USART, str, 1);
            Chip_UART_SendBlocking(LPC_USART, "  is probably open!\n", 20);
        } else {
            itoa(i, str, 10);
            Chip_UART_SendBlocking(LPC_USART, "Cell Group B, Cell ", 18);
            Chip_UART_SendBlocking(LPC_USART, str, 1);
            Chip_UART_SendBlocking(LPC_USART, "  is probably not open!\n", 24);
        }
    }

    /*
	int i = 0;
    if((a & 0xFFFF) == 0x9555 && (a >> 16) == 0x9555) {
        if((c & 0xFFFF) == 0x9555 && (c >> 16) == 0x9555) {
            Chip_UART_SendBlocking(LPC_USART, "Passed test!", 13);
        }
    } else {
        Chip_UART_SendBlocking(LPC_USART, "Didn't pass test!", 18);
        Chip_UART_SendBlocking(LPC_USART, "0x", 2);
        itoa(a >> 16, str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 4);
        Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
        itoa(a & 0xFFFF, str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 4);
        Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
        itoa(c >> 16, str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 4);
        Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
        itoa(c & 0xFFFF, str, 16);
        Chip_UART_SendBlocking(LPC_USART, str, 4);
        Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
    }
    */

    /*
	Chip_UART_SendBlocking(LPC_USART, "0x", 2);
	itoa(a >> 16, str, 16);
	Chip_UART_SendBlocking(LPC_USART, str, 4);
	Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
	itoa(a & 0xFFFF, str, 16);
	Chip_UART_SendBlocking(LPC_USART, str, 4);
	Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
	itoa(c >> 16, str, 16);
	Chip_UART_SendBlocking(LPC_USART, str, 4);
	Chip_UART_SendBlocking(LPC_USART, ", 0x", 4);
	itoa(c & 0xFFFF, str, 16);
	Chip_UART_SendBlocking(LPC_USART, str, 4);
	Chip_UART_SendBlocking(LPC_USART, "\r\n", 2);
    */

	while(1) {
		// Chip_GPIO_SetPinState(LPC_GPIO, CS, false);
	    // Chip_UART_SendBlocking(LPC_USART, "sending frames..\n",17);	
		// Chip_SSP_WriteFrames_Blocking(LPC_SSP, Tx_Buf, 4);
		// Chip_UART_SendBlocking(LPC_USART, "done sending frames..\n",22);

		// xf_setup.rx_data = Rx_Buf;
		// xf_setup.tx_data = Tx_Buf;
		// xf_setup.length = BUFFER_SIZE;
		// xf_setup.rx_cnt = 0;
		// xf_setup.tx_cnt = 0;

		//     itoa(Tx_Buf[2], str, 16);
		// Chip_UART_SendBlocking(LPC_USART, str, 2);
		// itoa(Rx_Buf[3], str, 16);
		// Chip_UART_SendBlocking(LPC_USART, " ", 1);
		// Chip_UART_SendBlocking(LPC_USART, str, 2);
		// Chip_UART_SendBlocking(LPC_USART, "\n", 1);

		// Chip_UART_SendBlocking(LPC_USART, "reading and writing frames..\n",28);
		// Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
		// Chip_UART_SendBlocking(LPC_USART, "done reading and writing frames..\n",34);
		// for(i = 0; i < 12; i++) {
		// 	// itoa(Rx_Buf[i], str, 10);
			// Chip_UART_SendBlocking(LPC_USART, str, 3);
			// Chip_UART_SendBlocking(LPC_USART, ",", 1);
		// }
		// Chip_UART_SendBlocking(LPC_USART, "\n", 1);

		// Chip_GPIO_SetPinState(LPC_GPIO, CS, true);

		delay(5);
	}

	return 0;
}

