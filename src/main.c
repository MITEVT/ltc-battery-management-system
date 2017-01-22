#include <string.h>
#include <stdlib.h>
#include "board.h"
#include "lc1024.h"
#include "state_types.h"
#include "ssm.h"
#include "sysinit.h"
#include "console.h"
// #include "eeprom_config.h"
#include "config.h"
#include "ltc6804.h"

#define ADDR_LEN 3
#define MAX_DATA_LEN 16

#define LED0 2, 10
#define LED1 2, 8

#define BAL_SW 1, 2
#define IOCON_BAL_SW IOCON_PIO1_2
#define CHRG_SW 1, 2
#define IOCON_CHRG_SW IOCON_PIO1_2
#define DISCHRG_SW 1, 2
#define IOCON_DISCHRG_SW IOCON_PIO1_2

volatile uint32_t msTicks;

static char str[10];

static uint8_t UART_Rx_Buf[UART_BUFFER_SIZE];

static uint8_t SPI_Rx_Buf[SPI_BUFFER_SIZE];
static uint8_t eeprom_data[MAX_DATA_LEN];
static uint8_t eeprom_address[ADDR_LEN];

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

// static void delay(uint32_t dlyTicks) {
// 	uint32_t curTicks = msTicks;
// 	while ((msTicks - curTicks) < dlyTicks);
// }

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
}

void Init_EEPROM(void) {
    LC1024_Init(LPC_SSP1, 600000, 0, 7);
    memset(SPI_Rx_Buf, 0, SPI_BUFFER_SIZE);
    LC1024_WriteEnable();
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

    pack_config.cell_min_mV = 2500;
    pack_config.cell_max_mV = 4200;
    pack_config.cell_capacity_cAh = 530;
    pack_config.num_modules = 1;
    pack_config.cell_charge_c_rating_cC = 70;
    pack_config.bal_on_thresh_mV = 4;
    pack_config.bal_off_thresh_mV = 1;
    pack_config.pack_cells_p = 6;
    pack_config.cv_min_current_mA = 50;
    pack_config.cv_min_current_ms = 60000;
    pack_config.cc_cell_voltage_mV = 4300;
    num_cells_in_modules[0] = 4;

    pack_config.cell_discharge_c_rating_cC = 200; // at 27 degrees C
    pack_config.max_cell_temp_C = 50;

    bms_state.pack_config = &pack_config;

    bms_input.pack_status = &pack_status;
}

void Process_Input(BMS_INPUT_T* bms_input) {
    // Read current mode request
    // Read pack status
    // Read hardware signal inputs
    // update and other fields in msTicks in &input

    if (Chip_GPIO_GetPinState(LPC_GPIO, BAL_SW)) {
        bms_input->mode_request = BMS_SSM_MODE_BALANCE;
        bms_input->balance_mV = 3300;
    } else if (Chip_GPIO_GetPinState(LPC_GPIO, CHRG_SW)) {
        bms_input->mode_request = BMS_SSM_MODE_CHARGE;
    } else if (Chip_GPIO_GetPinState(LPC_GPIO, DISCHRG_SW)) {
        bms_input->mode_request = BMS_SSM_MODE_DISCHARGE;
    } else {
        bms_input->mode_request = BMS_SSM_MODE_STANDBY;
    }
}

