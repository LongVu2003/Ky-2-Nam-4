#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

// ======================================================================
// >>> NODE DÈIND
// ======================================================================
#define NODE_ID 1 // <<< THAY ĐỔI ID CHO MỖI NODE (1, 2, 3)

// Chân cảm biến
#define DHTPIN 4      // Chân DHT22
#define PIRPIN 5      // Chân PIR
#define I2C_SDA 21    // Chân I2C SDA
#define I2C_SCL 22    // Chân I2C SCL
// ======================================================================

#define DHTTYPE DHT22
#define SLEEP_DURATION_SECONDS 5 // 5 phút

// Địa chỉ MAC của ESP32 Gateway
//uint8_t gatewayAddress[] = {0xEC,0xE3,0x34,0xbf,0x87,0x5c}; // EC:E3:34:BF:87:5C
uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broadcastAddress2[] = {0x10,0x06,0x1C,0xB5,0xC3,0x80}; //10:06:1C:B5:C3:80
uint8_t broadcastAddress3[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Cấu trúc dữ liệu mới, đã loại bỏ co2 và tvoc
typedef struct struct_message {
    int id;
    float temperature;
    float humidity;
    float light_intensity;
    bool motion_detected;
} struct_message;

struct_message myData;

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

// Khởi tạo các đối tượng cảm biến
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(0x23);

volatile bool sendSuccess = false;

// // Hàm callback khi dữ liệu được gửi
// void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
//     sendSuccess = (status == ESP_NOW_SEND_SUCCESS);
//     Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

void setup() {
    Serial.begin(115200);
    // Khởi tạo cảm biến
    dht.begin();
    pinMode(PIRPIN, INPUT);
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        Serial.println(F("Loi khoi tao BH1750"));
    }

    WiFi.mode(WIFI_STA);
    Serial.print("Node ID: ");
    Serial.print(NODE_ID);
    Serial.print(", MAC Address: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("Loi khoi tao ESP-NOW");
        ESP.restart();
    }
    esp_now_register_send_cb(OnDataSent);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
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
    
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
    sendSuccess = false;

    // Đọc dữ liệu từ các cảm biến
    myData.id = NODE_ID;
    myData.humidity = dht.readHumidity();
    myData.temperature = dht.readTemperature();
    //float h = dht.readHumidity();
     //Serial.printf("Data : Hum=%.2f",h);
     //Serial.println();
    myData.motion_detected = digitalRead(PIRPIN);
    myData.light_intensity = lightMeter.readLightLevel();

    if (isnan(myData.humidity) || isnan(myData.temperature)) {
        Serial.println("Loi doc tu cam bien DHT!");
        myData.temperature = -999.0f;
        myData.humidity = -999.0f;
    }
    if (myData.light_intensity < 0) {
        Serial.println("Loi doc tu cam bien BH1750!");
        myData.light_intensity = -999.0f;
    }
    
    // In dữ liệu ra Serial để debug
    Serial.printf("Sending data: Temp=%.2f, Hum=%.2f, Light=%.2f, Motion=%d\n", 
                  myData.temperature, myData.humidity, myData.light_intensity, myData.motion_detected);

    // Gửi dữ liệu qua ESP-NOW
    //esp_now_send(gatewayAddress, (uint8_t *) &myData, sizeof(myData));
    esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(struct_message));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
    // Chờ một chút để callback có thể được gọi
    delay(500);
    
    Serial.print("Di vao che do ngu sau trong ");
    Serial.print(SLEEP_DURATION_SECONDS);
    Serial.println(" giay...");
    esp_deep_sleep(SLEEP_DURATION_SECONDS * 1000000ULL);
}