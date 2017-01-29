/**
 * @file board.h
 * @author Eric Ponce
 * @date 25 June 2015
 * @brief Board peripheral driver
 */
#include <stdint.h>

#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef TEST_HARDWARE
	#include <stdbool.h>
	#include <stdio.h>
#else
 #include "chip.h"
 #include "ltc6804.h"
 #include "console.h"
#endif
 
#include "error_handler.h"
#include "config.h"
#include "state_types.h"

#define LED1_GPIO 2
#define LED1_PIN 10

#define LED2_GPIO 3
#define LED2_PIN 0

#define LED3_GPIO 3
#define LED3_PIN 1

#define LED4_GPIO 3
#define LED4_PIN 2

#define LED5_GPIO 3
#define LED5_PIN 3

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

/**
 * @details Initialize the on-chip CAN peripheral
 * 
 * @param baudRateHz the desired baud rate
 * @param CAN_rx CAN RX callback function
 * @param CAN_tx CAN TX callback function
 * @param CAN_error CAN error callback function
 */
void Board_CCAN_Init(uint32_t baudRateHz, void (*CAN_rx)(uint8_t), void (*CAN_tx)(uint8_t), void (*CAN_error)(uint32_t));

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

/**
 * @details Initialize board input switch
 */
void Board_Switch_Init(void);


void Board_Init_LTC6804(PACK_CONFIG_T * pack_config, uint32_t * cell_voltages_mV, uint32_t msTicks);

/**
 * @details get cell voltages
 *
 * @param mutable array of cell voltages
 * @return state of LTC6804 BMS slaves
 */
void Board_Get_Cell_Voltages(BMS_PACK_STATUS_T* pack_status, uint32_t msTicks);

bool Board_LTC6804_CVST(uint32_t msTicks);

void Board_LTC6804_UpdateBalanceStates(bool *balance_req, uint32_t msTicks);

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

/**
 * @details get mode request
 *
 * @param console_output output from console
 * @return latest mode request
 */
BMS_SSM_MODE_T Board_Get_Mode_Request(CONSOLE_OUTPUT_T * console_output);



#endif
