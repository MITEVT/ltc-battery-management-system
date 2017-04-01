// ltc-battery-management-system
#include "board.h"
#include "can_constants.h"
#include "bms_can.h"
#include "cell_temperatures.h"

// C libraries
#include <string.h>

// lpc11cx4-library
#include "brusa.h"

const uint32_t OscRateIn = 0;

#define UART_BUFFER_SIZE 100
#define BMS_HEARTBEAT_PERIOD 1000
#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))

//#define PRINT_MODE_REQUESTS

#ifndef TEST_HARDWARE
static RINGBUFF_T uart_rx_ring;
static uint8_t _uart_rx_ring[UART_BUFFER_SIZE];
static RINGBUFF_T uart_tx_ring;
static uint8_t _uart_tx_ring[UART_BUFFER_SIZE];

static LTC6804_CONFIG_T ltc6804_config;
static LTC6804_STATE_T ltc6804_state;
static Chip_SSP_DATA_SETUP_T ltc6804_xf_setup;
static uint8_t ltc6804_tx_buf[LTC6804_CALC_BUFFER_LEN(MAX_NUM_MODULES)]; 
static uint8_t ltc6804_rx_buf[LTC6804_CALC_BUFFER_LEN(MAX_NUM_MODULES)]; 
static uint8_t ltc6804_cfg[LTC6804_DATA_LEN]; 
static uint16_t ltc6804_bal_list[MAX_NUM_MODULES]; 
static LTC6804_ADC_RES_T ltc6804_adc_res;
static LTC6804_OWT_RES_T ltc6804_owt_res; 
// ltc6804 timing variables
static bool _ltc6804_gcv;
static uint32_t _ltc6804_last_gcv;
static uint32_t _ltc6804_gcv_tick_time;
static bool _ltc6804_owt;
static uint32_t _ltc6804_last_owt;
static uint32_t _ltc6804_owt_tick_time;

static bool _ltc6804_initialized;
static LTC6804_INIT_STATE_T _ltc6804_init_state;

static char str[10];

static BMS_SSM_MODE_T CAN_mode_request;

#ifdef FSAE_DRIVERS
static uint32_t last_bms_heartbeat_time = 0;
static uint8_t received_discharge_request = 0;
#endif

//CAN STUFF
CCAN_MSG_OBJ_T can_rx_msg;
uint32_t latest_vcu_heartbeat_time = 0;

#endif

//Cell temperature sensing stuff
static uint32_t board_lastThermistorShiftTime_ms = 0;
uint8_t currentThermistor = 0;
static bool ltc6804_setMultiplexerAddressFlag = false;
static bool ltc6804_getThermistorVoltagesFlag = false;

volatile uint32_t msTicks;

#ifdef PRINT_MODE_REQUESTS
    static uint32_t last_get_mode_request_debug_message = 0;
#endif

#ifndef TEST_HARDWARE

/**
 * @brief   UART Interrupt Handler
 * @return  Nothing
 * @note    stuff more stuff into the buffer
 */

void UART_IRQHandler(void) {
    Chip_UART_IRQRBHandler(LPC_USART, &uart_rx_ring, &uart_tx_ring);
}

void SysTick_Handler(void) {
    msTicks++;
}

/**
 * @details send a BMS discharge response indicating that the BMS is in discharge state
 */
void Send_BMS_Discharge_Response_Ready(void) {
    uint8_t bms_discharge_bytes = 1;
    uint8_t bms_discharge_bits = bms_discharge_bytes*8;
    uint8_t max_bit_bms_discharge = bms_discharge_bits - 1;
    uint8_t data[bms_discharge_bits];
    data[0] = 0 | (____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__READY << max_bit_bms_discharge);
    CAN_Transmit(BMS_DISCHARGE_RESPONSE__id, data, bms_discharge_bytes);
}

/**
 * @details send a BMS discharge response indicating that the BMS is not in discharge state
 */
