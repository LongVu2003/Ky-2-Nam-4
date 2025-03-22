#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "usart.h"
#include "bh1750.h"
#include "stdio.h"

int main(void){
	Delay_Init();
	UART_Init(9600);
	bh1750_init();
	
	while(1){
		uint16_t light = I2C_ReadLight();
		char buffer[50];
		sprintf(buffer, "Cuong do anh sang la: %d lux\n", light);
    UART_SendString(buffer);
		Delay_ms(1000);
	}
}