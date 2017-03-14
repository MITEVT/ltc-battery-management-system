// Unity
#include "unity.h"
#include "unity_fixture.h"

// ltc-battery-managament-system
#include "state_types.h"
#include "cell_temperatures.h"

// C libraries
#include <stdio.h>

/**
 * Testing Strategy
 * 
 * CellTemperatures_Step():
 * - iterate through all value of currentThermistor
 */

// Memory allocation for BMS_PACK_STATUS_T
BMS_PACK_STATUS_T pack_status;

TEST_GROUP(Cell_Temperatures_Test);

TEST_SETUP(Cell_Temperatures_Test) {
	printf("\r(Cell_Temperatures_Test)Setup");
	printf("...");
}

TEST_TEAR_DOWN(Cell_Temperatures_Test) {
	printf("...");
	printf("Teardown\r\n");
}

/**
 * Covers:
 * CellTemperatures_GetThermistorAddress():
 * - currentThermistor:
 *   - 0
 */
void test_CellTemperatures_GetThermistorAddress_CurrentThermistorZero(void) {
	const uint8_t currentThermistor = 0;
	const uint8_t 
		expectedThermistorAddress[NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS] = 
		{0, 0, 0, 0, 0};
	CellTemperatures_GetThermistorAddress(currentThermistor,
			BMS_PACK_STATUS_T * pack_status);
	for (uint8_t i=0; i<NUMBER_OF_MULTIPLEXER_LOGIC_CONTROL_INPUTS; i++) {
		// TODO
	}
}

TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
	// TODO
}
