#ifndef CHARGE_H
#define CHARGE_H
#include "state_types.h"

const uint32_t OscRateIn = 0;

typedef enum {
	BMS_CHARGE_MODE_OFF,
	BMS_CHARGE_MODE_INIT,
	BMS_CHARGE_MODE_RUN,
	BMS_CHARGE_MODE_DONE
} BMS_CHARGE_MODE_T;

static BMS_CHARGE_MODE_T CHARGE_MODE;

/*
 * public function prototypes
*/

void BMS_CHARGE_INIT(void);
BMS_SM_OUTPUT_T BMS_CHARGE_STEP(BMS_SM_INPUT_T input);
void BMS_CHARGE_RESET(void);
BMS_SM_OUTPUT_T BMS_CHARGE_GET_MODE(void);

#endif
//CHARGE_H
