#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#define DHTPIN 5
#define PIRPIN 13
#define I2C_SDA 21
#define I2C_SCL 22
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter(0x23);

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(PIRPIN, INPUT);
    Wire.begin(I2C_SDA, I2C_SCL);
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    Serial.println("Live Data Streamer Ready.");
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float l = lightMeter.readLightLevel();
    bool m = digitalRead(PIRPIN);

    if (!isnan(t) && !isnan(h) && l >= 0) {
        // In ra một dòng duy nhất, các giá trị cách nhau bởi dấu phẩy
        Serial.printf("%.2f,%.2f,%.2f,%d\n", t, h, l, m ? 1 : 0);
    }
    delay(1000); // Gửi dữ liệu 1 giây một lần
}