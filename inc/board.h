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
#endif

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


#endif
