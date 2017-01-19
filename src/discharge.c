#include "discharge.h"
#include "bms_utils.h"

static uint16_t total_num_cells;

static uint32_t min_cell_voltage_mV;
static uint32_t max_pack_current_mA;
static uint16_t max_cell_temp_thres_C;
// current, temperature, and voltage

void Discharge_Init(BMS_STATE_T *state) {
	state->discharge_state = BMS_DISCHARGE_OFF;
}

uint32_t Calculate_Max_Current(
        uint32_t cell_capacity_cAh,
        uint32_t discharge_rating_cC,
        uint32_t pack_cells_p,
        uint16_t cell_temp_C
    ) {
    return cell_capacity_cAh * discharge_rating_cC * pack_cells_p / 10;
}

void Discharge_Config(PACK_CONFIG_T *pack_config) {
	total_num_cells = Get_Total_Cell_Count(pack_config);

    min_cell_voltage_mV = pack_config->cell_min_mV;
    max_cell_temp_thres_C = pack_config->max_cell_temp_C;
    max_pack_current_mA = Calculate_Max_Current(
                            pack_config->cell_capacity_cAh,
                            pack_config->cell_discharge_c_rating_cC,
                            pack_config->pack_cells_p,
                            max_cell_temp_thres_C); // approx. initialization pt
}

uint8_t Discharge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
	switch (input->mode_request) {
		case BMS_SSM_MODE_INIT:
            // Invalid, shouldn't be requestable
			return BMS_INVALID_SSM_STATE_ERROR;

		case BMS_SSM_MODE_DISCHARGE:
			if (state->discharge_state == BMS_DISCHARGE_OFF) {
				state->discharge_state = BMS_DISCHARGE_INIT;
			}
			break;

        // we want to switch states (either to STANDBY/CHARGE/ERROR)
		default: 
            if(state->discharge_state == BMS_DISCHARGE_OFF) {
                state->curr_mode = input->mode_request;
            } else {
                state->discharge_state = BMS_DISCHARGE_DONE;
            }
			break;
	}

handler:
	switch (state->discharge_state) {
		case BMS_DISCHARGE_OFF:
			output->close_contactors = false;
			break;

		case BMS_DISCHARGE_INIT:
			output->close_contactors = true;

			if (input->contactors_closed == output->close_contactors) {
				if(input->mode_request == BMS_SSM_MODE_DISCHARGE) {
					state->discharge_state = BMS_DISCHARGE_RUN;
				} 
				goto handler;
			}
			break;

		case BMS_DISCHARGE_RUN:
            // error: if contactors are open when we ordered them close
            // error: if cell min_v gets too low shut off
            // error: if temperature gets too high shut off
            // error: if drawn current gets too low shut off
            
            if(!input->contactors_closed) {
                return BMS_CONTACTORS_ERRONEOUS_STATE;
            }

            uint8_t i;
			for (i = 0; i < total_num_cells; i++) {
                if(input->pack_status->cell_voltage_mV[i] < min_cell_voltage_mV) {
                    return BMS_CELL_UNDER_VOLTAGE;
                }
            }

			if(input->pack_status->max_cell_temp_C >= max_cell_temp_thres_C) {
                return BMS_CELL_UNDER_VOLTAGE;
            }

            // recalculate max current with new temperature
            max_pack_current_mA = Calculate_Max_Current(
                                    state->pack_config->cell_capacity_cAh,
                                    state->pack_config->cell_discharge_c_rating_cC,
                                    state->pack_config->pack_cells_p,
                                    input->pack_status->max_cell_temp_C);
            if(input->pack_status->pack_current_mA > max_pack_current_mA) {
                return BMS_OVER_CURRENT;
            }

			output->close_contactors = true;
			break;

		case BMS_DISCHARGE_DONE:
			output->close_contactors = false;
            // if contactors open, then we can turn discharge off
            if (!input->contactors_closed) {
                state->discharge_state = BMS_DISCHARGE_OFF;
            }
			break;
	}

    return 0;
}

uint32_t Read_Max_Current(void) {
    return max_pack_current_mA;
}
