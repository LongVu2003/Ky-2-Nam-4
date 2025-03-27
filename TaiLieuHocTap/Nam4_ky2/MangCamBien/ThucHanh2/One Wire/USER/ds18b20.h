#ifndef __DS18B20_H
#define __DS18B20_H

#include "stm32f10x.h"                  // Device header

#define DS18B20_PORT GPIOB
#define DS18B20_PIN  GPIO_Pin_5

void DS18B20_Init(void);
void DS18B20_WriteBit(uint8_t bit);
uint8_t DS18B20_ReadBit(void);
void DS18B20_WriteByte(uint8_t byte);
uint8_t DS18B20_ReadByte(void);
void DS18B20_StartConversion(void);
float DS18B20_ReadTemperature(void);

#endif /* __DS18B20_H */