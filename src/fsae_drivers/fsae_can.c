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

void Receive_Vcu_Heartbeat(BMS_INPUT_T *bms_input);
void Receive_Unknown_Message(void);

void Send_Bms_Heartbeat(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state);
void Send_Bms_Errors(uint32_t msTicks);
void Send_Bms_CellTemps(BMS_PACK_STATUS_T * pack_status);
void Send_Bms_PackStatus(BMS_PACK_STATUS_T * pack_status);

Can_Bms_ErrorID_T bms_error_to_can_error(ERROR_T error);
Can_Bms_ErrorID_T get_error_status(uint32_t msTicks);
bool is_pack_error(Can_Bms_ErrorID_T errorType);

void Fsae_Can_Init(uint32_t baud_rate, volatile uint32_t *msTicksPtr) {
    Can_Init(baud_rate, msTicksPtr);
}

void Fsae_Can_Receive(BMS_INPUT_T *bms_input, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    Can_MsgID_T msgType = Can_MsgType();
    if (msgType == Can_No_Msg) {
      return;
    }
    else if (msgType == Can_Unknown_Msg) {
      Receive_Unknown_Message();
    }
    else if (msgType == Can_Vcu_BmsHeartbeat_Msg){
      Receive_Vcu_Heartbeat(bms_input);
    }
    else {
        // TODO handle current messages
    }
}

void Fsae_Can_Transmit(BMS_INPUT_T *bms_input, BMS_STATE_T *bms_state, BMS_OUTPUT_T *bms_output) {
    UNUSED(bms_output);
    uint32_t msTicks = bms_input->msTicks;
    if ( (msTicks - last_bms_heartbeat_time) > BMS_HEARTBEAT_PERIOD) {
        last_bms_heartbeat_time = msTicks;
        Send_Bms_Heartbeat(bms_input, bms_state);
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

void Receive_Vcu_Heartbeat(BMS_INPUT_T *bms_input) {
    bms_input->last_vcu_msg_ms = bms_input->msTicks;
}

void Receive_Unknown_Message(void) {
    Frame frame;
    Can_UnknownRead(&frame);
}

void Send_Bms_Heartbeat(BMS_INPUT_T *bms_input, BMS_STATE_T * bms_state) {
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

    Can_Bms_Heartbeat_Write(&bmsHeartbeat);
}

void Send_Bms_Errors(uint32_t msTicks) {
    Can_Bms_Error_T error_msg;
    error_msg.type = get_error_status(msTicks);
    Can_Bms_Error_Write(&error_msg);
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

    Can_Bms_CellTemps_Write(&cellTemps);
}

/**
 * @details Sends pack status can message (details in fsae can spec)
 *
 * @param pack_status datatype containing information about the status of the pack
 */
void Send_Bms_PackStatus(BMS_PACK_STATUS_T * pack_status) {
    Can_Bms_PackStatus_T canPackStatus;
    canPackStatus.pack_voltage = pack_status->pack_voltage_mV;
    canPackStatus.pack_current = pack_status->pack_current_mA;
    canPackStatus.avg_cell_voltage = 0; //TODO: get actual average cell voltage
    canPackStatus.min_cell_voltage = pack_status->pack_cell_min_mV;
    canPackStatus.id_min_cell_voltage = 0; //TODO: get actual id min cell voltage
    canPackStatus.max_cell_voltage = pack_status->pack_cell_max_mV;
    canPackStatus.id_max_cell_voltage = 0; //TODO: get actual id max cell voltage

    Can_Bms_PackStatus_Write(&canPackStatus);
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