void Process_Output(BMS_INPUT_T* bms_input, BMS_OUTPUT_T* bms_output) {
    // If SSM changed state, output appropriate visual indicators
    // Carry out appropriate hardware output requests (CAN messages, charger requests, etc.)

    if (bms_output->read_eeprom_packconfig) {
        bms_input->eeprom_packconfig_read_done = 
            Load_EEPROM_PackConfig(&pack_config);
        // Board_Println("LOADED EEPROM");
    } else if (bms_output->check_packconfig_with_ltc) {
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
    Init_EEPROM();
    Init_BMS_Structs();
    Board_UART_Init(UART_BAUD);
    Default_Config();

    Board_Println("Started Up");

    // For now do all LTC6804 Init here, 100k Baud, Port 0_7 for CS
    LTC6804_CONFIG_T ltc6804_config;
    ltc6804_config.pSSP = LPC_SSP1;
    ltc6804_config.baud = 100000;
    ltc6804_config.cs_gpio = 0;
    ltc6804_config.cs_pin = 7;

    ltc6804_config.num_modules = pack_config.num_modules;
    ltc6804_config.module_cell_count = num_cells_in_modules;

    ltc6804_config.min_cell_mV = pack_config.cell_min_mV;
    ltc6804_config.max_cell_mV = pack_config.cell_max_mV;

    ltc6804_config.adc_mode = LTC6804_ADC_MODE_NORMAL;

    LTC6804_STATE_T ltc6804_state;
    Chip_SSP_DATA_SETUP_T ltc6804_xf_setup;
    uint8_t ltc6804_tx_buf[4+15*6];
    uint8_t ltc6804_rx_buf[4+15*6];
    uint8_t ltc6804_cfg[6];
    ltc6804_state.xf = &ltc6804_xf_setup;
    ltc6804_state.tx_buf = ltc6804_tx_buf;
    ltc6804_state.rx_buf = ltc6804_rx_buf;
    ltc6804_state.cfg = ltc6804_cfg;;

    LTC6804_ADC_RES_T ltc6804_adc_res;
    ltc6804_adc_res.cell_voltages_mV = pack_status.cell_voltage_mV;

    LTC6804_Init(&ltc6804_config, &ltc6804_state, msTicks);
    if (!LTC6804_VerifyCFG(&ltc6804_config, &ltc6804_state, msTicks)) {
        Board_Println("");
        Board_Println("LTC6804 failed cfg verification");
        
        int i;
        for (i = 0; i < 12; i++) {
            itoa(ltc6804_state.rx_buf[i], str, 16);
            Board_Println(str);
        }
        Board_Println("");
    } 

    LTC6804_STATUS_T res;
    while((res = LTC6804_CVST(&ltc6804_config, &ltc6804_state, msTicks)) != LTC6804_PASS) {
        if (res == LTC6804_FAIL) {
            Board_Println("");
            Board_Println("LTC6804 failed cvst");
            break;
        }
    }  

    // Test Balance States
    // balance_reqs[0] = 1; balance_reqs[1] = true;
    // balance_reqs[2] = 1; balance_reqs[3] = false;
    // LTC6804_SetBalanceStates(&ltc6804_config, &ltc6804_state, balance_reqs, msTicks);
    // Board_Println("");
    // int i;
    // for (i = 0; i < 12; i++) {
    //     itoa(ltc6804_state.tx_buf[i], str, 16);
    //     Board_Println(str);
    // }
    
    //setup readline
    microrl_init(&rl, Board_Print);
    microrl_set_execute_callback(&rl, executerl);
    console_init(&bms_input, &bms_state, &bms_output);

    SSM_Init(&bms_input, &bms_state, &bms_output);

    uint32_t last_count = msTicks;

	while(1) {
        bms_input.msTicks = msTicks;
        Process_Keyboard(); //do this if you want to add the command line
        Process_Input(&bms_input);

        SSM_Step(&bms_input, &bms_state, &bms_output);

        Process_Output(&bms_input, &bms_output);
        
        // Testing Code
        bms_input.contactors_closed = bms_output.close_contactors; // For testing purposes
        // Act as LTC6804 pack handler. ie balance and update pack_status

        // LED Heartbeat
        if (msTicks - last_count > 1000) {
            last_count = msTicks;
            Chip_GPIO_SetPinState(LPC_GPIO, LED0, 1 - Chip_GPIO_GetPinState(LPC_GPIO, LED0));

            while((res = LTC6804_GetCellVoltages(&ltc6804_config, &ltc6804_state, &ltc6804_adc_res, msTicks)) != LTC6804_PASS) {
                if (res == LTC6804_FAIL) {
                    Board_Println("");
                    Board_Println("LTC6804 failed get_cell_voltages");
                    break;
                } else if (res == LTC6804_SPI_ERROR) {
                    Board_Println("");
                    Board_Println("LTC6804 spi_error get_cell_voltages");
                    break;
                } else if (res == LTC6804_PEC_ERROR) {
                    Board_Println("");
                    Board_Println("LTC6804 pec_error get_cell_voltages");
                    break;
                }
            }  

            int i;
            for (i = 0; i < pack_config.num_cells_in_modules[0]; i++) {
                Board_Println(" ");
                itoa(pack_status.cell_voltage_mV[i], str, 10);
                Board_Print(str);
                Board_Print(", ");
            }

            Board_Println(" ");         
        }
	}

	return 0;
}

