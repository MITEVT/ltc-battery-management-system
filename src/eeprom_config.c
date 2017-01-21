#include "chip.h"
#include "eeprom_config.h"
#include "state_types.h"
#include "lc1024.h"

static PACK_CONFIG_T pack_config_defaults = {
    .cell_min_mV = 2700,
    .cell_max_mV = 3700,
    .cell_capacity_cAh = 400,
    .num_modules = 10,
    .cell_charge_c_rating_cC = 200,
    .bal_on_thresh_mV = 20,
    .bal_off_thresh_mV = 5,
    .pack_cells_p = 64,
    .cv_min_current_mA = 1000,
    .cv_min_current_ms = 1000,
    .cc_cell_voltage_mV = 3600,
    .num_cells_in_modules = 0 // [TODO] refactor to module_cell_count
};

void init_eeprom(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin){
    LC1024_Init(pSSP, baud, cs_gpio, cs_pin);
}

bool Load_EEPROM_PackConfig(PACK_CONFIG_T *pack_config) {
	load_table_eeprom(eeprom_table_buffer);
	if (!validate_table_eeprom(eeprom_table_buffer)){
		write_set_config_defaults_eeprom(eeprom_table_buffer,pack_config);
	} else {
		set_config_eeprom(eeprom_table_buffer, pack_config);
	}
	return true;
}

bool Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config) {
	return true;
}

void Default_Config(void){
    pack_config_defaults.cell_min_mV = 2700;
    pack_config_defaults.cell_max_mV = 3700;
    pack_config_defaults.cell_capacity_cAh = 400;
    pack_config_defaults.num_modules = 10;
    pack_config_defaults.cell_charge_c_rating_cC = 200;
    pack_config_defaults.bal_on_thresh_mV = 20;
    pack_config_defaults.bal_off_thresh_mV = 5;
    pack_config_defaults.pack_cells_p = 64;
    pack_config_defaults.cv_min_current_mA = 1000;
    pack_config_defaults.cv_min_current_ms = 1000;
    pack_config_defaults.cc_cell_voltage_mV = 3600;
    pack_config_defaults.num_cells_in_modules = 0; // [TODO] refactor to module_cell_coun
}

// SHOULD ONLY BE CALLED IN STANDBY MODE
uint8_t Change_Config(rw_loc_lable_t rw_loc, uint32_t val) {
    switch (rw_loc) {
        case RWL_cell_min_mV:
            pack_config_defaults.cell_min_mV = val;
            break;
        case RWL_cell_max_mV:
            pack_config_defaults.cell_max_mV = val;
            break;
        case RWL_cell_capacity_cAh:
            pack_config_defaults.cell_capacity_cAh = val;
            break;
        case RWL_num_modules:
            pack_config_defaults.num_modules = val;
            break;
        case RWL_num_cells_in_modules:
            // UNIMPLEMENTED
            return 1;
        case RWL_cell_charge_c_rating_cC:
            pack_config_defaults.cell_charge_c_rating_cC = val;
            break;  
        case RWL_bal_on_thresh_mV:
            pack_config_defaults.bal_on_thresh_mV = val;
            break; 
        case RWL_bal_off_thresh_mV:
            pack_config_defaults.bal_off_thresh_mV = val;
            break;
        case RWL_pack_cells_p:
            pack_config_defaults.pack_cells_p = val;
            break;
        case RWL_cv_min_current_mA:
            pack_config_defaults.cv_min_current_mA = val;
            break;
        case RWL_cv_min_current_ms:
            pack_config_defaults.cv_min_current_ms = val;
            break;
        case RWL_cc_cell_voltage_mV:
            pack_config_defaults.cc_cell_voltage_mV = val;
            break;
        case RWL_cell_discharge_c_rating_cC:
            pack_config_defaults.cell_discharge_c_rating_cC = val;
            break;
        case RWL_max_cell_temp_C:
            pack_config_defaults.max_cell_temp_C = val;
            break;
    }
    // write config to eeprom
    return 0;
}


static void load_table_eeprom(uint8_t* eeprom_table_buffer){

}
static bool validate_table_eeprom(uint8_t* eeprom_table_buffer) {
	return false;
}
static void write_table_eeprom(uint8_t* eeprom_table_buffer) {

}
static void write_checksum_eeprom(uint8_t* eeprom_table_buffer) {

}

static void write_set_config_defaults_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config) {
	pack_config->cell_min_mV = pack_config_defaults.cell_min_mV;
	pack_config->cell_max_mV = pack_config_defaults.cell_max_mV;
	pack_config->cell_capacity_cAh = pack_config_defaults.cell_capacity_cAh;
	pack_config->num_modules = pack_config_defaults.num_modules;
	pack_config->cell_charge_c_rating_cC = pack_config_defaults.cell_charge_c_rating_cC;
	pack_config->bal_on_thresh_mV = pack_config_defaults.bal_on_thresh_mV;
	pack_config->bal_off_thresh_mV = pack_config_defaults.bal_off_thresh_mV;
	pack_config->pack_cells_p = pack_config_defaults.pack_cells_p;
	pack_config->cv_min_current_mA = pack_config_defaults.cv_min_current_mA;
	pack_config->cv_min_current_ms = pack_config_defaults.cv_min_current_ms;
	pack_config->cc_cell_voltage_mV = pack_config_defaults.cc_cell_voltage_mV;
	write_table_eeprom(pack_config);
	write_checksum_eeprom(eeprom_table_buffer);
    Change_Config(RWL_cell_min_mV, 0); //[TODO] LOLWUT IS DIS SHIT
	// pack_config->*num_cells_in_modules; // [TODO] refactor to module_cell_count
}

static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config) {

}



