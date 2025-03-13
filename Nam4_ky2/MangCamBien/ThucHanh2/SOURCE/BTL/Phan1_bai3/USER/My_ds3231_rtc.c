#include "My_ds3231_rtc.h"
#include "My_i2c.h"
#include "stm32f10x.h"
//#include "stdio.h"
//#include "My_usart.h"

/*-------------------------------------------------------------define macro------------------------------------------------*/

/// convert from BCD to Decimal
#define BCDToDec(num) ((uint32_t)(((num)/16 * 10) + ((num) % 16)))

/// convert from Decimal to BCD
#define DecToBCD(num) ((uint8_t)(((num)/10 * 16) + ((num) % 10)))

/// Address of DS3231 register

#define DS3231_ADDRESS ((uint8_t) 0x68 << 1)
#define DS3231_REG_TIME ((uint8_t)0x00)
#define DS3231_REG_TIME_SEC ((uint8_t)0x00)
#define DS3231_REG_TIME_MIN ((uint8_t)0x01)
#define DS3231_REG_TIME_HOUR ((uint8_t)0x02)
#define DS3231_REG_TIME_DAY ((uint8_t)0x03)
#define DS3231_REG_TIME_DATE ((uint8_t)0x04)
#define DS3231_REG_TIME_MONTH ((uint8_t)0x05)
#define DS3231_REG_TIME_YEAR ((uint8_t)0x06)

#define DS3231_REG_ALARM_1_  ((uint8_t)0x07)
#define DS3231_REG_ALARM_2_  ((uint8_t)0x0B)

#define DS3231_REG_CONTROL  ((uint8_t)0x0E)
#define DS3231_REG_CONTROL_EOSC ((uint8_t)0x80)

#define DS3231_REG_STATUS   ((uint8_t)0x0F)
#define DS3231_REG_STATUS_OSF   ((uint8_t)0x80)

#define DS3231_REG_TEMPERATURE ((uint8_t)0x11)

#define _HOUR_FORMAT ((uint8_t)0x40)
#define _12H_format ((uint8_t)0x40)
#define _24H_format ((uint8_t)0x00)

#define _12H_AM_PM ((uint8_t)0x20)
#define _12H_format_AM ((uint8_t)0x00)
#define _12H_format_PM ((uint8_t)0x20)

#define CenturyNotify ((uint8_t)0x80)

/*---------------------------------------------------------Declare private function---------------------------------------------------*/

static uint8_t check_min_max(uint8_t val, uint8_t min_, uint8_t max_);

static uint8_t getControlReg_EOSC(I2C_TypeDef * i2cx);
static uint8_t getStatusReg_OSF(I2C_TypeDef * i2cx);

static void clear_EOSC(I2C_TypeDef * i2cx);
static void set_OSF(I2C_TypeDef * i2cx);

static void getHourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);

static void changeFormat_24H_To_12H(ds3231_data *ds3231_x,I2C_TypeDef * i2cx);
static void changeFormat_12H_To_24H(ds3231_data * ds3231_x,I2C_TypeDef * i2cx);

/*----------------------------------------------------------define main function------------------------------------------------------*/

void ds3231_init(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){ /// default value for all param of ds3231 just in case, not affect getime function, can be use for debug (option)
  ds3231_x->HourFormat = _12H_format; /// 12 hour format by default
  ds3231_x->AM_PM = _12H_format_AM;
  ds3231_x->second = 1;
  ds3231_x->minute = 1;
  ds3231_x->hour = 1;
  ds3231_x->day = Mon;
  ds3231_x->date = 1;
  ds3231_x->month = 1;
  ds3231_x->year = 1;

  clear_EOSC(i2cx);
  set_OSF(i2cx);
//  printf("all good, done config ds3231\n");
}

/*set time function*/

void set_ds3231_sec(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t sec){
  uint8_t temp = DecToBCD(check_min_max(sec,0,59));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_SEC,temp);
}

void set_ds3231_minute(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t minute){
  uint8_t temp = DecToBCD(check_min_max(minute,0,59));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_MIN,temp);
}