void Send_BMS_Discharge_Response_Not_Ready(void) { 
    uint8_t bms_discharge_bytes = 1;
    uint8_t bms_discharge_bits = bms_discharge_bytes*8;
    uint8_t max_bit_bms_discharge = bms_discharge_bits - 1;
    uint8_t data[bms_discharge_bytes];
    data[0] = 0 | (____BMS_DISCHARGE_RESPONSE__DISCHARGE_RESPONSE__NOT_READY << max_bit_bms_discharge);
    CAN_Transmit(BMS_DISCHARGE_RESPONSE__id, data, bms_discharge_bytes);
}

/**
 * @details sends a BMS discharge response over CAN
 */
void Send_BMS_Discharge_Response(BMS_SSM_MODE_T current_state) {
    if (current_state == BMS_SSM_MODE_DISCHARGE) {
        Send_BMS_Discharge_Response_Ready();
    } else {
        Send_BMS_Discharge_Response_Not_Ready();
    }
}

/**
 * @details sends BMS heartbeat
 *
 * @param bms_state current state of the BMS
 */
void Send_BMS_Heartbeat(BMS_STATE_T * bms_state) {
    const uint8_t bms_heartbeat_bytes = 2; 
    //TODO: don't hardcode bms_heartbeat_bytes
    uint16_t soc = 0; 
    //TODO: compute soc in the state machine and save it in some datatype
        uint8_t bms_heartbeat_data[bms_heartbeat_bytes];
    BMS_CAN_ConstructHeartbeatData(bms_state->curr_mode, soc, bms_heartbeat_data);
        CAN_Transmit(BMS_HEARTBEAT__id, bms_heartbeat_data, bms_heartbeat_bytes);
}
#endif

// ------------------------------------------------
// Public Functions

void Board_Chip_Init(void) {
#ifndef TEST_HARDWARE
    SysTick_Config(Hertz2Ticks(1000));
#endif
}


uint32_t Board_Print(const char *str) {
#ifdef TEST_HARDWARE
    return printf("%s", str);
#else
    return Chip_UART_SendRB(LPC_USART, &uart_tx_ring, str, strlen(str));
#endif
}

void Board_BlockingDelay(uint32_t dlyTicks) {
    uint32_t curTicks = msTicks;
    while ((msTicks - curTicks) < dlyTicks);
}

uint32_t Board_Println(const char *str) {
#ifdef TEST_HARDWARE
    return printf("%s\r\n", str);
#else
    uint32_t count = Board_Print(str);
    return count + Board_Print("\r\n");
#endif
}

uint32_t Board_PrintNum(uint32_t a, uint8_t base) {
#ifdef TEST_HARDWARE
    UNUSED(base);
    return printf("%d", a);
#else
    itoa(a, str, base);
    return Board_Print(str);
#endif
}

uint32_t Board_Write(const char *str, uint32_t count) {
#ifdef TEST_HARDWARE
    return printf("%.*s", count, str);
#else
    return Chip_UART_SendRB(LPC_USART, &uart_tx_ring, str, count);
#endif
}

uint32_t Board_Read(char *charBuffer, uint32_t length) {
#ifdef TEST_HARDWARE
    fgets(charBuffer, length, stdin);
    return strlen(charBuffer);
#else
    return Chip_UART_ReadRB(LPC_USART, &uart_rx_ring, charBuffer, length);
#endif
}

// USE THESE SPARINGLY. ONLY WHEN A PRINT WOULD RESULT IN A BUFFER OVERFLOW
uint32_t Board_Print_BLOCKING(const char *str) {
#ifdef TEST_HARDWARE
    return printf("%s", str);
#else
    return Chip_UART_SendBlocking(LPC_USART, str, strlen(str));
#endif
}

uint32_t Board_Println_BLOCKING(const char *str) {
    uint32_t count = Board_Print_BLOCKING(str);
    return count + Board_Print_BLOCKING("\r\n");
}



void Board_UART_Init(uint32_t baudRateHz) {
#ifdef TEST_HARDWARE
    (void)(baudRateHz);
#else
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
#endif
}

void Board_CAN_Init(uint32_t baudRateHz){
#ifndef TEST_HARDWARE
    CAN_Init(baudRateHz);
    CAN_mode_request = BMS_SSM_MODE_STANDBY;
#else
    UNUSED(baudRateHz);
#endif
}

