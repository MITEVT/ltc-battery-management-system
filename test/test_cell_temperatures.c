#ifdef FSAE_DRIVERS

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
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is negative
 *   - at least one cell temperature is 0
 *   - at least one cell temperature is positive
 * - max cell temp:
 *   - <0
 *   - 0
 *   - >0
 * - min cell temp:
 *   - <0
 *   - 0
 *   - >0
 * - average cell temp:
 *   - <0
 *   - 0
 *   - >0
 * - location of cell with max temp:
 *   - 0
 *   - middle
 *   - end
 * - location of cell with min temp:
 *   - 0
 *   - middle
 *   - end
 * - Edge cases:
 *   - number of cell with maximum cell temperature
 *     - 1
 *     - >1
 *   - number of cell with minimum cell temperature
 *     - 1
 *     - >1
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
 * - gpioVoltages contains at least 3 different voltages 
 *   in locations for GPIO 1
 * - UpdateCellTemperatures() sets at least one temperature in pack_status to a
 *   negative value
 */
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorZero) {
    printf("UpdateCellTemperatures_currentThermistorZero");

    uint8_t currentThermistor = 0;
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status, MAX_NUM_MODULES);

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
 * - gpioVoltages contains at least 3 different voltages 
 *   in locations for GPIO 1
 * - UpdateCellTemperatures() sets at least one temperature in pack_status to a
 *   negative value
*/
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTen) {
    printf("UpdateCellTemperatures_currentThermistorTen");

    uint8_t currentThermistor = 10;
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status, MAX_NUM_MODULES);

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
 * - gpioVoltages contains at least 3 different voltages 
 *   in locations for GPIO 1
 * - UpdateCellTemperatures() sets at least one temperature in pack_status to a
 *   negative value
 */
TEST(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTwentyThree) {
    printf("UpdateCellTemperatures_currentThermistorTwentyThree");

    uint8_t currentThermistor = 23;
    int16_t expectedCellTemperatures[MAX_NUM_MODULES * MAX_THERMISTORS_PER_MODULE];
    setExpectedCellTemperatures(expectedCellTemperatures, currentThermistor);
 
    CellTemperatures_UpdateCellTemperaturesArray(gpioVoltages, currentThermistor, 
            &pack_status, MAX_NUM_MODULES);

    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        TEST_ASSERT_EQUAL_INT(expectedCellTemperatures[i], 
                pack_status.cell_temperatures_dC[i]);
    }
}

/**
 * Covers:
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is negative
 * - max cell temp:
 *   - <0
 * - min cell temp:
 *   - <0
 * - average cell temp:
 *   - <0
 * - location of cell with max temp:
 *   - 0
 * - location of cell with min temp:
 *   - middle
 * - Edge cases:
 *   - number of cell with maximum cell temperature
 *     - 1
 *   - number of cell with minimum cell temperature
 *     - 1
 */
TEST(Cell_Temperatures_Test, UpdateMaxMinAvgCellTemperatures_AllTempsNegative) {

    printf("UpdateMaxMinAvgCellTemperatures_AllTempsNegative");
    
    // initialize array of cell temperatures
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = -5;
    }
    pack_status.cell_temperatures_dC[0] = -1;
    pack_status.cell_temperatures_dC[1] = -10;
    
    // set expected values
    const int16_t expected_max_cell_temp_dC = -1;
    const int16_t expected_min_cell_temp_dC = -10;
    const int16_t max_expected_avg_cell_temp_dC = -5;
    const int16_t min_expected_avg_cell_temp_dC = -6;
    const int16_t expected_max_cell_temp_position = 0;
    const int16_t expected_min_cell_temp_position = 1;

    CellTemperatures_UpdateMaxMinAvgCellTemperatures(&pack_status, MAX_NUM_MODULES);

    TEST_ASSERT_EQUAL_INT(expected_max_cell_temp_dC, pack_status.max_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(expected_min_cell_temp_dC, pack_status.min_cell_temp_dC);
    TEST_ASSERT_TRUE((min_expected_avg_cell_temp_dC <= pack_status.avg_cell_temp_dC) &&
            (pack_status.avg_cell_temp_dC <= max_expected_avg_cell_temp_dC));
    TEST_ASSERT_EQUAL_INT(expected_max_cell_temp_position, 
            pack_status.max_cell_temp_position);
    TEST_ASSERT_EQUAL_INT(expected_min_cell_temp_position, 
            pack_status.min_cell_temp_position);
}

