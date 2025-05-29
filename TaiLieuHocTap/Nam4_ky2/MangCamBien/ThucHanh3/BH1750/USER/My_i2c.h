#ifndef MY_I2C_H_INCLUDED
#define MY_I2C_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "stm32f10x.h"
#include "stm32f10x_i2c.h"

/*DoanLongVu-B21DCDT247*/
/*! Luu y: Thu vien chi ho tro giao tiep thiet bi dia chi 7 bit o thoi diem hien tai*/

void i2c_write_one_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t data);
void i2c_write_multi_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t len, uint8_t *data);
void i2c_write_one_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr, uint8_t data);
void i2c_write_multi_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr, uint8_t len, uint8_t *data);

void i2c_read_one_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t *data);
void i2c_read_multi_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t len,uint8_t *data);
void i2c_read_one_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr,uint8_t *data);
void i2c_read_multi_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t reg_addr, uint8_t len,uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif

/*end of file*/
