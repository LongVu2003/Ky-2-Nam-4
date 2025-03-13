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
#include "Delay.h"
#include "stm32f10x_exti.h"             // Keil::Device:StdPeriph Drivers:EXTI
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
          // Keil::Device:StdPeriph Drivers:GPIO
uint16_t u16Tim;
uint8_t u8Buff[6];
uint8_t uCheckSum;

volatile char stt_toggle = 0;

void Led_Init();
void Timer_Init();
//void NVIC_Config();
void Timer_Init()
{
	TIM_TimeBaseInitTypeDef time;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	time.TIM_CounterMode = TIM_CounterMode_Up;
	time.TIM_Period = 19999;
	time.TIM_ClockDivision = 0;
	time.TIM_Prescaler = 7199;
	time.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&time);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM_Cmd(TIM2,ENABLE);
}

void TIM2_IRQHandler()
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET)
	{
		stt_toggle = !stt_toggle;
		//GPIO_WriteBit(GPIOC,GPIO_Pin_13,(BitAction)(1 - GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)));
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
}
void Config_DHT11()
{
	//ENABLE CLK for GPIOC AND GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef gpio;
	// LED - PC3 - DEBUG
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
	
	// DHT 11 - PB12
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
}
void Read_DHT11()
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	delay_ms(20);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	
	
	// wait for response
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM12) < 10) {
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
			break;
		}
	}
	// check if DHT11 response
	
	u16Tim = TIM_GetCounter(TIM2);
	if(u16Tim >= 10){
		while(1) {
		}
	}
	
	// firt low bit
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM12) < 45) {
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
			break;
		}
	}
	// check if DHT11 response
	u16Tim = TIM_GetCounter(TIM2);
	if(u16Tim >= 45 || u16Tim <= 5){
		while(1) {
		}
	}
	// next high bit
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM12) < 90) {
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
			break;
		}
	}
	// check if DHT11 response
	u16Tim = TIM_GetCounter(TIM2);
	if(u16Tim >= 90 || u16Tim <= 70){
		while(1) {
		}
	}
	
	// next low bit
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM12) < 95) {
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
			break;
		}
	}
	// check if DHT11 response
	u16Tim = TIM_GetCounter(TIM2);
	if(u16Tim >= 95 || u16Tim <= 75){
		while(1) {
		}
	}
	
	// Read first 8 bit (Hum)
	for(int i = 0 ; i < 8 ; i ++){
		TIM_SetCounter(TIM2,0);
		while(TIM_GetCounter(TIM2) < 65){
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
				break;
			}
		}
		
		// check if time is out of range
		u16Tim = TIM_GetCounter(TIM2);
		if((u16Tim >= 65) || (u16Tim <= 45)){
			while(1){
			}
		}
		
		// Cho chan PB12 xuong thap
		TIM_SetCounter(TIM2,0);
		while(TIM_GetCounter(TIM12) < 80) {
			if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)){
				break;
			}
		}
	// check if time is out of range
		u16Tim = TIM_GetCounter(TIM2);
		if((u16Tim >= 80) || (u16Tim <= 10)){
			while(1){
			}
		}
		//shift left 1bit
		
		u8Buff[0] <<= 1;
		if(u16Tim > 45){
			// nhan dc bit 1;
			u8Buff[0] |= 1;
		}else {
			// nhan dc bit 0;
			u8Buff[0] &= ~1;
		}
	}
	
	uCheckSum = u8Buff[0] + u8Buff[1] + u8Buff[2] + u8Buff[3];
	if(uCheckSum != u8Buff[4]){
		while(1){
		}
	}
	
}
int main()
{
	SysTick_Init();
	usart1_cfg_A9A10(BAUD_9600);
	printf("hello world\n");
	while(1)
	{
		
		printf("Temperature : ");
		printf("%d",u8Buff[2]);
		printf("*C\n");
		printf("Humidity : ");
		printf("%d",u8Buff[0]);
		printf("%\n");
	}
}

