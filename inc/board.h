#ifndef _BOARD_H_
#define _BOARD_H_

#include "chip.h"


#ifdef DEBUG_ENABLE
	#define DEBUG_Print(str) Chip_UART_SendBlocking(LPC_USART, str, strlen(str))
	#define DEBUG_Println(str) {DEBUG_Print(str); DEBUG_Print("\r\n");}
	#define DEBUG_Write(str, count) Chip_UART_SendBlocking(LPC_USART, str, count)
#else
	#define DEBUG_Print(str)
	#define DEBUG_Println(str)
	#define DEBUG_Write(str, count) 
#endif


#endif