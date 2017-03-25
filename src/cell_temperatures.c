// ltc-battery-management-system
#include "cell_temperatures.h"
#include "state_types.h"
#include "config.h"

//lpc11cx4-library
#include "lpc_types.h"

/**************************************************************************************
 * Public Functions
 * ***********************************************************************************/

void CellTemperatures_Step(uint8_t * currentThermistor) {

    // move to next thermistor
    if (*currentThermistor < (MAX_THERMISTORS_PER_MODULE-1)) {
        *currentThermistor += 1;
    } else {
        *currentThermistor = 0;
    }

}

uint8_t CellTemperatures_GetThermistorAddressBit(uint8_t currentThermistor, 
        uint8_t bit) {
    const uint8_t currentThermistorAddress = thermistorAddresses[currentThermistor];
    const uint8_t bitMask = 0b00000001;
    const uint8_t thermistorAddressBit = (currentThermistorAddress >> bit) 
        & bitMask;
    return thermistorAddressBit;    
}
