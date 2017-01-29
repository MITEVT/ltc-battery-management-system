#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "ssm.h"
#include "sysinit.h"
#include "console.h"
#include "eeprom_config.h"
#include "config.h"
#include "ltc6804.h"
#include "error_handler.h"

#define LED0 2, 8
#define LED1 2, 10

#define BAL_SW 1, 2
#define IOCON_BAL_SW IOCON_PIO1_2
#define CHRG_SW 1, 2
#define IOCON_CHRG_SW IOCON_PIO1_2
#define DISCHRG_SW 1, 2
#define IOCON_DISCHRG_SW IOCON_PIO1_2

#define EEPROM_BAUD 600000
#define EEPROM_CS_PIN 1, 7

#define Hertz2Ticks(freq) SystemCoreClock / freq
#define LTC_CELL_VOLTAGE_FREQ 10 //[TODO] removeme

volatile uint32_t msTicks;

static char str[10];

// memory allocation for BMS_OUTPUT_T
static bool balance_reqs[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
static BMS_CHARGE_REQ_T charge_req;
static BMS_OUTPUT_T bms_output;

// memory allocation for BMS_INPUT_T
static BMS_PACK_STATUS_T pack_status;
static BMS_INPUT_T bms_input;

// memory allocation for BMS_STATE_T
static BMS_CHARGER_STATUS_T charger_status;
static uint32_t cell_voltages[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
static uint8_t module_cell_count[MAX_NUM_MODULES];
static PACK_CONFIG_T pack_config;
static BMS_STATE_T bms_state;

// // memory allocation for LTC6804
// static LTC6804_CONFIG_T ltc6804_config; //[TODO] removeme
// static LTC6804_STATE_T ltc6804_state;
// static Chip_SSP_DATA_SETUP_T ltc6804_xf_setup; //[TODO] removeme
// static uint8_t ltc6804_tx_buf[LTC6804_CALC_BUFFER_LEN(MAX_NUM_MODULES)]; //[TODO] removeme
// static uint8_t ltc6804_rx_buf[LTC6804_CALC_BUFFER_LEN(MAX_NUM_MODULES)]; //[TODO] removeme
// static uint8_t ltc6804_cfg[LTC6804_DATA_LEN]; //[TODO] removeme
// static uint16_t ltc6804_bal_list[MAX_NUM_MODULES]; //[TODO] removeme
// static LTC6804_ADC_RES_T ltc6804_adc_res; //[TODO] removeme

// // ltc6804 timing variables
// static bool ltc6804_get_cell_voltages; //[TODO] removeme

// memory for console
static microrl_t rl;
static CONSOLE_OUTPUT_T console_output;

/****************************
 *          IRQ Vectors
 ****************************/

void SysTick_Handler(void) {
	msTicks++;
}


/****************************
 *          HELPERS
 ****************************/

// [TODO] Remove
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
    // [TODO] verify that pins don't collide
    //  move pin selections to preprocesser defines
	Chip_GPIO_Init(LPC_GPIO);
	Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);

    Chip_GPIO_WriteDirBit(LPC_GPIO, BAL_SW, false);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_BAL_SW, IOCON_MODE_PULLUP);
    Chip_GPIO_WriteDirBit(LPC_GPIO, CHRG_SW, false);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_CHRG_SW, IOCON_MODE_PULLUP);
    Chip_GPIO_WriteDirBit(LPC_GPIO, DISCHRG_SW, false);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_DISCHRG_SW, IOCON_MODE_PULLUP);
    
    //SSP for EEPROM
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MOSI1 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK1 */

    //SSP for LTC6804
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MISO0 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MOSI0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_6, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK0 */
    Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO0_6);
}




void Init_BMS_Structs(void) {
    bms_output.charge_req = &charge_req;
    bms_output.close_contactors = false;
    bms_output.balance_req = balance_reqs;
    memset(balance_reqs, 0, sizeof(balance_reqs[0])*MAX_NUM_MODULES*MAX_CELLS_PER_MODULE);
    bms_output.read_eeprom_packconfig = false;
    bms_output.check_packconfig_with_ltc = false;

    charge_req.charger_on = 0;
    charge_req.charge_current_mA = 0;
    charge_req.charge_voltage_mV = 0;

    bms_state.charger_status = &charger_status;
    bms_state.pack_config = &pack_config;
    bms_state.curr_mode = BMS_SSM_MODE_INIT;
    bms_state.init_state = BMS_INIT_OFF;
    bms_state.charge_state = BMS_CHARGE_OFF;
    bms_state.discharge_state = BMS_DISCHARGE_OFF;
    bms_state.error_code = BMS_NO_ERROR;

    charger_status.connected = false;
    charger_status.error = false;

    pack_config.module_cell_count = module_cell_count;
    pack_config.cell_min_mV = 0;
    pack_config.cell_max_mV = 0;
    pack_config.cell_capacity_cAh = 0;
    pack_config.num_modules = 0;
    pack_config.cell_charge_c_rating_cC = 0;
    pack_config.bal_on_thresh_mV = 0;
    pack_config.bal_off_thresh_mV = 0;
    pack_config.pack_cells_p = 0;
    pack_config.cv_min_current_mA = 0;
    pack_config.cv_min_current_ms = 0;
    pack_config.cc_cell_voltage_mV = 0;

    pack_config.cell_discharge_c_rating_cC = 0; // at 27 degrees C
    pack_config.max_cell_temp_C = 0;

    //assign bms_inputs
    bms_input.hard_reset_line = false;
    bms_input.mode_request = BMS_SSM_MODE_STANDBY;
    bms_input.balance_mV = 0; // console request balance to mV
    bms_input.contactors_closed = false;
    bms_input.msTicks = msTicks;
    bms_input.pack_status = &pack_status;
    bms_input.eeprom_packconfig_read_done = false;
    bms_input.ltc_packconfig_check_done = false;
    bms_input.eeprom_read_error = false;

    memset(cell_voltages, 0, sizeof(cell_voltages));
    pack_status.cell_voltages_mV = cell_voltages;
    pack_status.pack_cell_max_mV = 0;
    pack_status.pack_cell_min_mV = 0xFFFFFFFF; // [TODO] this is a bodge fix
    pack_status.pack_current_mA = 0;
    pack_status.pack_voltage_mV = 0;
    pack_status.precharge_voltage = 0;
    pack_status.max_cell_temp_C = 0;
    pack_status.error = 0;

}

