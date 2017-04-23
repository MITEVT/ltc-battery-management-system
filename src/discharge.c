#include "discharge.h"

#include "board.h"

#ifdef FSAE_DRIVERS
    #include "fsae_can.h"
#endif

static uint16_t total_num_cells;
static uint32_t min_cell_voltage_mV;
static uint32_t max_pack_current_mA;
static uint16_t max_cell_temp_thres_C;
// current, temperature, and voltage


void Discharge_Init(BMS_STATE_T *state) {
    state->discharge_state = BMS_DISCHARGE_OFF;
}

uint32_t Calculate_Max_Current(
        uint32_t cell_capacity_cAh, uint32_t discharge_rating_cC,
        uint32_t pack_cells_p, uint16_t cell_temp_dC) {
    (void)(cell_temp_dC);
    return cell_capacity_cAh * discharge_rating_cC * pack_cells_p / 10;
}

void Discharge_Config(PACK_CONFIG_T *pack_config) {
    total_num_cells = Get_Total_Cell_Count(pack_config);

    min_cell_voltage_mV = pack_config->cell_min_mV;
    max_cell_temp_thres_C = pack_config->max_cell_temp_dC;
    max_pack_current_mA = Calculate_Max_Current(
                            pack_config->cell_capacity_cAh,
                            pack_config->cell_discharge_c_rating_cC,
                            pack_config->pack_cells_p,
                            max_cell_temp_thres_C); // approx. initialization pt
}

void Discharge_Step(BMS_INPUT_T *input, BMS_STATE_T *state, BMS_OUTPUT_T *output) {
    switch (input->mode_request) {
        case BMS_SSM_MODE_DISCHARGE:
            if (state->discharge_state == BMS_DISCHARGE_OFF) {
                state->discharge_state = BMS_DISCHARGE_INIT;
            }
            break;
        // we want to switch states (either to STANDBY/CHARGE/ERROR)
        default: 
            if(state->discharge_state != BMS_DISCHARGE_OFF) {
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
                state->discharge_state = BMS_DISCHARGE_RUN;
                goto handler;
            }
            break;
        case BMS_DISCHARGE_RUN:
            output->close_contactors = true;

            // recalculate max current with new temperature
            max_pack_current_mA = Calculate_Max_Current(
                                    state->pack_config->cell_capacity_cAh,
                                    state->pack_config->cell_discharge_c_rating_cC,
                                    state->pack_config->pack_cells_p,
                                    input->pack_status->max_cell_temp_dC);
            if(input->pack_status->pack_current_mA > max_pack_current_mA) {
                Error_Assert(ERROR_OVER_CURRENT, input->msTicks);
            } else {
                Error_Pass(ERROR_OVER_CURRENT);
            }

            if(!input->contactors_closed) {
                state->discharge_state = BMS_DISCHARGE_INIT;
            }

#ifdef FSAE_DRIVERS
            // Handle fan logic
            const int16_t curr_cell_temp = input->pack_status->max_cell_temp_dC;
            const int16_t fan_threshold_temp =
                    state->pack_config->fan_on_threshold_dC;
            output->fans_on = curr_cell_temp > fan_threshold_temp;

            // Handle VCU heartbeat logic
            uint32_t time_since_vcu_msg =
                    input->msTicks - input->last_vcu_msg_ms;
            bool vcu_timeout = time_since_vcu_msg > VCU_HEARTBEAT_TIMEOUT;
            if ((input->last_vcu_msg_ms != 0) && vcu_timeout) {
                Error_Assert(ERROR_VCU_DEAD, input->msTicks);
            }
#endif //FSAE_DRIVERS

            break;
        case BMS_DISCHARGE_DONE:
            output->close_contactors = false;
            // if contactors open, then we can turn discharge off
            if (!input->contactors_closed) {
                state->discharge_state = BMS_DISCHARGE_OFF;
            }
            break;
    }
}

uint32_t Read_Max_Current(void) {
    return max_pack_current_mA;
}
