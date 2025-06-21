import csv
import json
import os
import hashlib
import hmac

# Khóa HMAC trống, vì dữ liệu này không cần bảo mật đặc biệt
EMPTY_HMAC_KEY = "00000000000000000000000000000000"

# Tên file CSV đầu vào
input_csv_file = 'testing_set_node3.csv'
output_folder_name = 'testing_output'


# Đọc file CSV
try:
    with open(input_csv_file, mode='r', newline='') as infile:
        reader = csv.reader(infile)
        header = next(reader)  # Bỏ qua dòng tiêu đề

        # Lặp qua từng dòng dữ liệu trong file CSV
        for i, row in enumerate(reader):
            if len(row) < 5:
                print(f"Bo qua dong {i+2} vi thieu du lieu.")
                continue

            temperature, humidity, light_intensity, motion, label = row

            # <<< SỬA LỖI CUỐI CÙNG THEO ĐÚNG ĐỊNH DẠNG BẠN CUNG CẤP >>>
            # "values" phải là một mảng chứa các mảng con.
            # Mỗi mảng con là một lần lấy mẫu của TẤT CẢ các cảm biến.
            payload = {
                "device_name": "ESP32_Gateway",
                "device_type": "ESP32_Classifier",
                "interval_ms": 1000, # Giả định khoảng thời gian giữa các lần đọc là 1s
                "sensors": [
                    {"name": "temperature", "units": "Cel"},
                    {"name": "humidity", "units": "%"},
                    {"name": "light_intensity", "units": "lx"},
                    {"name": "motion", "units": "0/1"}
                ],
                "values": [
                    [ # Một mảng con đại diện cho một lần lấy mẫu
                        float(temperature),
                        float(humidity),
                        float(light_intensity),
                        int(motion)
                    ]
                ]
            }
            
            # Chuyển payload thành chuỗi JSON không có khoảng trắng
            encoded = json.dumps(payload, separators=(',', ':'))

            # Tạo chữ ký HMAC
            signature = hmac.new(
                bytes(EMPTY_HMAC_KEY, 'utf-8'),
                msg = encoded.encode('utf-8'),
                digestmod = hashlib.sha256
            ).hexdigest()

            # Tạo đối tượng JSON cuối cùng
            final_json = {
                "protected": {
                    "ver": "v1",
                    "alg": "HS256",
                    "iat": 0 
                },
                "signature": signature,
                "payload": payload
            }
            
            # Tạo thư mục con cho từng nhãn nếu chưa có
            label_dir = os.path.join('training_data', label.strip())
            if not os.path.exists(label_dir):
                os.makedirs(label_dir)
                
            # Ghi file JSON
            output_filename = os.path.join(label_dir, f"{label.strip()}.{i}.json")
            with open(output_filename, 'w') as outfile:
                json.dump(final_json, outfile, indent=4)
                
        print(f"Chuyen doi hoan tat! Du lieu da duoc luu trong thu muc 'training_data'.")

except FileNotFoundError:
    print(f"LOI: Khong tim thay file '{input_csv_file}'. Hay dam bao ban da tao file nay.")
except StopIteration:
    print(f"LOI: File '{input_csv_file}' bi trong. Hay dan du lieu vao file.")