void Board_LED_On(uint8_t led_gpio, uint8_t led_pin) {
#ifndef TEST_HARDWARE
    Chip_GPIO_SetPinOutHigh(LPC_GPIO, led_gpio, led_pin);
#else
    UNUSED(led_gpio); UNUSED(led_pin);
#endif
}

void Board_LED_Off(uint8_t led_gpio, uint8_t led_pin) {
#ifndef TEST_HARDWARE
    Chip_GPIO_SetPinOutLow(LPC_GPIO, led_gpio, led_pin);
#else
    UNUSED(led_gpio); UNUSED(led_pin);
#endif
}

void Board_LED_Toggle(uint8_t led_gpio, uint8_t led_pin) {
#ifndef TEST_HARDWARE
    Chip_GPIO_SetPinState(LPC_GPIO, led_gpio, led_pin, 
        1 - Chip_GPIO_GetPinState(LPC_GPIO, led_gpio, led_pin));
#else
    UNUSED(led_gpio); UNUSED(led_pin);
#endif
}

void Board_Headroom_Init(void){
#ifndef TEST_HARDWARE
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_HEADROOM, IOCON_FUNC1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, HEADROOM);
#endif
}

void Board_Headroom_Toggle(void){
#ifndef TEST_HARDWARE
    // Chip_GPIO_SetPinState(LPC_GPIO, HEADROOM, 1 - Chip_GPIO_GetPinState(LPC_GPIO, HEADROOM));
#endif
}

bool Board_Switch_Read(uint8_t gpio_port, uint8_t pin) {
#ifndef TEST_HARDWARE
    return Chip_GPIO_GetPinState(LPC_GPIO, gpio_port, pin);
#else
    UNUSED(gpio_port); UNUSED(pin);
    return true; // Modifiy to simulate switch behavior
#endif
}

void Board_Contactors_Close(bool close_contactors) {
    //TODO: implement function
    (void)(close_contactors);
}

bool Board_Contactors_IsClosed(void) {
    //TODO: implement function
    return false;
}

void Board_Init_EEPROM(void) {

}

void Board_GPIO_Init(void) {
#ifndef TEST_HARDWARE
    Chip_GPIO_Init(LPC_GPIO);

    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED0);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED1);
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED2);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_LED2, IOCON_FUNC1);	/* Select function PIO1_3 */ 
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, CTR_SWTCH);
#ifdef FSAE_DRIVERS
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAULT_GPIO);
    /* Select function PIO3_0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_FSAE_FAULT_GPIO, IOCON_FUNC0); 
#endif //FSAE_DRIVERS

    Chip_GPIO_WriteDirBit(LPC_GPIO, LED0, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED1, true);
    Chip_GPIO_WriteDirBit(LPC_GPIO, LED2, true);
#ifdef FSAE_DRIVERS
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FSAE_FAULT_GPIO);
#endif // FSAE_DRIVERS
    Board_Headroom_Init();

    // Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_CTR_SWTCH, IOCON_MODE_PULLUP);

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
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MISO0 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MOSI0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_6, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK0 */
    Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO0_6);
#endif

}

