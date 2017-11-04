// ltc-battery-management-system
#include "board.h"
#include "error_handler.h"
#include "can.h"
#include "FluxCAN.h"

// C libraries
#include <string.h>


#ifdef FSAE_DRIVERS
    #include "cell_temperatures.h"

#else // FSAE_DRIVERS

    #include "evt_can.h"
    #include "brusa.h"

#endif // FSAE_DRIVERS

const uint32_t OscRateIn = 24000000;

#define UART_BUFFER_SIZE 100
#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))

#define UNUSED(x) (void)(x)

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
static uint32_t _precharge_event = 0;

static bool _ltc6804_initialized;
static LTC6804_INIT_STATE_T _ltc6804_init_state;

static char str[10];

#ifdef FSAE_DRIVERS

//Cell temperature sensing stuff
static uint32_t board_lastThermistorShiftTime_ms = 0;
uint8_t currentThermistor = 0;

//Cell temperature sensing stuff
static bool ltc6804_setMultiplexerAddressFlag = false;
static bool ltc6804_getThermistorVoltagesFlag = false;

#endif // FSAE_DRIVERS

#endif // TEST_HARDWARE

volatile uint32_t msTicks;
static uint32_t last_bms_heartbeat_time = 0;
static uint32_t last_bms_errors_time = 0;
static uint32_t last_bms_cellTemps_time = 0;
static uint32_t last_bms_packStatus_time = 0;


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

#endif // TEST_HARDWARE

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

    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */
 
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
    CAN_SetMask1(0, 0x7FF); // Don't accept messages
    CAN_SetMask2(0, 0); // Accept all messages
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

void Board_FAULT_Off(void) {
#ifndef TEST_HARDWARE
    Chip_GPIO_SetPinOutLow(LPC_GPIO, FAULT);
#else
    UNUSED(led_gpio); UNUSED(led_pin);
#endif
}

void Board_FAULT_On(void) {
#ifndef TEST_HARDWARE
    Chip_GPIO_SetPinOutHigh(LPC_GPIO, FAULT);
#else
    UNUSED(led_gpio); UNUSED(led_pin);
#endif
}

bool Board_OFF_Read(void) {
    #ifndef TEST_HARDWARE
    return !Chip_GPIO_GetPinState(LPC_GPIO, OFF_IN);
#else
    UNUSED(led_gpio); UNUSED(led_pin);
    return false;
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

void Board_Headroom_Toggle(void){
#ifndef TEST_HARDWARE
#ifdef FSAE_DRIVERS
    // Do nothing here, we don't care
#else // FSAE_DRIVERS
    Evt_Headroom_Toggle();
#endif // FSAE_DRIVERS
#endif // TEST_HARDWARE
}


void Board_Contactors_Set(bool close_contactors, BMS_PACK_STATUS_T *status) {
    // if (close_contactors && (int32_t)(status->pack_voltage_mV*9/10000) - (int32_t)status->car_bus_V < 0) {
    //     Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_PRE, true);
    //     Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_P, false);
    //     Board_Println("Precharge Done");
    // } else
    if (close_contactors && (msTicks - _precharge_event > PRECHARGE_TIME)) {
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_PRE, true);
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_P, true);
        // Board_Println("Precharge Done");
    } else if (close_contactors) {
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_PRE, true);
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_P, false);
        // Board_Println("Precharge");
    } else { // Close contactor is false
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_PRE, false);
        Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_P, false);
        _precharge_event = msTicks;
    }
}

bool Board_Contactors_Closed(void) {
    return false;
}

