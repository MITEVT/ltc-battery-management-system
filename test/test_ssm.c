#include "ssm.h"
#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>

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

TEST_GROUP(SSM_Test);

TEST_SETUP(SSM_Test) {
	printf("(SSM/Init/Error) [setup] ");
    bms_output.charge_req = &charge_req;
    bms_output.balance_req = balance_reqs;
    bms_output.read_eeprom_packconfig = false;
    bms_output.check_packconfig_with_ltc = false;

    bms_state.charger_status = &charger_status;
    pack_status.cell_voltage_mV = cell_voltages;
    pack_config.num_cells_in_modules = num_cells_in_modules;
    bms_state.pack_config = &pack_config;

    bms_input.pack_status = &pack_status;
    bms_output.read_eeprom_packconfig = false;
    SSM_Init(&bms_input, &bms_state, &bms_output);
}

TEST_TEAR_DOWN(SSM_Test) {
	printf(" [done] \r\n");
}

TEST(SSM_Test, ssm_init) {
    printf("ssm_init");
    SSM_Init(&bms_input, &bms_state, &bms_output);
	TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_INIT);
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_OFF);
	TEST_ASSERT_EQUAL(bms_state.error_code, BMS_NO_ERROR);

    TEST_ASSERT_FALSE(bms_output.read_eeprom_packconfig);
    TEST_ASSERT_FALSE(bms_output.check_packconfig_with_ltc);
}

TEST(SSM_Test, init_step_complete) {
    printf("init_step_complete");

    Init_Step(&bms_input, &bms_state, &bms_output); 
	TEST_ASSERT(bms_output.read_eeprom_packconfig);
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_READ_PACKCONFIG);

    bms_input.eeprom_packconfig_read_done = true;
    
    Init_Step(&bms_input, &bms_state, &bms_output); 
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_CHECK_PACKCONFIG);
    TEST_ASSERT(bms_output.check_packconfig_with_ltc);
    
    bms_input.ltc_packconfig_check_done = true;

    Init_Step(&bms_input, &bms_state, &bms_output); 
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_DONE);
	TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_STANDBY);
    TEST_ASSERT_FALSE(bms_output.check_packconfig_with_ltc);

    Init_Step(&bms_input, &bms_state, &bms_output); 
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_DONE);
	TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_STANDBY);
}

TEST(SSM_Test, init_step_blocked) {
    printf("init_step_blocked");
    Init_Step(&bms_input, &bms_state, &bms_output); 
    Init_Step(&bms_input, &bms_state, &bms_output); 
	TEST_ASSERT(bms_output.read_eeprom_packconfig);
	TEST_ASSERT_EQUAL(bms_state.init_state, BMS_INIT_READ_PACKCONFIG);
}

TEST(SSM_Test, is_valid_jump) {
    printf("is_valid_jump");
	TEST_ASSERT(1);
}

TEST(SSM_Test, is_state_done) {
    printf("is_state_done");
	TEST_ASSERT(1);
}

TEST(SSM_Test, ssm_step) {
    printf("ssm_step");
	TEST_ASSERT(1);
}

TEST_GROUP_RUNNER(SSM_Test) {
	RUN_TEST_CASE(SSM_Test, ssm_init);
	RUN_TEST_CASE(SSM_Test, init_step_complete);
	RUN_TEST_CASE(SSM_Test, init_step_blocked);
	RUN_TEST_CASE(SSM_Test, is_valid_jump);
	RUN_TEST_CASE(SSM_Test, is_state_done);
	RUN_TEST_CASE(SSM_Test, ssm_step);
}

