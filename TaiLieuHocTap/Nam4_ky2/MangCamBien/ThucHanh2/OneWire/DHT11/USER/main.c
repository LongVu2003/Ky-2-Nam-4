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
#include "DHT.h"

#include "Delay.h"

#include "My_usart.h"

DHT_Name DHT1;
#define DHT11_Pin GPIO_Pin_12
#define DHT11_GPIO_Port GPIOB

int main(void){
	SysTick_Init();
	usart1_cfg_A9A10(BAUD_9600);
	DHT_Init(&DHT1, DHT11, DHT11_GPIO_Port, DHT11_Pin);
	//printf("START\n");
	while(1){
		printf("START\n");
		uint16_t sum = DHT_ReadTempHum(&DHT1);
		printf("START1\n %d",sum);
		printf("Nhiet do: %f *C\n", DHT1.Temp);
		printf("Do Am: %f % \n", DHT1.Humi);
		delay_ms(1000);
	}
}