import serial
import json
import os
import hashlib
import hmac
import time

# === CẤU HÌNH ===
# Thay đổi cổng COM cho đúng với ESP32 của bạn
SERIAL_PORT = 'COM4' 
BAUD_RATE = 115200
OUTPUT_FOLDER = 'collected_ei_data'
# =================

EMPTY_HMAC_KEY = "00000000000000000000000000000000"

if not os.path.exists(OUTPUT_FOLDER):
    os.makedirs(OUTPUT_FOLDER)

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f"Da ket noi voi ESP32 tai cong {SERIAL_PORT}")
except serial.SerialException as e:
    print(f"LOI: Khong the mo cong {SERIAL_PORT}. Error: {e}")
    exit()

def create_json_file(values, label, index):
    # ... (Hàm tạo JSON giữ nguyên như script trước)
    payload = { "device_name": "ESP32_Live", "device_type": "ESP32_Classifier", "interval_ms": 1000, "sensors": [{"name": "temperature", "units": "Cel"}, {"name": "humidity", "units": "%"}, {"name": "light_intensity", "units": "lx"}, {"name": "motion", "units": "0/1"}], "values": [values] }
    encoded = json.dumps(payload, separators=(',', ':'))
    signature = hmac.new(bytes(EMPTY_HMAC_KEY, 'utf-8'), msg=encoded.encode('utf-8'), digestmod=hashlib.sha256).hexdigest()
    final_json = { "protected": {"ver": "v1", "alg": "HS256", "iat": int(time.time())}, "signature": signature, "payload": payload }
    label_dir = os.path.join(OUTPUT_FOLDER, label)
    if not os.path.exists(label_dir): os.makedirs(label_dir)
    filename = os.path.join(label_dir, f"{label}.{int(time.time() * 1000)}.json")
    with open(filename, 'w') as f: json.dump(final_json, f)
    print(f"  -> Da luu file: {filename}")

# --- Vòng lặp chính để thu thập ---
while True:
    print("\n----------------------------------------------------")
    current_label = input("Nhap NHAN (label) ban muon thu thap (hoac go 'exit' de thoat): ")
    if current_label.lower() == 'exit':
        break
        
    while True:
        try:
            num_samples = int(input(f"Ban muon thu thap bao nhieu MAU cho nhan '{current_label}'? "))
            if num_samples > 0: break
        except ValueError:
            print("Vui long nhap mot con so.")

    input("Nhan Enter de bat dau thu thap...")
    
    ser.flushInput() # Xóa buffer cũ
    
    print(f"Dang thu thap {num_samples} mau cho nhan '{current_label}'. Vui long doi...")
    
    samples_collected = 0
    while samples_collected < num_samples:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                parts = line.split(',')
                if len(parts) == 4:
                    values = [float(p) for p in parts]
                    create_json_file(values, current_label, samples_collected)
                    samples_collected += 1
                else:
                    print(f"Bo qua du lieu khong hop le: {line}")
        except Exception as e:
            print(f"Co loi xay ra: {e}")
            
    print(f"HOAN TAT thu thap {num_samples} mau cho nhan '{current_label}'.")

ser.close()
print("Da dong ket noi Serial. Thoat chuong trinh.")