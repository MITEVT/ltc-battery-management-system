// Unity
#include "unity.h"
#include "unity_fixture.h"

// ltc-battery-managament-system
#include "state_types.h"
#include "cell_temperatures.h"
#include "board.h"

// C libraries
#include <stdio.h>


/**
 * Testing Strategy
 * 
 * CellTemperatures_UpdateCellTemperatures()
 * - currentThermistor:
 *   - 0
 *   - 0 < currentThermistor < 23
 *   - 23
 */

// Global variables
BMS_PACK_STATUS_T pack_status;
uint16_t cell_temperatures_mV[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
uint32_t gpioVoltages[MAX_NUM_MODULES * LTC6804_GPIO_COUNT];

TEST_GROUP(Cell_Temperatures_Test);

TEST_SETUP(Cell_Temperatures_Test) {
    printf("\r(Cell_Temperatures_Test)Setup");

    pack_status.cell_temperatures_mV = cell_temperatures_mV;
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_mV[i] = 0;
    }
    
    // Set gpioVoltages = {0,0,0,0,1,0,0,0,0,2,0,0,0,0,3,...}
    for (i=0; i<MAX_NUM_MODULES*LTC6804_GPIO_COUNT; i++) {
        gpioVoltages[i] = 0;
    }
    for (i=0; i<MAX_NUM_MODULES; i++) {
       gpioVoltages[i*LTC6804_GPIO_COUNT] = i;
    }

    printf("...");
}

TEST_TEAR_DOWN(Cell_Temperatures_Test) {
    printf("...");
    printf("Teardown\r\n");
}

 /** 
  * CellTemperatures_UpdateCellTemperatures()
  * - currentThermistor:
  *   - 0
  */
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorZero) {
    printf("UpdateCellTemperatures_currentThermistorZero");

    uint8_t currentThermistor = 0;
    uint8_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    // Set expectedCellTemperatures to {0,0(x23),1,0(x23),2(x23),...}
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        expectedCellTemperatures[i] = 0;
    }
    for (i=0; i<MAX_NUM_MODULES; i++) {
        expectedCellTemperatures[i*MAX_THERMISTORS_PER_MODULE] = i;
    }
    
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_mV[i]);
    }
}

 /** 
  * CellTemperatures_UpdateCellTemperatures()
  * - currentThermistor:
  *   - 0<currentThermistor<23
  */
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTen) {
    printf("UpdateCellTemperatures_currentThermistorTen");

    uint8_t currentThermistor = 10;
    uint8_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        expectedCellTemperatures[i] = 0;
    }
    for (i=0; i<MAX_NUM_MODULES; i++) {
        expectedCellTemperatures[i*MAX_THERMISTORS_PER_MODULE + currentThermistor] = i;
    }
    
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_mV[i]);
    }
}

 /** 
  * CellTemperatures_UpdateCellTemperatures()
  * - currentThermistor:
  *   - 23
  */
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTwentyThree) {
    printf("UpdateCellTemperatures_currentThermistorTwentyThree");

    uint8_t currentThermistor = 23;
    uint8_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        expectedCellTemperatures[i] = 0;
    }
    for (i=0; i<MAX_NUM_MODULES; i++) {
        expectedCellTemperatures[i*MAX_THERMISTORS_PER_MODULE + currentThermistor] = i;
    }
    
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_mV[i]);
    }
}


TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorZero);
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTen);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateCellTemperatures_currentThermistorTwentyThree);
}

