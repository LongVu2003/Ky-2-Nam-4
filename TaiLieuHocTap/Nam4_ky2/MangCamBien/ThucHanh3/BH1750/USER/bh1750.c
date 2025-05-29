#include "stm32f10x.h"                  // Device header
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_i2c.h"              // Keil::Device:StdPeriph Drivers:I2C
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "bh1750.h"
#include "My_usart.h"
#include "Delay.h"

#define BH1750_ADDRESS 0x46 // dia chi I2C cua BH1750
void bh1750_init(){
		GPIO_InitTypeDef GPIO_InitStructure;
		I2C_InitTypeDef I2C_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//Alternate Function - OD
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;// khi su dung standard mode
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;//standard mode
    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}
/*
// REMAP 
void bh1750_init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 1. Kích ho?t clock cho các ngo?i vi c?n thi?t
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // Kích ho?t clock cho AFIO

    // 2. Th?c hi?n remap chân I2C1
    GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE); // Remap I2C1 sang PB8 (SCL) và PB9 (SDA)

    // 3. C?u hình chân GPIO m?i cho I2C
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // S? d?ng PB8 và PB9
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;       // Alternate Function - Open Drain
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 4. C?u hình I2C (ph?n này gi? nguyên)
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00; // Ho?c d?a ch? riêng c?a STM32 n?u nó ho?t d?ng ? ch? d? slave
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100kHz (Standard mode)
    I2C_Init(I2C1, &I2C_InitStructure);

    // 5. Kích ho?t I2C1
    I2C_Cmd(I2C1, ENABLE);
}
//*/
uint16_t BH1750_ReadLight(void) {
    uint16_t light = 0; // 16 bit du lieu cam bien
		uint8_t data[2]; // luu tru 2 byte du lieu
		// Send "Continuously H-resolution mode " instruction
		i2c_write_one_byte_no_reg(I2C1,BH1750_ADDRESS,0x10); // master gui dia chi va yeu cau mode doc
		/*0x10 Start measurement at 1lx resolution.
		Continuously H-Resolution Mode
		Measurement Time is typically 120ms */
	 //Wait to complete 1st H-resolution mode measurement.( max. 180ms. )
    delay_ms(180);
		//Read measurement result
		i2c_read_multi_byte_no_reg(I2C1,BH1750_ADDRESS,2,data);
		light = (data[0] << 8) | data[1];
    return light / 1.2; // BH1750 outputs data in 1.2x format
}