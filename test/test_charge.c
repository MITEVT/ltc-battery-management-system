#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>
#include "state_types.h"
#include "charge.h"

#define NUM_MODULES 2

BMS_INPUT_T input;
BMS_OUTPUT_T output;
BMS_STATE_T state;
BMS_CHARGER_STATUS_T _charger_status;
BMS_PACK_STATUS_T _pack_status;
PACK_CONFIG_T config;
uint8_t mod_cell_count[2] = {4, 4};
BMS_CHARGE_REQ_T _charge_req;
bool balance_requests[NUM_MODULES];
BMS_ERROR_T bms_errors[NUM_MODULES];
TEST_GROUP(Charge_Test);

TEST_SETUP(Charge_Test) {
	printf("Charge_Test Setup...");
	state.charger_status = &_charger_status;
	state.pack_status = &_pack_status;
	state.pack_config = &config;
  	
  	config.cell_min_mV = 2400;
	config.cell_max_mV = 3600;
	config.cell_capacity_cAh = 100;
	config.num_modules = NUM_MODULES;

	config.num_cells_in_modules = mod_cell_count;
	config.cell_charge_c_rating_cC = 100;
	config.bal_on_thresh_mV = 4;
	config.bal_off_thresh_mV = 1;
	config.pack_cells_p = 2;

	input.mode_request = BMS_SSM_MODE_STANDBY;
	input.balance_mV = 0;
	input.contactors_closed = false;

	output.charge_req = &_charge_req;
	output.balance_req = balance_requests;
	output.error = bms_errors;

	Charge_Init(&state);
	Charge_Config(&config);
	printf("Done\r\n");
}

TEST_TEAR_DOWN(Charge_Test) {
	printf("Charge_Test Teardown...");
	input.mode_request = BMS_SSM_MODE_STANDBY;
	input.balance_mV = 0;
	input.contactors_closed = false;

	Charge_Step(&input, &state, &output);
	printf("Done\r\n");
}

TEST(Charge_Test, charge_off) {
	printf("Charge_Test charge_off...");
	TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);
	printf("Done\r\n");
}

TEST(Charge_Test, initialize) {
	printf("Charge_Test charge_off...");
	TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

	input.mode_request = BMS_SSM_MODE_CHARGE;
	printf("Done\r\n");
}


TEST_GROUP_RUNNER(Charge_Test) {
	RUN_TEST_CASE(Charge_Test, charge_off);
	RUN_TEST_CASE(Charge_Test, initialize);
}