bool Board_LTC6804_Init(PACK_CONFIG_T *pack_config, uint32_t *cell_voltages_mV) {
#ifdef TEST_HARDWARE
    UNUSED(pack_config); UNUSED(cell_voltages_mV);
    return true;
#else
    if (_ltc6804_initialized) return true;

    if (_ltc6804_init_state == LTC6804_INIT_NONE) {
        ltc6804_config.pSSP = LPC_SSP0;
        ltc6804_config.baud = LTC6804_BAUD;
        ltc6804_config.cs_gpio = 0;
        ltc6804_config.cs_pin = 2;

        ltc6804_config.num_modules = pack_config->num_modules;
        ltc6804_config.module_cell_count = pack_config->module_cell_count;

        ltc6804_config.min_cell_mV = pack_config->cell_min_mV;
        ltc6804_config.max_cell_mV = pack_config->cell_max_mV;

        ltc6804_config.adc_mode = LTC6804_ADC_MODE_NORMAL;
        
        ltc6804_state.xf = &ltc6804_xf_setup;
        ltc6804_state.tx_buf = ltc6804_tx_buf;
        ltc6804_state.rx_buf = ltc6804_rx_buf;
        ltc6804_state.cfg = ltc6804_cfg;
        ltc6804_state.bal_list = ltc6804_bal_list;

        ltc6804_adc_res.cell_voltages_mV = cell_voltages_mV;

        ltc6804_owt_res.failed_wire = 0;
        ltc6804_owt_res.failed_module = 0;

        _ltc6804_gcv = false;
        _ltc6804_last_gcv = 0;
        _ltc6804_gcv_tick_time = 100;
        _ltc6804_owt = true;
        _ltc6804_last_owt = 0;
        _ltc6804_owt_tick_time = 60000;

        LTC6804_Init(&ltc6804_config, &ltc6804_state, msTicks);

        _ltc6804_init_state = LTC6804_INIT_CFG;
    } else if (_ltc6804_init_state == LTC6804_INIT_CFG) { 
        bool res = Board_LTC6804_CVST();
        if (res) {
            _ltc6804_init_state = LTC6804_INIT_CVST;
        }
    } else if (_ltc6804_init_state == LTC6804_INIT_CVST) {
        bool res = Board_LTC6804_OpenWireTest();
        if (res) {
            _ltc6804_init_state = LTC6804_INIT_DONE;
        }
    } else if (_ltc6804_init_state == LTC6804_INIT_DONE) {
        _ltc6804_initialized = true;
        _ltc6804_init_state = 0;
        return true;
    }

    return false;
#endif
}

void Board_LTC6804_DeInit(void) {
#ifndef TEST_HARDWARE
    _ltc6804_initialized = false;
    _ltc6804_init_state = LTC6804_INIT_NONE;
#endif
}

void Board_Init_Drivers(void) {

}

void Board_LTC6804_ProcessInputs(BMS_PACK_STATUS_T *pack_status) {
    Board_LTC6804_GetCellVoltages(pack_status);
    Board_LTC6804_GetCellTemperatures(pack_status);
    Board_LTC6804_OpenWireTest();
}

void Board_LTC6804_ProcessOutput(bool *balance_req) {
    Board_LTC6804_UpdateBalanceStates(balance_req);
}

void Board_LTC6804_GetCellVoltages(BMS_PACK_STATUS_T* pack_status) {
#ifdef TEST_HARDWARE
    UNUSED(pack_status);
    return;
#else

    if (msTicks - _ltc6804_last_gcv > _ltc6804_gcv_tick_time) {
        _ltc6804_gcv = true;
    }

    if (!_ltc6804_gcv) {
        return;
    }

    LTC6804_STATUS_T res = LTC6804_GetCellVoltages(&ltc6804_config, &ltc6804_state, &ltc6804_adc_res, msTicks);
    switch (res) {
        case LTC6804_FAIL:
            Board_Println("Get Vol FAIL");
            break;
        case LTC6804_PEC_ERROR:
            Board_Println("Get Vol PEC_ERROR");
            Error_Assert(ERROR_LTC6804_PEC,msTicks);
            break;
        case LTC6804_PASS:
            pack_status->pack_cell_min_mV = ltc6804_adc_res.pack_cell_min_mV;
            pack_status->pack_cell_max_mV = ltc6804_adc_res.pack_cell_max_mV;
            LTC6804_ClearCellVoltages(&ltc6804_config, &ltc6804_state, msTicks); // [TODO] Use this to your advantage
            _ltc6804_gcv = false;
            _ltc6804_last_gcv = msTicks;
            Error_Pass(ERROR_LTC6804_PEC);
        case LTC6804_WAITING:
        case LTC6804_WAITING_REFUP:
            break;
        default:
            Board_Println("WTF");
    }
#endif
}

