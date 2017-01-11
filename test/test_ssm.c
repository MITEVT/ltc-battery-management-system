#include "ssm.h"
#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>


TEST_GROUP(SSM_Test);

TEST_SETUP(SSM_Test) {
	printf("(SSM/Init Test) ");
  
}

TEST_TEAR_DOWN(SSM_Test) {
	printf("\r\n");
}

TEST(SSM_Test, ssm_init) {
    printf("ssm_init");
	TEST_ASSERT(1);
}

TEST(SSM_Test, init_step) {
    printf("init_step");
	TEST_ASSERT(1);
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
	RUN_TEST_CASE(SSM_Test, init_step);
	RUN_TEST_CASE(SSM_Test, is_valid_jump);
	RUN_TEST_CASE(SSM_Test, is_state_done);
	RUN_TEST_CASE(SSM_Test, ssm_step);
}

