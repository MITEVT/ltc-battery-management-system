#ifndef CELL_TEMPERATURES_H
#define CELL_TEMPERATURES_H

// ltc-battery-controller
#include "state_types.h"

/**
 * @details get cell temperature of next thermistor
 *
 * @param pack_status datatype containing a mutable array of cell temperatures
 */
void CellTemperatures_Step(BMS_PACK_STATUS_T * pack_status);

#endif //CELL_TEMPERATURES