void Board_LTC6804_GetCellTemperatures(BMS_PACK_STATUS_T * pack_status) {
    if ((msTicks - board_lastThermistorShiftTime_ms) > TIME_PER_THERMISTOR_MS) {
        board_lastThermistorShiftTime_ms += TIME_PER_THERMISTOR_MS;

        // if we finished reading previous thermistor voltage, go to next thermistor
        if (!ltc6804_setMultiplexerAddressFlag && !ltc6804_getThermistorVoltagesFlag) {
            if (currentThermistor < (MAX_THERMISTORS_PER_MODULE-1)) {
                currentThermistor += 1;
            } else {
                currentThermistor = 0;
            }
            
            // set flags to true
            ltc6804_setMultiplexerAddressFlag = true;
            ltc6804_getThermistorVoltagesFlag = true;
        }

    }

#ifndef TEST_HARDWARE
     LTC6804_STATUS_T status;

    // set multiplexer address 
    // if flag is not true, skip this step
    if (ltc6804_setMultiplexerAddressFlag) {

        // initalize CLOCK and LATCH input to the shift register
        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state,
                LTC6804_SHIFT_REGISTER_CLOCK, 0, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state,
                LTC6804_SHIFT_REGISTER_LATCH, 0, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;
        
        // Get thermistor address
        uint8_t thermistorAddress;
        if (currentThermistor < GROUP_ONE_THERMISTOR_COUNT) {
            thermistorAddress = currentThermistor;
        } else {
            thermistorAddress = currentThermistor + GROUP_TWO_THERMISTOR_OFFSET;
        }
        
        // shift bits into shift resgister
        int8_t i;
        for (i=7; i>=0; i--) {
            uint8_t addressBit = (thermistorAddress & (1<<i) ) >> i;
            status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 
                    LTC6804_SHIFT_REGISTER_DATA_IN, addressBit, msTicks);
            Board_HandleLtc6804Status(status);
            if (status != LTC6804_PASS) return;

            status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state,
                    LTC6804_SHIFT_REGISTER_CLOCK, 1, msTicks);
            Board_HandleLtc6804Status(status);
            if (status != LTC6804_PASS) return;

            status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state,
                    LTC6804_SHIFT_REGISTER_CLOCK, 0, msTicks);
            Board_HandleLtc6804Status(status);
            if (status != LTC6804_PASS) return;

        }

        // Latch the outputs
        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 
                LTC6804_SHIFT_REGISTER_LATCH, 1, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 
                LTC6804_SHIFT_REGISTER_LATCH, 0, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        // Finished setting multiplexer address. Reset flag
        ltc6804_setMultiplexerAddressFlag = false;
        
    }


    // Get thermistor voltages
    // if flag is not true, return
    if (!ltc6804_getThermistorVoltagesFlag) {
        return;
    }
    
    uint32_t gpioVoltages[MAX_NUM_MODULES * LTC6804_GPIO_COUNT];
    status = LTC6804_GetGPIOVoltages(&ltc6804_config, &ltc6804_state, gpioVoltages, 
            msTicks);
    Board_HandleLtc6804Status(status);
    if (status != LTC6804_PASS) return;

    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            pack_status);

    // Finished getting thermistor voltages. Reset flag
    ltc6804_getThermistorVoltagesFlag = false;
    
#else 
    UNUSED(pack_status);
#endif //TEST_HARDWARE
}

#ifndef TEST_HARDWARE
void Board_HandleLtc6804Status(LTC6804_STATUS_T status) {
    switch (status) {
        case LTC6804_WAITING:
            break;
        case LTC6804_PASS:
            Error_Pass(ERROR_LTC6804_PEC);
            break;
        case LTC6804_FAIL:
            Board_Println("LTC6804 fail"); 
            break;
        case LTC6804_PEC_ERROR:
            Board_Println("LTC6804 PEC_ERROR");
            Error_Assert(ERROR_LTC6804_PEC, msTicks);
        case LTC6804_WAITING_REFUP:
            break;
        default:
            Board_Println("Entered default case in Board_HandleLtc6804Status(). You should never reach here");

    }
}
#endif //TEST_HARDWARE

void Board_PrintThermistorVoltages(uint8_t module, BMS_PACK_STATUS_T * pack_status) {
#ifndef TEST_HARDWARE
    uint8_t i;
    for (i=0; i<MAX_THERMISTORS_PER_MODULE; i++) {
        const uint8_t stringLength = 8;
        const uint8_t base10 = 10;
        char temperatureString[stringLength];
        itoa(pack_status->cell_temperatures_mV[module*MAX_THERMISTORS_PER_MODULE+i], 
                temperatureString, base10);
        Board_Print_BLOCKING(temperatureString);
        Board_Print_BLOCKING(",");
    }
    Board_Print_BLOCKING("\r\n");
#else
    UNUSED(module);
    UNUSED(pack_status);
#endif //TEST_HARDWARE
}

