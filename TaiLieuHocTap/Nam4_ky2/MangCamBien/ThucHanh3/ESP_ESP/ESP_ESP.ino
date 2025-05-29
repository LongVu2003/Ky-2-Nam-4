#include <esp_now.h>
#include <WiFi.h>

//Structure example to receive data
//Must match the sender structure

typedef struct test_struct {
  int x;
  int y;
 // int z;
  //int t;
} test_struct;

//Create a struct_message called myData
test_struct myData;

//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("gia tri cam bien anh sang: ");
  Serial.print(myData.x);
  Serial.print(",");
  Serial.print(myData.y);
 /* Serial.print("z: ");
  Serial.println(myData.z);
  Serial.print("t: ");
  Serial.println(myData.t);*/
  Serial.println();
}
 
void setup() {
  //Initialize Serial Monitor
  Serial.begin(9600);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("DoanLongVu");
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {

}
