#include <esp_now.h>
#include <WiFi.h>

// Cấu trúc dữ liệu NHẬN từ Node Cảm Biến
// *** RẤT QUAN TRỌNG: Cấu trúc này phải GIỐNG HỆT với cấu trúc
//     bên phía Node Cảm Biến mà bạn đã code trước đó. ***
typedef struct struct_message {
    int id;                 // ID của Node
    float temperature;
    float humidity;
    float light_intensity;  // Sẽ là -999.0 nếu node không có cảm biến này
    bool motion_detected;
    float co2_ppm;          // Sẽ là -999.0 nếu node không có cảm biến này
    float tvoc_ppb;         // Sẽ là -999.0 nếu node không có cảm biến này
} struct_message;

// Tạo một biến struct để lưu dữ liệu vừa nhận được
struct_message incomingData;

// Hàm callback được gọi mỗi khi Gateway nhận được dữ liệu qua ESP-NOW
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    // Sao chép dữ liệu nhận được vào biến struct của chúng ta
    memcpy(&incomingData, data, sizeof(incomingData));

    // In ra màn hình Serial để debug và kiểm tra
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
    Serial.printf("  eCO2: %.0f ppm\n", incomingData.co2_ppm);
    Serial.printf("  TVOC: %.0f ppb\n", incomingData.tvoc_ppb);
    Serial.println("-------------------------------------------");
}

void setup() {
    // Bắt đầu giao tiếp Serial ở baud rate 115200
    Serial.begin(115200);
    Serial.println("Khoi dong ESP32 Gateway (Che do Nhan ESP-NOW)...");

    // Đặt thiết bị ở chế độ Wi-Fi Station
    // Đây là yêu cầu để ESP-NOW có thể hoạt động
    WiFi.mode(WIFI_STA);

    // In địa chỉ MAC của Gateway ra màn hình
    // >>> BẠN SẼ DÙNG ĐỊA CHỈ NÀY ĐỂ ĐIỀN VÀO BIẾN `gatewayAddress` TRONG CODE CỦA CÁC NODE CẢM BIẾN <<<
    Serial.print("Dia chi MAC cua Gateway: ");
    Serial.println(WiFi.macAddress());

    // Khởi tạo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Loi khoi tao ESP-NOW");
        return;
    }

    // Sau khi ESP-NOW được khởi tạo thành công, chúng ta đăng ký hàm callback nhận dữ liệu
    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("Gateway da san sang nhan du lieu tu cac Node.");
}

void loop() {
    // Loop có thể để trống, vì toàn bộ việc nhận và xử lý dữ liệu
    // đã được thực hiện trong hàm callback `OnDataRecv` một cách tự động.
    // Việc này giúp tiết kiệm năng lượng hơn so với việc liên tục kiểm tra trong loop.
    delay(2000); // Có thể thêm một delay nhỏ để CPU nghỉ ngơi
}