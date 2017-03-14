// ltc-battery-management-system
#include "cell_temperatures.h"
#include "state_types.h"
#include "config.h"

//lpc11cx4-library
#include "lpc_types.h"

// state
static uint8_t currentThermistor = 0;

/**************************************************************************************
 * Public Functions
 * ***********************************************************************************/

void CellTemperatures_Step(BMS_STATE_T * bms_state) {

	// move to next thermistor
	if (currentThermistor < MAX_THERMISTORS_PER_MODULE) {
		currentThermistor += 1;
	} else {
		currentThermistor = 0;
	}

	bms_state->currentThermistor = currentThermistor;
}
