#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>
#include "state_types.h"
#include "discharge.h"

BMS_INPUT_T input;
BMS_OUTPUT_T output;
BMS_STATE_T state;

TEST_GROUP(Discharge_Test);

TEST_SETUP(Discharge_Test) {
	printf("\r(Discharge_Test) [Setup] ");
	Discharge_Init(&state);
	Discharge_Config(&state.pack_config);
}

TEST_TEAR_DOWN(Discharge_Test) {
	printf(" [done] \r\n");
}

TEST(Discharge_Test, calculate_current) {}
TEST(Discharge_Test, discharge_init_config) {}
TEST(Discharge_Test, discharge_step_to_junk) {}
TEST(Discharge_Test, discharge_step_to_standby) {}
TEST(Discharge_Test, discharge_step_from_junk) {}
TEST(Discharge_Test, discharge_step_to_run) {}
TEST(Discharge_Test, discharge_step_error_one) {}
TEST(Discharge_Test, discharge_step_error_two) {}
TEST(Discharge_Test, discharge_step_error_three) {}

TEST_GROUP_RUNNER(Discharge_Test) {
	RUN_TEST_CASE(Discharge_Test, calculate_current);
	RUN_TEST_CASE(Discharge_Test, discharge_init_config);
	RUN_TEST_CASE(Discharge_Test, discharge_step_to_junk);
	RUN_TEST_CASE(Discharge_Test, discharge_step_to_standby);
	RUN_TEST_CASE(Discharge_Test, discharge_step_from_junk);
	RUN_TEST_CASE(Discharge_Test, discharge_step_to_run);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_one);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_two);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_three);
}

