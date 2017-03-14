#ifndef CELL_TEMPERATURES_H
#define CELL_TEMPERATURES_H

// ltc-battery-management-system
#include "state_types.h"
#include "config.h"

// constants
#define NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS 5
static const uint8_t thermistorAddresses[MAX_THERMISTORS_PER_MODULE][NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS] = {
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1},
	{0, 0, 0, 1, 0},
	{0, 0, 0, 1, 1},
	{0, 0, 1, 0, 0},
	{0, 0, 1, 0, 1},
	{0, 0, 1, 1, 0},
	{0, 0, 1, 1, 1},
	{0, 1, 0, 0, 0},
	{0, 1, 0, 0, 1},
	{0, 1, 0, 1, 0},
	{0, 1, 0, 1, 1},
	{0, 1, 1, 0, 0},

	{1, 0, 0, 0, 0},
	{1, 0, 0, 0, 1},
	{1, 0, 0, 1, 0},
	{1, 0, 0, 1, 1},
	{1, 0, 1, 0, 0},
	{1, 0, 1, 0, 1},
	{1, 0, 1, 1, 0},
	{1, 0, 1, 1, 1},
	{1, 1, 0, 0, 0},
	{1, 1, 0, 0, 1},
	{1, 1, 0, 1, 0}
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

#endif //CELL_TEMPERATURES