void set_ds3231_hour_12HFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour,uint8_t AM_PM){
  ds3231_x->HourFormat = _12H_format;
  ds3231_x->AM_PM = AM_PM;
  uint8_t hour12 = DecToBCD(check_min_max(hour,1,12));
  uint8_t temp = hour12 | _12H_format | AM_PM;
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_HOUR,temp);
}

void set_ds3231_hour_24HFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour){
  ds3231_x->HourFormat = _24H_format;

  uint8_t hour24 = DecToBCD(check_min_max(hour,0,23));
  uint8_t temp = hour24 | _24H_format;
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_HOUR,temp);
}

void set_ds3231_day(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,day_type day){
  uint8_t temp = DecToBCD(check_min_max(day,1,7));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_DAY,temp);
}

void set_ds3231_date(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t date){
  uint8_t temp = DecToBCD(check_min_max(date,1,31));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_DATE,temp);
}

void set_ds3231_month(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t month){
  uint8_t temp = DecToBCD(check_min_max(month,1,12));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_MONTH,temp);
}

void set_ds3231_year(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t year){
  uint8_t temp = DecToBCD(check_min_max(year,0,99));
  i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_YEAR,temp);
}

void set_ds3231_time_12H_format(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, uint8_t hour,Format_12_AM_PM AM_PM, uint8_t minute, uint8_t sec){
  uint8_t AM_PM_x = _12H_format_AM; /// default
  switch(AM_PM){
    case AM:{
      AM_PM_x = _12H_format_AM;
      break;
    }
    case PM:{
      AM_PM_x = _12H_format_PM;
      break;
    }
  }
  set_ds3231_sec(ds3231_x,i2cx,sec);
  set_ds3231_minute(ds3231_x,i2cx,minute);
  set_ds3231_hour_12HFormat(ds3231_x,i2cx,hour,AM_PM_x);
}

void set_ds3231_time_24H_format(ds3231_data *ds3231_x,I2C_TypeDef * i2cx,uint8_t hour, uint8_t minute, uint8_t sec){
  set_ds3231_sec(ds3231_x,i2cx,sec);
  set_ds3231_minute(ds3231_x,i2cx,minute);
  set_ds3231_hour_24HFormat(ds3231_x,i2cx,hour);
}

void set_ds3231_calendar(ds3231_data *ds3231_x,I2C_TypeDef * i2cx, day_type day, uint8_t date, uint8_t month, uint8_t year){
  set_ds3231_day(ds3231_x,i2cx,day);
  set_ds3231_date(ds3231_x,i2cx,date);
  set_ds3231_month(ds3231_x,i2cx,month);
  set_ds3231_year(ds3231_x,i2cx,year);
}

void ChangeHourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  getHourFormat(ds3231_x,i2cx);
  switch(ds3231_x->HourFormat){
    case _12H_format:{
       changeFormat_12H_To_24H(ds3231_x,i2cx);
       break;
    }
    case _24H_format:{
      changeFormat_24H_To_12H(ds3231_x,i2cx);
      break;
    }
  }
}

/*get time function*/

void read_ds3231_second(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_SEC,&temp);
  ds3231_x->second = BCDToDec(temp);
}

void read_ds3231_minute(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_MIN,&temp);
  ds3231_x->minute = BCDToDec(temp);
}

void read_ds3231_hour(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_HOUR,&temp);
  switch(ds3231_x->HourFormat){
    case _12H_format:{
       ds3231_x->AM_PM = temp & _12H_AM_PM;
       ds3231_x->hour = BCDToDec((temp & 0x1F));
       break;
    }
    case _24H_format:{
      ds3231_x->hour = BCDToDec((temp & 0x3F));
      break;
    }
  }
}

void read_ds3231_day(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_DAY,&temp);
  ds3231_x->day = BCDToDec(temp);
}

void read_ds3231_date(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_DATE,&temp);
  ds3231_x->date = BCDToDec(temp);
}

void read_ds3231_month(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_MONTH,&temp);
  ds3231_x->month = BCDToDec((temp & 0xEF)); /// don't care century
}

void read_ds3231_year(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_YEAR,&temp);
  ds3231_x->year = BCDToDec(temp);
}

