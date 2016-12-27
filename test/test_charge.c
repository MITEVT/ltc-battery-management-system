#include "unity.h"
#include "unity_fixture.h"
#include <stdio.h>

TEST_GROUP(Charge_Test);

TEST_SETUP(Charge_Test) {
  
}

TEST_TEAR_DOWN(Charge_Test) {
	
}

TEST(Charge_Test, this_should_pass) {
	TEST_ASSERT(1);

}


TEST_GROUP_RUNNER(Charge_Test) {
	RUN_TEST_CASE(SSM_Test, this_should_pass);

}






