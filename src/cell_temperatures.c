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

void CellTemperatures_UpdateMaxMinAvgCellTemperatures(BMS_PACK_STATUS_T * pack_status) {
#ifdef FSAE_DRIVERS

    int16_t maxCellTemperature = INT16_MIN;
    int16_t minCellTemperature = INT16_MAX;
    int16_t cellTemperaturesSum = 0;
    uint16_t maxCellTempraturePosition = 0;
    uint16_t minCellTemperaturePosition = 0;

    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        cellTemperaturesSum += pack_status->cell_temperatures_dC[i];
        if (pack_status->cell_temperatures_dC[i] > maxCellTemperature) {
            maxCellTemperature = pack_status->cell_temperatures_dC[i];
            maxCellTempraturePosition = i;
        }
        if (pack_status->cell_temperatures_dC[i] <minCellTemperature) {
            minCellTemperature = pack_status->cell_temperatures_dC[i];
            minCellTemperaturePosition = i;
        }
    }

    //update pack_status
    pack_status->max_cell_temp_dC = maxCellTemperature;
    pack_status->min_cell_temp_dC = minCellTemperature;
    pack_status->avg_cell_temp_dC = 
            cellTemperaturesSum/(MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE);
    pack_status->max_cell_temp_position = maxCellTempraturePosition;
    pack_status->min_cell_temp_position = minCellTemperaturePosition;

#else
    UNUSED(pack_status);
#endif //FSAE_DRIVERS
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
