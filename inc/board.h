/**
 * @file board.h
 * @author Eric Ponce
 * @date 25 June 2015
 * @brief Board peripheral driver
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef TEST_HARDWARE
	#include <stdbool.h>
	#include <stdio.h>
#else
	#include "chip.h"
  	#include "can.h"
	#include "ltc6804.h"
	#include "console.h"
#endif
 
#include "error_handler.h"
#include "config.h"
#include "state_types.h"
#include <string.h>
#include <stdint.h>


#define LED0 2, 8
#define LED1 2, 10

#define BAL_SW 1, 2
#define IOCON_BAL_SW IOCON_PIO1_2
#define CHRG_SW 1, 2
#define IOCON_CHRG_SW IOCON_PIO1_2
#define DISCHRG_SW 1, 2
#define IOCON_DISCHRG_SW IOCON_PIO1_2

#define SWITCH_GPIO 0
#define SWITCH_PIN  1

#define SPI_BUFFER_SIZE 20 // may need to change based on number of BMS size
#define UART_BUFFER_SIZE 100 // may need to change based on number of BMS size

#define CONTACTOR_P_GPIO 	2
#define CONTACTOR_P_PIN  	8
#define CONTACTOR_N_GPIO 	2
#define CONTACTOR_N_PIN  	7
#define CONTACTOR_PRE_GPIO 	2
#define CONTACTOR_PRE_PIN 	1

#define HEADROOM 1, 3
 #define Hertz2Ticks(freq) SystemCoreClock / freq

/**
 * @details Non-blocking printing for user interface
 *
 * @param str string to print
 */
uint32_t Board_Print(const char *str);

/**
 * @details Non-blocking printing for user interface. Appends a newline
 *
 * @param str string to print
 */
uint32_t Board_Println(const char *str);

/**
 * @details Non-blocking printing for user interface. Appends a newline
 *
 * @param str string to print
 */
uint32_t Board_Write(const char *str, uint32_t count);

uint32_t Board_Read(char *charBuffer, uint32_t length);

uint32_t Board_Print_BLOCKING(const char *str);

uint32_t Board_Println_BLOCKING(const char *str);


/**
 * @details Initialize the UART used for debugging
 * 
 * @param baudRateHz the desired baud rate
 */
void Board_UART_Init(uint32_t baudRateHz);

/**
 * @details Initialize the SPI0 peripheral used in off-chip CAN
 * 
 * @param baudRateHz the desired baud rate
 */
void Board_SPI_Init(uint32_t baudRateHz);

// void Board_CCAN_Init(uint32_t baudRateHz, void (*CAN_rx)(uint8_t), void (*CAN_tx)(uint8_t), void (*CAN_error)(uint32_t));
void Board_CAN_Init(uint32_t baudRateHz);


void Board_GPIO_Init(void);

/**
 * @details Initialize Board Status LED
 */
void Board_LED_Init(void);

/**
 * @details Turn status LED on
 */
void Board_LED_On(void);

/**
 * @details Turn status LED off
 */
void Board_LED_Off(void);

void Board_Headroom_Init(void);

void Board_Headroom_Toggle(void);

/******** LTC6804 Board Functions ***********/

/**
 * @details Initialize board input switch
 */
bool Board_LTC6804_Init(PACK_CONFIG_T * pack_config, uint32_t * cell_voltages_mV, uint32_t msTicks);

void Board_LTC6804_DeInit(void);

/**
 * @details get cell voltages
 *
 * @param mutable array of cell voltages
 * @param msTicks current milisecond count
 */
void Board_LTC6804_GetCellVoltages(BMS_PACK_STATUS_T* pack_status, uint32_t msTicks);


/**
 * @details does a CVST
 *
 * @param msTicks current milisecond count
 * @return true if CVST shows working config, false if waiting. 
 */
bool Board_LTC6804_CVST(uint32_t msTicks);

/**
 * @details balance selected cell
 * 
 * @param balance_requests balance_requests[i] is true if ith cell should be 
 *                         balanced, false otherwise
 */
void Board_LTC6804_UpdateBalanceStates(bool *balance_req, uint32_t msTicks);

/**
 * @details checks that pack configuration is consistent with number of connected LTC6804 slaves
 *
 * @param pack_config configuration of the battery pack:
 * @return true if pack configuration is consistent with number of connected LTC6804 slaves, false otherwise
 */
bool Board_LTC6804_ValidateConfiguration(uint32_t msTicks);



bool Board_LTC6804_OpenWireTest(uint32_t msTicks);

void Board_Init_Timers(void);

void Board_Enable_Timers(void);

/**
 * @details closes or opens contactors
 *
 * @param close_contactors true if contactors should be closed, false otherwise
 */
void Board_Close_Contactors(bool close_contactors);

/**
 * @details get the status of the contactors
 *
 * @return true if contactors are closed, false otherwise
 */
bool Board_Are_Contactors_Closed(void);

#ifndef TEST_HARDWARE
/**
 * @details get mode request
 *
 * @param console_output output from console
 * @param bms_input pointer to input to SSMs to modify bal and req
 * @return none
 */
void Board_Get_Mode_Request(const CONSOLE_OUTPUT_T * console_output, BMS_INPUT_T* bms_input);
#endif


#endif
