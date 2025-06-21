#include <Arduino.h>
#include "FS.h"
#include "LittleFS.h"

// Đường dẫn đến file dữ liệu trong bộ nhớ của ESP32
const char* source_file_path = "/data.csv";
const char* dest_file_path = "/copy_data.csv";

void setup() {
    Serial.begin(115200);
    
    // Khởi tạo hệ thống file LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Lỗi khi mở hệ thống file LittleFS");
        return;
    }

    // Mở file gốc để đọc
    File sourceFile = LittleFS.open(source_file_path, FILE_READ);
    if (!sourceFile) {
        Serial.println("Không tìm thấy file data.csv để đọc.");
        return;
    }

    // Mở hoặc tạo file đích để ghi
    File destFile = LittleFS.open(dest_file_path, FILE_WRITE);
    if (!destFile) {
        Serial.println("Không thể tạo file copy_data.csv để ghi.");
        sourceFile.close();
        return;
    }

    Serial.println("\n--- Sao chép dữ liệu từ data.csv sang copy_data.csv ---");

    // Đọc và ghi từng byte
    while (sourceFile.available()) {
        char c = sourceFile.read();
        destFile.write(c);          // Ghi ra file
        Serial.write(c);            // In ra Serial (nếu cần)
    }

    sourceFile.close();
    destFile.close();

    Serial.println("\n--- Đã sao chép xong file ---");
}

void loop() {
    // Không cần làm gì trong loop
}
