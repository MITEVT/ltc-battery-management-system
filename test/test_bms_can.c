#include "unity.h"
#include "unity_fixture.h"
#include "can_constants.h"
#include "bms_can.h"
#include <stdio.h>
#include "state_types.h"

#define HEARTBEAT_DATA_BYTES 2

/**
 * Testing Strategy
 *
 * BMS_CAN_ConstructHeartbeatData
 * - state
 *   - init
 *   - not init
 * - soc
 *   - 0
 *   - >0
 */

TEST_GROUP(BMS_CAN_Test);

TEST_SETUP(BMS_CAN_Test) {
<<<<<<< HEAD
	printf("\r(BMS_CAN_Test)Setup");
	printf("...");
}

TEST_TEAR_DOWN(BMS_CAN_Test) {
	printf("...");
	printf("Teardown\r\n");
}

void create_bms_heartbeat_expected_data(uint16_t state, uint16_t soc, 
		uint8_t * expected_data) {
	const uint8_t bits_in_byte = 8;
	const uint8_t heartbeat_data_bytes = 2;
	const uint8_t state_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
		__BMS_HEARTBEAT__STATE__end);
	const uint8_t soc_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
		__BMS_HEARTBEAT__SOC_PERCENTAGE__end);
	uint16_t expected_data_string = 0 | (state << state_left_shift) |
		(soc << soc_left_shift);
	const uint16_t byte0_mask = 0xFF00;
	const uint16_t byte1_mask = 0x00FF;
	expected_data[0] = (expected_data_string & byte0_mask) >> bits_in_byte;
	expected_data[1] = expected_data_string & byte1_mask;
=======
    printf("\r(BMS_CAN_Test)Setup");
    printf("...");
}

TEST_TEAR_DOWN(BMS_CAN_Test) {
    printf("...");
    printf("Teardown\r\n");
}

void create_bms_heartbeat_expected_data(uint16_t state, uint16_t soc, 
        uint8_t * expected_data) {
    const uint8_t bits_in_byte = 8;
    const uint8_t heartbeat_data_bytes = 2;
    const uint8_t state_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
        __BMS_HEARTBEAT__STATE__end);
    const uint8_t soc_left_shift = (bits_in_byte*heartbeat_data_bytes - 1 - 
        __BMS_HEARTBEAT__SOC_PERCENTAGE__end);
    uint16_t expected_data_string = 0 | (state << state_left_shift) | 
        (soc << soc_left_shift);
    const uint16_t byte0_mask = 0xFF00;
    const uint16_t byte1_mask = 0x00FF;
    expected_data[0] = (expected_data_string & byte0_mask) >> bits_in_byte;
    expected_data[1] = expected_data_string & byte1_mask;
>>>>>>> 5683f1ae3d66e2287806c46cef596bc8fcfea39a
}

/**
 * Covers:
 * BMS_CAN_ConstructHeartbeatData
 * - state: init
 * - soc: 0
 */
TEST(BMS_CAN_Test, BMS_CAN_ConstructHeartbeatData_StateInitSocZero) {
<<<<<<< HEAD
	printf("BMS_CAN_ConstructHeartbeatData_StateInitSocZero");

	//create expected_data
	const uint16_t state = ____BMS_HEARTBEAT__STATE__INIT;
	const uint16_t soc = 0;
	uint8_t expected_data[HEARTBEAT_DATA_BYTES];
	create_bms_heartbeat_expected_data(state, soc, expected_data);

	//get actual data
	BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_INIT;
	uint8_t actual_data[HEARTBEAT_DATA_BYTES];
	BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);

	//check that actual data matches expected data
	uint8_t i;
	for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
		TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
	}		
=======
    printf("BMS_CAN_ConstructHeartbeatData_StateInitSocZero");
    
    //create expected_data
    const uint16_t state = ____BMS_HEARTBEAT__STATE__INIT;
    const uint16_t soc = 0;
    uint8_t expected_data[HEARTBEAT_DATA_BYTES];
    create_bms_heartbeat_expected_data(state, soc, expected_data);
    
    //get actual data
    BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_INIT;
    uint8_t actual_data[HEARTBEAT_DATA_BYTES];
    BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);
    
    //check that actual data matches expected data
    uint8_t i;
    for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
        TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
    }               
>>>>>>> 5683f1ae3d66e2287806c46cef596bc8fcfea39a
}

/**
 * Covers:
 * BMS_CAN_ConstructHeartbeatData
 * - state: not init
 * - soc: >0
 */
