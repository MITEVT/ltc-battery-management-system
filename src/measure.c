#include "console.h"
#include "board.h"
#include "measure.h"

#define VOLTAGES_PRINT_PERIOD_ms 1000
#define TEMPS_PRINT_PERIOD_ms 10000
#define PACK_CURRENT_PRINT_PERIOD_ms 1000
#define PACK_VOLTAGE_PRINT_PERIOD_ms 1000
static uint32_t lastVoltagesPrintTime = 0;
static uint32_t lastTempsPrintTime = 0;
static uint32_t lastPackCurrentPrintTime = 0;
static uint32_t lastPackVoltagePrintTime = 0;

void Output_Measurements(
        CONSOLE_OUTPUT_T *console_output, 
        BMS_INPUT_T* bms_input, 
        BMS_STATE_T* bms_state,
        uint32_t msTicks
) {

    char tempstr[20];

    if(console_output->measure_on) {
        if(console_output->measure_temp && (msTicks - lastTempsPrintTime) > TEMPS_PRINT_PERIOD_ms) {
            uint8_t module;
            for (module = 0; module < bms_state->pack_config->num_modules; module++) {
                utoa(msTicks, tempstr, 10); // print msTicks
                Board_Print_BLOCKING(tempstr);
                Board_Print_BLOCKING(",");

                utoa(module, tempstr, 10);
                Board_Print_BLOCKING("temps,");
                Board_Print_BLOCKING(tempstr); // print module

                Board_Print_BLOCKING(",");

                Board_PrintThermistorTemperatures(module, bms_input->pack_status);
            }
            lastTempsPrintTime = msTicks;
        }

        if(console_output->measure_voltage && (msTicks - lastVoltagesPrintTime) > VOLTAGES_PRINT_PERIOD_ms) {
            uint8_t i, j;
            uint32_t idx;
            idx = 0;
            for (i = 0; i < bms_state->pack_config->num_modules; i++) {

                utoa(msTicks, tempstr, 10); // print msTicks
                Board_Print_BLOCKING(tempstr);
                Board_Print_BLOCKING(",");

                utoa(i, tempstr, 10);
                Board_Print_BLOCKING("cvs,");
                Board_Print_BLOCKING(tempstr); // print module
                Board_Print_BLOCKING(",");

                for (j = 0; j+1 < bms_state->pack_config->module_cell_count[i]; j++) {
                    utoa(bms_input->pack_status->cell_voltages_mV[idx], tempstr, 10);
                    Board_Print_BLOCKING(tempstr);
                    Board_Print_BLOCKING(",");
                    idx++;
                }
                utoa(bms_input->pack_status->cell_voltages_mV[idx], tempstr, 10);
                Board_Print_BLOCKING(tempstr);
                idx++;
                Board_Print_BLOCKING("\n");
            }
            lastVoltagesPrintTime = msTicks;
        }

        bool printPackCurrent = (msTicks - lastPackCurrentPrintTime) > 
                PACK_CURRENT_PRINT_PERIOD_ms;
        if(console_output->measure_packcurrent && printPackCurrent) {
            utoa(msTicks, tempstr, 10); // print msTicks
            Board_Print_BLOCKING(tempstr);
            Board_Print_BLOCKING(",pcurr,");
            utoa(bms_input->pack_status->pack_current_mA, tempstr, 10);
            Board_Println_BLOCKING(tempstr); // print pack current
            lastPackCurrentPrintTime = msTicks;
        }

        bool printPackVoltage = (msTicks - lastPackVoltagePrintTime) > 
                PACK_VOLTAGE_PRINT_PERIOD_ms;
        if(console_output->measure_packvoltage && printPackVoltage) {
            utoa(msTicks, tempstr, 10); // print msTicks
            Board_Print_BLOCKING(tempstr);
            Board_Print_BLOCKING(",pvolt,");
            utoa(bms_input->pack_status->pack_voltage_mV, tempstr, 10);
            Board_Println_BLOCKING(tempstr); // print pack voltage
            lastPackVoltagePrintTime = msTicks;
        }
    }
}
