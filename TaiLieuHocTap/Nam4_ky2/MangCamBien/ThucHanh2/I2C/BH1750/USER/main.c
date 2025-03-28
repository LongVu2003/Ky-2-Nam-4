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

#include "Delay.h"
#include "bh1750.h"
#include "My_usart.h"

int main(void){
	SysTick_Init();
	usart1_cfg_A9A10(BAUD_9600);
	bh1750_init();

	while(1){
		uint16_t light = BH1750_ReadLight();
		printf("Cuong do anh sang la: %d lux\n", light);
		delay_ms(1000);
	}
}