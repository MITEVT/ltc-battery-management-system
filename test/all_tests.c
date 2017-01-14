#include "unity_fixture.h"

static void RunAllTests(void) {
  RUN_TEST_GROUP(Charge_Test);
  RUN_TEST_GROUP(SSM_Test);
  RUN_TEST_GROUP(Discharge_Test);
}

int main(int argc, char * argv[]) {
  return UnityMain(argc, argv, RunAllTests);
}
