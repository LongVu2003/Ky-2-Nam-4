#include "ds18b20.h"
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "delay.h"

void DS18B20_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);

    GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);
}

uint8_t DS18B20_Reset(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(DS18B20_PORT, DS18B20_PIN);
    Delay_us(600);
    GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);
    Delay_us(100);

    GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
    
    uint8_t presence = GPIO_ReadInputDataBit(DS18B20_PORT, DS18B20_PIN) == 0;
    if (!presence) return 0;

    Delay_us(410);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
    
    return presence;
}

void DS18B20_WriteBit(uint8_t bit) {
    GPIO_ResetBits(DS18B20_PORT, DS18B20_PIN);
    Delay_us(bit ? 2 : 60);
    GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);
    Delay_us(bit ? 60 : 2);
}

uint8_t DS18B20_ReadBit(void) {
    GPIO_ResetBits(DS18B20_PORT, DS18B20_PIN);
    Delay_us(2);
    GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
    
    Delay_us(10);
    uint8_t bit = GPIO_ReadInputDataBit(DS18B20_PORT, DS18B20_PIN);
    Delay_us(50);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
    
    return bit;
}

void DS18B20_WriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(byte & 0x01);//and
        byte >>= 1;
    }
}

uint8_t DS18B20_ReadByte(void) {
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        byte >>= 1;
        if (DS18B20_ReadBit()) {
            byte |= 0x80;//or
        }
    }
    return byte;
}

void DS18B20_StartConversion(void) {
    if (DS18B20_Reset()) {
        DS18B20_WriteByte(0xCC);
        DS18B20_WriteByte(0x44);
        Delay_ms(750);
    }
}

float DS18B20_ReadTemperature(void) {
    uint8_t temp_lsb, temp_msb;
    int16_t temp;

    if (!DS18B20_Reset()) {
        return -1000.0;
    }

    DS18B20_WriteByte(0xCC);
    DS18B20_WriteByte(0xBE);

    temp_lsb = DS18B20_ReadByte();
    temp_msb = DS18B20_ReadByte();

    if (temp_lsb == 0xFF && temp_msb == 0xFF) return -1000.0;

    temp = (temp_msb << 8) | temp_lsb;
    return temp * 0.0625;
}
