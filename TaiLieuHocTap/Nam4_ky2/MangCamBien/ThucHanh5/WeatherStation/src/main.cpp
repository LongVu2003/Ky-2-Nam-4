#include <DHT.h>
#include <DHT11_ESP32_inferencing.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Buffer để chứa dữ liệu cảm biến (temperature, humidity)
float features[EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME];

// Hàm callback để lấy dữ liệu
int get_data(size_t offset, size_t length, float *out_ptr) {
  for (size_t i = 0; i < length; i++) {
    if (offset + i < EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME) {
      out_ptr[i] = features[offset + i];
    } else {
      out_ptr[i] = 0.0; // Điền giá trị mặc định nếu vượt quá phạm vi
    }
  }
  return length;
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Kiểm tra cấu hình của Edge Impulse
  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 2) { // 2 kênh: temperature, humidity
    Serial.println("Error: Number of features does not match!");
    while (1);
  }
}

void loop() {
  delay(2000); // Đọc mỗi 2 giây (khớp với Sample length 2000 ms)

  // Đọc dữ liệu từ DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Điền dữ liệu vào buffer (theo thứ tự: temperature, humidity)
  features[0] = t; // Temperature
  features[1] = h; // Humidity

  // Chuẩn bị tín hiệu cho suy luận
  ei_impulse_result_t result = {0};
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME;
  signal.get_data = get_data;

  // Chạy suy luận
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result);
  if (res != EI_IMPULSE_OK) {
    Serial.printf("Failed to run classifier: %d\n", res);
    return;
  }

  // In kết quả dự đoán
  Serial.printf("Prediction: ");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.printf("%s: %.2f ", result.classification[ix].label, result.classification[ix].value);
  }
  Serial.println();

  // (Tùy chọn) In giá trị cảm biến để kiểm tra
  Serial.printf("Temperature: %.2f°C, Humidity: %.2f%%\n", t, h);
}