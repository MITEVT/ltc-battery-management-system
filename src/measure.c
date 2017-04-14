#include "console.h"
#include "board.h"
#include "measure.h"

#define VOLTAGES_PRINT_PERIOD_ms 200
static uint32_t lastVoltagesPrintTime = 0;

void Output_Measurements(
        CONSOLE_OUTPUT_T *console_output, 
        BMS_INPUT_T* bms_input, 
        BMS_STATE_T* bms_state,
        uint32_t msTicks
) {

    char tempstr[20];

    if(console_output->measure_on) {
        if(console_output->measure_temp) {
            Board_Println("Not 1 implemented yet!");
        }

        if(console_output->measure_voltage && (msTicks - lastVoltagesPrintTime) > VOLTAGES_PRINT_PERIOD_ms) {
            uint32_t i, j, idx;
            idx = 0;
            // Board_Print_BLOCKING("cvs: ");
            for (i = 0; i < bms_state->pack_config->num_modules; i++) {
                for (j = 0; j < bms_state->pack_config->module_cell_count[i]; j++) {
                    utoa(bms_input->pack_status->cell_voltages_mV[idx], tempstr, 10);
                    Board_Print_BLOCKING(tempstr);
                    if(j != bms_state->pack_config->module_cell_count[i] - 1 || i != bms_state->pack_config->num_modules - 1) {
                        Board_Print_BLOCKING(",");
                    }
                    idx++;
                }
            }
            Board_Print_BLOCKING("\n");
            lastVoltagesPrintTime = msTicks;
        }

        if(console_output->measure_packcurrent) {
            Board_Println("Not 2 implemented yet!");
        }

        if(console_output->measure_packvoltage) {
            Board_Println("Not 3 implemented yet!");
        }
    }
}
