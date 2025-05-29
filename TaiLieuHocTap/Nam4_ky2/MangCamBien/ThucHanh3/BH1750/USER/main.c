#include "stm32f10x.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
#include "lib_message.h"
#include "lib_convert.h"


#include "Delay.h"
#include "bh1750.h"
#include "uart.h"

int main(void){
	SysTick_Init();
	USART1_Init(9600);
	bh1750_init();
	//printf("START");
	while(1){
		uint16_t light = BH1750_ReadLight();
		//printf("Cuong do anh sang la: %d lux", light);
		//printf("\n");
		uint8_t data[20];
		uint8_t length = Message_Create_Frame_Respond_LUX(PORT_A, light, data);
		//printf("Cuong do anh sang la: %d lux", length);
		USART1_Send_Data(data, length);
		GPIOA->ODR ^= (1 << 15);
		delay_ms(1000);
	}
}