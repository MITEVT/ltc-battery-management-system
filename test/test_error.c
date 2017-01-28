#include "error_handler.h"
#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>


TEST_GROUP(ERROR_Test);

TEST_SETUP(ERROR_Test) {
    Error_Init();
}

TEST_TEAR_DOWN(ERROR_Test) {
	printf(" [done] \r\n");
}

TEST(ERROR_Test, INIT_PASS) {
    TEST_ASSERT_EQUAL(Error_Handle(0), HANDLER_FINE);
}

//[TODO] magic numbers
TEST(ERROR_Test, UNDERVOLTAGE_PASS_NEVER_HALTS) {
    for (int i = 0; i < 1000; ++i){
        Error_Pass(ERROR_CELL_UNDER_VOLTAGE, (i*1000)+1);
        TEST_ASSERT_EQUAL(Error_Handle((i+1)*1000), HANDLER_FINE);
    }
}

//[TODO] magic numbers
TEST(ERROR_Test, UNDERVOLTAGE_ASSERT_HALTS_NEEDED) {
    Error_Assert(ERROR_CELL_UNDER_VOLTAGE, 0);
    for (int i = 0; i < 1000; ++i) {
        TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(i));
        Error_Assert(ERROR_CELL_UNDER_VOLTAGE,i);
    }
    TEST_ASSERT_EQUAL(HANDLER_HALT, Error_Handle(1000));
}

TEST(ERROR_Test, UNDERVOLTAGE_HANDLED_FINE) {
    Error_Assert(ERROR_CELL_UNDER_VOLTAGE, 0);
    for (int i = 0; i < 1000; ++i) {
        Error_Assert(ERROR_CELL_UNDER_VOLTAGE,i);
        TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(i));
    }
    Error_Pass(ERROR_CELL_UNDER_VOLTAGE, 1000);
    TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(1000));
}


/*
/ OVERVOLTAGE TESTS
*/

//[TODO] magic numbers
TEST(ERROR_Test, OVERVOLTAGE_PASS_NEVER_HALTS) {
    for (int i = 0; i < 1000; ++i){
        Error_Pass(ERROR_CELL_UNDER_VOLTAGE, (i*1000)+1);
        TEST_ASSERT_EQUAL(Error_Handle((i+1)*1000), HANDLER_FINE);
    }
}

//[TODO] magic numbers
TEST(ERROR_Test, OVERVOLTAGE_ASSERT_HALTS_NEEDED) {
    Error_Assert(ERROR_CELL_OVER_VOLTAGE, 0);
    for (int i = 0; i < 1; ++i) {
        Error_Assert(ERROR_CELL_OVER_VOLTAGE,i);
        TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(i));

    }
    TEST_ASSERT_EQUAL(HANDLER_HALT, Error_Handle(1000));
}

TEST(ERROR_Test, OVERVOLTAGE_HANDLED_FINE) {
    Error_Assert(ERROR_CELL_OVER_VOLTAGE, 0);
    for (int i = 0; i < 1000; ++i) {
        Error_Assert(ERROR_CELL_OVER_VOLTAGE,i);
        TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(i));
    }
    Error_Pass(ERROR_CELL_OVER_VOLTAGE, 1000);
    TEST_ASSERT_EQUAL(HANDLER_FINE, Error_Handle(1000));
}





TEST_GROUP_RUNNER(ERROR_Test) {
    RUN_TEST_CASE(ERROR_Test, INIT_PASS);
	RUN_TEST_CASE(ERROR_Test, UNDERVOLTAGE_PASS_NEVER_HALTS);
    RUN_TEST_CASE(ERROR_Test, UNDERVOLTAGE_ASSERT_HALTS_NEEDED);

    RUN_TEST_CASE(ERROR_Test, OVERVOLTAGE_PASS_NEVER_HALTS);
    RUN_TEST_CASE(ERROR_Test, OVERVOLTAGE_ASSERT_HALTS_NEEDED);

}