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

// Memory allocation for BMS_STATE_T
BMS_STATE_T bms_state;

TEST_GROUP(Cell_Temperatures_Test);

TEST_SETUP(Cell_Temperatures_Test) {
	printf("\r(Cell_Temperatures_Test)Setup");
	bms_state.currentThermistor = 0;
	printf("...");
}

TEST_TEAR_DOWN(Cell_Temperatures_Test) {
	printf("...");
	printf("Teardown\r\n");
}

/**
 * Covers:
 * CellTemperatures_Step():
 * - iterate through all value of currentThermistor
 */
TEST(Cell_Temperatures_Test, step) {
	printf("step");

	TEST_ASSERT_EQUAL_INT(0, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(1, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(2, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(3, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(4, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(5, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(6, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(7, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(8, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(9, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(10, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(11, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(12, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(13, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(14, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(15, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(16, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(17, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(18, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(19, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(20, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(21, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(22, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(23, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(0, bms_state.currentThermistor);
	CellTemperatures_Step(&bms_state);
	TEST_ASSERT_EQUAL_INT(1, bms_state.currentThermistor);

}


TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
	RUN_TEST_CASE(Cell_Temperatures_Test, step);
}
