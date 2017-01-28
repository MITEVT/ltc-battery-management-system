#include "error_handler.h"

ERROR_STATUS_T error_vector[ERROR_NUM_ERRORS];
ERROR_HANDLER error_handler_vector[ERROR_NUM_ERRORS];

void assert_error(ERROR_T er_t, uint32_t msTicks) {
	if (error_vector[er_t].error == false) {
		error_vector[er_t].error = true;
		error_vector[er_t].time_stamp = msTicks;
		error_vector[er_t].count = 1;
	}
	else {
		error_vector[er_t].count+=1;
	}

}
void pass_error(ERROR_T er_t, uint32_t msTicks) {
	error_vector[er_t].error = false;

}

ERROR_HANDLER_STATUS_T handle_errors(void) {
	ERROR_T i;
	for (i = 0; i < ERROR_NUM_ERRORS; ++i) {
		if (error_vector[i].error == true) {
			error_handler_vector[i];
		}
	}
}