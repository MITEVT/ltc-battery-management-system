#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>
#include "state_types.h"
#include "discharge.h"

#define MAX_NUM_MODULES 20
#define MAX_CELLS_PER_MODULE 12

// memory allocation for BMS_OUTPUT_T
bool balance_reqs[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
BMS_CHARGE_REQ_T charge_req;
BMS_OUTPUT_T bms_output;

// memory allocation for BMS_INPUT_T
BMS_PACK_STATUS_T pack_status;
BMS_INPUT_T bms_input;

//memory allocation for BMS_STATE_T
BMS_CHARGER_STATUS_T charger_status;
uint32_t cell_voltages[MAX_NUM_MODULES*MAX_CELLS_PER_MODULE];
uint8_t num_cells_in_modules[MAX_NUM_MODULES];
PACK_CONFIG_T pack_config;
BMS_STATE_T bms_state;

TEST_GROUP(Discharge_Test);

TEST_SETUP(Discharge_Test) {
    bms_output.charge_req = &charge_req;
    bms_output.balance_req = balance_reqs;
    bms_output.read_eeprom_packconfig = false;
    bms_output.check_packconfig_with_ltc = false;

    bms_state.charger_status = &charger_status;
    pack_status.cell_voltage_mV = cell_voltages;
    pack_config.num_cells_in_modules = num_cells_in_modules;
    bms_state.pack_config = &pack_config;

    bms_input.pack_status = &pack_status;

    // Go through SSM initialization until in DISCHARGE STATE
    bms_input.mode_request = BMS_SSM_MODE_DISCHARGE;
    SSM_Init(&bms_input, &bms_state, &bms_output);
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    bms_input.eeprom_packconfig_read_done = true;
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    bms_input.ltc_packconfig_check_done = true;
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
	printf("\r(Discharge_Test) [Setup] ");
}

TEST_TEAR_DOWN(Discharge_Test) {
	printf(" [done] \r\n");
}

TEST(Discharge_Test, calculate_max_current) {
    printf("calculate_max_current");
    uint32_t result = Calculate_Max_Current(10, 12, 3, 424217);
    TEST_ASSERT_EQUAL(result, 36);
}

TEST(Discharge_Test, config) {
    printf("config");
    bms_state.pack_config->cell_capacity_cAh = 10;
    bms_state.pack_config->cell_discharge_c_rating_cC = 10;
    bms_state.pack_config->pack_cells_p = 10;
    bms_state.pack_config->max_cell_temp_C = 10;
	Discharge_Config(bms_state.pack_config);
    TEST_ASSERT_EQUAL(Read_Max_Current(), 100);
}

TEST(Discharge_Test, discharge_step_invalid_mode_req) {
    printf("discharge_step_invalid_mode_req");
    bms_input.mode_request = BMS_SSM_MODE_INIT;
    TEST_ASSERT_EQUAL(Discharge_Step(&bms_input, &bms_state, &bms_output),
                        BMS_INVALID_SSM_STATE_ERROR);
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
}

TEST(Discharge_Test, discharge_step_to_standby) {
    printf("discharge_step_to_standby");
    bms_input.mode_request = BMS_SSM_MODE_STANDBY;
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
    TEST_ASSERT_EQUAL(0, Discharge_Step(&bms_input, &bms_state, &bms_output));
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_DISCHARGE, bms_state.curr_mode);
    TEST_ASSERT_FALSE(bms_output.close_contactors);  
    bms_input.contactors_closed = false;
    TEST_ASSERT_EQUAL(0, Discharge_Step(&bms_input, &bms_state, &bms_output));
    TEST_ASSERT_EQUAL(0, Discharge_Step(&bms_input, &bms_state, &bms_output));
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_STANDBY, bms_state.curr_mode);
    TEST_ASSERT_EQUAL(bms_state.discharge_state, BMS_DISCHARGE_OFF);
}

TEST(Discharge_Test, discharge_step_from_junk) {}
TEST(Discharge_Test, discharge_step_to_run) {}
TEST(Discharge_Test, discharge_step_error_one) {}
TEST(Discharge_Test, discharge_step_error_two) {}
TEST(Discharge_Test, discharge_step_error_three) {}

TEST_GROUP_RUNNER(Discharge_Test) {
	RUN_TEST_CASE(Discharge_Test, calculate_max_current);
	RUN_TEST_CASE(Discharge_Test, config);
	RUN_TEST_CASE(Discharge_Test, discharge_step_invalid_mode_req);
	RUN_TEST_CASE(Discharge_Test, discharge_step_to_standby);
	RUN_TEST_CASE(Discharge_Test, discharge_step_from_junk);
	RUN_TEST_CASE(Discharge_Test, discharge_step_to_run);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_one);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_two);
	RUN_TEST_CASE(Discharge_Test, discharge_step_error_three);
}

