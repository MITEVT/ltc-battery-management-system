#include "eeprom_config.h"
#include "board.h"


// TODO update eeprom driver to handle pages
// local buf copies of packconfig
#define DATA_BLOCK_SIZE sizeof(PACK_CONFIG_T) + CHECKSUM_BYTESIZE + VERSION_BYTESIZE + MAX_NUM_MODULES
static uint8_t eeprom_data_buf[DATA_BLOCK_SIZE];
static PACK_CONFIG_T eeprom_packconf_buf;
static uint8_t mcc[MAX_NUM_MODULES];
static uint8_t eeprom_data_addr[3]; // LC1024 eeprom address length is 3 bytes

static bool Validate_PackConfig(PACK_CONFIG_T *pack_config, uint16_t version, uint8_t checksum);
static uint8_t Calculate_Checksum(PACK_CONFIG_T *pack_config);
static void Load_PackConfig_Defaults(PACK_CONFIG_T *pack_config);
static void Print_EEPROM_DataBuffer(void);
static void Zero_EEPROM_DataBuffer(void);
static void Run_EEPROM_Test(void);
static void Write_PackConfig_EEPROM(void);

static void Zero_EEPROM_DataBuffer(void) {
	uint8_t i;
	for (i = 0; i < DATA_BLOCK_SIZE; i++) {
		eeprom_data_buf[i] = 0;
	}
}

// static void Print_EEPROM_DataBuffer(void) {
//     Board_Print_BLOCKING("0x");
//     uint8_t i;
//     for(i = 0; i < DATA_BLOCK_SIZE; i++) {
//         itoa(eeprom_data_buf[i], str, 16);
//         if(eeprom_data_buf[i] < 16) {
//             Board_Print_BLOCKING("0");
//         	Board_Print_BLOCKING(str);
//         } else {
//         	Board_Print_BLOCKING(str);
// 		}
//         Board_Print_BLOCKING(" ");
//     }
//     Board_Println_BLOCKING("");
// }

static void Run_EEPROM_Test(void) {
    Zero_EEPROM_DataBuffer();
    LC1024_WriteEnable();
    LC1024_WriteEnable();
    LC1024_ReadStatusReg(eeprom_data_buf);
	Board_Println_BLOCKING("Running EEPROM test...");

    eeprom_data_buf[0] = 0x0E;
    eeprom_data_buf[1] = 0x0E;

    LC1024_WriteMem(eeprom_data_addr, eeprom_data_buf, 2);
	Board_BlockingDelay(200);
	
    LC1024_ReadMem(eeprom_data_addr, eeprom_data_buf, 2);
	Board_BlockingDelay(200);
    
    if(eeprom_data_buf[0] == 0xE) {
        Board_Println_BLOCKING("EEPROM passed write test!");
    } else {
        Board_Println_BLOCKING("EEPROM failed write test!");
    }
}

void EEPROM_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin){
	LC1024_Init(pSSP, baud, cs_gpio, cs_pin);

	// eeprom_data_addr[0] = EEPROM_DATA_START >> 16;
	// eeprom_data_addr[1] = (EEPROM_DATA_START & 0xFF00) >> 8;
	// eeprom_data_addr[2] = (EEPROM_DATA_START & 0xFF);

	eeprom_data_addr[0] = 0x00;
	eeprom_data_addr[1] = 0x00;
	eeprom_data_addr[2] = 0x00;

	// Run_EEPROM_Test();

	Zero_EEPROM_DataBuffer();
	eeprom_packconf_buf.module_cell_count = mcc;

	Board_Println_BLOCKING("Finished init...");
	Board_BlockingDelay(200);
}

