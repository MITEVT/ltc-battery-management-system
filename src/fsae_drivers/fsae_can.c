#include "fsae_can.h"

#include "MY17_Can_Library.h"
#include "error_handler.h"
#include "board.h"

#define BMS_HEARTBEAT_PERIOD    1000
#define BMS_ERRORS_PERIOD       10000
#define BMS_CELL_TEMPS_PERIOD   10000
#define BMS_PACK_STATUS_PERIOD  100

static uint32_t last_bms_heartbeat_time = 0;
static uint32_t last_bms_errors_time = 0;
static uint32_t last_bms_cellTemps_time = 0;
static uint32_t last_bms_packStatus_time = 0;

static bool isResetting = false;

void handle_can_error(Can_ErrorID_T err);

void Send_Bms_Heartbeat(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output);
void Send_Bms_Errors(uint32_t msTicks);
void Send_Bms_CellTemps(BMS_PACK_STATUS_T * pack_status);
void Send_Bms_PackStatus(BMS_PACK_STATUS_T * pack_status);

Can_Bms_ErrorID_T bms_error_to_can_error(ERROR_T error);
Can_Bms_ErrorID_T get_error_status(uint32_t msTicks);
bool is_pack_error(Can_Bms_ErrorID_T errorType);

void Fsae_Can_Init(uint32_t baud_rate) {
    Can_Init(baud_rate);
}

void Fsae_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    Can_MsgID_T msgType = Can_MsgType();

    if (msgType == Can_No_Msg) {
        // No message, so do nothing this round
        return;
    } else if (msgType == Can_Error_Msg) {
        Can_ErrorID_T err = Can_Error_Read();
        handle_can_error(err);
        return;
    }

    // We had a successful reception, so finish reset (if it's still happening)
    isResetting = false;

    if (msgType == Can_Unknown_Msg) {
        // TODO use masking instead of this forced read
        Frame f;
        Can_Unknown_Read(&f);
    } else if (msgType == Can_Vcu_BmsHeartbeat_Msg){
        // Yes, you actually have to call read here,
        // even though the contents of the message are unused.
        Can_Vcu_BmsHeartbeat_T msg;
        Can_Vcu_BmsHeartbeat_Read(&msg);
        bms_input->last_vcu_msg_ms = bms_input->msTicks;
    } else if (msgType == Can_CurrentSensor_Current_Msg) {
        Can_CurrentSensor_Current_T msg;
        Can_CurrentSensor_Current_Read(&msg);
        bms_input->pack_status->pack_current_mA = msg.current_mA > 0 ? msg.current_mA : -msg.current_mA;
    } else if (msgType == Can_CurrentSensor_Voltage_Msg) {
        Can_CurrentSensor_Voltage_T msg;
        Can_CurrentSensor_Voltage_Read(&msg);
        bms_input->pack_status->pack_voltage_mV = msg.voltage_mV > 0 ? msg.voltage_mV : -msg.voltage_mV;
    } else if (msgType == Can_Vcu_DashHeartbeat_Msg) {
        Can_Vcu_DashHeartbeat_T msg;
        Can_Vcu_DashHeartbeat_Read(&msg);
        bms_input->hv_enabled = msg.hv_light;
    } else {
        // note other errors
    }
}

void Fsae_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    uint32_t msTicks = bms_input->msTicks;
    if ( (msTicks - last_bms_heartbeat_time) > BMS_HEARTBEAT_PERIOD) {
        last_bms_heartbeat_time = msTicks;
        Send_Bms_Heartbeat(bms_input, bms_state, bms_output);
    }
    if ( (msTicks - last_bms_errors_time) > BMS_ERRORS_PERIOD) {
        last_bms_errors_time = msTicks;
        Send_Bms_Errors(msTicks);
    }
    if ( (msTicks - last_bms_cellTemps_time) > BMS_CELL_TEMPS_PERIOD) {
        last_bms_cellTemps_time = msTicks;
        Send_Bms_CellTemps(bms_input->pack_status);
    }
    if ( (msTicks - last_bms_packStatus_time) > BMS_PACK_STATUS_PERIOD) {
        last_bms_packStatus_time = msTicks;
        Send_Bms_PackStatus(bms_input->pack_status);
    }
}

