
#include "stm32f10x.h"                  // Device header
	
#ifndef __ABC__
#define __ABC__

#ifdef __cplusplus
 extern "C" {
#endif

#define I2C_ADDRESS     0x4e
#define I2C1_SCL 		GPIO_Pin_6
#define I2C1_SDA		GPIO_Pin_7

#define LCD_COMMAND_CLEAR_DISPLAY 0x01 // Clear display screen
#define LCD_COMMAND_RETURN_HOME 0x02 // Return home
#define LCD_COMMAND_DECREMENT_CURSOR 0x04 // Decrement cursor (shift cursor to left)
#define LCD_COMMAND_INCREMENT_CURSOR 0x06 // Increment cursor (shift cursor to right)
#define LCD_COMMAND_SHIFT_DISPLAY_RIGHT 0x05 // Shift display right
#define LCD_COMMAND_SHIFT_DISPLAY_LEFT 0x07 // Shift display left
#define LCD_COMMAND_DISPLAY_OFF_CURSOR_OFF 0x08 // Display off, cursor off
#define LCD_COMMAND_DISPLAY_OFF_CURSOR_ON 0x0A // Display off, cursor on
#define LCD_COMMAND_DISPLAY_ON_CURSOR_OFF 0x0C // Display on, cursor off
#define LCD_COMMAND_DISPLAY_ON_CURSOR_BLINKING 0x0E // Display on, cursor blinking
#define LCD_COMMAND_SHIFT_CURSOR_POSITION_LEFT 0x10 // Shift cursor position to left
#define LCD_COMMAND_SHIFT_CURSOR_POSITION_RIGHT 0x14 // Shift the cursor position to the right
#define LCD_COMMAND_SHIFT_ENTIRE_DISPLAY_LEFT 0x18 // Shift the entire display to the left
#define LCD_COMMAND_SHIFT_ENTIRE_DISPLAY_RIGHT 0x1C // Shift the entire display to the right
#define LCD_COMMAND_1ST_LINE 0x80 // Force cursor to the beginning ( 1st line)
#define LCD_COMMAND_2ND_LINE 0xC0 // Force cursor to the beginning ( 2nd line)
#define LCD_COMMAND_TWO_LINES_AND_5X7_MATRIX 0x38 // 2 lines and 5×7 matrix

void Lcd_Config(void);
void Lcd_First_Config(void);
void I2c_Lcd_Data_Write(uint8_t data);
void I2c_Lcd_Send_String(uint8_t *String);
void clear_LCD(void);
void I2c_Lcd_Command_Write(uint8_t data);
void Delay_Ms(uint32_t time);

#ifdef __cplusplus
}
#endif

#endif
