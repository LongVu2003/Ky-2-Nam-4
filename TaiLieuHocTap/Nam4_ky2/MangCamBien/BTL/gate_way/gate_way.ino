/*********
 * FileNmae: gate_way.ino
 * Author : Doan Long Vu
 * Date : 19/06/2025
*********/

// --- THƯ VIỆN VÀ CẤU HÌNH ---
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <esp_now.h>

// <<< THÊM MỚI: Thư viện AI từ Edge Impulse >>>
// !!! Hãy chắc chắn tên này đúng với project của bạn !!!
#include <SmartClassRoom_inferencing.h>


// --- CẤU HÌNH GPIO VÀ STRUCT---
const int numLights = 4;
const int numFans = 4;
const int lightPins[numLights] = {13, 12, 14, 27};
const int fanPins[numFans] = {32, 33, 25, 26};

typedef struct struct_message {
    int id; float temperature; float humidity; float light_intensity; bool motion_detected;
} struct_message;
struct_message incomingData, dataToProcess; 
volatile bool newData = false;

// Biến trạng thái cho chế độ AI
bool autoMode = true;


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


// --- BIẾN ĐỊNH THỜI  ---
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 3000;


// --- KHAI BÁO HÀM ---
void processData(AsyncResult &aResult);
// <<< THÊM MỚI: Khai báo trước hàm AI >>>
void run_ai_inference();


// <<<Hàm helper để in log từ thư viện AI >>>
void ei_printf(const char *format, ...) {
    static char print_buf[1024] = { 0 };
    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);
    if (r > 0) {
        Serial.write(print_buf);
    }
}


// --- HÀM CALLBACK ESP-NOW  ---
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    memcpy(&incomingData, data, sizeof(incomingData));
    newData = true; 
}


void setup(){
    // Hàm setup 
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
        
        // Phần in log và gửi dữ liệu cảm biến
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

        // <<< Gọi hàm AI sau khi có dữ liệu mới >>>
        run_ai_inference();

        Serial.println("-------------------------------------------");
    }

    if (app.ready()) {
        unsigned long currentTime = millis();
        if (currentTime - lastCheckTime >= checkInterval) {
            lastCheckTime = currentTime;
            
            // <<< THÊM MỚI: Gửi yêu cầu lấy trạng thái autoMode >>>
            Database.get(aClient, "/controls/autoMode", processData, false, "Get_AutoMode");
            
            // Phần lấy trạng thái đèn/quạt giữ nguyên
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
    if (aResult.isEvent())
        Firebase.printf("Event: %s, msg: %s\n", aResult.uid().c_str(), aResult.eventLog().message().c_str());
    
    // báo lỗi 
    if (aResult.isError())
       Firebase.printf("Error: %s, msg: %s\n", aResult.uid().c_str(), aResult.error().message().c_str());

    if (aResult.available()) {
        String uid = aResult.uid();
        String payload = aResult.c_str();
        bool state = (payload == "true");

        //  Xử lý kết quả cho autoMode 
        if (uid == "Get_AutoMode") {
            autoMode = state;
            return;
        }

        // Phần điều khiển đèn/quạt
        for (int i = 1; i <= numFans; i++) {
            if (uid == "Get_Fan" + String(i)) {
                digitalWrite(fanPins[i - 1], state);
                return;
            }
        }
        for (int i = 1; i <= numLights; i++) {
            if (uid == "Get_LED" + String(i)) {
                digitalWrite(lightPins[i - 1], state);
                return; 
            }
        }
        
    }
}

void run_ai_inference() {
    if (!autoMode) {
        Database.set<String>(aClient, "/ai_status/prediction", "Manual Mode", processData, "");
        return;
    }

    Serial.println("\n[AI] Analysing data...");
    
    signal_t signal;
    ei_impulse_result_t result = { 0 };
    float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
    
    features[0] = dataToProcess.temperature;
    features[1] = dataToProcess.humidity;
    features[2] = dataToProcess.light_intensity;
    features[3] = dataToProcess.motion_detected ? 1.0f : 0.0f;
    
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) { ei_printf("ERR: Failed to create signal from buffer (%d)\n", err); return; }

    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    if (res != EI_IMPULSE_OK) { ei_printf("ERR: Failed to run classifier (%d)\n", res); return; }

    ei_printf("Predictions (DSP: %d ms., Classification: %d ms.): \n",
            result.timing.dsp, result.timing.classification);
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
    
    // === LOGIC XỬ LÝ HÒA THÔNG MINH ===

    // 1. Tìm xác suất cao nhất
    float max_confidence = 0.0;
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > max_confidence) {
            max_confidence = result.classification[ix].value;
        }
    }

    // 2. Liệt kê tất cả các nhãn có cùng xác suất cao nhất (bị hòa)
    String tied_labels[EI_CLASSIFIER_LABEL_COUNT];
    int num_tied = 0;
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        // Sử dụng một sai số nhỏ để so sánh số thực
        if (result.classification[ix].value >= (max_confidence - 0.0001)) {
            tied_labels[num_tied++] = result.classification[ix].label;
        }
    }

    // 3. Áp dụng luật ưu tiên để chọn ra kết luận cuối cùng
    String final_prediction = tied_labels[0]; // Mặc định chọn cái đầu tiên
    // Ưu tiên cao nhất: Bật cả đèn và quạt
    for (int i = 0; i < num_tied; i++) {
        if (tied_labels[i] == "can_bat_den_va_quat") {
            final_prediction = "can_bat_den_va_quat";
            break;
        }
    }
    // Ưu tiên tiếp theo: Chỉ bật đèn (nếu không có can_bat_den_va_quat)
    if (final_prediction != "can_bat_den_va_quat") {
      for (int i = 0; i < num_tied; i++) {
        if (tied_labels[i] == "can_bat_den") {
            final_prediction = "can_bat_den";
            break;
        }
      }
    }

    ei_printf("[AI] Ket luan (da xu ly hoa): %s\n", final_prediction.c_str());

    // 4. Gửi kết quả và ra quyết định
    Database.set<String>(aClient, "/ai_status/prediction", final_prediction, processData, "");

    if (final_prediction == "can_bat_den") {
        Database.set<bool>(aClient, "/LED/LED1", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED2", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED3", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED4", true, processData, ""); 
        
    } else if (final_prediction == "can_bat_quat") {
        Database.set<bool>(aClient, "/LED/Fan1", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan2", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan3", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan4", true, processData, ""); 

    } else if (final_prediction == "can_bat_den_va_quat") {
        Database.set<bool>(aClient, "/LED/LED1", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED2", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED3", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/LED4", true, processData, "");
        Database.set<bool>(aClient, "/LED/Fan1", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan2", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan3", true, processData, ""); 
        Database.set<bool>(aClient, "/LED/Fan4", true, processData, "");
    } else if (final_prediction == "phong_trong") {
        for (int i = 1; i <= numLights; i++) Database.set<bool>(aClient, "/LED/LED" + String(i), false, processData, "");
        for (int i = 1; i <= numFans; i++) Database.set<bool>(aClient, "/LED/Fan" + String(i), false, processData, "");
    }
}