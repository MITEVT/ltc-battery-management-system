// ltc-battery-management-system
#include "cell_temperatures.h"
#include "state_types.h"
#include "config.h"
#include "board.h"

//lpc11cx4-library
#include "lpc_types.h"

/**************************************************************************************
 * Public Functions
 * ***********************************************************************************/

void CellTemperatures_UpdateCellTemperaturesArray(uint32_t * gpioVoltages, 
        uint8_t currentThermistor, BMS_PACK_STATUS_T * pack_status) {
    uint8_t i;
    for (i=0; i<MAX_NUM_MODULES; i++) {
        uint32_t thermistorTemperature = gpioVoltages[i*LTC6804_GPIO_COUNT];
        pack_status->cell_temperatures_mV[i*MAX_THERMISTORS_PER_MODULE 
            + currentThermistor] = thermistorTemperature;
    }
}