/**
 * Covers:
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is negative
 *   - at least one cell temperature is zero
 * - max cell temp:
 *   - 0
 * - min cell temp:
 *   - <0
 * - average cell temp:
 *   - <0
 * - location of cell with max temp:
 *   - middle
 * - location of cell with min temp:
 *   - 0
 * - Edge cases:
 *   - number of cells with maximum cell temperature
 *     - 2
 *   - number of cells with minimum cell temperature
 *     - 1
 */
TEST(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_AllTempsNegativeExceptTwoTempsAtZero) {
    printf("UpdateMaxMinAvgCellTemperatures_AllTempsNegativeExceptTwoTempsAtZero");

    const int16_t maxCellTemp = 0;
    const int16_t minCellTemp = -10;
    const int16_t otherCellTemps = -5;
    
    // initialize array of cell temperatures
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = otherCellTemps;
    }
    const uint8_t maxCellTempPosition1 = 1;
    const uint8_t maxCellTempPosition2 = 2;
    const uint8_t minCellTempPosition = 0;
    pack_status.cell_temperatures_dC[maxCellTempPosition1] = maxCellTemp;
    pack_status.cell_temperatures_dC[maxCellTempPosition2] = maxCellTemp;
    pack_status.cell_temperatures_dC[minCellTempPosition] = minCellTemp;
    
    // set expected values
    const int16_t max_expected_avg_cell_temp_dC = -4;
    const int16_t min_expected_avg_cell_temp_dC = -5;

    CellTemperatures_UpdateMaxMinAvgCellTemperatures(&pack_status, MAX_NUM_MODULES);

    TEST_ASSERT_EQUAL_INT(maxCellTemp, pack_status.max_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(minCellTemp, pack_status.min_cell_temp_dC);
    TEST_ASSERT_TRUE((min_expected_avg_cell_temp_dC <= pack_status.avg_cell_temp_dC) &&
            (pack_status.avg_cell_temp_dC <= max_expected_avg_cell_temp_dC));
    TEST_ASSERT_TRUE((pack_status.max_cell_temp_position==maxCellTempPosition1) ||
            (pack_status.max_cell_temp_position==maxCellTempPosition2));
    TEST_ASSERT_EQUAL_INT(minCellTempPosition, pack_status.min_cell_temp_position);
}

/**
 * Covers:
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is zero
 *   - at least one cell temperature is positive
 * - max cell temp:
 *   - >0
 * - min cell temp:
 *   - 0
 * - average cell temp:
 *   - >0
 * - location of cell with max temp:
 *   - end
 * - location of cell with min temp:
 *   - middle
 * - Edge cases:
 *   - number of cells with maximum cell temperature
 *     - 1
 *   - number of cells with minimum cell temperature
 *     - >1
 */
TEST(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_MaxCellTempPositive) {
    printf("UpdateMaxMinAvgCellTemperatures_MaxCellTempPositive");

    const int16_t maxCellTemp = 10;
    const int16_t minCellTemp = 0;
    const int16_t otherCellTemps = 5;
    
    // initialize array of cell temperatures
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = otherCellTemps;
    }
    const uint16_t maxCellTempPosition = MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE - 1;
    const uint8_t minCellTempPosition1 = 1;
    const uint8_t minCellTempPosition2 = 2;
    pack_status.cell_temperatures_dC[maxCellTempPosition] = maxCellTemp;
    pack_status.cell_temperatures_dC[minCellTempPosition1] = minCellTemp;
    pack_status.cell_temperatures_dC[minCellTempPosition2] = minCellTemp;
    
    // set expected values
    const int16_t min_expected_avg_cell_temp_dC = 4;
    const int16_t max_expected_avg_cell_temp_dC = 5;

    CellTemperatures_UpdateMaxMinAvgCellTemperatures(&pack_status, MAX_NUM_MODULES);

    TEST_ASSERT_EQUAL_INT(maxCellTemp, pack_status.max_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(minCellTemp, pack_status.min_cell_temp_dC);
    TEST_ASSERT_TRUE((min_expected_avg_cell_temp_dC <= pack_status.avg_cell_temp_dC) &&
            (pack_status.avg_cell_temp_dC <= max_expected_avg_cell_temp_dC));
    TEST_ASSERT_EQUAL_INT(maxCellTempPosition, pack_status.max_cell_temp_position);
    TEST_ASSERT_TRUE((pack_status.min_cell_temp_position==minCellTempPosition1) ||
            (pack_status.min_cell_temp_position==minCellTempPosition2));
}

