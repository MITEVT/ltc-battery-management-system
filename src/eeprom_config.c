#include "eeprom_config.h"


uint8_t eeprom_table_buffer[TABLE_SIZE];

uint8_t num_cells_in_modules[] = {12};

static void load_table_eeprom(uint8_t* eeprom_table_buffer);
static bool validate_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_checksum_eeprom(uint8_t* eeprom_table_buffer);
static void write_set_config_defaults_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);
static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);


PACK_CONFIG_T pack_config_defaults;

static void EEPROM_Default_Config(void) {
    pack_config_defaults.cell_min_mV = 2500;
    pack_config_defaults.cell_max_mV = 4200;
    pack_config_defaults.cell_capacity_cAh = 530;
    pack_config_defaults.num_modules = 1;
    pack_config_defaults.cell_charge_c_rating_cC = 70;
    pack_config_defaults.bal_on_thresh_mV = 4;
    pack_config_defaults.bal_off_thresh_mV = 1;
    pack_config_defaults.pack_cells_p = 6;
    pack_config_defaults.cv_min_current_mA = 50;
    pack_config_defaults.cv_min_current_ms = 60000;
    pack_config_defaults.cc_cell_voltage_mV = 4300;
    pack_config_defaults.cell_discharge_c_rating_cC = 200; // at 27 degrees C
    pack_config_defaults.max_cell_temp_C = 50;
    pack_config_defaults.num_cells_in_modules = &num_cells_in_modules; // [TODO] Fix
}

void EEPROM_init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin){
    LC1024_Init(pSSP, baud, cs_gpio, cs_pin);
    EEPROM_Default_Config();
}

// entry from Process_Output(..) in main.c, executed during start
bool EEPROM_Load_PackConfig(PACK_CONFIG_T *pack_config) {
	load_table_eeprom(eeprom_table_buffer);
	if (!validate_table_eeprom(eeprom_table_buffer)){
		write_set_config_defaults_eeprom(eeprom_table_buffer, pack_config);
	} else {
		set_config_eeprom(eeprom_table_buffer, pack_config);
	}
    return true;
}

bool EEPROM_Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config) {
	return true;
}



// SHOULD ONLY BE CALLED IN STANDBY MODE
uint8_t EEPROM_Change_Config(rw_loc_lable_t rw_loc, uint32_t val) {
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
    pack_config->num_cells_in_modules[0] = 12;
	write_table_eeprom(pack_config);
	write_checksum_eeprom(eeprom_table_buffer);
	// pack_config->*num_cells_in_modules; // [TODO] refactor to module_cell_count
}

static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config) {

}



