#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "string.h"
#include "Delay.h"
#include "My_usart.h"

void GPIO_Config(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1, ENABLE);
	GPIO_InitTypeDef GPIO;
	GPIO.GPIO_Pin = GPIO_Pin_0;
	GPIO.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO);
	
}

void ADC_Config(void)
{
		/*cau hinh ADC1*/
    ADC_InitTypeDef ADC;
    ADC.ADC_Mode = ADC_Mode_Independent;
    ADC.ADC_ScanConvMode = DISABLE;
    ADC.ADC_ContinuousConvMode = ENABLE;
    ADC.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC.ADC_DataAlign = ADC_DataAlign_Right;
    ADC.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC);
		/* Cau hinh chanel, rank, thoi gian lay mau */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);
		/* Cho phep bo ADC1 hoa dong */
    ADC_Cmd(ADC1, ENABLE);
		/* Reset thanh ghi cablib */
    ADC_ResetCalibration(ADC1);
		/* Cho thanh ghi cablib reset xong */
    while (ADC_GetResetCalibrationStatus(ADC1));
		/* Khoi dong bo ADC */
    ADC_StartCalibration(ADC1);
		/* Cho trang thai cablib duoc bat */
    while (ADC_GetCalibrationStatus(ADC1));
		/* Bat dau chuyen doi ADC */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


int main(void)
{
	SystemInit();  
  SystemCoreClockUpdate();  
	SysTick_Init();
	GPIO_Config();
	usart1_cfg_A9A10(BAUD_9600);
	ADC_Config();
	printf("Hello\n");
	
	while(1)
	{
		uint16_t adc_value = ADC_GetConversionValue(ADC1);
		float voltage = (adc_value/4096.0) * 3.3;
		printf("Voltage: %.2fV\n", voltage);
		//printf("haha\n");
		delay_ms(200);
	}
}
