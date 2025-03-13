#include "My_usart.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

/*******************************************************************************************/

//#if defined(__GNUC__)

//#define PUTCHAR_PROTOTYPE int _write_(int fd, char * ch, int len)

//#elif defined(__CC_ARM)

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE * f)

//#endif

int fputc(int ch, FILE * f)
{

//    USART_ClearFlag(USART1,USART_FLAG_TC);
		USART_SendData(USART1,(u8)ch);
//    while (USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET){}
    while (USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){}
		return ch;


}

/*******************************************************************************************/

//#define string_size 80
volatile int8_t RX_available = 0;
volatile uint8_t RX_Buffer[string_size];
volatile uint8_t RX_index = 0;
volatile uint16_t receive_length = 0;

void usart1_cfg_A9A10(uint32_t baud_rate) /// datasheet, trang 31, bang remap
{
  /// cap clock cho uart1
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

  /// cau hinh cho UART1

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

static void send_char(uint8_t c){ /// Hàm gửi một kí tự ra COM
//  USART_ClearFlag(USART2,USART_FLAG_TXE);
  USART_SendData(USART1,c);
  while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET); /// chờ đến khi cờ TXE được set -> đã chuyển xong một frame
}

void printf_string(uint8_t *str){ /// hàm đưa một xâu kí tự ra COM
  while((*(str)) != NULL){
    send_char(*(str++));
  }
  while (USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET){}; /// Chờ đến khi cờ TC được set -> kết thúc quá trình truyền
}

static char convert_int_to_char(uint8_t n){ /// hàm chuyển đổi số ra kiểu char
   return n+48;
}

void printf_num(int32_t num){ /// hàm đưa một số ra COM
  if(!num){
    send_char(convert_int_to_char(num));
    return;
  }
  if(num < 0){
    num *= -1;
    send_char('-');
  }
  uint8_t rev_num[20];
  int i = 0;
  while(num != 0 ){
    rev_num[i++] = num%10;
    num/=10;
  }
  i--;
  for(i;i>=0;i--){
    send_char(convert_int_to_char(rev_num[i]));
  }
  while (USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET){};
}
/*******************************************************************************************/

/*end of file*/
