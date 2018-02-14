#include "eeprom_config.h"
#include "board.h"
#include "state_types.h"

static uint32_t soc, init_soc;

//7.1 or 7.2 kilawatthours
//units in watt-hours
#define MAX_CHARGE 7100

void SOC_Full(void){
	soc = MAX_CHARGE;
}

void SOC_Init(void) {
	soc = EEPROM_LoadCCPage_Num(0);
	init_soc = soc;
	//}
}

void SOC_Estimate(BMS_INPUT_T* bms_input) {

	soc = init_soc - bms_input->pack_status->pack_energy;

	if(soc > MAX_CHARGE){
		soc = MAX_CHARGE;
	}

	bms_input->pack_status->state_of_charge = soc;

}
void SOC_Write(void){
	EEPROM_WriteCCPage_Num(0,soc);
}
