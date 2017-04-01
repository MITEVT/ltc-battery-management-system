#include "unity_fixture.h"

static void RunAllTests(void) {
  RUN_TEST_GROUP(Charge_Test);
  RUN_TEST_GROUP(SSM_Test);
  RUN_TEST_GROUP(Discharge_Test);
  RUN_TEST_GROUP(ERROR_Test);
  RUN_TEST_GROUP(BMS_CAN_Test);
<<<<<<< HEAD
=======
  RUN_TEST_GROUP(Cell_Temperatures_Test);
>>>>>>> 5683f1ae3d66e2287806c46cef596bc8fcfea39a
}

int main(int argc, const char * argv[]) {
  return UnityMain(argc, argv, RunAllTests);
}
