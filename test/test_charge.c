#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>
#include "state_types.h"
#include "charge.h"

#define NUM_MODULES 2
#define TOTAL_CELLS NUM_MODULES*2
#define CELL_MAX 3600
#define CELL_MIN 2400
#define CHARGE_C_RATING 1
#define CELL_CAPACITY_CAh 100
#define CC_CHARGE_VOLTAGE CELL_MAX*TOTAL_CELLS
#define CC_CHARGE_CURRENT CELL_CAPACITY_CAh*CHARGE_C_RATING*10
#define CV_CHARGE_VOLTAGE CELL_MAX*TOTAL_CELLS
#define CV_CHARGE_CURRENT CC_CHARGE_CURRENT

BMS_INPUT_T input;
BMS_OUTPUT_T output;
BMS_STATE_T state;
BMS_CHARGER_STATUS_T _charger_status;
BMS_PACK_STATUS_T _pack_status;
PACK_CONFIG_T config;
uint8_t mod_cell_count[NUM_MODULES] = {2, 2};
uint32_t cell_voltages_mV[TOTAL_CELLS] = {3400, 3401, 3402, 3403};
BMS_CHARGE_REQ_T _charge_req;
bool balance_requests[NUM_MODULES];

void Test_Charge_SM_Shutdown(void);

TEST_GROUP(Charge_Test);

TEST_SETUP(Charge_Test) {
    printf("\r(Charge_Test)Setup");
    state.charger_status = &_charger_status;
    state.pack_config = &config;
    
    config.cell_min_mV = CELL_MIN;
    config.cell_max_mV = CELL_MAX;
    config.cell_capacity_cAh = CELL_CAPACITY_CAh;
    config.num_modules = NUM_MODULES;

    config.module_cell_count = mod_cell_count;
    config.cell_charge_c_rating_cC = CHARGE_C_RATING*100;
    config.bal_on_thresh_mV = 4;
    config.bal_off_thresh_mV = 1;
    config.pack_cells_p = 1;
    config.cc_cell_voltage_mV = CELL_MAX;
    config.cv_min_current_mA = 100;
    config.cv_min_current_ms = 100; 

    input.mode_request = BMS_SSM_MODE_STANDBY;
    input.balance_mV = 0;
    input.contactors_closed = false;
    input.pack_status = &_pack_status;
    input.pack_status->cell_voltages_mV = cell_voltages_mV;
    input.pack_status->pack_cell_max_mV = CELL_MAX;
    input.charger_on = false;
    input.msTicks = 0;
    
    output.charge_req = &_charge_req;
    output.balance_req = balance_requests;

    Charge_Init(&state);
    Charge_Config(&config);
    printf("...");
}

TEST_TEAR_DOWN(Charge_Test) {
    printf("...");
    input.mode_request = BMS_SSM_MODE_STANDBY;
    input.balance_mV = 0;
    input.contactors_closed = false;
    input.charger_on = false;

    Charge_Step(&input, &state, &output);
    printf("Teardown\r\n");
}

TEST(Charge_Test, charge_off) {
    printf("off");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);
}

TEST(Charge_Test, initialize) {
    printf("init");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = false;
    input.charger_on = false;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_INIT, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);

    input.contactors_closed = true;
    input.charger_on = true;
    Charge_Step(&input, &state, &output);
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);

    input.mode_request = BMS_SSM_MODE_STANDBY;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on)

    input.contactors_closed = false;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);

    input.charger_on = false;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_OFF, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
}

