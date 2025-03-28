#ifndef __DHT_H
#define __DHT_H

#include "stm32f10x.h" 
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#define DHT11_STARTTIME 18000
#define DHT22_STARTTIME 12000
#define DHT11 0x01
#define DHT22 0x02
typedef struct
{	
	uint8_t Respone;
	uint16_t Type;
	uint16_t Pin;
	GPIO_TypeDef* Port;
	float Temp;
	float Humi;
}DHT_Name;

void DHT_Init(DHT_Name* DHT, uint8_t DHT_Type, GPIO_TypeDef* DH_PORT, uint16_t DH_Pin);
uint8_t DHT_ReadTempHum(DHT_Name* DHT);
#endif