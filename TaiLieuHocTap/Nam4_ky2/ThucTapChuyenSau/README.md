
# Báo cáo THCS.
# Thông tin nhóm sinh viên thực hiện

1. Đoàn Long Vũ - B21DCDT247
2. Nguyễn Sỹ Toản - B21DCDT223

# DE10-ADC
Kho lưu trữ này chứa các tệp mô tả phần cứng và hướng dẫn cấu hình bộ chuyển đổi tương tự số (ADC) tích hợp trên các FPGA dòng DE để đạt tốc độ lấy mẫu 2MHz.

Hướng dẫn này sử dụng Quartus Prime 17.1 và công cụ IP Qsys, nhưng cũng có thể được hỗ trợ bởi Platform Designer (trước đây là Qsys) và các phiên bản Quartus mới hơn.

Thông tin cấu hình FPGA

Bo mạch được sử dụng cho cấu hình này là DE10-Lite (10M50DAF484C7G), có hai FPGA tích hợp, nhưng chỉ sử dụng ADC0. Vui lòng tham khảo sổ tay người dùng và hướng dẫn cấu hình để biết thông tin về gán chân (pin assignment) và cấu hình ADC nếu bạn sử dụng bo mạch khác.

Dự án này sử dụng SW[2:0] để chọn đầu vào analog mà ADC sẽ lấy mẫu, và kết quả lựa chọn được hiển thị trên HEX5.

Giá trị 12 bit từ ADC được hiển thị dưới dạng hexadecimal (thập lục phân) trên các màn hình HEX3, HEX2 và HEX1.

Được tạo bởi sinh viên PTIT
(Cập nhật ngày 14/06/2025)

## A. Sơ đồ khối hệ thống 