TEST(Charge_Test, to_cc) {
    printf("to_cc");
    TEST_ASSERT_EQUAL(BMS_CHARGE_OFF, state.charge_state);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3400;
    input.pack_status->pack_cell_max_mV = 3403;
    cell_voltages_mV[0] = 3400; cell_voltages_mV[1] = 3401; cell_voltages_mV[2] = 3402; cell_voltages_mV[3] = 3403;
    
    Charge_Step(&input, &state, &output); // [TODO] This is why I use goto
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CC_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CC_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }


    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cc_w_bal) {
    printf("to_cc_w_bal");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3400;
    input.pack_status->pack_cell_max_mV = 3405;
    cell_voltages_mV[0] = 3400; cell_voltages_mV[1] = 3401; cell_voltages_mV[2] = 3402; cell_voltages_mV[3] = 3405;
    
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CC_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CC_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i=0; i<TOTAL_CELLS-1; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }
    TEST_ASSERT_TRUE(output.balance_req[TOTAL_CELLS - 1]);

    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cc_w_bal_2) {
    // Balances and then finishes balancing in cc
    printf("to_cc_w_bal_2");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3400;
    input.pack_status->pack_cell_max_mV = 3405;
    cell_voltages_mV[0] = 3400; cell_voltages_mV[1] = 3401; cell_voltages_mV[2] = 3402; cell_voltages_mV[3] = 3405;

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_EQUAL(CC_CHARGE_VOLTAGE, output.charge_req->charge_voltage_mV);
    TEST_ASSERT_EQUAL(CC_CHARGE_CURRENT, output.charge_req->charge_current_mA);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i=0; i<TOTAL_CELLS-1; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }
    TEST_ASSERT_TRUE(output.balance_req[TOTAL_CELLS - 1]);  

    input.pack_status->pack_cell_max_mV = 3401;
    cell_voltages_mV[TOTAL_CELLS - 1] = 3401;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CC_CHARGE_VOLTAGE);
        TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CC_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cv) {
    printf("to_cv");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3599;
    input.pack_status->pack_cell_max_mV = 3600;
    cell_voltages_mV[0] = 3599; cell_voltages_mV[1] = 3600; cell_voltages_mV[2] = 3599; cell_voltages_mV[3] = 3600;

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }
    
    Charge_Step(&input, &state, &output);   
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CV_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CV_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    
    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cv_w_bal) {
    printf("to_cv_w_bal");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3594;
    input.pack_status->pack_cell_max_mV = 3600;
    cell_voltages_mV[0] = 3594; cell_voltages_mV[1] = 3595; cell_voltages_mV[2] = 3599; cell_voltages_mV[3] = 3600;
    
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CV_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CV_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS - 2; i++)
        TEST_ASSERT_FALSE(output.balance_req[i]);
    TEST_ASSERT_TRUE(output.balance_req[2]); TEST_ASSERT_TRUE(output.balance_req[3]);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);

    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cc_to_cv) {
    // CC to CV with no balance
    printf("to_cc_to_cv");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3400;
    input.pack_status->pack_cell_max_mV = 3403;
    cell_voltages_mV[0] = 3400; cell_voltages_mV[1] = 3401; cell_voltages_mV[2] = 3402; cell_voltages_mV[3] = 3403;
    
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CC, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CC_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CC_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS; i++) {
        TEST_ASSERT_FALSE(output.balance_req[i]);
    }

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.pack_status->pack_cell_min_mV = 3599;
    input.pack_status->pack_cell_max_mV = 3600;
    cell_voltages_mV[0] = 3599; cell_voltages_mV[1] = 3600; cell_voltages_mV[2] = 3599; cell_voltages_mV[3] = 3600;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CV_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CV_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS; i++)
        TEST_ASSERT_FALSE(output.balance_req[i]);
    
    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cc_to_cv_w_bal) {
    
}

TEST(Charge_Test, to_bal) {
    printf("to_bal");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_BALANCE;
    input.contactors_closed = false;
    input.charger_on = false;
    input.pack_status->pack_cell_min_mV = 3400;
    input.pack_status->pack_cell_max_mV = 3403;
    input.balance_mV = 3395;
    cell_voltages_mV[0] = 3400; cell_voltages_mV[1] = 3401; cell_voltages_mV[2] = 3402; cell_voltages_mV[3] = 3403;
    
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_BAL, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_BAL, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++)
        TEST_ASSERT_TRUE(output.balance_req[i]);

    input.contactors_closed = true;
    Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_cv_finish) {
    printf("to_cv_finish");
    TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

    input.mode_request = BMS_SSM_MODE_CHARGE;
    input.contactors_closed = true;
    input.charger_on = true;
    input.pack_status->pack_cell_min_mV = 3599;
    input.pack_status->pack_cell_max_mV = 3600;
    cell_voltages_mV[0] = 3599; cell_voltages_mV[1] = 3600; cell_voltages_mV[2] = 3599; cell_voltages_mV[3] = 3600;

    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_TRUE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    int i;
    for (i = 0; i < TOTAL_CELLS; i++) {
            TEST_ASSERT_FALSE(output.balance_req[i]);
    }
    
    Charge_Step(&input, &state, &output);   
    TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CV_CHARGE_VOLTAGE);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CV_CHARGE_CURRENT);
    TEST_ASSERT_TRUE(output.charge_req->charger_on);
    TEST_ASSERT_TRUE(output.close_contactors);

    input.pack_status->pack_current_mA = 99;
    uint32_t j;
    for (j = 0; j < config.cv_min_current_ms; j++) {
        Charge_Step(&input, &state, &output);   
        TEST_ASSERT_EQUAL(BMS_CHARGE_CV, state.charge_state);
        TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, CV_CHARGE_VOLTAGE);
        TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, CV_CHARGE_CURRENT);
        TEST_ASSERT_TRUE(output.charge_req->charger_on);
        TEST_ASSERT_TRUE(output.close_contactors);
        input.msTicks++;
    }

    Charge_Step(&input, &state, &output);   
    TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
    TEST_ASSERT_EQUAL(output.charge_req->charge_voltage_mV, 0);
    TEST_ASSERT_EQUAL(output.charge_req->charge_current_mA, 0);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
    TEST_ASSERT_FALSE(output.close_contactors);

    Test_Charge_SM_Shutdown();
}

