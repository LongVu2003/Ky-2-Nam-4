#include "stm32f10x.h"                  // Device header
#include "stm32f10x_i2c.h"              // Keil::Device:StdPeriph Drivers:I2C
#include "lcd.h"



void Lcd_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = I2C1_SCL| I2C1_SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 100000;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0;
	I2C_Init(I2C1,&I2C_InitStructure);
	I2C_Cmd(I2C1,ENABLE); 	
}

//giao tiep I2C
void I2C_Write_byte(uint8_t data)
{
	I2C_GenerateSTART( I2C1,ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, I2C_ADDRESS, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
	I2C_SendData(I2C1,data);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP( I2C1,ENABLE);
}

void I2c_Lcd_Command_Write(uint8_t data)
{
	uint8_t data_u, data_1;
	uint8_t data_t[4], i = 0;
	data_u = data & 0xf0;
	data_1 = (data << 4) & 0xf0;
	data_t[0] = data_u | 0x0c;
	data_t[1] = data_u | 0x08;
	data_t[2] = data_1 | 0x0c;
	data_t[3] = data_1 | 0x08;
	for(i = 0;i < 4; i++)
	{
		I2C_Write_byte(data_t[i]);
	}
}

void I2c_Lcd_Data_Write(uint8_t data)
{
	uint8_t data_u, data_1;
	uint8_t data_t[4], i = 0;
	data_u = data & 0xf0;
	data_1 = (data << 4) & 0xf0;
	data_t[0] = data_u | 0x0d;
	data_t[1] = data_u | 0x09;
	data_t[2] = data_1 | 0x0d;
	data_t[3] = data_1 | 0x09;
	for(i = 0;i < 4; i++)
	{
		I2C_Write_byte(data_t[i]);
	}
}
//khoi tao LCD
void Lcd_First_Config(void)
{
	I2c_Lcd_Command_Write(0x33);
	Delay_Ms(10);
	I2c_Lcd_Command_Write(0x32);
	Delay_Ms(50);
	I2c_Lcd_Command_Write(0x28);
	Delay_Ms(50);
	I2c_Lcd_Command_Write(0x01);
	Delay_Ms(50);
	
	I2c_Lcd_Command_Write(0x06);
	Delay_Ms(50);
	I2c_Lcd_Command_Write(0x0c);
	Delay_Ms(50);
	I2c_Lcd_Command_Write(0x02);
	Delay_Ms(50);
}

void I2c_Lcd_Send_String(uint8_t *String)
{
	while(*String)
	{
		I2c_Lcd_Data_Write(*String);
		String++;
	}
}
//clear man hinh
void clear_LCD(void)
{
	I2c_Lcd_Command_Write(0x01);
	Delay_Ms(10);
}

void Delay_Ms(uint32_t time)
{
	volatile uint32_t nCount;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq (&RCC_Clocks);

	nCount=(RCC_Clocks.HCLK_Frequency/10000)*time;
	for (; nCount!=0; nCount--);
}
