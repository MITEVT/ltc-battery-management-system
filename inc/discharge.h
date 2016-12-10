#ifndef DISCHARGE_H
#define DISCHARGE_H

#include "state_types.h"


static BMS_DISCHARGE_MODE_T DISCHARGE_MODE;

/*
 * public function prototypes
*/

void BMS_DISCHARGE_INIT(void);
BMS_SM_OUTPUT_T BMS_DISCHARGE_STEP(BMS_SM_INPUT_T input);
void BMS_DISCHARGE_RESET(void);
BMS_SSM_DISCHARGE_T BMS_DISCHARGE_GET_MODE(void);

#endif
//DISCHARGE_H