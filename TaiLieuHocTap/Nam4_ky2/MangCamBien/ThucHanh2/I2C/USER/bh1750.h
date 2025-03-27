#ifndef __BH1750_H
#define __BH1750_H

#include "stm32f10x.h"                  // Device header

void bh1750_init(void);
void I2C_START(void);
void I2C_SENDADDRESS(uint8_t addr, uint8_t direct);
void I2C_SENDBYTE(uint8_t data);
void I2C_STOP(void);
uint8_t I2C_RECEIVE(void);
uint16_t I2C_ReadLight(void);

#endif /* __BH1750_H */