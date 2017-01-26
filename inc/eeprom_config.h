#ifndef _EEPROM_CONFIG_H
#define _EEPROM_CONFIG_H

#include <stddef.h>
#include "state_types.h"
#include "console_types.h"



#define EEPROM_DATA_START 0x00
#define TABLE_SIZE 64 //units of bytes
#define STORAGE_VERSION 0x00

static uint8_t eeprom_table_buffer[TABLE_SIZE];

static PACK_CONFIG_T pack_config_defaults;


static void load_table_eeprom(uint8_t* eeprom_table_buffer);
static bool validate_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_checksum_eeprom(uint8_t* eeprom_table_buffer);
static void write_set_config_defaults_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);
static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);


void EEPROM_init(LPC_SSP_T *pSSP, uint32_t baud, uint8_t cs_gpio, uint8_t cs_pin);
uint8_t EEPROM_Change_Config(rw_loc_lable_t rw_loc, uint32_t val);
bool EEPROM_Load_PackConfig(PACK_CONFIG_T *pack_config);
bool EERPOM_Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config);
void EEPROM_Default_Config(void);

#endif