//[TODO] check saftey 
bool Board_LTC6804_CVST(void) {
#ifdef TEST_HARDWARE
    return false;
#else
    LTC6804_STATUS_T res;
    res = LTC6804_CVST(&ltc6804_config, &ltc6804_state, msTicks);

    switch (res) {
        case LTC6804_FAIL:
            Board_Println("CVST FAIL");
            Error_Assert(ERROR_LTC6804_CVST, msTicks);
            return false;
        case LTC6804_PEC_ERROR:
            Board_Println("CVST PEC_ERROR");
            Error_Assert(ERROR_LTC6804_PEC, msTicks);
            return false;
        case LTC6804_PASS:
            Board_Println("CVST PASS");
            Error_Pass(ERROR_LTC6804_CVST);
            return true;
        case LTC6804_WAITING:
        case LTC6804_WAITING_REFUP:
            return false;
        default:
            Board_Println("WTF");
            return false;
    }

    return false;
#endif
}

//[TODO] add error handling
void Board_LTC6804_UpdateBalanceStates(bool *balance_req) {
#ifdef TEST_HARDWARE
    UNUSED(balance_req);
    return;
#else
    LTC6804_UpdateBalanceStates(&ltc6804_config, &ltc6804_state, balance_req, msTicks);
#endif
}

bool Board_LTC6804_ValidateConfiguration(void) {
#ifdef TEST_HARDWARE
    return false;
#else
    Board_Print("Initializing LTC6804. Verifying..");
    if (!LTC6804_VerifyCFG(&ltc6804_config, &ltc6804_state, msTicks)) {
        Board_Print(".FAIL. ");
        return false;
    } else {
        Board_Print(".PASS. ");
        return true;
    }
#endif
}

bool Board_LTC6804_OpenWireTest(void) {
#ifdef TEST_HARDWARE
    return true; // Change to simulate during test
#else

    if (msTicks - _ltc6804_last_owt > _ltc6804_owt_tick_time) {
        _ltc6804_owt = true;
    }

    if (!_ltc6804_owt) {
        return false;
    }

    LTC6804_STATUS_T res;
    res = LTC6804_OpenWireTest(&ltc6804_config, &ltc6804_state, &ltc6804_owt_res, msTicks);

    switch (res) {
        case LTC6804_FAIL:
            Board_Print("OWT FAIL, mod=");
            itoa(ltc6804_owt_res.failed_module, str, 10);
            Board_Print(str);
            Board_Print(" wire=");
            itoa(ltc6804_owt_res.failed_wire, str, 10);
            Board_Println(str);
            Error_Assert(ERROR_LTC6804_OWT, msTicks);
            return false;
        case LTC6804_PEC_ERROR:
            Board_Println("OWT PEC_ERROR");
            Error_Assert(ERROR_LTC6804_PEC,msTicks);
            return false;
        case LTC6804_PASS:
            Board_Println("OWT PASS");
            _ltc6804_owt = false;
            _ltc6804_last_owt = msTicks;
            Error_Pass(ERROR_LTC6804_OWT);
            return true;
        case LTC6804_WAITING:
        case LTC6804_WAITING_REFUP:
            // Board_Println("*");
            return false;
        default:
            Board_Println("WTF");
            return false;
    }
#endif
}

#ifndef TEST_HARDWARE

