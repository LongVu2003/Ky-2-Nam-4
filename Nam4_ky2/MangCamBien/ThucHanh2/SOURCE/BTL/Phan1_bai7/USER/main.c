#include "stm32f10x.h"
#include "stm32f10x_gpio.h"            
#include "stm32f10x_rcc.h"             
#include "stm32f10x_tim.h"
#include "Delay.h"
/*Khai bao bien*/
GPIO_InitTypeDef					GPIO_InitStructure;
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_OCInitTypeDef					TIM_OCInitStructure; 

void Delay_ms(uint16_t time);
void TIM4_Configuraion(uint32_t freq, uint8_t duty_cycle);

int main(void)
{
	TIM4_Configuraion(1000,30);
	SysTick_Init();
	unsigned int i = 0;

    while(1){	
			/*
      for(i  = 1; i <= 100; i++){
				TIM4_Configuraion(1000, i); 
				delay_ms(100);
			}
			*/
    }

}

void TIM4_Configuraion(uint32_t freq, uint8_t duty_cycle)
{
	/*Cap clock*/
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	//Tinh toan gia tri arr va crr cho tan so va do rong xung 
    uint32_t prescaler = 71;  //Tan so cua Timer
    uint32_t arr = 1000000 / freq - 1;  //Tao ra chu ky 1000 xung (0 - 999)
    uint32_t ccr = (arr + 1) * duty_cycle / 100;  //voi 30% => se co 300 xung cao

	/*Cau hinh chân*/
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	 /* cau hinh timer base */
	
  TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = arr;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* cau hinh channel4 o mode PWM */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = ccr;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
  TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
	
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	
	/* cho phep TIM4 hoat dong */
  TIM_Cmd(TIM4, ENABLE);
}