// entry from Process_Output(..) in main.c, executed during start
bool EEPROM_LoadPackConfig(PACK_CONFIG_T *pack_config) {

	Board_Println_BLOCKING("Loading PackConfig from EEPROM...");
    LC1024_WriteEnable();
    LC1024_WriteEnable();
	LC1024_ReadMem(eeprom_data_addr, eeprom_data_buf, DATA_BLOCK_SIZE);
	Board_BlockingDelay(200);

	// offset in the below line: we do not copy module cell count ptr (1 byte)
	// loading into the eeprom driver packconfig buffer
	memcpy(&eeprom_packconf_buf, eeprom_data_buf, sizeof(PACK_CONFIG_T)-sizeof(void*)); 
	memcpy(mcc, &eeprom_data_buf[sizeof(PACK_CONFIG_T)], MAX_NUM_MODULES); 
	uint16_t version = eeprom_data_buf[DATA_BLOCK_SIZE - CHECKSUM_BYTESIZE - VERSION_BYTESIZE];
	uint8_t checksum = eeprom_data_buf[DATA_BLOCK_SIZE - CHECKSUM_BYTESIZE];

	if (Validate_PackConfig(&eeprom_packconf_buf, version, checksum)) {
		Board_Println_BLOCKING("Passed validation, using load from EEPROM...");
		// loading from eeprom driver packconfig buffer
		uint8_t * their_mccp = pack_config->module_cell_count;
		memcpy(pack_config, &eeprom_packconf_buf, sizeof(PACK_CONFIG_T)-sizeof(void*));
		pack_config->module_cell_count = their_mccp;
		Board_Println_BLOCKING("cpy pack_config");
		memcpy(pack_config->module_cell_count, mcc, MAX_NUM_MODULES); 
		Board_Println_BLOCKING("cpy mcc");
		return true;
	} else {
		Board_Println_BLOCKING("Using pre-configured defaults...");
		Load_PackConfig_Defaults(pack_config);
		Load_PackConfig_Defaults(&eeprom_packconf_buf);
		Board_Println_BLOCKING("Finished loading pre-configured defaults...");
		Write_PackConfig_EEPROM();
		Board_Println_BLOCKING("Wrote pre-configured defaults to EEPROM.");
		return true;
	}

}

void Write_EEPROM_PackConfig_Defaults(void) {
	Board_Println_BLOCKING("Using pre-configured defaults...");
    LC1024_WriteEnable();
    LC1024_WriteEnable();
	Load_PackConfig_Defaults(&eeprom_packconf_buf);
	Board_Println_BLOCKING("Finished loading pre-configured defaults...");
	Write_PackConfig_EEPROM();
	Board_Println_BLOCKING("Wrote pre-configured defaults to EEPROM.");
}

static void Load_PackConfig_Defaults(PACK_CONFIG_T *pack_config) {
	pack_config->cell_min_mV = 2800;
	pack_config->cell_max_mV = 3600;
	pack_config->cell_capacity_cAh = 530;
	pack_config->num_modules = 2;
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
	pack_config->module_cell_count[1] = 12;
	uint8_t i;
	for(i = pack_config->num_modules; i < MAX_NUM_MODULES; i++) {
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

	Write_PackConfig_EEPROM();
	return 0;
}

static void Write_PackConfig_EEPROM(void) {
	// offset in the below line: we do not copy the module cell count ptr (1 byte) 
	memcpy(eeprom_data_buf, &eeprom_packconf_buf, sizeof(PACK_CONFIG_T)-sizeof(void*)); 
	memcpy(&eeprom_data_buf[sizeof(PACK_CONFIG_T)], mcc, MAX_NUM_MODULES); 
	eeprom_data_buf[DATA_BLOCK_SIZE - CHECKSUM_BYTESIZE - VERSION_BYTESIZE] = STORAGE_VERSION;
	eeprom_data_buf[DATA_BLOCK_SIZE - CHECKSUM_BYTESIZE] = Calculate_Checksum(&eeprom_packconf_buf);


	LC1024_WriteEnable();
	LC1024_WriteEnable();

	LC1024_WriteMem(eeprom_data_addr, eeprom_data_buf, DATA_BLOCK_SIZE);
	Board_BlockingDelay(200);
	Board_Println_BLOCKING("Finished writing pack config to EEPROM.");
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
