#include "error_handler.h"

static const uint32_t CELL_OVER_VOLTAGE_timeout_ms = 1000;
static const uint32_t CELL_UNDER_VOLTAGE_timeout_ms = 1000;
static const uint32_t OVER_CURRENT_timeout_ms = 500;
static const uint32_t LTC6802_PEC_timeout_count = 50;
static const uint32_t LTC6802_CVST_timeout_count = 2;
static const uint32_t LTC6802_OWT_timeout_count = 10;



static ERROR_STATUS_T error_vector[ERROR_NUM_ERRORS];

static ERROR_HANDLER_STATUS_T handle_LTC6804_PEC(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_LTC6804_CVST(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_LTC6804_OWT(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_ERROR(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_INVALID_SSM_STATE(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_CONTACTORS_ERRONEOUS_STATE(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_CELL_UNDER_VOLTAGE(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_CELL_OVER_VOLTAGE(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_CELL_OVER_TEMP(ERROR_STATUS_T* er_stat, uint32_t msTicks);
static ERROR_HANDLER_STATUS_T handle_OVER_CURRENT(ERROR_STATUS_T* er_stat, uint32_t msTicks);

static ERROR_HANDLER error_handler_vector[ERROR_NUM_ERRORS] = {
														handle_LTC6804_PEC,
														handle_LTC6804_CVST,
														handle_LTC6804_OWT,
														handle_ERROR,
														handle_INVALID_SSM_STATE,
														handle_CONTACTORS_ERRONEOUS_STATE,
														handle_CELL_UNDER_VOLTAGE,
														handle_CELL_OVER_VOLTAGE,
														handle_CELL_OVER_TEMP,
														handle_OVER_CURRENT};



void Error_Init(void){
	uint32_t i;
	for (i = 0; i < ERROR_NUM_ERRORS; ++i) {
		error_vector[i].error = false;
		error_vector[i].handling = false;
		error_vector[i].time_stamp = 0;
		error_vector[i].count = 0;
	}
}

void Error_Assert(ERROR_T er_t, uint32_t msTicks) {
	switch (er_t) {
		ERROR_LTC6804_CVST:
		ERROR_LTC6804_OWT:
			Error_Pass(ERROR_LTC6804_PEC);
			break;
	}
	if (!error_vector[er_t].error) {
		error_vector[er_t].error = true;
		error_vector[er_t].time_stamp = msTicks;
		error_vector[er_t].count = 1;
	}
	else {
		error_vector[er_t].count+=1;
	}

}
void Error_Pass(ERROR_T er_t) {
	error_vector[er_t].error = false;
	switch (er_t) {
		ERROR_LTC6804_CVST:
		ERROR_LTC6804_OWT:
			Error_Pass(ERROR_LTC6804_PEC);
			break;
	}
}

ERROR_HANDLER_STATUS_T Error_Handle(uint32_t msTicks) {
	ERROR_T i;
	for (i = 0; i < ERROR_NUM_ERRORS; ++i) {
		if (error_vector[i].error || error_vector[i].handling) {
			if (error_handler_vector[i](&error_vector[i], msTicks) == HANDLER_HALT) {
				return HANDLER_HALT;
			}
		}
	}
	return HANDLER_FINE;
}

// [TODO] Refactor similiar functions
static ERROR_HANDLER_STATUS_T handle_LTC6804_PEC(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(msTicks);
	Board_Println_BLOCKING("handling pec");
	if (!er_stat->error) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changeme 
		if (er_stat->count < LTC6802_PEC_timeout_count) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}
static ERROR_HANDLER_STATUS_T handle_LTC6804_CVST(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(msTicks);
	Board_Println_BLOCKING("handling CVST");
	if (!er_stat->error) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changeme 
		if (er_stat->count < LTC6802_CVST_timeout_count) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}
static ERROR_HANDLER_STATUS_T handle_LTC6804_OWT(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(msTicks);
	// Board_Println_BLOCKING("handling OWT");
	if (!er_stat->error) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changeme 
		if (er_stat->count < LTC6802_OWT_timeout_count) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}

static ERROR_HANDLER_STATUS_T handle_ERROR(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(er_stat);
	UNUSED(msTicks);
	return HANDLER_HALT;
}
static ERROR_HANDLER_STATUS_T handle_INVALID_SSM_STATE(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(er_stat);
	UNUSED(msTicks);
	return HANDLER_HALT;
}
static ERROR_HANDLER_STATUS_T handle_CONTACTORS_ERRONEOUS_STATE(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	UNUSED(er_stat);
	UNUSED(msTicks);
	return HANDLER_HALT;
}
static ERROR_HANDLER_STATUS_T handle_CELL_UNDER_VOLTAGE(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	if (er_stat->error == false) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changem
		if (msTicks - er_stat->time_stamp < CELL_UNDER_VOLTAGE_timeout_ms) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}
static ERROR_HANDLER_STATUS_T handle_CELL_OVER_VOLTAGE(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	if (er_stat->error == false) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changem
		if (msTicks - er_stat->time_stamp < CELL_OVER_VOLTAGE_timeout_ms) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}
static ERROR_HANDLER_STATUS_T handle_CELL_OVER_TEMP(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	if (er_stat->error == false) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changem
		if (msTicks - er_stat->time_stamp < CELL_OVER_VOLTAGE_timeout_ms) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}
static ERROR_HANDLER_STATUS_T handle_OVER_CURRENT(ERROR_STATUS_T* er_stat, uint32_t msTicks) {
	if (er_stat->error == false) {
		er_stat->handling = false;
		return HANDLER_FINE;
	} else {
		//[TODO] magic numbers changem
		if (msTicks - er_stat->time_stamp < OVER_CURRENT_timeout_ms) {
			er_stat->handling = true;
			return HANDLER_FINE;
		} else {
			return HANDLER_HALT;
		}
	}
}