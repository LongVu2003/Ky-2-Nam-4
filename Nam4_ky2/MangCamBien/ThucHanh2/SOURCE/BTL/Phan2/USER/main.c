#include "stm32f10x.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "My_usart.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"

/*define rtos*/

EventGroupHandle_t event_hdl;

#define EVENT_RECV_DONE (1UL<<0)									// 0..00001
#define EVENT_CHECK_DONE (1UL<<1)									// 0..00010
#define EVENT_ALLS (EVENT_RECV_DONE | EVENT_CHECK_DONE) // 0..00011

xSemaphoreHandle _Semaphore;

/*define peripheral*/


#define maxlengthDes 20
#define maxSyntaxLength 11

void seperate_each_word(char str[],char* des[],int *lengthReceive,const char *delim); /// tach tu 
void checkSyntax(char* des[],int lengthReceive); /// check syntax from rx buffer
void led_config();
void control_led();
/*Declare Task*/

void Task_receive_uart (void *pvParameters);
void Task_send_data(void *pvParameters);
void Task_check_syntax(void *pvParameters);
void Task_controll_led(void *pvParameters);
//void Task_rtc_read(void *pvParameters);

bool on_led_flag = false;
bool off_led_flag = false;
bool toggle_led_flag = false;

/*Main function*/

int main (void){
	
	_Semaphore = xSemaphoreCreateMutex();
	event_hdl = xEventGroupCreate();
	
	SystemInit();
	SystemCoreClockUpdate();
	//Core_begin();
	
  usart1_cfg_A9A10(BAUD_9600);
	printf("hello world\n");
	
	led_config();
	
	printf("done config i2s\n");
  
	xTaskCreate(Task_receive_uart, "nhan uart ", 128, NULL, 1, NULL);
	xTaskCreate(Task_send_data, "Task_send", 128, NULL, 1, NULL);
	xTaskCreate(Task_check_syntax, "Task_check_syntax", 128, NULL, 1, NULL);
	xTaskCreate(Task_controll_led, "Task_controll_led", 128, NULL, 1, NULL);

	
	printf("Start VTaskScheduler\n");
	vTaskStartScheduler();

	return 0;
}                                                                    

/*define task*/

int ready_rx;
void Task_receive_uart (void *pvParameters){
	while(1){
		//printf("taskReceiveRx\n");
		xSemaphoreTake(_Semaphore,portMAX_DELAY); /// take semaphore
    if(RX_available == 1){
				//xSemaphoreTake(_Semaphore,portMAX_DELAY); /// take semaphore
        RX_available = 0;
				//xSemaphoreGive(_Semaphore); /// give semaphore
			  xEventGroupSetBits(event_hdl,EVENT_RECV_DONE); /// Da nhan du lieu tu ban phim
    }
		xSemaphoreGive(_Semaphore); /// give semaphore
		
		vTaskDelay(1); /// wait 1 tick
	}
}
void Task_send_data(void *pvParameters){
	EventBits_t event;
	while(1){
		//printf("taskCheck\n");
		event = xEventGroupWaitBits(event_hdl, EVENT_RECV_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    if(event & EVENT_RECV_DONE){
			//printf("vafo roi");
			  xSemaphoreTake(_Semaphore,portMAX_DELAY); /// take semaphore
        printf("%s\n",RX_Buffer);
			  xSemaphoreGive(_Semaphore); /// give semaphore
			  //xEventGroupSetBits(event_hdl,EVENT_CHECK_DONE); /// da cap nhat rtc xong, lcd ngay lap tuc hien thi
    }
		vTaskDelay(1); /// wait 1 tick
	}
}

void Task_check_syntax(void *pvParameters){
	EventBits_t event;
	while(1){
		//printf("taskCheck\n");
		event = xEventGroupWaitBits(event_hdl, EVENT_RECV_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
    if(event & EVENT_RECV_DONE){
			//printf("vafo roi");
			  xSemaphoreTake(_Semaphore,portMAX_DELAY); /// take semaphore
        char* des[maxlengthDes];
        int lengthReceive;
        seperate_each_word(RX_Buffer,des,&lengthReceive,"_");
        checkSyntax(des,lengthReceive);
			  xSemaphoreGive(_Semaphore); /// give semaphore
			  xEventGroupSetBits(event_hdl,EVENT_CHECK_DONE); /// da cap nhat rtc xong, lcd ngay lap tuc hien thi
    }
		vTaskDelay(1); /// wait 1 tick
	}
}

void Task_controll_led(void *pvParameters){
	EventBits_t event;
	while(1){
		//printf("taskCheck\n");
		event = xEventGroupWaitBits(event_hdl, EVENT_ALLS, pdTRUE, pdFALSE, portMAX_DELAY);
    if(event & EVENT_CHECK_DONE){
			//printf("vafo roi");
			  xSemaphoreTake(_Semaphore,portMAX_DELAY); /// take semaphore
        control_led();
			  xSemaphoreGive(_Semaphore); /// give semaphore
    }
		vTaskDelay(1); /// wait 1 tick
	}
}

/// separate word
void seperate_each_word(char str[],char* des[],int *lengthReceive,const char *delim){

    char * p = NULL;
    int i=0;

//    /// bien tat ca thanh chu thuong
//    p = strlwr(str);
//    strcpy(str,p);

    /// tach chuoi lan dau
    p = strtok(str, delim);
//    printf("%s\n", p);
    des[i++] = p;

    /// tach chuoi lien tuc
//    if(strcmp(str,"func") == 0){
      while(p != NULL){
          p = strtok(NULL, delim);

          if(p != NULL) {
//              printf("%s\n", p);
            des[i++] = p;
          }
      }
    *lengthReceive = i;
}

/// check syntax

void checkSyntax(char* des[],int lengthReceive){
	if(lengthReceive-1 > maxSyntaxLength){
//		printf("wrong syntax\n");
		return;
	}
	if(strcmp(des[0],"ON") == 0){
		printf("ON LED\n");
		on_led_flag = 1;
		off_led_flag = 0;
		return;
	}
	if(strcmp(des[0],"OFF") == 0){
		printf("OFF LED\n");
		on_led_flag = 0;
		off_led_flag = 1;
    //ChangeHourFormat(&ds3231_obj,rtc_i2c);
		return;
	}
	if(strcmp(des[0],"TOGGLE") == 0){
		printf("TOGGLE LED\n");
		on_led_flag = !on_led_flag;
    off_led_flag = !off_led_flag;
    //ChangeHourFormat(&ds3231_obj,rtc_i2c);
		return;
	}
//	printf("wrong syntax\n");
}

void control_led(){
	printf("%d - %d\n",on_led_flag,off_led_flag);
	if(on_led_flag){
		printf("ONNING\n");
		 (GPIOB,GPIO_Pin_2);
	}
	if(off_led_flag){
		printf("OFFING\n");
		GPIO_ResetBits(GPIOB,GPIO_Pin_2);
	}
}
/*end of file*/
void led_config(){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
    GPIO_InitTypeDef led_init;

    led_init.GPIO_Mode = GPIO_Mode_Out_PP;
    led_init.GPIO_Pin = GPIO_Pin_2;
    led_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &led_init);

    //SysTick_Init();
}