TEST(BMS_CAN_Test, BMS_CAN_ConstructHeartbeatData_StateNotInitSocGreaterThanZero) {
<<<<<<< HEAD
	printf("BMS_CAN_ConstructHeartbeatData_StateNotInitSocGreaterThanZero");

	//create expected_data
	const uint16_t state = ____BMS_HEARTBEAT__STATE__DISCHARGE;
	const uint16_t soc = 1023;
	uint8_t expected_data[HEARTBEAT_DATA_BYTES];
	create_bms_heartbeat_expected_data(state, soc, expected_data);

	//get actual data	
	BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_DISCHARGE;
	uint8_t actual_data[HEARTBEAT_DATA_BYTES];
	BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);

	//check that actual data matches expected data
	uint8_t i;
	for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
		TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
	}		
=======
    printf("BMS_CAN_ConstructHeartbeatData_StateNotInitSocGreaterThanZero");
    
    //create expected_data
    const uint16_t state = ____BMS_HEARTBEAT__STATE__DISCHARGE;
    const uint16_t soc = 1023;
    uint8_t expected_data[HEARTBEAT_DATA_BYTES];
    create_bms_heartbeat_expected_data(state, soc, expected_data);
    
    //get actual data       
    BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_DISCHARGE;
    uint8_t actual_data[HEARTBEAT_DATA_BYTES];
    BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);
    
    //check that actual data matches expected data
    uint8_t i;
    for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
        TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
    }               
>>>>>>> 5683f1ae3d66e2287806c46cef596bc8fcfea39a
}


/**
 * Covers:
 * BMS_CAN_ConstructHeartbeatData
 * - state: not init
 * - soc: >0
 */
TEST(BMS_CAN_Test, BMS_CAN_ConstructHeartbeatData_StateStandbySocSixtyThree) {
<<<<<<< HEAD
	printf("BMS_CAN_ConstructHeartbeatData_StateStandbySocSixtyThree");

	//create expected_data
	const uint16_t state = ____BMS_HEARTBEAT__STATE__STANDBY;
	const uint16_t soc = 63;
	uint8_t expected_data[HEARTBEAT_DATA_BYTES];
	create_bms_heartbeat_expected_data(state, soc, expected_data);

	//get actual data	
	BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_STANDBY;
	uint8_t actual_data[HEARTBEAT_DATA_BYTES];
	BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);

	//check that actual data matches expected data
	uint8_t i;
	for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
		TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
	}		
}

TEST_GROUP_RUNNER(BMS_CAN_Test) {
	RUN_TEST_CASE(BMS_CAN_Test, BMS_CAN_ConstructHeartbeatData_StateInitSocZero);
	RUN_TEST_CASE(BMS_CAN_Test,
		BMS_CAN_ConstructHeartbeatData_StateNotInitSocGreaterThanZero);
	RUN_TEST_CASE(BMS_CAN_Test, 
		BMS_CAN_ConstructHeartbeatData_StateStandbySocSixtyThree);
=======
    printf("BMS_CAN_ConstructHeartbeatData_StateStandbySocSixtyThree");
    
    //create expected_data
    const uint16_t state = ____BMS_HEARTBEAT__STATE__STANDBY;
    const uint16_t soc = 63;
    uint8_t expected_data[HEARTBEAT_DATA_BYTES];
    create_bms_heartbeat_expected_data(state, soc, expected_data);
    
    //get actual data       
    BMS_SSM_MODE_T state_bms_ssm_mode_type = BMS_SSM_MODE_STANDBY;
    uint8_t actual_data[HEARTBEAT_DATA_BYTES];
    BMS_CAN_ConstructHeartbeatData(state_bms_ssm_mode_type, soc, actual_data);
    
    //check that actual data matches expected data
    uint8_t i;
    for (i=0; i<HEARTBEAT_DATA_BYTES; i++) {
    TEST_ASSERT_EQUAL_INT(expected_data[i], actual_data[i]);
    }               
}

TEST_GROUP_RUNNER(BMS_CAN_Test) {
    RUN_TEST_CASE(BMS_CAN_Test, BMS_CAN_ConstructHeartbeatData_StateInitSocZero);
    RUN_TEST_CASE(BMS_CAN_Test, 
        BMS_CAN_ConstructHeartbeatData_StateNotInitSocGreaterThanZero);
    RUN_TEST_CASE(BMS_CAN_Test, 
        BMS_CAN_ConstructHeartbeatData_StateStandbySocSixtyThree);
>>>>>>> 5683f1ae3d66e2287806c46cef596bc8fcfea39a
}
