#include <Arduino.h>
#include "lib_convert.h"
#include <esp_now.h>
#include <WiFi.h>

#define BUFFER_SIZE 20
#define HEADER_1 0xAA
#define HEADER_2 0x55
#define MESSAGE_LENGTH 14  // Độ dài bản tin cần nhận

void processMessage(uint8_t* data);
void loop_lux_sensor(void);

uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broadcastAddress2[] = {0x2c,0xbc ,0xbb ,0x92 , 0x75, 0x48 }; //2c:bc:bb:92:75:48
uint8_t broadcastAddress3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct test_struct {
  int x;
  int y;
} test_struct;

test_struct test;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
   
  // register peer
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // register first peer  
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register second peer  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  /// register third peer
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Start UART Monitor");
}



void loop() 
{
  esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);

  loop_lux_sensor();
  //delay(1000);
}

void processMessage(uint8_t* data) {
  // AA 55 00 04 00 0A 01 01 41 B0 66 66 96 F5
  //                         ^ Bắt đầu từ đây
  // Đảo thứ tự byte từ Little Endian sang Big Endian
  float value = Convert_Bytes_To_Float(data[11], data[10], data[9], data[8]);
  
  Serial.print("Raw bytes: ");
  for(int i = 8; i < 12; i++) {
      if(data[i] < 0x10) Serial.print("0");
      Serial.print(data[i], HEX);
      Serial.print(" ");
  }
  Serial.println();
  
  Serial.print("Converted float value: ");
  Serial.println(value, 2); // In ra với 2 chữ số thập phân

  test.x = (int) value;
  test.y = (int)((value - test.x ) * 100);
  
}

void loop_lux_sensor(void)
{

  static uint8_t data[BUFFER_SIZE];
  static uint8_t length_data_receive = 0;
  
  while (Serial2.available()) {
    uint8_t incomingByte = Serial2.read();
   /*
    // Debug mỗi byte nhận được
    Serial.print("Byte received: 0x");
    if(incomingByte < 0x10) Serial.print("0");
    Serial.println(incomingByte, HEX);
    Serial.println(" ");
    */
    data[length_data_receive] = incomingByte;
    length_data_receive++;
    
    // Kiểm tra header và xử lý bản tin hoàn chỉnh
    if (length_data_receive >= 2) {
      if (data[0] == HEADER_1 && data[1] == HEADER_2) {
        if (length_data_receive == MESSAGE_LENGTH) {
          Serial.println("\nComplete message received!");
          Serial.print("Message: ");
          for (int i = 0; i < MESSAGE_LENGTH; i++) {
            if(data[i] < 0x10) Serial.print("0");
            Serial.print(data[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
          
          // Xử lý 4 byte thành float
          processMessage(data);
          
          // Reset buffer sau khi nhận đủ bản tin
          length_data_receive = 0;
        }
      } else if (data[length_data_receive-1] == HEADER_1) {
        // Nếu tìm thấy header mới, dịch buffer
        data[0] = HEADER_1;
        length_data_receive = 1;
      }
    }
    
    // Kiểm tra overflow buffer
    if (length_data_receive >= BUFFER_SIZE) {
      Serial.println("Buffer overflow - resetting");
      length_data_receive = 0;
    }
  }
}