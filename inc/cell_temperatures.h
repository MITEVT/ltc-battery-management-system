#ifndef CELL_TEMPERATURES_H
#define CELL_TEMPERATURES_H

// ltc-battery-management-system
#include "state_types.h"
#include "config.h"

// constants
#define NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS 5

static const uint8_t thermistorAddresses[MAX_THERMISTORS_PER_MODULE] = {
    // First group of thermistors
    0b00000,
    0b00001,
    0b00010,
    0b00011,
    0b00100,
    0b00101,
    0b00110,
    0b00111,
    0b01000,
    0b01001,
    0b01010,
    0b01011,
    0b01100,
    // Second group of thermistors
    0b10000,
    0b10001,
    0b10010,
    0b10011,
    0b10100,
    0b10101,
    0b10110,
    0b10111,
    0b11000,
    0b11001,
    0b11010
};

/****************************************************************************************
 * Public Functions
 * *************************************************************************************/

/**
 * @details get cell temperature of next thermistor
 *
 * @param currentThermistor number of thermistor currently selected
 */
void CellTemperatures_Step(uint8_t * currentThermistor);

/**
 * @details gets bit bit of current thermistor address 
 *
 * @param currentThermistor number of thermistor currently selected
 * @param bit bit of current thermistor address we want to obtain
 * @return bit of current thermistor address
 */
uint8_t CellTemperatures_GetThermistorAddressBit(uint8_t currentThermistor, uint8_t bit);

/**
 * @details updates array of cell temperatures in pack_status with values stored in 
 *          gpioVoltages
 *
 * @param gpioVoltages array of voltages measured on each GPIO of the LTC6804 chips
 *                     gpioVoltages is structured as follows {GPIO1_module1, ..., 
 *                     GPIO5_module1, GPIO1_module2, ..., GPIO5_module2, ...}
 * @param currentThermistor number of thermistor currently selected
 * @param pack_status mutable datatype containing array of cell temperatures
 */
void CellTemperatures_UpdateCellTemperaturesArray(uint32_t * gpioVoltages, 
        uint8_t currentThermistor, BMS_PACK_STATUS_T * pack_status);

#endif //CELL_TEMPERATURES
