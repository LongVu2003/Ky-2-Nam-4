#ifndef MY_DS3231_H_INCLUDED
#define MY_DS3231_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f10x.h"
#include "stm32f10x_i2c.h"

/*
 * Pham The Anh - B20DCDT017
 * DS3231 Library for stm32f103
 * Only support basic set-get time for rtc.
 * not support alarm, calib, read ram, config control yet
 */

typedef struct{ /// ds3231 type
  uint16_t second;
  uint16_t minute;
  uint16_t hour;
  uint16_t day;
  uint16_t date;
  uint16_t month;
  uint16_t year;
  uint8_t HourFormat;
  uint8_t AM_PM; /// only for 12 Hour format
} ds3231_data;

typedef enum { /// day time in natural language
  Mon = 1,
  Tue = 2,
  Wed = 3,
  Thu = 4,
  Fri = 5,
  Sat = 6,
  Sun = 7,
}day_type;

typedef enum { /// AM/PM type
  AM = 0,
  PM = 1,
}Format_12_AM_PM;

/// function

void ds3231_init(ds3231_data *ds3231_x,I2C_TypeDef * i2cx); /// only function must call when start

/*-------------------------------------------------------set time function----------------------------------------------*/

/*set each time param*/

void set_ds3231_sec(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t sec); /// set second
void set_ds3231_minute(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t minute); /// set minute
void set_ds3231_hour_12HFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour,uint8_t AM_PM); /// set hour base on 12h format
void set_ds3231_hour_24HFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour); /// set hour base on 24h format
void set_ds3231_day(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,day_type day); /// set day
void set_ds3231_date(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t date); /// set date
void set_ds3231_month(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t month); /// set month
void set_ds3231_year(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t year); /// set year

/*full set time*/

/*
 * default 12h format when start
 * must call function to set 12h or 24h format
 */

void set_ds3231_time_12H_format(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour,Format_12_AM_PM AM_PM, uint8_t minute, uint8_t sec); /// setup 12h format
void set_ds3231_time_24H_format(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t hour, uint8_t minute, uint8_t sec); /// setup 24h format
void ChangeHourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx); /// switch format between 12h and 24 h

/*full set calendar*/

void set_ds3231_calendar(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, day_type day, uint8_t date, uint8_t month, uint8_t year); /// for calendar

/*-------------------------------------------------------get time function----------------------------------------------*/

/// none of function below can print on its own.
/// These Function only update ds3231_data variable, print its variable's param on your own

/*get each time param*/

void read_ds3231_second(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_minute(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_hour(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_day(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_date(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_month(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_year(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);

/*get full time param*/

int8_t is_12HourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
Format_12_AM_PM get_AM_PM(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
void read_ds3231_time(ds3231_data *ds3231_x,I2C_TypeDef * i2cx); /// just call this function then print all ds3231x field

#ifdef __cplusplus
}
#endif

#endif

/*end of file*/
