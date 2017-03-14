// ltc-battery-management-system
#include "cell_temperatures.h"
#include "state_types.h"
#include "config.h"

//lpc11cx4-library
#include "lpc_types.h"

/**************************************************************************************
 * Public Functions
 * ***********************************************************************************/

void CellTemperatures_Step(BMS_STATE_T * bms_state) {

	// move to next thermistor
	if (bms_state->currentThermistor < (MAX_THERMISTORS_PER_MODULE-1)) {
		bms_state->currentThermistor += 1;
	} else {
		bms_state->currentThermistor = 0;
	}

}
