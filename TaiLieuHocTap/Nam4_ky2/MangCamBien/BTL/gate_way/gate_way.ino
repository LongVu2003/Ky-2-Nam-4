/*********
 * TÊN FILE: GATEWAY_THE_TRUE_FINAL.INO
 * MÔ TẢ:
 * - Áp dụng cách kiểm tra lỗi aClient.lastError() chính xác theo ví dụ bạn cung cấp.
 * - Đây là phiên bản hoàn thiện và ổn định nhất dựa trên tất cả các thảo luận của chúng ta.
*********/

// --- THƯ VIỆN ---
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <esp_now.h>

// --- CẤU HÌNH GPIO ---
const int numLights = 4;
const int numFans = 4;
const int lightPins[numLights] = {26, 25, 33, 32};
const int fanPins[numFans] = {27, 14, 12, 13};

// --- CẤU TRÚC DỮ LIỆU ---
typedef struct struct_message {
    int id;
    float temperature;
    float humidity;
    float light_intensity;
    bool motion_detected;
} struct_message;
struct_message incomingData;

// --- CẤU HÌNH MẠNG VÀ FIREBASE ---
#define WIFI_SSID "Xuong"
#define WIFI_PASSWORD "68686868"
#define Web_API_KEY "AIzaSyAH76sndFX2iDnoJq8aiDVBRvyJFerP4Yo"
#define DATABASE_URL "https://espproject-ccd63-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "doanlongvu2003@gmail.com"
#define USER_PASS "Vu26122002:)"

// --- CÁC ĐỐI TƯỢNG FIREBASE ---
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

void processData(AsyncResult &aResult);

// --- HÀM CALLBACK KHI NHẬN DỮ LIỆU ESP-NOW ---
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    memcpy(&incomingData, data, sizeof(incomingData));

    Serial.println("-------------------------------------------");
    Serial.print("Nhan du lieu tu Node co dia chi MAC: ");
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
    Serial.printf("  Node ID: %d\n", incomingData.id);
    Serial.printf("  Nhiet do: %.2f *C\n", incomingData.temperature);
    Serial.printf("  Do am: %.2f %%\n", incomingData.humidity);
    Serial.printf("  Cuong do anh sang: %.2f lx\n", incomingData.light_intensity);
    Serial.printf("  Phat hien chuyen dong: %s\n", incomingData.motion_detected ? "Co" : "Khong");

    // <<<<<<<<<<<<<<< LOGIC GỬI FIREBASE ĐƯỢC CHUYỂN VÀO ĐÂY >>>>>>>>>>>>>>>
    if (app.ready()){
        Serial.println("Firebase da san sang. Dang day du lieu len...");
        String basePath = "/sensorData/node" + String(incomingData.id);
        
        Database.set<float>(aClient, basePath + "/temperature", incomingData.temperature, processData, "Send_Temp");
        Database.set<float>(aClient, basePath + "/humidity", incomingData.humidity, processData, "Send_Hum");
        Database.set<float>(aClient, basePath + "/light_intensity", incomingData.light_intensity, processData, "Send_Light");
        Database.set<bool>(aClient, basePath + "/motion_detected", incomingData.motion_detected, processData, "Send_Motion");
        Database.set<String>(aClient, basePath + "/last_update", ".sv", processData, "Set_Timestamp");
    } else {
        Serial.println("Firebase CHUA san sang, khong the gui du lieu.");
    }
    Serial.println("-------------------------------------------");
}

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < numLights; i++) { pinMode(lightPins[i], OUTPUT); digitalWrite(lightPins[i], LOW); }
    for (int i = 0; i < numFans; i++) { pinMode(fanPins[i], OUTPUT); digitalWrite(fanPins[i], LOW); }

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("."); delay(300);
    }
    Serial.println("\nWiFi connected!");
    
    configTime(7 * 3600, 0, "pool.ntp.org");
    
    ssl_client.setInsecure();
    initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) { return; }
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    Serial.println("Gateway da san sang.");
}

void loop() {
    app.loop();

    if (app.ready()) {
        // Lặp qua 4 đèn
        for (int i = 0; i < numLights; i++) {
            String path = "/LED/LED" + String(i + 1);
            // Lấy trạng thái của đèn
            bool state = Database.get<bool>(aClient, path);
            
            // << SỬA LỖI CUỐI CÙNG: Dùng aClient.lastError() để kiểm tra, theo đúng ví dụ bạn gửi >>
            if (aClient.lastError().code() == 0) {
                 digitalWrite(lightPins[i], state);
            }
        }
        // Lặp qua 4 quạt
        for (int i = 0; i < numFans; i++) {
            String path = "/LED/Fan" + String(i + 1);
            bool state = Database.get<bool>(aClient, path);

            // << SỬA LỖI CUỐI CÙNG: Dùng aClient.lastError() để kiểm tra, theo đúng ví dụ bạn gửi >>
            if (aClient.lastError().code() == 0) {
                digitalWrite(fanPins[i], state);
            }
        }
    }
    delay(1000);
}

// Hàm callback này chủ yếu để xử lý lỗi hoặc các sự kiện nền
void processData(AsyncResult &aResult) {
    if (!aResult.isResult()) return;
    if (aResult.isError()) {
        Firebase.printf("Loi tac vu: %s, Thong diep: %s, Ma loi: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
}