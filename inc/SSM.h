#ifndef SSM_H
#define SSM_H

#include "state_types.h"


static BMS_SSM_MODE_T SSM_MODE;

/*
 * public function prototypes
*/

void BMS_SSM_INIT(void);
BMS_SM_OUTPUT_T BMS_SSM_STEP(BMS_SM_INPUT_T input);
void BMS_SSM_RESET(void);
BMS_SSM_MODE_T BMS_SSM_GET_MODE(void);

#endif
//SSM_H
