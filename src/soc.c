#include "eeprom_config.h"
#include "board.h"
#include "state_types.h"

static uint32_t soc, init_soc;

//7.1 or 7.2 kilawatthours
//units in watt-hours
#define MAX_CHARGE 7100

void SOC_Init(/*fullycharged?,*/) {
	// if(/*fully_charged*/){
	// 	init_soc = MAX_CHARGE;
	// 	soc = init_soc;
	// 	/*fully_charged = false;*/
	// 	last_tick_soc = ms_ticks;
	// } else {
		soc = EEPROM_LoadCCPage_Num(0);
		//soc = MAX_CHARGE;
		init_soc = soc;
	//}
}

uint32_t SOC_Estimate(BMS_INPUT_T* bms_input) {

	soc = init_soc - bms_input->pack_status->pack_energy;
	
	if(soc>MAX_CHARGE){ 
		soc = MAX_CHARGE;
		//Board_Println("OVERCHARGED!");
	}

	//write soc to eeprom
	EEPROM_WriteCCPage_Num(0,soc);
	bms_input->pack_status->state_of_charge = soc;
   //Board_PrintNUm
    return soc;
}


