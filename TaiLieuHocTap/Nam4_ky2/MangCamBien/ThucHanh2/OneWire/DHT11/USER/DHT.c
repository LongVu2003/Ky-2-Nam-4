#include "DHT.h"
#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_rcc.h" 
#include "Delay.h"
#include "My_usart.h"
//************************** Low Level Layer ********************************************************//

static void DHT_SetPinOut(DHT_Name* DHT)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DHT->Pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DHT->Port, &GPIO_InitStructure);
}
static void DHT_SetPinIn(DHT_Name* DHT)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DHT->Pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DHT->Port, &GPIO_InitStructure);
}
static void DHT_WritePin(DHT_Name* DHT, uint8_t Value)
{
	GPIO_WriteBit(DHT->Port, DHT->Pin, Value);
}
static uint8_t DHT_ReadPin(DHT_Name* DHT)
{
	uint8_t Value;
	Value =  GPIO_ReadInputDataBit(DHT->Port, DHT->Pin);
	return Value;
}
//********************************* Middle level Layer ****************************************************//
static uint8_t DHT_Start(DHT_Name* DHT)
{
	uint8_t Response = 0;
	DHT_SetPinOut(DHT);  
	DHT_WritePin(DHT, 0);
	delay_us(DHT->Type);   
	DHT_SetPinIn(DHT);
	delay_us(50); 
	if (!DHT_ReadPin(DHT))
	{
		delay_us(90); 
		if(DHT_ReadPin(DHT))
		{
			Response = 1;   
		}
		else 
		{
			Response = 0; 
		}
	}	
	printf("Response : %d\n",Response);
	printf("DHT_ReadPin : %d\n",DHT_ReadPin(DHT));
	while(!Response){
		printf("DHT no respond\n");
	}
	return Response;
}
static uint8_t DHT_Read(DHT_Name* DHT)
{
	uint8_t Value = 0;
	DHT_SetPinIn(DHT);
	for(int i = 0; i<8; i++)
	{
		while(!DHT_ReadPin(DHT)){
			printf("DHT no 1\n");
		}
		delay_us(90);
		if(!DHT_ReadPin(DHT))
		{
			Value &= ~(1<<(7-i));	
		}
		else Value |= 1<<(7-i);
		while(DHT_ReadPin(DHT)){
			printf("DHT no 2\n");
		}
	}
	return Value;
}

//************************** High Level Layer ********************************************************//
void DHT_Init(DHT_Name* DHT, uint8_t DHT_Type, GPIO_TypeDef* DH_PORT, uint16_t DH_Pin)
{
	if(DHT_Type == DHT11)
	{
		DHT->Type = DHT11_STARTTIME;
	}
	else if(DHT_Type == DHT22)
	{
		DHT->Type = DHT22_STARTTIME;
	}
	DHT->Port = DH_PORT;
	DHT->Pin = DH_Pin;
}

uint8_t DHT_ReadTempHum(DHT_Name* DHT)
{
	DHT->Respone =0;
	uint8_t Temp1, Temp2, RH1, RH2;
	uint16_t Temp, Humi, SUM = 0;
	DHT->Respone = DHT_Start(DHT);
	RH1 = DHT_Read(DHT);
	RH2 = DHT_Read(DHT);
	Temp1 = DHT_Read(DHT);
	Temp2 = DHT_Read(DHT);
	SUM = DHT_Read(DHT);
	Temp = (Temp1<<8)|Temp2;
	Humi = (RH1<<8)|RH2;
	DHT->Temp = (float)(Temp1);
	DHT->Humi = (float)(RH1);
	return SUM;
}