void handle_can_error(Can_ErrorID_T err) {
    if (err == Can_Error_NONE || err == Can_Error_NO_RX) {
        // Neither of these are real errors
        isResetting = false;
        return;
    }
    if (!isResetting) {
        // We have an error, and should start a reset.
        // TODO change behavior depending on error type.
        isResetting = true;
        CAN_ResetPeripheral();
        Fsae_Can_Init(500000);
        UNUSED(err);
    }

}
void Send_Bms_Heartbeat(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {
    Can_Bms_Heartbeat_T bmsHeartbeat;
    Can_Bms_ErrorID_T error_type = get_error_status(bms_input->msTicks);
    if (error_type != CAN_BMS_ERROR_NONE) {
        if (is_pack_error(error_type)) {
            bmsHeartbeat.state = CAN_BMS_STATE_BATTERY_FAULT;
        } else {
            bmsHeartbeat.state = CAN_BMS_STATE_BMS_FAULT;
        }
    } else {
        switch (bms_state->curr_mode) {
            case BMS_SSM_MODE_INIT: 
                bmsHeartbeat.state = CAN_BMS_STATE_INIT;
                break;
            case BMS_SSM_MODE_STANDBY:
                bmsHeartbeat.state = CAN_BMS_STATE_STANDBY;
                break;
            case BMS_SSM_MODE_CHARGE:
                bmsHeartbeat.state = CAN_BMS_STATE_CHARGE;
                break;
            case BMS_SSM_MODE_BALANCE:
                bmsHeartbeat.state = CAN_BMS_STATE_BALANCE;
                break;
            case BMS_SSM_MODE_DISCHARGE:
                bmsHeartbeat.state = CAN_BMS_STATE_DISCHARGE;
                break;
            default:
                // You should never reach here
                Board_Println("Unexpected curr_mode");
                Error_Assert(ERROR_CONTROL_FLOW, bms_input->msTicks);
                break;
        }
    }

    // TODO when we get SOC working
    bmsHeartbeat.soc = 0;
    bmsHeartbeat.fan_enable = bms_output->fans_on;
    bmsHeartbeat.dcdc_enable = bms_output->dc_dc_on;
    bmsHeartbeat.dcdc_fault = bms_input->dcdc_fault;

    handle_can_error(Can_Bms_Heartbeat_Write(&bmsHeartbeat));
}

void Send_Bms_Errors(uint32_t msTicks) {
    Can_Bms_Error_T error_msg;
    error_msg.type = get_error_status(msTicks);
    handle_can_error(Can_Bms_Error_Write(&error_msg));
}

/**
 * @details Sends cell temperatures over Can
 *
 * @param pack_status datatype containing average cell temperature, maximum cell
 * temperature, max cell temperature id, minimum cell temperature, and minimum
 * cell temperature id
 */
void Send_Bms_CellTemps(BMS_PACK_STATUS_T * pack_status) {
    Can_Bms_CellTemps_T cellTemps;
    cellTemps.avg_cell_temp = pack_status->avg_cell_temp_dC;
    cellTemps.min_cell_temp = pack_status->min_cell_temp_dC;
    cellTemps.id_min_cell_temp = pack_status->min_cell_temp_position;
    cellTemps.max_cell_temp = pack_status->max_cell_temp_dC;
    cellTemps.id_max_cell_temp = pack_status->max_cell_temp_position;

    handle_can_error(Can_Bms_CellTemps_Write(&cellTemps));
}

/**
 * @details Sends pack status can message (details in fsae can spec)
 *
 * @param pack_status datatype containing information about the status of the pack
 */
void Send_Bms_PackStatus(BMS_PACK_STATUS_T * pack_status) {
    Can_Bms_PackStatus_T canPackStatus;
    canPackStatus.pack_voltage = pack_status->pack_voltage_mV / 1000;
    canPackStatus.pack_current = pack_status->pack_current_mA / 1000;
    canPackStatus.avg_cell_voltage = 0; //TODO: get actual average cell voltage
    canPackStatus.min_cell_voltage = pack_status->pack_cell_min_mV / 10;
    canPackStatus.id_min_cell_voltage = 0; //TODO: get actual id min cell voltage
    canPackStatus.max_cell_voltage = pack_status->pack_cell_max_mV / 10;
    canPackStatus.id_max_cell_voltage = 0; //TODO: get actual id max cell voltage

    handle_can_error(Can_Bms_PackStatus_Write(&canPackStatus));
}

Can_Bms_ErrorID_T get_error_status(uint32_t msTicks) {
    uint8_t errorType;
    for (errorType=ERROR_LTC6804_PEC; errorType<(ERROR_NUM_ERRORS); errorType++) {
        bool has_error = Error_ShouldHalt(errorType, msTicks);
        if (has_error) {
            return bms_error_to_can_error(errorType);
        }
    }
    return CAN_BMS_ERROR_NONE;
}

bool is_pack_error(Can_Bms_ErrorID_T error) {
    switch(error) {
        case CAN_BMS_ERROR_LTC6804_PEC:
        case CAN_BMS_ERROR_LTC6804_CVST:
        case CAN_BMS_ERROR_LTC6804_OWT:
        case CAN_BMS_ERROR_EEPROM:
        case CAN_BMS_ERROR_CAN:
        case CAN_BMS_ERROR_CONFLICTING_MODE_REQUESTS:
        case CAN_BMS_ERROR_VCU_DEAD:
        case CAN_BMS_ERROR_CONTROL_FLOW:
        case CAN_BMS_ERROR_OTHER:
            return false;
        case CAN_BMS_ERROR_CELL_UNDER_VOLTAGE:
        case CAN_BMS_ERROR_CELL_OVER_VOLTAGE:
        case CAN_BMS_ERROR_CELL_UNDER_TEMP:
        case CAN_BMS_ERROR_CELL_OVER_TEMP:
        case CAN_BMS_ERROR_OVER_CURRENT:
            return true;
        default:
            return false;
    }
}

Can_Bms_ErrorID_T bms_error_to_can_error(ERROR_T error) {
    switch (error) {
        case ERROR_LTC6804_PEC:
            return CAN_BMS_ERROR_LTC6804_PEC;
        case ERROR_LTC6804_CVST:
            return CAN_BMS_ERROR_LTC6804_CVST;
        case ERROR_LTC6804_OWT:
            return CAN_BMS_ERROR_LTC6804_OWT;
        case ERROR_EEPROM:
            return CAN_BMS_ERROR_EEPROM;
        case ERROR_CELL_UNDER_VOLTAGE:
            return CAN_BMS_ERROR_CELL_UNDER_VOLTAGE;
        case ERROR_CELL_OVER_VOLTAGE:
            return CAN_BMS_ERROR_CELL_OVER_VOLTAGE;
        case ERROR_CELL_UNDER_TEMP:
            return CAN_BMS_ERROR_CELL_UNDER_TEMP;
        case ERROR_CELL_OVER_TEMP:
            return CAN_BMS_ERROR_CELL_OVER_TEMP;
        case ERROR_OVER_CURRENT:
            return CAN_BMS_ERROR_OVER_CURRENT;
        case ERROR_CAN:
            return CAN_BMS_ERROR_CAN;
        case ERROR_CONFLICTING_MODE_REQUESTS:
            return CAN_BMS_ERROR_CONFLICTING_MODE_REQUESTS;
        case ERROR_VCU_DEAD:
            return CAN_BMS_ERROR_VCU_DEAD;
        case ERROR_CONTROL_FLOW:
            return CAN_BMS_ERROR_CONTROL_FLOW;
        case ERROR_BRUSA:
        case ERROR_NUM_ERRORS:
            return CAN_BMS_ERROR_OTHER;
        default:
            // Unexpected error is control flow problem
            return CAN_BMS_ERROR_CONTROL_FLOW;
    }

}

