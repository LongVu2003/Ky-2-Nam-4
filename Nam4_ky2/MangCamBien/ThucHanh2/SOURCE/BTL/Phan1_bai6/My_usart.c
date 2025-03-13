#include "My_usart.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

/*******************************************************************************************/
/**new retarget**/

int fputc(int ch, FILE *f){
//  USART_ClearFlag(USART1,USART_FLAG_TC);
  while (USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
  USART_SendData(USART1,ch);
  while (USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
  return ch;
}
/*******************************************************************************************/

//#define string_size 80
volatile int8_t RX_available;
volatile uint8_t RX_Buffer[string_size];
volatile uint8_t RX_index;
volatile uint16_t receive_length;

void usart1_cfg_A9A10(uint32_t baud_rate) /// datasheet, trang 31, bang remap
{
  /// cap clock cho uart2
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); /// Cấp clock cho USART1R
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);/// Cấp clock cho GPIOA

  /// cấu hình gpio cho A9,A10
//  GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
  /// remap các pin cho USART1.
  /// datasheet, trang 31, bang remap

  GPIO_InitTypeDef GPIO_struct;
  GPIO_struct.GPIO_Pin = GPIO_Pin_9; /// TX
  GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_struct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA,&GPIO_struct);

  GPIO_struct.GPIO_Pin = GPIO_Pin_10; /// RX
  GPIO_struct.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA,&GPIO_struct);

  /// cau hinh cho UART2

  USART_InitTypeDef USART_struct;
  USART_struct.USART_BaudRate =baud_rate;
  USART_struct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_struct.USART_Parity = USART_Parity_No;
  USART_struct.USART_StopBits = USART_StopBits_1;
  USART_struct.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART1,&USART_struct);

  USART_ClearFlag(USART1,USART_FLAG_TC);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_SetPriority(USART1_IRQn,1);

  USART_Cmd(USART1,ENABLE);

  RX_available = 0;
  RX_index = 0;
  receive_length = 0;
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	{
    char temp_char = USART_ReceiveData(USART1);

		if(temp_char != '\n')
		{
			RX_Buffer[RX_index] = temp_char;
			RX_index++;
		}
		else
		{
      receive_length = RX_index;
			RX_Buffer[RX_index] = 0x00; /// NULL
			RX_available=1;
			RX_index = 0;
		}
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

/*******************************************************************************************/

/*end of file*/
