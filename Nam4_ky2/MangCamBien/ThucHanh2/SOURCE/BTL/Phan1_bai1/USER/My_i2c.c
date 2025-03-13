#include "My_i2c.h"

/*Pham The Anh - B20DCDT017*/
/*! Luu y: Thu vien chi ho tro giao tiep thiet bi dia chi 7 bit o thoi diem hien tai*/

/*Declare private function*/

static void set_i2c_7bit_address(I2C_TypeDef * i2cx, uint16_t i2c_address, uint8_t direct_i2c);
static void start_i2c(I2C_TypeDef * i2cx, uint16_t i2c_address);
static void stop_i2c(I2C_TypeDef * i2cx);
static void i2c_standard_send_a_byte(I2C_TypeDef * i2cx, uint8_t serial_line);
static uint8_t i2c_receive_a_byte_ack(I2C_TypeDef * i2cx);
static uint8_t i2c_receive_a_byte_nack(I2C_TypeDef * i2cx);

/*define main function*/

void i2c_write_one_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  i2c_standard_send_a_byte(i2cx,data);
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)){} /// wait till transmitted last byte complete
  stop_i2c(i2cx);
}

void i2c_write_multi_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t len, uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  for(int i=0;i<len;i++){
      i2c_standard_send_a_byte(i2cx,*(data+i));
  }
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)){} /// wait till transmitted last byte complete
  stop_i2c(i2cx);
}

void i2c_write_one_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr, uint8_t data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  i2c_standard_send_a_byte(i2cx,reg_addr);
  i2c_standard_send_a_byte(i2cx,data);
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)){} /// wait till transmitted last byte complete
  stop_i2c(i2cx);
}

void i2c_write_multi_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr, uint8_t len, uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  i2c_standard_send_a_byte(i2cx,reg_addr);
  for(int i=0;i<len;i++){
      i2c_standard_send_a_byte(i2cx,*(data+i));
  }
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)){} /// wait till transmitted last byte complete
  stop_i2c(i2cx);
}

/// Function for i2c receiver (Master receive, slave write)
/// only support 7bit address device for now

void i2c_read_one_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Receiver);
  *data = i2c_receive_a_byte_nack(i2cx);
  stop_i2c(i2cx);
}

void i2c_read_multi_byte_no_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t len,uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Receiver);
  for(int i=0;i<len;i++){
      if(i==len-1){
        *(data+i) = i2c_receive_a_byte_nack(i2cx);
      }
      else{
        *(data+i) = i2c_receive_a_byte_ack(i2cx) ;
      }
  }
  stop_i2c(i2cx);
}

void i2c_read_one_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address,uint8_t reg_addr,uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  i2c_standard_send_a_byte(i2cx,reg_addr);
  stop_i2c(i2cx);
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Receiver);
  *data = i2c_receive_a_byte_nack(i2cx);
  stop_i2c(i2cx);
}

void i2c_read_multi_byte_with_reg(I2C_TypeDef * i2cx, uint16_t address, uint8_t reg_addr, uint8_t len,uint8_t *data){
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Transmitter);
  i2c_standard_send_a_byte(i2cx,reg_addr);
  stop_i2c(i2cx);
  start_i2c(i2cx,address);
  set_i2c_7bit_address(i2cx,address,I2C_Direction_Receiver);
  for(int i=0;i<len;i++){
      if(i==len-1){
        *(data+i) = i2c_receive_a_byte_nack(i2cx);
      }
      else{
        *(data+i) = i2c_receive_a_byte_ack(i2cx) ;
      }
  }
  stop_i2c(i2cx);
}

/*define private function*/

static void set_i2c_7bit_address(I2C_TypeDef * i2cx, uint16_t i2c_address, uint8_t direct_i2c){
  I2C_Send7bitAddress(i2cx,i2c_address,direct_i2c);

  switch(direct_i2c){
    case I2C_Direction_Transmitter:{
      while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){} /// wait till i2c -> master transmitter mode
      break;
    }
    case I2C_Direction_Receiver:{
      while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)){} /// wait till i2c -> master receiver mode
      break;
    }
  }
}

static void start_i2c(I2C_TypeDef * i2cx, uint16_t i2c_address){ /// after that call, stm32 default turn to master mode. Slave use that to recognize a start serial communicate
  I2C_GenerateSTART(i2cx,ENABLE);/// bat dau chuyen i2c
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_MODE_SELECT)){} /// i2c select to be master mode
}

static void stop_i2c(I2C_TypeDef * i2cx){ /// end i2c serial communicate, i2c turn to idle mode
  I2C_GenerateSTOP(i2cx,ENABLE); /// truyen tin hieu ket thuc
}

static void i2c_standard_send_a_byte(I2C_TypeDef * i2cx, uint8_t serial_line){ /// send as much as you want during transmit
  I2C_SendData(i2cx,serial_line);
  while(!I2C_CheckEvent(i2cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)){} /// check if i2c master transmitted complete and receive ack
}

static uint8_t i2c_receive_a_byte_ack(I2C_TypeDef * i2cx)
{
	// Enable ACK of received data
	I2C_AcknowledgeConfig(i2cx, ENABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

	// Read and return data byte from I2C data register
	return I2C_ReceiveData(i2cx);
}

static uint8_t i2c_receive_a_byte_nack(I2C_TypeDef * i2cx)
{
	// Disable ACK of received data
	I2C_AcknowledgeConfig(i2cx, DISABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

	// Read and return data byte from I2C data register
	return I2C_ReceiveData(i2cx);
}
/*end of file*/
