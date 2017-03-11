#include "eeprom_config.h"
#include "board.h"

void SOC_Init(void) {
    EEPROM_WriteCC(18);
}

uint16_t SOC_Estimate(void) {
    return EEPROM_LoadCC();
}
