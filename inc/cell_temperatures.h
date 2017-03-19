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
 * @param bms_state datatype that contains a variable representing the current thermistor
 *                  selected
 */
void CellTemperatures_Step(BMS_STATE_T * bms_state);

/**
 * @details gets bit bit of current thermistor address 
 *
 * @param currentThermistor number of thermistor currently selected
 * @param bit bit of current thermistor address we want to obtain
 * @return bit of current thermistor address
 */
uint8_t CellTemperatures_GetThermistorAddressBit(uint8_t currentThermistor, uint8_t bit);

#endif //CELL_TEMPERATURES
