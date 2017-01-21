#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "lc1024.h"
#include "state_types.h"
#include "ssm.h"
#include "sysinit.h"
#include "console.h"
#include "eeprom_config.h"
#include "config.h"

#define ADDR_LEN 3
#define MAX_DATA_LEN 16

#define LED0 2, 10
#define LED1 2, 8

volatile uint32_t msTicks;

static char str[10];

static uint8_t UART_Rx_Buf[UART_BUFFER_SIZE];

static void PrintRxBuffer(uint8_t *rx_buf, uint32_t size);
static void ZeroRxBuf(uint8_t *rx_buf, uint32_t size);

// memory allocation for BMS_OUTPUT_T
static bool balance_reqs[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
static BMS_CHARGE_REQ_T charge_req;
static BMS_OUTPUT_T bms_output;

// memory allocation for BMS_INPUT_T
static BMS_PACK_STATUS_T pack_status;
static BMS_INPUT_T bms_input;

//memory allocation for BMS_STATE_T
static BMS_CHARGER_STATUS_T charger_status;
static uint32_t cell_voltages[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
static uint8_t num_cells_in_modules[MAX_NUM_MODULES];
static PACK_CONFIG_T pack_config;
static BMS_STATE_T bms_state;

void SysTick_Handler(void) {
	msTicks++;
}

/****************************
 *          HELPERS
 ****************************/

static void PrintRxBuffer(uint8_t *rx_buf, uint32_t size) {
    Chip_UART_SendBlocking(LPC_USART, "0x", 2);
    uint8_t i;
    for(i = 0; i < size; i++) {
        itoa(rx_buf[i], str, 16);
        if(rx_buf[i] < 16) {
            Chip_UART_SendBlocking(LPC_USART, "0", 1);
        }
        Chip_UART_SendBlocking(LPC_USART, str, 2);
    }
    Chip_UART_SendBlocking(LPC_USART, "\n", 1);
}

static void delay(uint32_t dlyTicks) {
	uint32_t curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks);
}

/****************************
 *       INITIALIZERS
 ****************************/

static void Init_Core(void) {
	SysTick_Config (TicksPerMS);
}

static void Init_GPIO(void) {
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);
    
    //SSP for EEPROM
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MOSI1 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK1 */
}

void Init_EEPROM_config(void) {
    init_eeprom(LPC_SSP1, 600000, 0, 7);
}

void Init_BMS_Structs(void) {
    bms_output.charge_req = &charge_req;
    bms_output.balance_req = balance_reqs;

    bms_state.charger_status = &charger_status;
    pack_status.cell_voltage_mV = cell_voltages;
    pack_status.pack_cell_max_mV = 0;
    pack_status.pack_cell_min_mV = 0;
    pack_status.pack_current_mA = 0;
    pack_status.pack_voltage_mV = 0;
    pack_status.precharge_voltage = 0;
    pack_status.error = 0;;
    pack_config.num_cells_in_modules = num_cells_in_modules;
    bms_state.pack_config = &pack_config;

    bms_input.pack_status = &pack_status;
}

void Process_Input(BMS_INPUT_T* bms_input) {
    // Read current mode request
    // Read pack status
    // Read hardware signal inputs
    // update and other fields in msTicks in &input
    bms_input->msTicks = msTicks;
}

void Process_Output(BMS_INPUT_T* bms_input, BMS_OUTPUT_T* bms_output) {
    // If SSM changed state, output appropriate visual indicators
    // Carry out appropriate hardware output requests (CAN messages, charger requests, etc.)
    if (bms_output->read_eeprom_packconfig){
        bms_input->eeprom_packconfig_read_done = 
            Load_EEPROM_PackConfig(&pack_config);
        // Board_Println("LOADED EEPROM");
        
    }
    else if (bms_output->check_packconfig_with_ltc) {
        bms_input->ltc_packconfig_check_done = 
            Check_PackConfig_With_LTC(&pack_config);
    }
    
}

void Process_Keyboard(void) {
    uint32_t readln = Board_Read(str,50);
    uint32_t i;
    for(i = 0; i < readln; i++) {
        microrl_insert_char(&rl, str[i]);
    }
}


int main(void) {

    Init_Core();
    Init_GPIO();
    Init_EEPROM_config();
    Init_BMS_Structs();
    Board_UART_Init(UART_BAUD);
    Default_Config();

    uint32_t last_count = msTicks;
    while (msTicks - last_count > 1000) {
    }

    Board_Println("I'm Up");
    
    //setup readline
    microrl_init(&rl, Board_Print);
    microrl_set_execute_callback(&rl, executerl);
    console_init(&bms_input, &bms_state, &bms_output);

    SSM_Init(&bms_input, &bms_state, &bms_output);

    last_count = msTicks;
    // itoa(&commands,str,16);
    // Board_Println(commands[0]);

	while(1) {

        bms_input.msTicks = msTicks;
        Process_Keyboard(); //do this if you want to add the command line
        Process_Input(&bms_input);
        SSM_Step(&bms_input, &bms_state, &bms_output); 
        Process_Output(&bms_input, &bms_output);
        
        // LED Heartbeat
        if (msTicks - last_count > 1000) {
            last_count = msTicks;
            Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1 - Chip_GPIO_GetPinState(LPC_GPIO, LED0));
        }
	}

	return 0;
}