void Board_GetModeRequest(const CONSOLE_OUTPUT_T * console_output, BMS_INPUT_T* bms_input) {
    BMS_SSM_MODE_T console_mode_request = BMS_SSM_MODE_STANDBY;
    if (console_output -> valid_mode_request) {
            console_mode_request = console_output->mode_request;
            bms_input->balance_mV = console_output->balance_mV;
    } else {
            console_mode_request = BMS_SSM_MODE_STANDBY;
    }

    if (console_mode_request == BMS_SSM_MODE_STANDBY) {
        bms_input->mode_request = CAN_mode_request;
    } else if (CAN_mode_request == BMS_SSM_MODE_STANDBY) {
        bms_input->mode_request = console_mode_request;
    } else if (console_mode_request == CAN_mode_request) {
        bms_input->mode_request = console_mode_request;
    } else {
        Board_Println("Error! Illegal combination of CAN mode request and console mode request");
        //TODO: go into error state
    }

#ifdef PRINT_MODE_REQUESTS
    bool print_debug_message;
    if ((msTicks - last_get_mode_request_debug_message) > 1000) {
        Board_Print("console_mode_request: ");
        switch(console_mode_request) {
            case BMS_SSM_MODE_STANDBY:
                Board_Print("standby    ");
                break;
            case BMS_SSM_MODE_DISCHARGE:
                Board_Print("discharge    ");
                break;
            default:
                Board_Print("other state");
        }
        Board_Print("CAN_mode_request: ");
        switch (CAN_mode_request) {
            case BMS_SSM_MODE_STANDBY:
                Board_Print("standby    ");
                break;
            case BMS_SSM_MODE_DISCHARGE:
                Board_Print("discharge    ");
                break;
            default:
                Board_Print("other state");
        }
        Board_Print("bms_input->mode_request: ");
        switch (bms_input->mode_request) {
            case BMS_SSM_MODE_STANDBY:
                Board_Println("standby    ");
                break;
            case BMS_SSM_MODE_DISCHARGE:
                Board_Print("discharge\r\n");
                break;
            default:
                Board_Println("other state");
        }

        print_debug_message = true;
        last_get_mode_request_debug_message = msTicks;
    } else {
        print_debug_message = false;
    }
#endif //PRINT_MODE_REQUESTS

}

/**
 * @details Reads CAN messages and empties CAN ring buffer. Mutates bms_input
 *          to reflect CAN messages received
 * 
 * @param bms_input data strcuture representing BMS inputs
 */
// [TODO] Refactor to case
void Board_CAN_ProcessInput(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output) {
    CCAN_MSG_OBJ_T rx_msg;
    if (CAN_Receive(&rx_msg) != NO_RX_CAN_MESSAGE) {
        latest_vcu_heartbeat_time = msTicks;
        if (rx_msg.mode_id == NLG5_STATUS) { 
            // [TODO] use info from brusa message
        } else if (rx_msg.mode_id == NLG5_ACT_I) {
            NLG5_ACT_I_T act_i;
            Brusa_DecodeActI(&act_i, &rx_msg);
            bms_input->pack_status->pack_current_mA = act_i.output_cAmps*10; // [TODO] Consider using current sense as well
            bms_input->pack_status->pack_voltage_mV = act_i.output_mVolts;

            // If current > requested current + thresh throw error
        } else if (rx_msg.mode_id == NLG5_ACT_II) {

        } else if (rx_msg.mode_id == NLG5_TEMP) {

        } else if (rx_msg.mode_id == NLG5_ERR && bms_output->charge_req->charger_on) {
            // [TODO] distinguish errors
            if (!Brusa_CheckErr(&rx_msg)) { // We've recevied a Brusa Error Message
                // if (output->charge_req->charger_on) {
                    Error_Assert(ERROR_BRUSA, msTicks);
                // }
                // We should try to clear but also assert error for count
                // Timing idea: Brusa error msg happens as often as ctrl message
            } else {
                Error_Pass(ERROR_BRUSA);
            }
#ifdef FSAE_DRIVERS
        } else if (rx_msg.mode_id == VCU_HEARTBEAT__id) {
            //TODO: create helper function that parses VCU heartbeat
#ifdef PRINT_CAN_MESSAGES
            Board_Print("VCU Heartbeat    ");
#endif //PRINT_CAN_MESSAGES
            if ((rx_msg.data[0]>>7) == ____VCU_HEARTBEAT__STATE__DISCHARGE) {
#ifdef PRINT_CAN_MESSAGES
                Board_Println("state: discharge    ");
#endif //PRINT_CAN_MESSAGES
                CAN_mode_request = BMS_SSM_MODE_DISCHARGE;
            } else if ((rx_msg.data[0])>>7 == ____VCU_HEARTBEAT__STATE__STANDBY) {
#ifdef PRINT_CAN_MESSAGES
                Board_Println("state: standby    "); 
#endif //PRINT_CAN_MESSAGES
                CAN_mode_request = BMS_SSM_MODE_STANDBY;
            } else {
                DEBUG_Print("Unrecognized VCU heartbeat state. You should never reach here.");
            }
        } else if (rx_msg.mode_id == VCU_DISCHARGE_REQUEST__id) {
            //TODO: create helper function that parses VCU discharge request message
            if ((rx_msg.data[0]>>7) == ____VCU_DISCHARGE_REQUEST__DISCHARGE_REQUEST__ENTER_DISCHARGE) {
                CAN_mode_request = BMS_SSM_MODE_DISCHARGE;
                //set flag that tells Board_CAN_ProcessOutputs() to send discharge response message
                received_discharge_request = 1;
            } else {
                DEBUG_Print("Invalid discharge request. You should never reach here");
            }
#endif //FSAE_DRIVERS
        } else {
            // [TODO] handle other types of CAN messages
        }
    }
    
    const uint32_t vcu_heartbeat_timeout = 10000;
    if ( (msTicks - latest_vcu_heartbeat_time) > vcu_heartbeat_timeout) {
        CAN_mode_request = BMS_SSM_MODE_STANDBY;
    }
}