void Board_GPIO_Init(void) {
#ifndef TEST_HARDWARE
    Chip_GPIO_Init(LPC_GPIO);

    // Initialize devices common to both.
    // Note that values of the LED constants differ between devices.

    // LED1
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED1);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_LED1, IOCON_LED1_FUNC);
    Chip_GPIO_SetPinState(LPC_GPIO, LED1, false);

    // LED2
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED2);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_LED2, IOCON_LED2_FUNC);
    Chip_GPIO_SetPinState(LPC_GPIO, LED2, false);


    Chip_GPIO_SetPinDIROutput(LPC_GPIO, FAULT);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_FAULT, IOCON_FAULT_FUNC);
    Chip_GPIO_SetPinState(LPC_GPIO, FAULT, false);

    // OFF
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, OFF_IN);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_OFF_IN, IOCON_OFF_IN_FUNC);

    //SSP for EEPROM
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_2, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MISO1 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_3, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* MOSI1 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_1, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK1 */

    //SSP for LTC6804
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MISO0 */ 
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MOSI0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_6, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK0 */
    Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO0_6);

    // device-specific pins initalized here

    // Contactor GPIOs
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_6, (IOCON_FUNC0 | IOCON_MODE_INACT)); /* Pre-charge enable */
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, CONTACTOR_PRE);
    Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_PRE, false);

    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_0, (IOCON_FUNC0 | IOCON_MODE_INACT)); /* Main contactor enable */
    Chip_GPIO_SetPinDIROutput(LPC_GPIO, CONTACTOR_P);
    Chip_GPIO_SetPinState(LPC_GPIO, CONTACTOR_P, false);

#endif // TEST_HARDWARE
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

void Board_LTC6804_ProcessInputs(BMS_PACK_STATUS_T *pack_status, BMS_STATE_T* bms_state) {
    Board_LTC6804_GetCellVoltages(pack_status);
    Board_LTC6804_GetCellTemperatures(pack_status, bms_state->pack_config->num_modules);
    Board_LTC6804_OpenWireTest();
    if(Board_OFF_Read()){
        Error_Assert(ERROR_NOMINAL_SHUTDOWN,msTicks);
    }
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

void Board_LTC6804_GetCellTemperatures(BMS_PACK_STATUS_T * pack_status, uint8_t num_modules) {
#ifndef TEST_HARDWARE
    if ((msTicks - board_lastThermistorShiftTime_ms) > TIME_PER_THERMISTOR_MS) {
        board_lastThermistorShiftTime_ms = msTicks;

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

    LTC6804_STATUS_T status;

    // set multiplexer address 
    // if flag is not true, skip this step
    if (ltc6804_setMultiplexerAddressFlag) {
        // [TODO] send bit vector of updated GPIO
        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 4, currentThermistor & 1, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 5, (currentThermistor >> 1) & 1, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 2, (currentThermistor >> 2) & 1, msTicks);
        Board_HandleLtc6804Status(status);
        if (status != LTC6804_PASS) return;

        status = LTC6804_SetGPIOState(&ltc6804_config, &ltc6804_state, 3, (currentThermistor >> 3) & 1, msTicks);
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
    status = LTC6804_GetGPIOVoltages(&ltc6804_config, &ltc6804_state, gpioVoltages, msTicks);
    Board_HandleLtc6804Status(status);
    if (status != LTC6804_PASS) return;

    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            pack_status, num_modules);

    // Finished getting thermistor voltages. Reset flag
    ltc6804_getThermistorVoltagesFlag = false;

    if (currentThermistor == MAX_THERMISTORS_PER_MODULE - 1) {
        CellTemperatures_UpdateMaxMinAvgCellTemperatures(pack_status, num_modules);
    }
#else 
    UNUSED(pack_status);
    UNUSED(num_modules);
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
#ifdef FSAE_DRIVERS
            Error_Assert(ERROR_CONTROL_FLOW, msTicks);
#endif

    }
}
#endif //TEST_HARDWARE

