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
void GPIO_EXTI(void);
void configEXTI(void);
void NVIC_EXTI(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
// cau hinh cho led chan B2
void LEDconfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //cai dat cac cau hinh tren cho GPIOB
}
//cau hinh ngat ngoai cho 2 chan A8 va C13// B1:cap xung va cau hinh chan A8 va C13 nhan tin hieu ngat ngoai la ngo vao keo len
// (cau hinh GPIO)
void GPIO_EXTI(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	//do ngat ngoai la chuc nang thay the nen phai bat AIFO
	//cau hinh chan A8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//cau hinh chan C13
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//chon chan A8 va C13 la chan nhan tin hieu ngat ngoai
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);
}
// B2:Cau hinh va cho phep ngat ngoai o EXTI
void configEXTI(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line13; // chon kenh 8 va kenh 13 ung voi A8 va C13
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; //chon che do ngat ngoai 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //chon canh tich cuc la canh xuong
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; //cho phep kenh ngat ngoai duoc cau hinh
	EXTI_Init(&EXTI_InitStructure); //lenh cau hinh cac thong so duoc luu trong EXTI_InitStructure
}
// B3: cau hinh cap do uu tien va cho phep ngat ngoai o NVIC
void NVIC_EXTI()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); //0 cap PreemptionPriority va 16 cap SubPriority
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	//chon kenh tu 10 den 15
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //chon muc uu tien
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
// B4: Viet chuong trinh con phuc vu ngat ngoai
int reading,lastBtnState = 0,btnState,lastDebTime;
bool led_state = 0;
//chuong trinh con phuc vu ngat ngoai cho chan C13
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line13) != RESET) 
    {			
        if (SysTick_Millis() - lastDebTime > 200) { // Th?i gian debounce
            led_state = !led_state; // Ð?i tr?ng thái LED
						lastDebTime = SysTick_Millis();
					GPIO_WriteBit(GPIOB, GPIO_Pin_2, led_state); // Ghi tr?ng thái LED
        }
        
        EXTI_ClearITPendingBit(EXTI_Line13); // Xóa c? ng?t
    }
}

int main(void)
{
	SystemInit();
	SysTick_Init();
	LEDconfig();
	GPIO_EXTI();
	configEXTI();
	NVIC_EXTI();
	EXTI15_10_IRQHandler();
	usart1_cfg_A9A10(BAUD_9600);
	//GPIO_SetBits(GPIOB, GPIO_Pin_2); //tat Led
	while(1){
		//reading = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13);
		//printf("reading : %d\n",reading);
	}
}