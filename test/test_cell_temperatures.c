// Unity
#include "unity.h"
#include "unity_fixture.h"

// ltc-battery-managament-system
#include "state_types.h"
#include "cell_temperatures.h"
#include "board.h"

// C libraries
#include <stdio.h>


/*********************************************************************************
 * Helper Functions
 * ******************************************************************************/

void setExpectedCellTemperatures(int16_t * expectedCellTemperatures, 
          uint8_t currentThermistor) {
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        expectedCellTemperatures[i] = -39;
    }
    expectedCellTemperatures[0*MAX_THERMISTORS_PER_MODULE + currentThermistor] = 60;
    expectedCellTemperatures[1*MAX_THERMISTORS_PER_MODULE + currentThermistor] = 60;
    expectedCellTemperatures[2*MAX_THERMISTORS_PER_MODULE + currentThermistor] = 0;
    expectedCellTemperatures[3*MAX_THERMISTORS_PER_MODULE + currentThermistor] = -14;
    expectedCellTemperatures[4*MAX_THERMISTORS_PER_MODULE + currentThermistor] = 600;
    expectedCellTemperatures[5*MAX_THERMISTORS_PER_MODULE + currentThermistor] = 600;
}

void printArrayUint32(uint32_t * array, uint16_t arraySize) {
    printf("{");
    uint16_t i;
    for (i=0; i<arraySize; i++) {
        printf("%d,", array[i]);
    }
    printf("{\r\n");
}

void printArrayInt16(int16_t * array, uint16_t arraySize) {
    printf("{");
    uint16_t i;
    for (i=0; i<arraySize; i++) {
        printf("%d,", array[i]);
    }
    printf("{\r\n");
}

/**
 * Testing Strategy
 * 
 * CellTemperatures_UpdateCellTemperatures()
 * - currentThermistor:
 *   - 0
 *   - 0 < currentThermistor < 23
 *   - 23
 * - gpioVoltages contains at least 3 different voltages 
 *   in locations for GPIO 1
 * - UpdateCellTemperatures() sets at least one temperature in pack_status to a
 *   negative value
 */

// Global variables
BMS_PACK_STATUS_T pack_status;
int16_t cell_temperatures_dC[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
uint32_t gpioVoltages[MAX_NUM_MODULES * LTC6804_GPIO_COUNT];

TEST_GROUP(Cell_Temperatures_Test);

TEST_SETUP(Cell_Temperatures_Test) {
    printf("\r(Cell_Temperatures_Test)Setup");

    pack_status.cell_temperatures_dC = cell_temperatures_dC;
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = -39;
    }
    
    for (i=0; i<MAX_NUM_MODULES*LTC6804_GPIO_COUNT; i++) {
        gpioVoltages[i] = 0;
    }
    gpioVoltages[0*LTC6804_GPIO_COUNT] = 399; // should be translated to  60  dC
    gpioVoltages[1*LTC6804_GPIO_COUNT] = 396; // should be translated to  60  dC 
    gpioVoltages[2*LTC6804_GPIO_COUNT] = 156; // should be translated to  0   dC
    gpioVoltages[3*LTC6804_GPIO_COUNT] = 100; // should be translated to -14 dC
    gpioVoltages[4*LTC6804_GPIO_COUNT] = 2556; // should be translated to -600 dC
    gpioVoltages[5*LTC6804_GPIO_COUNT] = 2557; // should be translated to -600 dC

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
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_dC[i]);
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
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_dC[i]);
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
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status);

    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_dC[i]);
    }
}


TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorZero);
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTen);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateCellTemperatures_currentThermistorTwentyThree);
}

