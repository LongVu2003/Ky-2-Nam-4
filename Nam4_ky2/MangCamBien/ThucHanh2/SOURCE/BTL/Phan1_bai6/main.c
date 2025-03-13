#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>
#include "misc.h"

#define BH1750_ADDRESS 0x46 // ??a ch? I2C c?a BH1750

void SysTick_Config_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

volatile uint32_t sysTickCounter = 0;
void SysTick_Handler(void)
{
    if(sysTickCounter > 0)
    {
        sysTickCounter--;
    }
}

void delay_ms(uint32_t ms)
{
    sysTickCounter = ms;
    while(sysTickCounter != 0);
}

void USART_Config(void);
void I2C_Config(void);
void I2C_StartTransmission(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_StopTransmission(I2C_TypeDef* I2Cx);
uint16_t BH1750_ReadLight(void);

int main(void) {
		SysTick_Config_Init();
    USART_Config();
    I2C_Config();

    while (1) {
			uint16_t light = BH1750_ReadLight();
			char buffer[20];
			int i; // Khai báo bi?n tru?c các l?nh th?c thi

			sprintf(buffer, "Light: %d\n", light);
			for (i = 0; buffer[i] != '\0'; i++) {
					while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
					USART_SendData(USART1, buffer[i]);
			}
	}

}

void USART_Config(void) {
    // C?u h?nh USART
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // Tx pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // Rx pin
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void I2C_Config(void) {
    // C?u h?nh I2C
		GPIO_InitTypeDef GPIO_InitStructure;
		I2C_InitTypeDef I2C_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}

void I2C_StartTransmission(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction) {
    // Generate Start condition
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2Cx, address, direction);
    if (direction == I2C_Direction_Transmitter) {
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    } else {
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

void I2C_StopTransmission(I2C_TypeDef* I2Cx) {
    // Generate Stop condition
    I2C_GenerateSTOP(I2Cx, ENABLE);
}

uint16_t BH1750_ReadLight(void) {
    uint16_t light = 0;
        
    I2C_StartTransmission(I2C1, BH1750_ADDRESS, I2C_Direction_Transmitter);
    I2C_SendData(I2C1, 0x10); 
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_StopTransmission(I2C1);
    delay_ms(1200);

    I2C_StartTransmission(I2C1, BH1750_ADDRESS, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    light = I2C_ReceiveData(I2C1) << 8;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    light |= I2C_ReceiveData(I2C1);
    I2C_StopTransmission(I2C1);
    return light / 1.2; // BH1750 outputs data in 1.2x format
}