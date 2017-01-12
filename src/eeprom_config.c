 #include "state_types.h"


bool Load_EEPROM_PackConfig(PACK_CONFIG_T *pack_config) {
	//TODO make this not a MOCK
	pack_config->cell_min_mV = 2700;
	pack_config->cell_max_mV = 3700;
	pack_config->cell_capacity_cAh = 200;
	pack_config->num_modules = 10;
	// pack_config->*num_cells_in_modules; // [TODO] refactor to module_cell_count
	pack_config->cell_charge_c_rating_cC = 2000;
	pack_config->bal_on_thresh_mV = 20;
	pack_config->bal_off_thresh_mV = 5;
	pack_config->pack_cells_p = 32;
	pack_config->cv_min_current_mA = 0;
	pack_config->cv_min_current_ms = 0;
	pack_config->cc_cell_voltage_mV = 0;
	return true;
}

bool Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config) {
	return true;
}