int8_t is_12HourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  switch(ds3231_x->HourFormat){
    case _12H_format:{
      return 1;
    }
    case _24H_format:{
      return 0;
    }
  }
}

Format_12_AM_PM get_AM_PM(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  switch(ds3231_x->AM_PM){
    case _12H_format_AM:{
      return AM;
    }
    case _12H_format_PM:{
      return PM;
    }
  }
}

void read_ds3231_time(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t time_data[7] = {0};
  i2c_read_multi_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME,7,time_data);
  ds3231_x->second = BCDToDec(time_data[0]);
  ds3231_x->minute = BCDToDec(time_data[1]);
  switch(ds3231_x->HourFormat){
    case _12H_format:{
       ds3231_x->AM_PM = time_data[2] & _12H_AM_PM;
       ds3231_x->hour = BCDToDec((time_data[2] & 0x1F));
       break;
    }
    case _24H_format:{
      ds3231_x->hour = BCDToDec((time_data[2] & 0x3F));
      break;
    }
  }
  ds3231_x->day = BCDToDec(time_data[3]);
  ds3231_x->date = BCDToDec(time_data[4]);
  ds3231_x->month = BCDToDec(time_data[5]);
  ds3231_x->year = BCDToDec(time_data[6]);
}

/*----------------------------------------------------------define private function---------------------------------------------------*/

static uint8_t check_min_max(uint8_t val, uint8_t min_, uint8_t max_)
{
	if (val < min_)
		return min_;
	if (val > max_)
		return max_;
	return val;
}

static uint8_t getControlReg_EOSC(I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_CONTROL,&temp);
  return (temp & DS3231_REG_CONTROL_EOSC);
}

static uint8_t getStatusReg_OSF(I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_STATUS,&temp);
  return (temp & DS3231_REG_STATUS_OSF);
}

static void clear_EOSC(I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  temp = getControlReg_EOSC(i2cx);
  if(temp  == DS3231_REG_CONTROL_EOSC){
//    printf("EOSC be set -> Vcc not stable\n");
    i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_CONTROL,(temp &~ DS3231_REG_CONTROL_EOSC));
  }
}

static void set_OSF(I2C_TypeDef * i2cx){
  uint8_t temp = 0;
  temp = getStatusReg_OSF(i2cx);
  if(!(temp  == DS3231_REG_STATUS_OSF)){
//    printf("OSF be clear -> Oscillator Stop\n");
//    printf("Restart Oscillator...\n");
    i2c_write_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_STATUS,(temp | DS3231_REG_STATUS_OSF));
//    printf("Restart Oscillator done\n");
  }
}

static void getHourFormat(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t Hour_reg = 0;
  i2c_read_one_byte_with_reg(i2cx,DS3231_ADDRESS,DS3231_REG_TIME_HOUR,&Hour_reg);
  uint8_t format = Hour_reg & _HOUR_FORMAT;
  ds3231_x->HourFormat = format;
}

static void changeFormat_24H_To_12H(ds3231_data *ds3231_x,I2C_TypeDef * i2cx){
  uint8_t AM_PM = _12H_format_AM;
  uint8_t hour = ds3231_x->hour;
  uint8_t minute = ds3231_x->minute;
  uint8_t second = ds3231_x->second;

  if(ds3231_x->hour < 12){
      AM_PM = _12H_format_AM;
  }
  if(ds3231_x->hour > 12){
      hour -= 12;
      AM_PM = _12H_format_PM;
  }
  if(ds3231_x->hour == 12 && (ds3231_x->minute > 0 || ds3231_x->second > 0)){
      hour -= 12;
      AM_PM = _12H_format_PM;
  }
  set_ds3231_time_12H_format(ds3231_x,i2cx,hour,AM_PM,minute,second);
}

static void changeFormat_12H_To_24H(ds3231_data * ds3231_x,I2C_TypeDef * i2cx){
  uint8_t hour = ds3231_x->hour;
  uint8_t minute = ds3231_x->minute;
  uint8_t second = ds3231_x->second;

  if(ds3231_x->AM_PM == _12H_format_PM){
     ds3231_x->hour += 12;
  }

  set_ds3231_time_24H_format(ds3231_x,i2cx,hour,minute,second);
}

/*end of file*/
