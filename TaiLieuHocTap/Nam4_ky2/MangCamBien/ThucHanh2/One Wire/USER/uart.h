#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"                  // Device header

void UART_Init(uint32_t baudrate);
void UART_SendChar(char c);
void UART_SendString(char *str);
char UART_ReceiveChar(void);
void UART_ReceiveString(char *buffer, uint32_t bufferSize);

#endif /* __UART_H */