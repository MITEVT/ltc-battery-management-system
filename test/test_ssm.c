#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>


TEST_GROUP(SSM_Test);

TEST_SETUP(SSM_Test) {
  
}

TEST_TEAR_DOWN(SSM_Test) {
}

TEST(SSM_Test, is_valid_jump) {
	TEST_ASSERT(1);
}

TEST(SSM_Test, ssm_init) {
	TEST_ASSERT(1);
}

TEST(SSM_Test, init_step) {
	TEST_ASSERT(1);
}

TEST_GROUP_RUNNER(SSM_Test) {
	RUN_TEST_CASE(SSM_Test, this_should_pass);
}







