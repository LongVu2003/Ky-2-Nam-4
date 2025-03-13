#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
volatile char stt_toggle = 0;

void Led_Init();
void Timer_Init();
//void NVIC_Config();

int main()
{
	Led_Init();
	Timer_Init();
	//NVIC_Config();
	while(1)
	{
		if(stt_toggle) GPIOB->ODR = 0x0000;
		else GPIOB->ODR = 0xFFFF;
	}
}

void Led_Init()
{
	GPIO_InitTypeDef gpio;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);
}

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
