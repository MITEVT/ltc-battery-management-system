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

#define EEPROM_DATA_START 0x00F0F0
#define DATA_BLOCK_SIZE 72 // [size of PACK_CONFIG_T in 53 bytes] + 16 bytes (max module cell count) + 2 bytes for version + 1 bytes for checksum
#define STORAGE_VERSION 0x01

void EEPROM_Init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin);
uint8_t EEPROM_ChangeConfig(rw_loc_lable_t rw_loc, uint32_t val);
bool EEPROM_LoadPackConfig(PACK_CONFIG_T *pack_config);

#endif
