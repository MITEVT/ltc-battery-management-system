#include "evt_can.h"

#include "board.h"
#include "brusa.h"
#include "can.h"
#include "error_handler.h"

static uint32_t _last_brusa_ctrl = 0;
static volatile uint32_t *msTicksPtr;

void Evt_Can_Init(uint32_t baudRateHz, volatile uint32_t* msTicksPtrArg) {
    CAN_Init(baudRateHz, msTicksPtr);
    msTicksPtr = msTicksPtrArg;
}

void Evt_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {

    UNUSED(bms_state);

    // [TODO] Make timing.h that has this (or board.h)
    // Make python script generate
    #define NLG5_CTL_DLY_mS 99

    // Easy way to turn off charger in case of accident
    if (bms_output->charge_req->charger_on && bms_input->msTicks - _last_brusa_ctrl >= NLG5_CTL_DLY_mS) {
        NLG5_CTL_T brusa_control;
        CCAN_MSG_OBJ_T temp_msg;
        brusa_control.enable = 1;
        brusa_control.ventilation_request = 0;
        brusa_control.max_mains_cAmps = 1000; // [TODO] Magic Numbers
        brusa_control.output_mV = bms_output->charge_req->charge_voltage_mV;
        brusa_control.output_cA = bms_output->charge_req->charge_current_mA / 10;
        const ERROR_STATUS_T * stat = Error_GetStatus(ERROR_BRUSA);
        if (stat->handling) {
            brusa_control.clear_error = stat->count & 1;
            brusa_control.output_mV = 0;
            brusa_control.output_cA = 0;
            bms_input->charger_on = false;
        } else {
             brusa_control.clear_error = 0;
             bms_input->charger_on = true;
        }

        Brusa_MakeCTL(&brusa_control, &temp_msg);
        CAN_TransmitMsgObj(&temp_msg);
        _last_brusa_ctrl = bms_input->msTicks;
    }

    if (!bms_output->charge_req->charger_on) {
        bms_input->charger_on = false;
    }
    if (CAN_GetErrorStatus()) {
        Board_Println("CAN Error");
        Error_Assert(ERROR_CAN, bms_input->msTicks);
        CAN_ResetPeripheral();
        Board_CAN_Init(CAN_BAUD, msTicksPtr);
    }

}

void Evt_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output) {
    CCAN_MSG_OBJ_T rx_msg;
    if (CAN_Receive(&rx_msg) != NO_RX_CAN_MESSAGE) {
        if (rx_msg.mode_id == NLG5_STATUS) { 
            // [TODO] use info from brusa message
        } else if (rx_msg.mode_id == NLG5_ACT_I) {
            NLG5_ACT_I_T act_i;
            Brusa_DecodeActI(&act_i, &rx_msg);
            bms_input->pack_status->pack_current_mA = act_i.output_cAmps*10; // [TODO] Consider using current sense as well
            bms_input->pack_status->pack_voltage_mV = act_i.output_mVolts;

            // If current > requested current + thresh throw error
        } else if (rx_msg.mode_id == NLG5_ACT_II) {

        } else if (rx_msg.mode_id == NLG5_TEMP) {

        } else if (rx_msg.mode_id == NLG5_ERR && bms_output->charge_req->charger_on) {
            // [TODO] distinguish errors
            if (!Brusa_CheckErr(&rx_msg)) { // We've recevied a Brusa Error Message
                // if (output->charge_req->charger_on) {
                    Error_Assert(ERROR_BRUSA, bms_input->msTicks);
                // }
                // We should try to clear but also assert error for count
                // Timing idea: Brusa error msg happens as often as ctrl message
            } else {
                Error_Pass(ERROR_BRUSA);
            }
        }
    }
}
