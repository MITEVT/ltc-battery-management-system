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
    int16_t thermistorTemperatures[MAX_NUM_MODULES];
    getThermistorTemperatures(gpioVoltages, thermistorTemperatures);

    uint8_t i;
    for (i=0; i<MAX_NUM_MODULES; i++) {
        pack_status->cell_temperatures_dC[i*MAX_THERMISTORS_PER_MODULE 
            + currentThermistor] = thermistorTemperatures[i];
    }
}

/**************************************************************************************
 * Private Functions
 * ***********************************************************************************/

void getThermistorTemperatures(uint32_t * gpioVoltages, 
        int16_t * thermistorTemperatures) {
    uint8_t i;
    for (i=0; i<MAX_NUM_MODULES; i++) {
        // calculate temperature using linear curve fit relating thermistor voltages
        // (in mV) to temperatures in (dC)
        thermistorTemperatures[i] = 
              (gpioVoltages[i*LTC6804_GPIO_COUNT]>>SHIFT_VOLTAGE) + A0;
    }
}
