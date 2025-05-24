// Libraries
#include <DHT.h>

// Define sensor pin and type
#define DHTPIN 12     // Chân DATA của DHT22 nối với chân D12 trên Arduino
#define DHTTYPE DHT22 // Loại cảm biến là DHT22

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Variables
float hum;      // Lưu giá trị độ ẩm
float temp_c;   // Lưu giá trị nhiệt độ (Celsius)
float humidex;  // Lưu giá trị chỉ số humidex

// Timing for sending data
// Cảm biến DHT22 nên được đọc nhiều nhất mỗi 2 giây một lần (tần suất 0.5 Hz).
// Khi gửi dữ liệu ở tần suất này, bạn BẮT BUỘC phải khai báo với Edge Impulse data forwarder.
// Sử dụng lệnh: edge-impulse-data-forwarder --frequency 0.5
const unsigned long SENSOR_READ_INTERVAL_MS = 2000; // Đọc mỗi 2 giây (2000 ms)
static unsigned long last_read_time_ms = 0;         // Thời điểm đọc cuối cùng

void setup() {
  // Bắt đầu giao tiếp Serial ở baud rate cao hơn để truyền dữ liệu nhanh hơn
  Serial.begin(115200);
  Serial.println("DHT22 Sensor Data for Edge Impulse");
  Serial.println("Format: humidity,temperature_C,humidex"); // Thông báo định dạng dữ liệu

  dht.begin(); // Khởi tạo cảm biến DHT

  // Đợi một chút để cảm biến ổn định (tùy chọn)
  delay(1000);
}

void loop() {
  // Kiểm tra xem đã đến lúc đọc dữ liệu mới chưa
  if (millis() - last_read_time_ms >= SENSOR_READ_INTERVAL_MS) {
    last_read_time_ms = millis(); // Cập nhật thời điểm đọc cuối cùng

    // Đọc độ ẩm và nhiệt độ
    hum = dht.readHumidity();
    temp_c = dht.readTemperature(); // Mặc định đọc nhiệt độ ở Celsius

    // Kiểm tra xem có đọc lỗi không (thư viện DHT.h trả về NaN nếu lỗi)
    if (isnan(hum) || isnan(temp_c)) {
      // Serial.println("Failed to read from DHT sensor!"); // Tùy chọn: Bỏ comment để debug
      return; // Bỏ qua chu kỳ này nếu đọc lỗi
    }

    // Tính toán chỉ số humidex theo công thức của bạn
    // humidex = Nhiệt độ_C + (0.5555 * (e - 10))
    // với e = (0.06 * Độ ẩm tương đối_% * 10^(0.03 * Nhiệt độ_C))
    float e_component = 0.06 * hum * pow(10, (0.03 * temp_c));
    humidex = temp_c + (0.5555 * (e_component - 10.0));

    // Gửi dữ liệu dưới dạng các giá trị cách nhau bằng dấu phẩy, kết thúc bằng ký tự xuống dòng
    // Ví dụ: 52.5,23.8,27.3
    Serial.print(hum, 1);       // Độ ẩm, 1 chữ số thập phân
    Serial.print(",");
    Serial.print(temp_c, 1);    // Nhiệt độ, 1 chữ số thập phân
    Serial.print(",");
    Serial.println(humidex, 1); // Humidex, 1 chữ số thập phân, sau đó xuống dòng
  }
}