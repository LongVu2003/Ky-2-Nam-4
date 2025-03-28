#ifndef __BH1750_H
#define __BH1750_H

#include "stm32f10x.h"                  // Device header
#include "My_i2c.h"

void bh1750_init(void);
uint16_t BH1750_ReadLight(void);

#endif /* __BH1750_H */