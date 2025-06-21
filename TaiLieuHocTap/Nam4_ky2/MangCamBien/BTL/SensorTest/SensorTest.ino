#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

// Chân kết nối cảm biến
#define DHTPIN 5
#define PIRPIN 13
#define DHTTYPE DHT22
#define I2C_SDA 21
#define I2C_SCL 22

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(0x23);

void setup() {
  Serial.begin(115200);

  // Khởi tạo cảm biến DHT22
  dht.begin();

  // Khởi tạo PIR
  pinMode(PIRPIN, INPUT);

  // Khởi tạo I2C và cảm biến ánh sáng BH1750
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Lỗi khởi tạo BH1750!");
  } else {
    Serial.println("Khởi tạo BH1750 thành công!");
  }
}

void loop() {
  delay(10); // Đợi giữa các lần đo

  // Đọc DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.print("Error | Error | ");
  } else {
    Serial.printf("%.2f | %.2f | ", t, h);
  }

  // Đọc BH1750
  float lux = lightMeter.readLightLevel();
  if (lux < 0) {
    Serial.print("Error | ");
  } else {
    Serial.printf("%.2f | ", lux);
  }

  // Đọc PIR
  bool motion = digitalRead(PIRPIN);
  Serial.println(motion ? "1" : "0");
}

