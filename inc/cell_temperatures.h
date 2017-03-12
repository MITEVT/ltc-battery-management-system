#ifndef CELL_TEMPERATURES_H
#define CELL_TEMPERATURES_H

// ltc-battery-controller
#include "state_types.h"

// constants
#define NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS 5


/****************************************************************************************
 * Private Functions
 * *************************************************************************************/

/**
 * @details get address of current thermistor
 *
 * @param currentThermistor value of current thermistor
 * @param thermistorAddress array of bits representing the address of currentThermistor
 */
void getThermistorAddress(uint8_t currentThermistor, uint8_t * thermistorAddress);

/**
 * @details sets address of multiplexer by controlling shit register
 *
 * @param thermistorAddress array of bits representing the address of a thermistor
 */
void setMultiplexerAddress(uint8_t * thermistorAddress);

/**
 * @details get voltage of currentThermistor
 *
 * @param currentThermistor value of current thermistor
 * @param pack_status datatype containing mutable array of cell temperatures
 */
void getThermistorVoltage(uint8_t currentThermistor, BMS_PACK_STATUS_T * pack_status);


/****************************************************************************************
 * Public Functions
 * *************************************************************************************/

/**
 * @details get cell temperature of next thermistor
 *
 * @param pack_status datatype containing a mutable array of cell temperatures
 */
void CellTemperatures_Step(BMS_PACK_STATUS_T * pack_status);

#endif //CELL_TEMPERATURES
