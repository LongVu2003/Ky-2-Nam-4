#include "stm32f10x.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "My_usart.h"
#include "delay.h"
#include "stm32f10x_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
          // Keil::Device:StdPeriph Drivers:GPIO

volatile char stt_toggle = 0;

void Led_Init();
void Timer_Init();
//void NVIC_Config();

int main()
{
	usart1_cfg_A9A10(BAUD_9600);
	printf("hello world\n");
	while(1)
	{
	}
}

