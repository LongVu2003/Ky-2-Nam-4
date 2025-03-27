#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "uart.h"
#include "ds18b20.h"
#include "stdio.h"

int main(void) {
    Delay_Init();   
    UART_Init(9600);
    DS18B20_Init(); 

    while (1) {
        DS18B20_StartConversion();
        Delay_ms(750);

        float temperature = DS18B20_ReadTemperature();

        char buffer[50];
			  sprintf(buffer, "Nhiet do la: %.2f °C\r\n", temperature);
        UART_SendString(buffer);

        Delay_ms(1000);
    }
}