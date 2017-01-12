 #include "state_types.h"

#define EEPROM_DATA_START 0x00
#define TABLE_SIZE 64 //units of bytes
#define STORAGE_VERSION 0x00

static uint8_t eeprom_table_buffer[TABLE_SIZE];


static void load_table_eeprom(uint8_t* eeprom_table_buffer);
static bool validate_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_table_eeprom(uint8_t* eeprom_table_buffer);
static void write_checksum_eeprom(uint8_t* eeprom_table_buffer);
static void write_set_config_defaults_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);
static void set_config_eeprom(uint8_t* eeprom_table_buffer, PACK_CONFIG_T *pack_config);

bool Load_EEPROM_PackConfig(PACK_CONFIG_T *pack_config);

bool Check_PackConfig_With_LTC(PACK_CONFIG_T *pack_config);