/**
 * Covers:
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is positive
 * - max cell temp:
 *   - >0
 * - min cell temp:
 *   - >0
 * - average cell temp:
 *   - >0
 * - location of cell with max temp:
 *   - middle
 * - location of cell with min temp:
 *   - end
 * - Edge cases:
 *   - number of cells with maximum cell temperature
 *     - 1
 *   - number of cells with minimum cell temperature
 *     - 1
 */
TEST(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_MinCellTempPositive) {
    printf("UpdateMaxMinAvgCellTemperatures_MinCellTempPositive");

    const int16_t maxCellTemp = 20;
    const int16_t minCellTemp = 10;
    const int16_t otherCellTemps = 15;
    
    // initialize array of cell temperatures
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = otherCellTemps;
    }
    const uint16_t maxCellTempPosition = 1;
    const uint16_t minCellTempPosition = MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE - 1;
    pack_status.cell_temperatures_dC[maxCellTempPosition] = maxCellTemp;
    pack_status.cell_temperatures_dC[minCellTempPosition] = minCellTemp;
    
    // set expected values
    const int16_t expected_avg_cell_temp_dC = 15;

    CellTemperatures_UpdateMaxMinAvgCellTemperatures(&pack_status, MAX_NUM_MODULES);

    TEST_ASSERT_EQUAL_INT(maxCellTemp, pack_status.max_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(minCellTemp, pack_status.min_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(expected_avg_cell_temp_dC, pack_status.avg_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(maxCellTempPosition, pack_status.max_cell_temp_position);
    TEST_ASSERT_EQUAL_INT(minCellTempPosition, pack_status.min_cell_temp_position);
}

/**
 * Covers:
 *
 * CellTemperatures_UpdateMaxMinAvgCellTemperatures()
 * - values in array of cell temperatures
 *   - at least one cell temperature is positive
 * - max cell temp:
 *   - >0
 * - min cell temp:
 *   - >0
 * - average cell temp:
 *   - >0
 * - location of cell with max temp:
 *   - middle
 * - location of cell with min temp:
 *   - end
 * - Edge cases:
 *   - number of cells with maximum cell temperature
 *     - 1
 *   - number of cells with minimum cell temperature
 *     - 1
 */
TEST(Cell_Temperatures_Test, UpdateMaxMinAvgCellTemperatures_AvgTemperatureZero) {
    printf("UpdateMaxMinAvgCellTemperatures_AvgTemperatureZero");

    const int16_t maxCellTemp = 20;
    const int16_t minCellTemp = -20;
    const int16_t otherCellTemps = 0;
    
    // initialize array of cell temperatures
    uint16_t i;
    for (i=0; i<MAX_NUM_MODULES*MAX_THERMISTORS_PER_MODULE; i++) {
        pack_status.cell_temperatures_dC[i] = otherCellTemps;
    }
    const uint16_t maxCellTempPosition = 1;
    const uint16_t minCellTempPosition = 5;
    pack_status.cell_temperatures_dC[maxCellTempPosition] = maxCellTemp;
    pack_status.cell_temperatures_dC[minCellTempPosition] = minCellTemp;
    
    // set expected values
    const int16_t expected_avg_cell_temp_dC = 0;

    CellTemperatures_UpdateMaxMinAvgCellTemperatures(&pack_status, MAX_NUM_MODULES);

    TEST_ASSERT_EQUAL_INT(maxCellTemp, pack_status.max_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(minCellTemp, pack_status.min_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(expected_avg_cell_temp_dC, pack_status.avg_cell_temp_dC);
    TEST_ASSERT_EQUAL_INT(maxCellTempPosition, pack_status.max_cell_temp_position);
    TEST_ASSERT_EQUAL_INT(minCellTempPosition, pack_status.min_cell_temp_position);
}

TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorZero);
    RUN_TEST_CASE(Cell_Temperatures_Test, UpdateCellTemperatures_currentThermistorTen);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateCellTemperatures_currentThermistorTwentyThree);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_AllTempsNegative);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_AllTempsNegativeExceptTwoTempsAtZero);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_MaxCellTempPositive);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_MinCellTempPositive);
    RUN_TEST_CASE(Cell_Temperatures_Test, 
            UpdateMaxMinAvgCellTemperatures_AvgTemperatureZero);
}

#endif // FSAE_DRIVERS
