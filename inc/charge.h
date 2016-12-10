#ifndef CHARGE_H
#define CHARGE_H
#include "state_types.h"

const uint32_t OscRateIn = 0;


static BMS_DISCHARGE_MODE_T DISCHARGE_MODE;

/*
 * public function prototypes
*/

void BMS_CHARGE_INIT(void);
BMS_SM_OUTPUT_T BMS_CHARGE_STEP(BMS_SM_INPUT_T input);
void BMS_CHARGE_RESET(void);
BMS_SM_OUTPUT_T BMS_CHARGE_GET_MODE(void);

#endif
//CHARGE_H