#include "eeprom_config.h"

// local buf copies of packconfig
static uint8_t eeprom_data_buf[DATA_BLOCK_SIZE];
static PACK_CONFIG_T eeprom_packconf_buf;
static uint8_t module_cell_count[MAX_NUM_MODULES];
static uint8_t eeprom_data_addr[3]; // LC1024 eeprom address length is 3 bytes

static bool Validate_PackConfig(PACK_CONFIG_T *pack_config, uint16_t version, uint8_t checksum);
static uint8_t Calculate_Checksum(PACK_CONFIG_T *pack_config);
static void Load_PackConfig_Defaults(PACK_CONFIG_T *pack_config);

void EEPROM_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin){
	LC1024_Init(pSSP, baud, cs_gpio, cs_pin);
	Load_PackConfig_Defaults(&eeprom_data_buf);
	eeprom_data_addr[0] = EEPROM_DATA_START >> 16;
	eeprom_data_addr[1] = (EEPROM_DATA_START & 0xFF00) >> 8;
	eeprom_data_addr[2] = (EEPROM_DATA_START & 0xFF);
}

// entry from Process_Output(..) in main.c, executed during start
bool EEPROM_LoadPackConfig(PACK_CONFIG_T *pack_config) {
	LC1024_ReadMem(&eeprom_data_addr[0], &eeprom_data_buf, DATA_BLOCK_SIZE);

	// TODO: auto-calculate pointer/alloc/idx constants with macro magic
	//
	// offset in the below line: we do not copy
	// 		the module cell count ptr (1 byte) 
	// 		+ version (2 byte) 
	// 		+ checksum (2 byte) 
	// 		+ max cell module (16) = 21 bytes
	memcpy(&eeprom_packconf_buf, &eeprom_data_buf, DATA_BLOCK_SIZE-21); 
	memcpy(&module_cell_count, &eeprom_data_buf[52], MAX_NUM_MODULES); 
	uint16_t version = eeprom_data_buf[69];
	uint8_t checksum = eeprom_data_buf[71];

	if (Validate_PackConfig(&eeprom_data_buf, version, checksum)) {
		memcpy(pack_config, &eeprom_packconf_buf, DATA_BLOCK_SIZE-21); 
		memcpy(pack_config->module_cell_count, &module_cell_count, MAX_NUM_MODULES); 
		return true;
	} else {
		Load_PackConfig_Defaults(pack_config);
		return false;
	}
}

static void Load_PackConfig_Defaults(PACK_CONFIG_T *pack_config) {
	pack_config->cell_min_mV = 1000;
	pack_config->cell_max_mV = 4000;
	pack_config->cell_capacity_cAh = 530;
	pack_config->num_modules = 1;
	pack_config->cell_charge_c_rating_cC = 70;
	pack_config->bal_on_thresh_mV = 4;
	pack_config->bal_off_thresh_mV = 1;
	pack_config->pack_cells_p = 6;
	pack_config->cv_min_current_mA = 50;
	pack_config->cv_min_current_ms = 60000;
	pack_config->cc_cell_voltage_mV = 4300;
	pack_config->cell_discharge_c_rating_cC = 200; // at 27 degrees C
	pack_config->max_cell_temp_C = 50;
	pack_config->module_cell_count[0] = 12;
	uint8_t i;
	for(i = 1; i < MAX_NUM_MODULES; i++) {
		pack_config->module_cell_count[i] = 0;
	}
}

// SHOULD ONLY BE CALLED IN STANDBY MODE, so only need to run checks
uint8_t EEPROM_ChangeConfig(rw_loc_lable_t rw_loc, uint32_t val) {
	switch (rw_loc) {
		case RWL_cell_min_mV:
			eeprom_packconf_buf.cell_min_mV = val;
			break;
		case RWL_cell_max_mV:
			eeprom_packconf_buf.cell_max_mV = val;
			break;
		case RWL_cell_capacity_cAh:
			eeprom_packconf_buf.cell_capacity_cAh = val;
			break;
		case RWL_num_modules:
			eeprom_packconf_buf.num_modules = val;
			break;
		case RWL_module_cell_count:
			// TODO 
			// module_cell_count[0...15] = 
			return 1;
		case RWL_cell_charge_c_rating_cC:
			eeprom_packconf_buf.cell_charge_c_rating_cC = val;
			break;  
		case RWL_bal_on_thresh_mV:
			eeprom_packconf_buf.bal_on_thresh_mV = val;
			break; 
		case RWL_bal_off_thresh_mV:
			eeprom_packconf_buf.bal_off_thresh_mV = val;
			break;
		case RWL_pack_cells_p:
			eeprom_packconf_buf.pack_cells_p = val;
			break;
		case RWL_cv_min_current_mA:
			eeprom_packconf_buf.cv_min_current_mA = val;
			break;
		case RWL_cv_min_current_ms:
			eeprom_packconf_buf.cv_min_current_ms = val;
			break;
		case RWL_cc_cell_voltage_mV:
			eeprom_packconf_buf.cc_cell_voltage_mV = val;
			break;
		case RWL_cell_discharge_c_rating_cC:
			eeprom_packconf_buf.cell_discharge_c_rating_cC = val;
			break;
		case RWL_max_cell_temp_C:
			eeprom_packconf_buf.max_cell_temp_C = val;
			break;
		case RWL_LENGTH:
			//wat TODO error
			break;
	}

	// TODO: auto-calculate pointer/alloc/idx constants with macro magic
	//
	// offset in the below line: we do not copy
	// 		the module cell count ptr (1 byte) 
	// 		+ version (2 byte) 
	// 		+ checksum (2 byte) 
	// 		+ max cell module (16) = 21 bytes
	memcpy(&eeprom_data_buf, &eeprom_packconf_buf, DATA_BLOCK_SIZE-21); 
	memcpy(&eeprom_data_buf[52], &module_cell_count, MAX_NUM_MODULES); 
	eeprom_data_buf[69] = STORAGE_VERSION;
	eeprom_data_buf[71] = Calculate_Checksum(&eeprom_packconf_buf);
	// write config to eeprom
	
	LC1024_WriteMem(&eeprom_data_addr[0], &eeprom_data_buf, DATA_BLOCK_SIZE);

	return 0;
}


static uint8_t Calculate_Checksum(PACK_CONFIG_T *pack_config) {
	// TODO
	UNUSED(pack_config);
	return 1;
}

static bool Validate_PackConfig(PACK_CONFIG_T *pack_config, uint16_t version, uint8_t checksum) {
	// TODO
	UNUSED(pack_config);
	UNUSED(checksum);
	return version == STORAGE_VERSION;
}
