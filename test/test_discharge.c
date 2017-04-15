#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>
#include "state_types.h"
#include "bms_utils.h"
#include "ssm.h"
#include "discharge.h"

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

void Set_PackConfig(
        uint8_t cell_capacity_cAh, uint8_t cell_discharge_c_rating_cC,
        uint8_t pack_cells_p, uint8_t max_cell_temp_dC,
        uint8_t min_cell_voltage_mV,
        uint8_t max_cell_voltage_mV,
        uint8_t num_modules, uint8_t num_cells_in_module1, uint8_t num_cells_in_module2
        );

TEST_GROUP(Discharge_Test);

TEST_SETUP(Discharge_Test) {
    bms_output.charge_req = &charge_req;
    bms_output.balance_req = balance_reqs;
    bms_output.read_eeprom_packconfig = false;
    bms_output.check_packconfig_with_ltc = false;

    bms_state.charger_status = &charger_status;
    pack_status.cell_voltages_mV = cell_voltages;
    pack_config.module_cell_count = num_cells_in_modules;
    bms_state.pack_config = &pack_config;

    bms_input.pack_status = &pack_status;

    // Go through SSM initialization until in DISCHARGE STATE
    bms_input.mode_request = BMS_SSM_MODE_DISCHARGE;
    SSM_Init(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_INIT, bms_state.curr_mode);
    TEST_ASSERT_EQUAL(BMS_INIT_OFF, bms_state.init_state);
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    bms_input.eeprom_packconfig_read_done = true;
    TEST_ASSERT_EQUAL(BMS_INIT_READ_PACKCONFIG, bms_state.init_state);
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_INIT, bms_state.curr_mode);
    bms_input.ltc_packconfig_check_done = true;
    SSM_Step(&bms_input, &bms_state, &bms_output); 
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_STANDBY, bms_state.curr_mode);
    
    bms_input.contactors_closed = false;
    Set_PackConfig(10, 10, 10, 10, 40, 100, 2, 3, 3);
    Discharge_Config(bms_state.pack_config);
    bms_input.pack_status->pack_cell_min_mV = 60;
    // necessary because first goes to standby then discharge
    
    SSM_Step(&bms_input, &bms_state, &bms_output);  
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);


    printf("\r(Discharge_Test)Setup...");
}

TEST_TEAR_DOWN(Discharge_Test) {
    printf("Teardown\r\n");
}

TEST(Discharge_Test, calculate_max_current) {
    printf("calculate_max_current...");
    uint32_t result = Calculate_Max_Current(10, 12, 3, 100);
    TEST_ASSERT_EQUAL(result, 36);
}

void Set_PackConfig(
        uint8_t cell_capacity_cAh, uint8_t cell_discharge_c_rating_cC,
        uint8_t pack_cells_p, uint8_t max_cell_temp_dC,
        uint8_t min_cell_voltage_mV,
        uint8_t max_cell_voltage_mV,
        uint8_t num_modules, uint8_t num_cells_in_module1, uint8_t num_cells_in_module2
        ) {
    bms_state.pack_config->cell_capacity_cAh = cell_capacity_cAh;
    bms_state.pack_config->cell_discharge_c_rating_cC = cell_discharge_c_rating_cC;
    bms_state.pack_config->pack_cells_p = pack_cells_p;
    bms_state.pack_config->max_cell_temp_dC = max_cell_temp_dC;
    bms_state.pack_config->cell_min_mV = min_cell_voltage_mV;
    bms_state.pack_config->cell_max_mV = max_cell_voltage_mV;
    bms_state.pack_config->num_modules = num_modules;
    bms_state.pack_config->module_cell_count[0] = num_cells_in_module1;
    bms_state.pack_config->module_cell_count[1] = num_cells_in_module2;
}

TEST(Discharge_Test, config) {
    printf("config...");
    Discharge_Config(bms_state.pack_config);
    TEST_ASSERT_EQUAL(100, Read_Max_Current());
}

TEST(Discharge_Test, discharge_step_invalid_mode_req) {
    printf("discharge_step_invalid_mode_req...");
    bms_input.mode_request = BMS_SSM_MODE_INIT;
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
}

TEST(Discharge_Test, discharge_step_to_standby) {
    printf("discharge_step_to_standby...");
    bms_input.mode_request = BMS_SSM_MODE_STANDBY;
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_DISCHARGE, bms_state.curr_mode);
    TEST_ASSERT_FALSE(bms_output.close_contactors);  
    bms_input.contactors_closed = false;
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(bms_state.discharge_state, BMS_DISCHARGE_OFF);
    SSM_Step(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(BMS_SSM_MODE_STANDBY, bms_state.curr_mode);
}

TEST(Discharge_Test, discharge_step_to_run) {
    printf("discharge_step_to_run...");
    // from test init, input request is already in discharge
    Discharge_Step(&bms_input, &bms_state, &bms_output);

    bms_input.contactors_closed = true;
    bms_input.pack_status->pack_current_mA = 10;
    uint8_t i = 0;
    for(i = 0; i < Get_Total_Cell_Count(bms_state.pack_config); i++) {
        bms_input.pack_status->cell_voltages_mV[i] = 70;
    }
    bms_input.pack_status->max_cell_temp_dC = 1;
    
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    TEST_ASSERT_EQUAL(bms_state.curr_mode, BMS_SSM_MODE_DISCHARGE);
    TEST_ASSERT_EQUAL(bms_state.discharge_state, BMS_DISCHARGE_RUN);
}

TEST(Discharge_Test, discharge_step_undervoltage_error) {
    printf("discharge_step_undervoltage_error...");
    // from test init, input request is already in discharge
    Discharge_Step(&bms_input, &bms_state, &bms_output);
    
    bms_input.contactors_closed = true;
    bms_input.pack_status->pack_current_mA = 10;
    uint8_t i = 0;
    for(i = 0; i < Get_Total_Cell_Count(bms_state.pack_config); i++) {
        bms_input.pack_status->cell_voltages_mV[i] = 0;
    }
    bms_input.pack_status->max_cell_temp_dC = 1;
    
    Discharge_Step(&bms_input, &bms_state, &bms_output);
}

TEST(Discharge_Test, discharge_step_overcurrent_error) {
    printf("discharge_step_overcurrent_error...");
    // from test init, input request is already in discharge
    Discharge_Step(&bms_input, &bms_state, &bms_output);

    bms_input.contactors_closed = true;
    bms_input.pack_status->pack_current_mA = 10000;
    uint8_t i = 0;
    for(i = 0; i < Get_Total_Cell_Count(bms_state.pack_config); i++) {
        bms_input.pack_status->cell_voltages_mV[i] = 70;
    }
    bms_input.pack_status->max_cell_temp_dC = 1;
    
    Discharge_Step(&bms_input, &bms_state, &bms_output);
}


TEST_GROUP_RUNNER(Discharge_Test) {
    RUN_TEST_CASE(Discharge_Test, calculate_max_current);
    RUN_TEST_CASE(Discharge_Test, config);
    RUN_TEST_CASE(Discharge_Test, discharge_step_invalid_mode_req);
    RUN_TEST_CASE(Discharge_Test, discharge_step_to_standby);
    RUN_TEST_CASE(Discharge_Test, discharge_step_to_run);
    // RUN_TEST_CASE(Discharge_Test, discharge_step_undervoltage_error);
    // RUN_TEST_CASE(Discharge_Test, discharge_step_overcurrent_error);
}