/*
 * Testing strategy:
 *   - Test standby mode requests
 *       - starting state:
 *           - BMS_CHARGE_OFF (expected transition: OFF->OFF)
 *           - BMS_CHARGE_INIT (expected transition: CHARGE->DONE->OFF)
 *           - BMS_CHARGE_CC (expected transition: CHARGE->DONE->OFF)
 *           - BMS_CHARGE_CV (expected transition: CHARGE->DONE->OFF)
 *           - BMS_CHARGE_BAL (expected transition: CHARGE->DONE->OFF)
 *           - BMS_CHARGE_DONE (expected transition: DONE->OFF)
 */

/*
 * Covers:
 *   - Test standby mode requests
 *       - starting state:
 *           - BMS_CHARGE_INIT (expected transition: CHARGE->DONE->OFF)
 */
TEST(Charge_Test, test_standby_mode_request_from_charge_init) {
        printf("test_standby_mode_request_from_charge_init");
        TEST_ASSERT_EQUAL(state.charge_state, BMS_CHARGE_OFF);

        //Move to BMS_CHARGE_INIT
        input.mode_request = BMS_SSM_MODE_CHARGE;
        Charge_Step(&input, &state, &output);
        TEST_ASSERT_EQUAL(BMS_CHARGE_INIT, state.charge_state);
        TEST_ASSERT_TRUE(output.close_contactors);
        TEST_ASSERT_FALSE(output.charge_req->charger_on);
        uint32_t i;
        for (i=0; i<(sizeof(output.balance_req[0])*TOTAL_CELLS); i++) {
                TEST_ASSERT_EQUAL(0, output.balance_req[i]);
        }

//      input.mode_request = BMS_SSM_MODE_STANDBY;
//      Charge_Step(&input, &state, &output);
//      TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
//      TEST_ASSERT_FALSE(output.close_contactors);
//      TEST_ASSERT_FALSE(output.charge_req->charger_on);
//      for (i=0; i<(sizeof(output.balance_req[0])*TOTAL_CELLS); i++) {
//                TEST_ASSERT_EQUAL(0, output.balance_req[i]);
//        }

        //Don't go to BMS_CHARGE_OFF yet because the contactors are closed
        input.mode_request = BMS_SSM_MODE_STANDBY;
        input.contactors_closed = true;
         input.charger_on = true;
        Charge_Step(&input, &state, &output);
        TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
        TEST_ASSERT_FALSE(output.close_contactors);
        TEST_ASSERT_FALSE(output.charge_req->charger_on);
        for (i=0; i<(sizeof(output.balance_req[0])*TOTAL_CELLS); i++) {
                TEST_ASSERT_EQUAL(0, output.balance_req[i]);
        }

        // //Open contactors. Expect transition to BMS_CHARGE_OFF
        // input.mode_request = BMS_SSM_MODE_STANDBY;
        // input.contactors_closed = false;
        // Charge_Step(&input, &state, &output);
        // TEST_ASSERT_EQUAL(BMS_CHARGE_OFF, state.charge_state);
        // TEST_ASSERT_FALSE(output.close_contactors);
        // TEST_ASSERT_FALSE(output.charge_req->charger_on);
        // for (i=0; i<(sizeof(output.balance_req[0])*TOTAL_CELLS); i++) {
        //         TEST_ASSERT_EQUAL(0, output.balance_req[i]);
        // }

        Test_Charge_SM_Shutdown();
}

TEST(Charge_Test, to_bal_finish) {

}

TEST(Charge_Test, finish_charge) {

}


TEST_GROUP_RUNNER(Charge_Test) {
    RUN_TEST_CASE(Charge_Test, charge_off);
    RUN_TEST_CASE(Charge_Test, initialize);
    RUN_TEST_CASE(Charge_Test, to_cc);
    RUN_TEST_CASE(Charge_Test, to_cc_w_bal);
    RUN_TEST_CASE(Charge_Test, to_cc_w_bal_2);
    RUN_TEST_CASE(Charge_Test, to_cv);
    RUN_TEST_CASE(Charge_Test, to_cv_w_bal);
    RUN_TEST_CASE(Charge_Test, to_cc_to_cv);
    RUN_TEST_CASE(Charge_Test, to_bal);
    RUN_TEST_CASE(Charge_Test, to_cv_finish);
    RUN_TEST_CASE(Charge_Test, test_standby_mode_request_from_charge_init);
}

void Test_Charge_SM_Shutdown(void) {
    int i;
    input.mode_request = BMS_SSM_MODE_STANDBY;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_DONE, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    for (i = 0; i < TOTAL_CELLS; i++)
        TEST_ASSERT_FALSE(output.balance_req[i]);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);

    input.contactors_closed = false;
    input.charger_on = false;
    Charge_Step(&input, &state, &output);
    TEST_ASSERT_EQUAL(BMS_CHARGE_OFF, state.charge_state);
    TEST_ASSERT_FALSE(output.close_contactors);
    TEST_ASSERT_FALSE(output.charge_req->charger_on);
}

