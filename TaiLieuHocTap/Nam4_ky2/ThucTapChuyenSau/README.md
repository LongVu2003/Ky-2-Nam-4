
# Báo cáo THCS.
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

![Image](https://github.com/user-attachments/assets/ac0ded53-5302-4c9a-b8b5-d6f79555611c)

- Cấu hình Tham số:
    - Number of channels: 1: Chỉ sử dụng một chiết áp làm đầu vào.
    - Resolution: 12-bit. : Tận dụng độ phân giải tối đa của ADC MAX 10.
    - Sample rate: 1 MSPS. : Đặt tốc độ lấy mẫu tối đa của ADC. Mặc dù việc đọc chiết áp không yêu cầu tốc độ cao như vậy, việc này cho thấy khả năng của ADC và có thể hữu ích cho các ứng dụng tương lai. Tốc độ này đạt được thông qua sự phối hợp giữa clock đầu vào của IP core và các bộ chia tần nội bộ của khối ADC cứng.
    - Input voltage range: 0 to 2.5V (internal reference). Sử dụng điện áp tham chiếu nội 2.5V của MAX 10 để đơn giản hóa thiết kế phần cứng bên ngoài, kết hợp với mạch chia áp đã thiết kế ở mục 322). Trong GUI của IP core, tùy chọn này thường là "Voltage reference: Internal".
    - Mode: Single-channel, continuous sampling. : Hệ thống chỉ đọc một kênh ADC liên tục để cập nhật giá trị điện áp.
    - Xuất cổng (Export Ports): Các cổng giao diện Avalon Streaming command và response cần được xuất ra để logic Verilog tùy chỉnh có thể giao tiếp với IP core.

![Image](https://github.com/user-attachments/assets/010bde0a-47cc-4448-a5ee-bcf85d7dfa6d)

- Nhấn Finish, chúng ta sẽ thấy nó xuất hiện trong tab System Contents.
- Ở dòng "command", hãy xuất cổng bằng cách nhấp đúp vào chỗ có dòng chữ "Double-click to export".
- Tương tự, ở dòng "response", cũng nhấp đúp vào dòng chữ "Double-click to export" để xuất cổng.
- Bây giờ, trong cột Export, chúng ta sẽ thấy các tên đã được gán.
### 2. Cấu hình IP Core "Avalon ALTPLL".
PLL (Phase-Locked Loop) là một bộ điều khiển xung nhịp trong FPGA, có chức năng tạo ra các tín hiệu xung nhịp có tần số và pha chính xác từ một tín hiệu xung nhịp tham chiếu. Nó được sử dụng để nhân tần số, chia tần số, hoặc tạo các tín hiệu xung nhịp đồng bộ với nhau.
Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Avalon ALTPLL" (Hình 2.4) và thêm vào hệ thống.

![Image](https://github.com/user-attachments/assets/29ffc6d9-15fc-466e-8111-046c2f6000d8)

Cấu hình Tham số: 
- Input clock frequency : 50.0 MHz. Đây là tần số của bộ dao động chính trên kit DE10-Lite.
- Output clock C0 frequency : 50.0 MHz. Dùng làm clock hệ thống. Đạt được bằng cách đặt clk0_multiply_by = 1, clk0_divide_by = 1
- Output clock C1 frequency : 2.0 MHz.Dùng làm clock cho IP Modular ADC, cụ thể là đầu vào adc_pll_clock_clk. Tần số này được chọn để đảm bảo hoạt động ổn định cho ADC và cho phép IP core cấu hình các bộ chia tần nội bộ để đạt được tốc độ lấy mẫu 1 MSPS. Đạt được bằng cách đặt clk1_multiply_by = 1, clk1_divide_by = 25 với đầu vào 50MHz.
- Các thông số về pha (phase shift) và chu kỳ hoạt động (duty cycle) thường được để mặc định (0 độ và 50%) cho các ứng dụng này, vì sự ổn định tần số là quan trọng nhất.

![Image](https://github.com/user-attachments/assets/2bfecdcd-d37e-492f-89bc-e37efa95ff14)

![Image](https://github.com/user-attachments/assets/3242a00b-339f-4caa-9f24-eb54a12d0e91)

![Image](https://github.com/user-attachments/assets/ee8d42ab-b07e-4edb-a1de-22421821792e)

### 3. Cấu hình IP Core "Reset Controller".

Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Reset Controller" và thêm hai thực thể vào hệ thống

![Image](https://github.com/user-attachments/assets/fd4a7045-aa69-4e9b-9bb4-3e0a868c0649)

Cấu hình Tham số :
Reset_controller_0:
- Đầu vào: Nhận clock hệ thống 50MHz (từ nguồn clock chính của bo mạch) và tín hiệu reset hệ thống.
- Đầu ra: Cung cấp tín hiệu reset đã được đồng bộ hóa cho khối ALTPLL.
- Thông số chính: Số đầu ra reset (Number of reset outputs), Clock đồng bộ hóa (Synchronization clock), Mức tích cực của reset (Active level).
Reset_controller_1:
- Đầu vào: Nhận clock 50MHz từ đầu ra C0 của ALTPLL và tín hiệu reset hệ thống.
- Đầu ra: Cung cấp tín hiệu reset đã được đồng bộ hóa cho IP Modular ADC (cổng reset_sink_reset_n) và có thể cho cả logic Verilog tùy chỉnh.
- Thông số chính: Tương tự như Reset_controller_0.

![Image](https://github.com/user-attachments/assets/50b8e453-7c38-4b07-b0a7-171ea8797988)

Nhấn Finish. Bây giờ bạn sẽ thấy nó trong tab System Contents.
Lặp lại ba bước cuối cùng để thêm một bộ điều khiển reset khác.

### 4. Cấu hình IP Core "Clock Bridge"
Tìm kiếm và Thêm IP: Trong IP Catalog, tìm kiếm "Clock Bridge" và thêm vào hệ thống.

![Image](https://github.com/user-attachments/assets/d04129c9-58e3-46d3-99a2-a3ae3224c017)

Cấu hình Tham số:
- Cấu hình đơn giản là xuất cổng out_clk.

### 5. 

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

![Image](https://github.com/user-attachments/assets/94281c38-a0e9-45f3-847a-0bfc16888c23)

## C. Kết nối phần cứng
Mục địch đo và kiểm thử

![Image](https://github.com/user-attachments/assets/56f286d7-24ba-45a2-90d2-97cc2b2d0aad)

  
