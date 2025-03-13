#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Delay.h"
#include "My_usart.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "stm32f10x.h"

void LEDconfig(void);
void LEDconfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //cai dat cac cau hinh tren cho GPIOB
}


int main(void)
{
	SystemInit();
	SysTick_Init();
	LEDconfig();
	usart1_cfg_A9A10(BAUD_9600);
	//GPIO_SetBits(GPIOB, GPIO_Pin_2); //tat Led
	while(1){
		if(RX_available == 1){
			//printf("char : %s\n",RX_Buffer);
			const char *a = RX_Buffer;
			if(strcmp(a,"A") == 0){
				GPIO_SetBits(GPIOB,GPIO_Pin_2);
			}
			if(strcmp(a,"B") == 0){
				GPIO_ResetBits(GPIOB,GPIO_Pin_2);
			}
		}
	}
}