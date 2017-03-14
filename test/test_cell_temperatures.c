// Unity
#include "unity.h"
#include "unity_fixture.h"

// ltc-battery-managament-system
#include "state_types.h"

// C libraries
#include <stdio.h>

/**
 * Testing Strategy
 *
 * TODO
 */

TEST_GROUP(Cell_Temperatures_Test);

TEST_SETUP(Cell_Temperatures_Test) {
	printf("\r(Cell_Temperatures_Test)Setup");
	printf("...");
}

TEST_TEAR_DOWN(Cell_Temperatures_Test) {
	printf("...");
	printf("Teardown\r\n");
}

/**
 * Covers:
 * TODO
 */

TEST_GROUP_RUNNER(Cell_Temperatures_Test) {
	// TODO
}
