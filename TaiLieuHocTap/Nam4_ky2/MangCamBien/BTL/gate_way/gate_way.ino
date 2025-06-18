/*********
 * TÊN FILE: GATEWAY_FINAL_WITH_LOGGING.INO
 * MÔ TẢ:
 * - Đã thêm các dòng Serial.printf để in ra các giá trị cần thiết,
 * giúp theo dõi và gỡ lỗi dễ dàng hơn.
*********/

// --- THƯ VIỆN VÀ CẤU HÌNH (Giữ nguyên) ---
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <esp_now.h>

const int numLights = 4;
const int numFans = 4;
const int lightPins[numLights] = {26, 25, 33, 32};
const int fanPins[numFans] = {27, 14, 12, 13};

typedef struct struct_message {
    int id; float temperature; float humidity; float light_intensity; bool motion_detected;
} struct_message;
struct_message incomingData, dataToProcess; 
volatile bool newData = false;

#define WIFI_SSID "Tang 7_2"
#define WIFI_PASSWORD "gongangsachse"
#define Web_API_KEY "AIzaSyAH76sndFX2iDnoJq8aiDVBRvyJFerP4Yo"
#define DATABASE_URL "https://espproject-ccd63-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "doanlongvu2003@gmail.com"
#define USER_PASS "Vu26122002:)"

UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 500;

void processData(AsyncResult &aResult);

// --- HÀM CALLBACK ESP-NOW (Giữ nguyên) ---
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    memcpy(&incomingData, data, sizeof(incomingData));
    newData = true; 
}

void setup(){
    Serial.begin(115200);

    for (int i = 0; i < numLights; i++) { pinMode(lightPins[i], OUTPUT); digitalWrite(lightPins[i], LOW); }
    for (int i = 0; i < numFans; i++) { pinMode(fanPins[i], OUTPUT); digitalWrite(fanPins[i], LOW); }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(300); }
    Serial.println("\nWiFi connected!");
    
    configTime(7 * 3600, 0, "pool.ntp.org");
    
    ssl_client.setInsecure();
    initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) { return; }
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    Serial.println(">>> Gateway da san sang. <<<");
}

void loop() {
    app.loop();

    if (newData) {
        newData = false;
        noInterrupts();
        dataToProcess = incomingData;
        interrupts();
        
        // <<< IN RA: Dữ liệu cảm biến vừa nhận được >>>
        Serial.println("\n-------------------------------------------");
        Serial.printf("> Nhan duoc du lieu tu Node ID: %d\n", dataToProcess.id);
        Serial.printf("  - Nhiet do        : %.2f *C\n", dataToProcess.temperature);
        Serial.printf("  - Do am           : %.2f %%\n", dataToProcess.humidity);
        Serial.printf("  - Anh sang        : %.2f lx\n", dataToProcess.light_intensity);
        Serial.printf("  - Chuyen dong     : %s\n", dataToProcess.motion_detected ? "Co" : "Khong");

        if (app.ready()){
            Serial.println("> Trang thai: Firebase OK. Bat dau gui du lieu...");
            String basePath = "/sensorData/node" + String(dataToProcess.id);
            Database.set<float>(aClient, basePath + "/temperature", dataToProcess.temperature, processData, "Send_Temp");
            Database.set<float>(aClient, basePath + "/humidity", dataToProcess.humidity, processData, "Send_Hum");
            Database.set<float>(aClient, basePath + "/light_intensity", dataToProcess.light_intensity, processData, "Send_Light");
            Database.set<bool>(aClient, basePath + "/motion_detected", dataToProcess.motion_detected, processData, "Send_Motion");
            Database.set<String>(aClient, basePath + "/last_update", ".sv", processData, "Set_Timestamp");
        } else {
            Serial.println("> Trang thai: Firebase CHUA san sang.");
        }
        Serial.println("-------------------------------------------");
    }

    if (app.ready()) {
        unsigned long currentTime = millis();
        if (currentTime - lastCheckTime >= checkInterval) {
            lastCheckTime = currentTime;
            
            for (int i = 1; i <= numLights; i++) {
                Database.get(aClient, "/LED/LED" + String(i), processData, false, "Get_LED" + String(i));
            }
            for (int i = 1; i <= numFans; i++) {
                Database.get(aClient, "/LED/Fan" + String(i), processData, false, "Get_Fan" + String(i));
            }
        }
    }
}

void processData(AsyncResult &aResult){
    // Xử lý các sự kiện
    if (aResult.isEvent())
        Firebase.printf("Event: %s, msg: %s\n", aResult.uid().c_str(), aResult.eventLog().message().c_str());
    
    // <<< IN RA: Bật lại phần báo lỗi quan trọng >>>
    //if (aResult.isError())
       // Firebase.printf("Error: %s, msg: %s\n", aResult.uid().c_str(), aResult.error().message().c_str());

    // Chỉ xử lý khi có dữ liệu payload trả về
    if (aResult.available()) {
        String uid = aResult.uid();
        String payload = aResult.c_str();
        bool state = (payload == "true");

        // Dùng vòng lặp để kiểm tra và điều khiển ĐÈN
        for (int i = 1; i <= numLights; i++) {
            if (uid == "Get_LED" + String(i)) {
                // <<< IN RA: Hành động điều khiển cụ thể >>>
                //Serial.printf("  [CONTROL] Dieu khien Den %d -> %s\n", i, state ? "BAT" : "TAT");
                digitalWrite(lightPins[i - 1], state);
                return; 
            }
        }
        
        // Dùng vòng lặp để kiểm tra và điều khiển QUẠT
        for (int i = 1; i <= numFans; i++) {
            if (uid == "Get_Fan" + String(i)) {
                // <<< IN RA: Hành động điều khiển cụ thể >>>
               // Serial.printf("  [CONTROL] Dieu khien Quat %d -> %s\n", i, state ? "BAT" : "TAT");
                digitalWrite(fanPins[i - 1], state);
                return;
            }
        }
    }
}