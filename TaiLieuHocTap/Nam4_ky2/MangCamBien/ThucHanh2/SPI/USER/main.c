#include "stm32f10x.h"                  // Device header
#include "uart.h"
#include "rfid.h"
#include "delay.h"

int main(){
	Timer2_Init();
	USART1_Init(9600);
	Delay_ms(200);
	RFID_Init();
	USART1_Send_String("RFID Reader Initialized\r\n");
	while(1){
		RFID_Debug_ReadCard();
		Delay_ms(1000);
	}
}