static uint32_t _last_brusa_ctrl = 0; // [TODO] Refactor dummy


// [TODO] Make timing.h that has this (or board.h)
    // Make pythong script generate
#define NLG5_CTL_DLY_mS 99
// #define NLG5_CTL_DLY_mS 1000
#ifdef DEBUG_ENABLE
    #define NLG5_CTL_STATE_REQ(curr_mode) (curr_mode==ASDF && conditional)
#else
    #define NLG5_CTL_STATE_REQ(curr_mode) ()
#endif

void Board_CAN_ProcessOutput(BMS_INPUT_T *bms_input, BMS_STATE_T * bms_state, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_state);
    
    // Easy way to turn off charger in case of accident
    if (bms_output->charge_req->charger_on && msTicks - _last_brusa_ctrl >= NLG5_CTL_DLY_mS) {
        NLG5_CTL_T brusa_control;
        CCAN_MSG_OBJ_T temp_msg;
        brusa_control.enable = 1;
        brusa_control.ventilation_request = 0;
        brusa_control.max_mains_cAmps = 1000; // [TODO] Magic Numbers
        brusa_control.output_mV = bms_output->charge_req->charge_voltage_mV;
        brusa_control.output_cA = bms_output->charge_req->charge_current_mA / 10;
        const ERROR_STATUS_T * stat = Error_GetStatus(ERROR_BRUSA);
        if (stat->handling) {
            brusa_control.clear_error = stat->count & 1;
            brusa_control.output_mV = 0;
            brusa_control.output_cA = 0;
            bms_input->charger_on = false;
        } else {
             brusa_control.clear_error = 0;
             bms_input->charger_on = true;
        }
        // brusa_control.output_mV = 0;
        // brusa_control.output_cA = 0;
        Brusa_MakeCTL(&brusa_control, &temp_msg);
        CAN_TransmitMsgObj(&temp_msg);
        _last_brusa_ctrl = msTicks;

        // bms_input->charger_on = true;
        // Board_Print("B_V: ");
        // Board_PrintNum(bms_output->charge_req->charge_voltage_mV, 10);
        // Board_Println("");
        // Board_Print("B_C: ");
        // Board_PrintNum(bms_output->charge_req->charge_current_mA, 10);
        // Board_Println("");
    }

    if (!bms_output->charge_req->charger_on) {
        bms_input->charger_on = false;
    }

#ifdef FSAE_DRIVERS
    //Send BMS Heartbeat
    if (msTicks - last_bms_heartbeat_time > BMS_HEARTBEAT_PERIOD) {
        Send_BMS_Heartbeat(bms_state);
        last_bms_heartbeat_time = msTicks;
    }
    
    //Send BMS Discharge response
    if (received_discharge_request) {
        Send_BMS_Discharge_Response(bms_state->curr_mode);
        received_discharge_request = 0;
    } 
#endif //FSAE_DRIVERS

    if (CAN_GetErrorStatus()) {
        Board_Println("CAN Error");
        Error_Assert(ERROR_CAN, msTicks);
        CAN_ResetPeripheral();
        Board_CAN_Init(CAN_BAUD);
    }
}
#endif