![Image](https://github.com/user-attachments/assets/d5901517-7573-41d7-8c2f-808a10676ae8)

## B. Thiết kế IP ADC trên nền tảng Platfrom Design.
### 1. Cấu hình IP Core "Altera Modular ADC".
IP Core "Altera Modular ADC" cung cấp giao diện để điều khiển và thu thập dữ liệu từ khối ADC cứng tích hợp trong chip MAX 10.
- Tìm kiếm và Thêm IP: Trong IP Catalog của Platform Designer, tìm kiếm "Altera Modular ADC core"

![Image](https://github.com/user-attachments/assets/b3e4cf02-76c0-41f4-a238-c3b0853467ab)

- Cấu hình Tham số:
    - Number of channels: 1: Chỉ sử dụng một chiết áp làm đầu vào.
    - Resolution: 12-bit. : Tận dụng độ phân giải tối đa của ADC MAX 10.
    - Sample rate: 1 MSPS. : Đặt tốc độ lấy mẫu tối đa của ADC. Mặc dù việc đọc chiết áp không yêu cầu tốc độ cao như vậy, việc này cho thấy khả năng của ADC và có thể hữu ích cho các ứng dụng tương lai. Tốc độ này đạt được thông qua sự phối hợp giữa clock đầu vào của IP core và các bộ chia tần nội bộ của khối ADC cứng.
    - Input voltage range: 0 to 2.5V (internal reference). Sử dụng điện áp tham chiếu nội 2.5V của MAX 10 để đơn giản hóa thiết kế phần cứng bên ngoài, kết hợp với mạch chia áp đã thiết kế ở mục 322). Trong GUI của IP core, tùy chọn này thường là "Voltage reference: Internal".
    - Mode: Single-channel, continuous sampling. : Hệ thống chỉ đọc một kênh ADC liên tục để cập nhật giá trị điện áp.
    - Xuất cổng (Export Ports): Các cổng giao diện Avalon Streaming command và response cần được xuất ra để logic Verilog tùy chỉnh có thể giao tiếp với IP core.

![Image](https://github.com/user-attachments/assets/e34aa9a5-094f-4044-be86-2828f2ed06a7)

- Nhấn Finish, chúng ta sẽ thấy nó xuất hiện trong tab System Contents.
- Ở dòng "command", hãy xuất cổng bằng cách nhấp đúp vào chỗ có dòng chữ "Double-click to export".
- Tương tự, ở dòng "response", cũng nhấp đúp vào dòng chữ "Double-click to export" để xuất cổng.
- Bây giờ, trong cột Export, chúng ta sẽ thấy các tên đã được gán.
### 2. Cấu hình IP Core "Avalon ALTPLL".
PLL (Phase-Locked Loop) là một bộ điều khiển xung nhịp trong FPGA, có chức năng tạo ra các tín hiệu xung nhịp có tần số và pha chính xác từ một tín hiệu xung nhịp tham chiếu. Nó được sử dụng để nhân tần số, chia tần số, hoặc tạo các tín hiệu xung nhịp đồng bộ với nhau.
Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Avalon ALTPLL" (Hình 2.4) và thêm vào hệ thống.

![Image](https://github.com/user-attachments/assets/98e0ac99-d966-4f97-96ec-3eb68b187cd3)

Cấu hình Tham số: 
- Input clock frequency : 50.0 MHz. Đây là tần số của bộ dao động chính trên kit DE10-Lite.
- Output clock C0 frequency : 50.0 MHz. Dùng làm clock hệ thống. Đạt được bằng cách đặt clk0_multiply_by = 1, clk0_divide_by = 1
- Output clock C1 frequency : 2.0 MHz.Dùng làm clock cho IP Modular ADC, cụ thể là đầu vào adc_pll_clock_clk. Tần số này được chọn để đảm bảo hoạt động ổn định cho ADC và cho phép IP core cấu hình các bộ chia tần nội bộ để đạt được tốc độ lấy mẫu 1 MSPS. Đạt được bằng cách đặt clk1_multiply_by = 1, clk1_divide_by = 25 với đầu vào 50MHz.
- Các thông số về pha (phase shift) và chu kỳ hoạt động (duty cycle) thường được để mặc định (0 độ và 50%) cho các ứng dụng này, vì sự ổn định tần số là quan trọng nhất.

![Image](https://github.com/user-attachments/assets/1647f3b1-6691-449d-8304-e6a5ad2295c0)

![Image](https://github.com/user-attachments/assets/dc94b476-c0cc-4cf8-81cd-079ae25b6dc9)

![Image](https://github.com/user-attachments/assets/9050225d-628f-4f75-aabc-25b8a94a0e87)

### 3. Cấu hình IP Core "Reset Controller".

Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Reset Controller" và thêm hai thực thể vào hệ thống

![Image](https://github.com/user-attachments/assets/dac2d8dc-4c6f-4f52-b527-2f12e84f397e)

Cấu hình Tham số :
Reset_controller_0:
- Đầu vào: Nhận clock hệ thống 50MHz (từ nguồn clock chính của bo mạch) và tín hiệu reset hệ thống.
- Đầu ra: Cung cấp tín hiệu reset đã được đồng bộ hóa cho khối ALTPLL.
- Thông số chính: Số đầu ra reset (Number of reset outputs), Clock đồng bộ hóa (Synchronization clock), Mức tích cực của reset (Active level).
Reset_controller_1:
- Đầu vào: Nhận clock 50MHz từ đầu ra C0 của ALTPLL và tín hiệu reset hệ thống.
- Đầu ra: Cung cấp tín hiệu reset đã được đồng bộ hóa cho IP Modular ADC (cổng reset_sink_reset_n) và có thể cho cả logic Verilog tùy chỉnh.
- Thông số chính: Tương tự như Reset_controller_0.

![Image](https://github.com/user-attachments/assets/4d3b7c93-66fb-4c12-a3fb-0f8e97a427db)

Nhấn Finish. Bây giờ bạn sẽ thấy nó trong tab System Contents.
Lặp lại ba bước cuối cùng để thêm một bộ điều khiển reset khác.

### 4. Cấu hình IP Core "Clock Bridge"
Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Clock Bridge" và thêm vào hệ thống.

![Image](https://github.com/user-attachments/assets/cbce5250-bac2-4d47-8554-a3f23e426ec5)

Cấu hình Tham số:
- Cấu hình đơn giản là xuất cổng out_clk.

### 5. Kết nối các IP Core trong Platform Designer (Qsys).

- Clock:
  - Nguồn clock 50MHz chính của bo mạch vào đầu vào của ALTPLL và Reset_controller_0.
  - Đầu ra C0 (50MHz) của ALTPLL vào Reset_controller_1 và Clock_Bridge (để xuất ra logic người dùng).
  - Đầu ra C1 (2MHz) của ALTPLL vào đầu vào adc_pll_clock_clk của Modular_ADC.
  - Tín hiệu locked từ ALTPLL vào đầu vào adc_pll_locked_export của Modular_ADC.
- Reset:
  - Đầu ra của Reset_controller_0 vào đầu vào reset của ALTPLL.
  - Đầu ra của Reset_controller_1 vào đầu vào reset_sink_reset_n của Modular_ADC.
- Dữ liệu ADC:
  - Các cổng command (channel, valid, startofpacket, endofpacket) của Modular_ADC được xuất ra để kết nối với logic Verilog.
  - Các cổng response (channel, data, valid) của Modular_ADC được xuất ra để kết nối với logic Verilog.

![Image](https://github.com/user-attachments/assets/4249eb2a-cd99-42cb-b601-0e2a508a1726)

## C. Kết nối phần cứng
Mục địch đo và kiểm thử

![Image](https://github.com/user-attachments/assets/176c2273-0077-476d-9d72-f889ce649987)


  
