#include "eeprom_config.h"
#include "board.h"
#include "state_types.h"

static uint32_t soc;
static uint32_t last_tick_soc;


#define MAX_CHARGE 10000000

void SOC_Init(/*fullycharged?,*/ uint32_t ms_ticks) {
	// if(/*fully_charged*/){
	// 	init_soc = MAX_CHARGE;
	// 	soc = init_soc;
	// 	/*fully_charged = false;*/
	// 	last_tick_soc = ms_ticks;
	// } else {
		last_tick_soc = ms_ticks;
		//soc = EEPROM_LoadCCPage_Num(0);
		
		//for testing:
		soc = MAX_CHARGE;
	//}
}

#define DELTA_T 50
uint32_t SOC_Estimate(BMS_INPUT_T* bms_input, uint32_t ms_ticks) {

	//rectangular integration
	if((ms_ticks - last_tick_soc)> DELTA_T){
		last_tick_soc = ms_ticks;
		//units are coulombs
		soc = (soc - (bms_input->pack_status->pack_current_mA) * DELTA_T);
	
		if(soc>MAX_CHARGE) soc = MAX_CHARGE;

		//write soc to eeprom
		EEPROM_WriteCCPage_Num(0,soc);
		bms_input->pack_status->state_of_charge = soc;
    	return soc;
    } else {
    	return soc;
    }
}

