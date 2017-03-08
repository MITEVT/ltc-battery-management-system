#include "state_types.h"
#include "can_constants.h"
#include "board.h"
#include "bms_can.h"

void BMS_CAN_ConstructHeartbeatData(BMS_SSM_MODE_T state, uint16_t soc, uint8_t * data) {
	//get state
	uint16_t state_int_type = ____BMS_HEARTBEAT__STATE__INIT;
	switch(state) {
		case BMS_SSM_MODE_INIT:
			state_int_type = ____BMS_HEARTBEAT__STATE__INIT;
		        break;
		case BMS_SSM_MODE_STANDBY:
			state_int_type = ____BMS_HEARTBEAT__STATE__STANDBY;
			break;
		case BMS_SSM_MODE_CHARGE:
		        state_int_type = ____BMS_HEARTBEAT__STATE__CHARGE;
			break;
		case BMS_SSM_MODE_BALANCE:
			state_int_type = ____BMS_HEARTBEAT__STATE__BALANCE;
		        break;
		case BMS_SSM_MODE_DISCHARGE:
			state_int_type = ____BMS_HEARTBEAT__STATE__DISCHARGE;
	       		break;
		default:
		        Board_Println("Unrecognized state. You should never reach here.");
		        //TODO: go to error state
	}
	
	//create data string
	const uint8_t heartbeat_data_bytes = 2;
	const uint8_t bits_in_byte = 8;
	const uint8_t state_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
		__BMS_HEARTBEAT__STATE__end);
	const uint8_t soc_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
		__BMS_HEARTBEAT__SOC_PERCENTAGE__end);
	uint16_t data_string = 0 | (state_int_type << state_left_shift) |
		(soc << soc_left_shift);
	
	//mutate data
	const uint16_t byte0_mask = 0xFF00;
	const uint16_t byte1_mask = 0x00FF;
	data[0] = (data_string & byte0_mask) >> bits_in_byte;
	data[1] = data_string & byte1_mask;
	
}

