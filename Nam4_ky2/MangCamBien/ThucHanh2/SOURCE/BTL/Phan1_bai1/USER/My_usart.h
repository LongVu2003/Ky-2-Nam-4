#ifndef MY_USART_INCLUDE_H_
#define MY_USART_INCLUDE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 * Pham The Anh - B20DCDT017
 * USART1 Library for stm32f103
 * Retarget Printf
 * Still can use 2 function printf_string and printf_num instead printf if neccesary
 */

/*******************************************************************************************/
#define  BAUD_3M     3000000
#define  BAUD_2M     2000000
#define  BAUD_1M5    1500000
#define  BAUD_1M     1000000
#define  BAUD_115200  115200
#define  BAUD_9600      9600
/*******************************************************************************************/
/// library auto retarget printf, but can use another send string function as well
#define string_size 80
extern volatile int8_t RX_available;
extern volatile uint8_t RX_Buffer[string_size];
extern volatile uint8_t RX_index;
extern volatile uint16_t receive_length;

void usart1_cfg_A9A10(uint32_t baud_rate);
void USART1_IRQHandler(void);
/// can use printf instead, do not neccessary to use these function
void printf_string(uint8_t *str); /// send a string to COM
void printf_num(int32_t num); /// send a number to COM

/*******************************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
/*end of file*/