void Board_PrintThermistorTemperatures(uint8_t module, BMS_PACK_STATUS_T * pack_status) {
#ifndef TEST_HARDWARE
    uint8_t i;
    char temperatureString[8];

    for (i=0; i+1<MAX_THERMISTORS_PER_MODULE; i++) {
        itoa(pack_status->cell_temperatures_dC[module*MAX_THERMISTORS_PER_MODULE+i], temperatureString, 10);
        Board_Print_BLOCKING(temperatureString);
        Board_Print_BLOCKING(",");
    }
    itoa(pack_status->cell_temperatures_dC[module*MAX_THERMISTORS_PER_MODULE+i], temperatureString, 10);
    Board_Print_BLOCKING(temperatureString);

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
            if (ltc6804_owt_res.failed_module == 0 && ltc6804_owt_res.failed_wire == 12) {
                Error_Pass(ERROR_LTC6804_OWT);
                _ltc6804_owt = false;
                _ltc6804_last_owt = msTicks;
                return true;
             } else {
                Board_Print("OWT FAIL, mod=");
                itoa(ltc6804_owt_res.failed_module, str, 10);
                Board_Print(str);
                Board_Print(" wire=");
                itoa(ltc6804_owt_res.failed_wire, str, 10);
                Board_Println(str);
                Error_Assert(ERROR_LTC6804_OWT, msTicks);
            }
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


//[TODO] change to just go into drive and mvoe out of drive on off
void Board_GetModeRequest(const CONSOLE_OUTPUT_T * console_output, BMS_INPUT_T* bms_input) {
    BMS_SSM_MODE_T console_mode_request = BMS_SSM_MODE_STANDBY;
    if (console_output -> valid_mode_request) {
            console_mode_request = console_output->mode_request;
            bms_input->balance_mV = console_output->balance_mV;
    }
    BMS_SSM_MODE_T can_mode_request = BMS_SSM_MODE_STANDBY;

    if (console_mode_request == BMS_SSM_MODE_STANDBY) {
        bms_input->mode_request = can_mode_request;
        Error_Pass(ERROR_CONFLICTING_MODE_REQUESTS);
    } else if (can_mode_request == BMS_SSM_MODE_STANDBY) {
        bms_input->mode_request = console_mode_request;
        Error_Pass(ERROR_CONFLICTING_MODE_REQUESTS);
    } else if (console_mode_request == can_mode_request) {
        bms_input->mode_request = console_mode_request;
        Error_Pass(ERROR_CONFLICTING_MODE_REQUESTS);
    } else {
        Error_Assert(ERROR_CONFLICTING_MODE_REQUESTS, msTicks);
    }
}

/**
 * @details Reads CAN messages and empties CAN ring buffer. Mutates bms_input
 *          to reflect CAN messages received
 * 
 * @param bms_input data strcuture representing BMS inputs
 */
void Board_CAN_ProcessInput(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    CCAN_MSG_OBJ_T rx_msg;
    CAN_ERROR_T can_status;
    can_status = CAN_Receive(&rx_msg);
    if(can_status == NO_CAN_ERROR) {
        // We have something to preocess
        Error_Pass(ERROR_CAN);
    } else if (can_status == NO_RX_CAN_MESSAGE) {
        Error_Pass(ERROR_CAN);
        return;
    } else { //CAN ERRROR. Note this, 
        DEBUG_Print("CAN Error (Rx): ");
        itoa(can_status, str, 2);
        DEBUG_Print(str);
        DEBUG_Print("\r\n");
        Error_Assert(ERROR_CAN, msTicks);
        return;
    }

    switch (rx_msg.mode_id) {
        //4bytes - bus current
        //4bytes - bus voltage            

        case MOTORDATAAV_MSG_ID_2: 
            bms_input->pack_status->car_bus_V = rx_msg.data[0] | rx_msg.data[1]<<8 | rx_msg.data[2]<<16 | rx_msg.data[3]<<24;
            Board_Println("got bus voltage");
            break;
        case ARRAY_EMETER_MSG_ID:
            if (rx_msg.dlc ==8) {
                bms_input->pack_status->pack_voltage_mV = rx_msg.data[0] | rx_msg.data[1]<<8 | rx_msg.data[2]<<16 | rx_msg.data[3]<<24;
                bms_input->pack_status->pack_current_mA = rx_msg.data[4] | rx_msg.data[5]<<8 | rx_msg.data[6]<<16 | rx_msg.data[7]<<24;
                // DEBUG_Print("got pack voltage\r\n");
                // for ( i = 7; i >=0; --i)
                // {
                //     itoa(rx_msg.data[i],str,16);
                //     DEBUG_Print(str);
                //     DEBUG_Print(" ");
                // }
                // DEBUG_Print(" ");
               
            } else {
                DEBUG_Print("AHH");
            }
            break;
        case 0x016:
            if(rx_msg.dlc ==1) {
                Error_HB_rx(ERROR_MAIN_HB, msTicks);
                if (rx_msg.data[0] == 1) {
                bms_input->mode_request = BMS_SSM_MODE_DISCHARGE;
                    DEBUG_Print("dis");
                } else {
                    bms_input->mode_request = BMS_SSM_MODE_CHARGE;
                    DEBUG_Print("chrg");
                }
                
                break;
            } else {
                DEBUG_Print("AHH");
            }
            break;
    }
}


void _handle_can_error(CAN_ERROR_T err) {
    if (err == NO_CAN_ERROR || err == NO_RX_CAN_MESSAGE) {
        // Neither of these are real errors
        Error_Pass(ERROR_CAN);
        return;
    }
    else {
        Error_Assert(ERROR_CAN, msTicks);
        return;
        
    }

}

void _Send_Bms_Errors(uint32_t msTicks) {
    uint8_t error_msg[1];
    error_msg[0] = Error_ShouldHalt_Status(msTicks);
    _handle_can_error(CAN_Transmit(0x260, error_msg,1));
}

/**
 * @details Sends cell temperatures over Can
 *
 * @param pack_status datatype containing average cell temperature, maximum cell
 * temperature, max cell temperature id, minimum cell temperature, and minimum
 * cell temperature id
 */
void _Send_Bms_CellTemps(BMS_PACK_STATUS_T * pack_status) {
    uint8_t cellTemps[8];

    cellTemps[0] = pack_status->avg_cell_temp_dC & 0xFF;
    cellTemps[1] = (pack_status->avg_cell_temp_dC >> 8) & 0xFF;


    cellTemps[2] = pack_status->min_cell_temp_dC & 0xFF;
    cellTemps[3] = (pack_status->min_cell_temp_dC >> 8) & 0xFF;

    cellTemps[4] = pack_status->min_cell_temp_position & 0xFF;

    cellTemps[5] = pack_status->max_cell_temp_dC & 0xFF;
    cellTemps[6] = (pack_status->max_cell_temp_dC >> 8) & 0xFF;

    cellTemps[7] = pack_status->max_cell_temp_position & 0xFF;

    _handle_can_error(CAN_Transmit(0x160,cellTemps,8));
}

/**
 * @details Sends pack status can message (details in fsae can spec)
 *
 * @param pack_status datatype containing information about the status of the pack
 */
void _Send_Bms_PackStatus(BMS_PACK_STATUS_T * pack_status) {
    uint8_t canPackStatus[8];


    canPackStatus[0] = 0; //TODO: get actual average cell voltage
    canPackStatus[1] = 0;
    canPackStatus[2] = (pack_status->pack_cell_min_mV) & 0xFF;
    canPackStatus[3] = (pack_status->pack_cell_min_mV >> 8) & 0xFF;
    canPackStatus[4] = 0; //TODO: get actual id min cell voltage
    canPackStatus[5] = (pack_status->pack_cell_max_mV) & 0xFF;
    canPackStatus[6] = (pack_status->pack_cell_max_mV >> 8) & 0xFF;
    canPackStatus[7] = 0; //TODO: get actual id max cell voltage

    _handle_can_error(CAN_Transmit(0x161, canPackStatus,8));
}


void _Send_Bms_Heartbeat(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    uint8_t bmsHeartbeat[1];
    ERROR_T error = Error_ShouldHalt_Status(bms_input->msTicks);
    bmsHeartbeat[0] = 0;
    bmsHeartbeat[0] |= (error==ERROR_NO_ERRORS?(bms_state->curr_mode):0x05);
    _handle_can_error(CAN_Transmit(0x060,bmsHeartbeat,1));
}

void Board_CAN_ProcessOutput(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);

    UNUSED(bms_output);
    uint32_t msTicks = bms_input->msTicks;
    if ( (msTicks - last_bms_heartbeat_time) > BMS_HEARTBEAT_PERIOD) {
        last_bms_heartbeat_time = msTicks;
        _Send_Bms_Heartbeat(bms_input, bms_state, bms_output);
    }
    if ( (msTicks - last_bms_errors_time) > BMS_ERRORS_PERIOD) {
        last_bms_errors_time = msTicks;
        _Send_Bms_Errors(msTicks);
    }
    if ( (msTicks - last_bms_cellTemps_time) > BMS_CELL_TEMPS_PERIOD) {
        last_bms_cellTemps_time = msTicks;
        _Send_Bms_CellTemps(bms_input->pack_status);
    }
    if ( (msTicks - last_bms_packStatus_time) > BMS_PACK_STATUS_PERIOD) {
        last_bms_packStatus_time = msTicks;
        _Send_Bms_PackStatus(bms_input->pack_status);
    }

}




#endif // TEST_HARDWARE


