#include "ssm.h"

static void Load_EEPROM_PackConfig(PACK_CONFIG_T *pack_config) {
}

void SSM_Init(BMS_STATE_T *state) {
    // Initialize BMS state variables
    // Load pack configuration from EEPROM
}

void SSM_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    // Check if in standby:
    //     if in standby:
    //          process appropriate inputs and return
    //          check if config commands in config buffer currently
    //          (if so write those to EEPROM)
    //     else dispatch step to appropriate SM step
}
