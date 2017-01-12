#include "eeprom_config.h"
#include "state_types.h"
#include "lc1024.h"



bool Load_EEPROM_PackConfig(PACK_CONFIG_T *pack_config) {
	load_table_eeprom(eeprom_table_buffer);
	if (!validate_table_eeprom(eeprom_table_buffer)){
		write_set_config_defaults_eeprom(eeprom_table_buffer,pack_config);
	}
	else {
		set_config_eeprom(eeprom_table_buffer, pack_config);
	}


	

	return true;
}

bool Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config) {
	return true;
}


static void load_table_eeprom(uint8_t* eeprom_table_buffer) {

}
static bool validate_table_eeprom(uint8_t* eeprom_table_buffer) {
	return false;
}
static void write_table_eeprom(uint8_t* eeprom_table_buffer) {

}
static void write_checksum_eeprom(uint8_t* eeprom_table_buffer) {

}

static void write_set_config_defaults_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config) {
	pack_config->cell_min_mV = 2700;
	pack_config->cell_max_mV = 3700;
	pack_config->cell_capacity_cAh = 200;
	pack_config->num_modules = 10;
	pack_config->cell_charge_c_rating_cC = 2000;
	pack_config->bal_on_thresh_mV = 20;
	pack_config->bal_off_thresh_mV = 5;
	pack_config->pack_cells_p = 32;
	pack_config->cv_min_current_mA = 0;
	pack_config->cv_min_current_ms = 0;
	pack_config->cc_cell_voltage_mV = 0;
	write_table_eeprom(pack_config);
	write_checksum_eeprom(eeprom_table_buffer);
	// pack_config->*num_cells_in_modules; // [TODO] refactor to module_cell_count
}

static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config) {

}
