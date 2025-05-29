#include "uart.h"

void USART1_Init(uint32_t baud_rate)
	
{
	
    RCC->APB2ENR |= (1 << 14) | (1 << 2); // Enable USART1 clock and GPIOA clock

    // Configure PA9 as TX
    GPIOA->CRH &= ~(unsigned int)(0xF << 4); // Clear PA9 configuration bits
    GPIOA->CRH |= (0xB << 4);  // Set PA9 as Alternate function push-pull, 50 MHz

    // Configure PA10 as RX
    GPIOA->CRH &= ~(unsigned int)(0xF << 8); // Clear PA10 configuration bits
    GPIOA->CRH |= (0x4 << 8);  // Set PA10 as Input floating

    USART1->BRR = (unsigned short)(72000000 / baud_rate );// Set baud rate
    USART1->CR1 |= ( 1 << 2) | (1 << 3) | (1 << 13) | (1 << 5); // Enable USART1, transmitter, receiver, and RX interrupt
}
/* REMAP
void USART1_Init(uint32_t baud_rate)
{

    // Bat clock cho USART1, GPIOB (thay v� GPIOA), v� AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // Bat Remap cho USART1
    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_struct;

    // Cau h�nh PB6 (TX moi)
    GPIO_struct.GPIO_Pin = GPIO_Pin_6;
    GPIO_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_struct.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate function push-pull
    GPIO_Init(GPIOB, &GPIO_struct);

    // Cau h�nh PB7 (RX moi)
    GPIO_struct.GPIO_Pin = GPIO_Pin_7;
    GPIO_struct.GPIO_Mode = GPIO_Mode_IPU; // Input pull-up (ho?c Floating)
    GPIO_Init(GPIOB, &GPIO_struct);

    // C?u h�nh USART1 (gi? nguy�n ph?n c�n l?i)
    USART1->BRR = (unsigned short)(72000000 / baud_rate );
    USART1->CR1 |= ( 1 << 2) | (1 << 3) | (1 << 13) | (1 << 5); // Enable USART1, TX, RX, RXNE interrupt

}
*/
void USART1_Send_Char(char chr)
{
    while (!(USART1->SR & ( 1 << 7)));
    USART1->DR = chr;
}

void USART1_Send_String(char* str)
{
    while(*str) {
        while( !(USART1->SR & ( 1 << 7)));
        USART1->DR = *str++;
    }
}

void USART1_Send_Data(uint8_t* data, uint8_t length)
{
    for (int i = 0; i < length; i++) {
        while( !(USART1->SR & ( 1 << 7)));
        USART1->DR = data[i];
    }
}

void USART1_Send_Number(int16_t num)
{
    if (num < 0) {
        USART1_Send_Char('-');
        num = -num;
    }
    uint8_t length = 0;
    uint8_t temp[10];
    if (num == 0) {
        USART1_Send_Char('0');
        return;
    } else {
        while (num != 0) {
            uint8_t value = num % 10;
            temp[length++] = value + '0';
            num /= 10;
        }
        for (int i = length - 1; i >= 0; i--) {
            USART1_Send_Char(temp[i]);
        }
    }
}

void USART1_Send_Float(float num)
{
    if (num < 0) {
        USART1_Send_Char('-');
        num = -num;
    }
    int16_t integer = (int16_t)num;
    float decimal = num - integer;
    USART1_Send_Number(integer);
    USART1_Send_Char('.');
    decimal *= 1000;
    USART1_Send_Number((int16_t)decimal);
}

void USART1_Send_Hex(uint8_t num)
{
    uint8_t temp;
    temp = num >> 4;
    if(temp > 9) {
        temp += 0x37;
    } else {
        temp += 0x30;
    }
    USART1_Send_Char(temp);
    temp = num & 0x0F;
    if(temp > 9) {
        temp += 0x37;
    } else {
        temp += 0x30;
    }
    USART1_Send_Char(temp);
}
