
#include <string.h>
#include "chip.h"
#include "util.h"
#include "config.h"
#include "ltc6804.h"
#include "SSM.h"
#include "charge.h"
#include "charge.h"


static uint32_t consoleTask(void){
    return 0;
}
/**
 * @brief	Main routine for SSP example
 * @return	Nothing
 */
int main(void)
{
	SystemCoreClockUpdate();

	if (SysTick_Config (SystemCoreClock / 1000)) {
		//Error
		while(1);
	}

	/* LED Initialization */

	while(1) {
		uint32_t status = consoleTask();

	}

	return 0;
}

