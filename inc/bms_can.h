#include "state_types.h"

/**
 * @details constructs data for a BMS heartbeat
 *
 * @param state state of the bms
 * @param soc state of charge of the battery pack
 * @param data array that is mutated to store the data in the BMS heartbeat
*/
void BMS_CAN_ConstructHeartbeatData(BMS_SSM_MODE_T state, uint16_t soc, uint8_t * data);

