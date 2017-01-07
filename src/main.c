#include <string.h>
#include "board.h"
#include "chip.h"
#include "util.h"
#include "config.h"
#include "lc1024.h"
#include "state_types.h"
#include <string.h>
#include "ssm.h"

#define SSP_IRQ           SSP1_IRQn
#define SSPIRQHANDLER     SSP1_IRQHandler
#define ADDR_LEN 3
#define MAX_DATA_LEN 16

#define LED0 2, 10
volatile uint32_t msTicks;

static char str[100];

static uint8_t Rx_Buf[SPI_BUFFER_SIZE];
static uint8_t eeprom_data[MAX_DATA_LEN];
static uint8_t eeprom_address[ADDR_LEN];

static void PrintRxBuffer(void);
static void ZeroRxBuf(void);

static BMS_INPUT_T bms_input;
static BMS_OUTPUT_T bms_output;
static BMS_STATE_T bms_state;

void SysTick_Handler(void) {
	msTicks++;
}

/****************************
 *          HELPERS
 ****************************/

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

static void ZeroRxBuf(void) {
	uint8_t i;
	for (i = 0; i < SPI_BUFFER_SIZE; i++) {
		Rx_Buf[i] = 0;
	}
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

/****************************
 *       INITIALIZERS
 ****************************/

static void Init_Core(void) {
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		while(1); // error state
	}
}

static void Init_GPIO(void) {
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
}

static void Init_UART(void) {
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */

	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, UART_BAUD);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);
}

void Init_EEPROM(void) {
    LC1024_Init(600000, 0, 7);
    ZeroRxBuf();
    LC1024_WriteEnable();
}

void Process_Input(BMS_INPUT_T* bms_input) {
    // Read current mode request
    // Read pack status
    // Read hardware signal inputs
}

void Process_Output(BMS_OUTPUT_T* bms_output) {
    // If SSM changed state, output appropriate visual indicators
    // Carry out appropriate hardware output requests (CAN messages, charger requests, etc.)
}

void Process_Keyboard_Debug(void) {
    // Process keyboard strokes and output correpsonding debug messages
}

int main(void) {

    Init_Core();
    Init_GPIO();
    Init_UART();
    Init_EEPROM();

    // SSM_Init(&bms_state);

	while(1) {
        Process_Keyboard_Debug();
        Process_Input(&bms_input);
        // SSM_Step(&bms_input, &bms_state, &bms_output); 
        Process_Output(&bms_output);
	}

	return 0;
}

