// ltc-battery-controller
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

void getThermistorAddress(uint8_t currentThermistor, uint8_t * thermistorAddress) {
	UNUSED(currentThermistor);
	UNUSED(thermistorAddress);
	// TODO
}

void setMultiplexerAddress(uint8_t * thermistorAddress) {
	UNUSED(thermistorAddress);
	// TODO
}

void getThermistorVoltage(uint8_t currentThermistor, BMS_PACK_STATUS_T * pack_status) {
	
	// get thermistor address
	uint8_t thermistorAddress[NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS];	
	getThermistorAddress(currentThermistor, thermistorAddress);

	// set multiplexer address
	setMultiplexerAddress(thermistorAddress);

	UNUSED(pack_status);
	
	// TODO: read thermistor voltage and store result in pack_status
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

	// get thermistor voltage
	getThermistorVoltage(currentThermistor, pack_status);
}
