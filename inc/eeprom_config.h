#ifndef _EEPROM_CONFIG_H
#define _EEPROM_CONFIG_H

#include <stddef.h>
#include "chip.h"
#include "state_types.h"
#include "console_types.h"
#include "state_types.h"
#include "lc1024.h"
#include "config.h"
#include "board.h"



#define EEPROM_DATA_START 0x00
#define TABLE_SIZE 64 //units of bytes
#define STORAGE_VERSION 0x00





void EEPROM_init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin);
uint8_t EEPROM_Change_Config(rw_loc_lable_t rw_loc, uint32_t val);
bool EEPROM_Load_PackConfig(PACK_CONFIG_T *pack_config);
bool EEPROM_Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config);

#endif
