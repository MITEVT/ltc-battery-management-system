// ltc-battery-management-system
#include "cell_temperatures.h"
#include "state_types.h"
#include "config.h"

//lpc11cx4-library
#include "lpc_types.h"

// state
static uint8_t currentThermistor = 0;

/**************************************************************************************
 * Private Functions
 * ***********************************************************************************/

void CellTemperatures_GetThermistorAddress(uint8_t currentThermistor, 
		BMS_PACK_STATUS_T * pack_status) {
	UNUSED(currentThermistor);
	UNUSED(pack_status);
	// TODO
}

/**************************************************************************************
 * Public Functions
 * ***********************************************************************************/

void CellTemperatures_Step(BMS_PACK_STATUS_T * pack_status) {

	// move to next thermistor
	if (currentThermistor < MAX_THERMISTORS_PER_MODULE) {
		currentThermistor += 1;
	} else {
		currentThermistor = 0;
	}

	// Get multiplexer address
	CellTemperatures_GetThermistorAddress(currentThermistor, pack_status);
}