void Process_Input(BMS_INPUT_T* bms_input) {
    // Read current mode request
    // Override Console Mode Request
    // Read pack status
    // Read hardware signal inputs
    // update and other fields in msTicks in &input

    if (console_output.valid_mode_request) {
        bms_input->mode_request = console_output.mode_request;
        bms_input->balance_mV = console_output.balance_mV;
    } else {
        bms_input->mode_request = BMS_SSM_MODE_STANDBY; // [TODO] Change this
    }

    // if (Chip_GPIO_GetPinState(LPC_GPIO, BAL_SW)) {
    //     bms_input->mode_request = BMS_SSM_MODE_BALANCE;
    //     bms_input->balance_mV = 3300;
    // } else if (Chip_GPIO_GetPinState(LPC_GPIO, CHRG_SW)) {
    //     bms_input->mode_request = BMS_SSM_MODE_CHARGE;
    // } else if (Chip_GPIO_GetPinState(LPC_GPIO, DISCHRG_SW)) {
    //     bms_input->mode_request = BMS_SSM_MODE_DISCHARGE;
    // } else {
    //     bms_input->mode_request = BMS_SSM_MODE_STANDBY;
    // }

    Board_Get_Cell_Voltages(&pack_status, msTicks);
        

    bms_input->msTicks = msTicks;
}

void Process_Output(BMS_INPUT_T* bms_input, BMS_OUTPUT_T* bms_output) {
    // If SSM changed state, output appropriate visual indicators
    // Carry out appropriate hardware output requests (CAN messages, charger requests, etc.)
    // Board_Println_BLOCKING("Process output");
    
    if (bms_output->read_eeprom_packconfig){
        bms_input->eeprom_packconfig_read_done = EEPROM_Load_PackConfig(&pack_config);
        Charge_Config(&pack_config);
        Discharge_Config(&pack_config);
        Board_DeInit_LTC6804(); // [TODO] Think about this
    }
    else if (bms_output->check_packconfig_with_ltc) {
        bms_input->ltc_packconfig_check_done = 
            EEPROM_Check_PackConfig_With_LTC(&pack_config);

        Board_Init_LTC6804(&pack_config, cell_voltages, msTicks);
        bms_input->ltc_packconfig_check_done = Board_LTC6804_CVST(msTicks);
    }

    if (bms_state.curr_mode == BMS_SSM_MODE_CHARGE || bms_state.curr_mode == BMS_SSM_MODE_BALANCE) {
        Board_LTC6804_UpdateBalanceStates(bms_output->balance_req, msTicks);
    }

}

// [TODO] Turn off and move command prompt when others are typing....bitch
//      Board Print should tell microrl to gtfo
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
    Board_Init_Timers(); // [TODO] Think about proper place to put this
    //ltc6804_get_cell_voltages = false; // [TODO] Same as above
    EEPROM_init(LPC_SSP0, EEPROM_BAUD, EEPROM_CS_PIN);

    Init_BMS_Structs();
    Board_UART_Init(UART_BAUD);

    Board_Println("Started Up");    
    

    //setup readline
    microrl_init(&rl, Board_Print);
    microrl_set_execute_callback(&rl, executerl);
    console_init(&bms_input, &bms_state, &console_output);
    Error_Init();
    SSM_Init(&bms_input, &bms_state, &bms_output);

    uint32_t last_count = msTicks;

	while(1) {
        Process_Keyboard(); //do this if you want to add the command line
        Process_Input(&bms_input);
        SSM_Step(&bms_input, &bms_state, &bms_output); 
        Process_Output(&bms_input, &bms_output);
        if (Error_Handle(bms_input.msTicks) == HANDLER_HALT) {
            break;
        }
        
        // Testing Code
        bms_input.contactors_closed = bms_output.close_contactors; // [DEBUG] For testing purposes

        // LED Heartbeat
        if (msTicks - last_count > 1000) {
            last_count = msTicks;
            Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1 - Chip_GPIO_GetPinState(LPC_GPIO, LED0));     
        }
    }
    Board_Println_BLOCKING("GOT REKT");
    while(1) {
        //set bms_outputs
        //process_output(bms_outputs);
        //process_keyboard()
    }
	return 0;
}

