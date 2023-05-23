#include <mcs51/8052.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define display_port P2    // Định nghĩa chân dữ liệu ra cho LCD
#define DQ           P3_7  // Chân dữ liệu của DS18B20

// Mảng lưu trữ giá trị nhiệt độ trong khoảng thời gian 1 phút theo dõi trước đó
// tính từ index = 1 -> 10
int tempArr[11] = {0};
int currentTempIndex = 1;  // Chỉ ra index của nhiệt độ hiện tại
unsigned char temp;  // lưu trữ giá trị nhiệt độ đọc từ cảm biến
int isFahrenheit = 1;  // 0: hiển thị độ C - 1 : hiển thị độ F

// Tạo độ trễ chính xác (ms) sử dụng Timer 0
void delay_ms(long ms) {
    ms *= 1000;  // Tính số chu kì máy (1000ckm = 1ms)
    while (ms > 0) {
        // Xóa thiết lập cũ của Timer0
        TMOD = TMOD & 0xF0;
        // Thiết lập Mode 1, 16-bit Timer/Counter
        TMOD = TMOD | 0x01;
        ET0 = 0;  // Che ngắt Timer 0
        // Kiểm tra xem số ckm còn lại có lớn hơn 65536
        if (ms >= 65536) {
            // Nếu lớn hơn thiết lập giá trị thanh ghi về 0
            TH0 = 0;
            TL0 = 0;
            ms -= 65536;
        } else {
            // Nếu nhỏ hơn thiết lập giá trị thanh ghi về 65536 - ms
            ms = 65536 - ms;
            TH0 = ms / 256;
            TL0 = ms % 256;
            ms = 0;
        }
        TF0 = 0;  // Xóa cờ tràn Timer 0
        TR0 = 1;  // Khởi động Timer 0
        while (TF0 == 0)
            ;     // Chờ đến khi tràn số
        TR0 = 0;  // Dừng Timer 0
    }
}
//---------------------------------------------------------------------
// Các hàm làm việc với ngắt ngoài

// Khởi tạo ngắt ngoài 0
void initilize_int0() {
    P3_2 = 1;  // P3_2 là input
    EX0 = 1;   // Cho phép ngắt ngoài 0
    IT0 = 1;   // Ngắt theo sườn xuống
    EA = 1;    // Cho phép ngắt toàn cục
}

// Xử lý ngắt ngoài 0
void external0_interrupt() __interrupt(0) {
    EA = 0;  // Che toàn bộ các ngắt
    // Thực hiện chuyển đổi hiển thị qua lại giữa độ C và F
    isFahrenheit = 1 - isFahrenheit;
    EA = 1;  // Cho phép ngắt toàn cục
}
//---------------------------------------------------------------------
// Các hàm làm việc với LCD

__bit __at(0x87) rs; /* I/O port 0, bit 7 */
__bit __at(0x86) rw; /* I/O port 0, bit 6 */
__bit __at(0x85) en; /* I/O port 0, bit 5 */

// Hàm đợi chi LCD
void Wait_For_LCD() { delay_ms(1); }

// Gửi lệnh tới LCD (mã lệnh trong bảng lệnh LCD1602)
void lcd_cmd(unsigned char command) {
    display_port = command;
    rs = 0;  // Chế độ gửi lệnh
    rw = 0;  // Chế độ ghi
    // Tạo sườn xuống để ghi lệnh
    en = 1;
    delay_ms(1);
    en = 0;
    // Đợi LCD chuẩn bị xong
    Wait_For_LCD();
    en = 1;
}

// Gửi dữ liệu tới LCD (mã ACSII)
void lcd_bytea(unsigned char disp_bytea) {
    display_port = disp_bytea;
    rs = 1;  // Chế độ gửi dữ liệu
    rw = 0;  // Che do ghi
    // Tạo sườn xuống để ghi dữ liệu
    en = 1;
    delay_ms(1);
    en = 0;
    // Đợi LCD chuẩn bị xong
    Wait_For_LCD();
    en = 1;
}

// Khởi tạo LCD
void lcd_init() {
    P0 = 0xFF;
    lcd_cmd(0x38);  // Sử dụng 2 dòng và ma trận 5x7 cho mỗi ký tự trên LCD
    lcd_cmd(0x0C);  // Tắt con trỏ
    lcd_cmd(0x01);  // Xóa màn hình LCD
    lcd_cmd(0x80);  // Đặt con trỏ trở về đầu dòng 1
}

// Hiển thị chuỗi ký tự trên LCD
// a: chuỗi muốn hiển thị trên LCD
// line: dòng muốn hiển thị ký tự (line = 1 hoặc 2);
// conti: tùy chọn hiển thị ký tự tại vị trí con trỏ đang đứng
// hoặc trở về đầu dòng (conti = 0 hoặc 1)
void display_str_lcd1602(unsigned char a[], int line, int conti) {
    int i = 0;
    // Hiển thị từng ký tự trong chuỗi a
    while (a[i] != '\0') {
        // Kiểm tra xem có cần đặt lại vị trí con trỏ hay không
        if (i == 0 && conti == 0) {
            if (line == 1)
                lcd_cmd(0x80);  // Đặt con trỏ trở về đầu dòng 1
            else if (line == 2)
                lcd_cmd(0xC0);  // Đặt con trỏ trở về đầu dòng 2
            else {
                lcd_cmd(0x01);  // Xóa màn hình LCD
                lcd_cmd(0x80);  // Đặt con trỏ trở về đầu dòng 1
            }
        }
        lcd_bytea(a[i]);  // Gửi ký tự ra màn hình LCD
        i++;
    }
}

// Hiển thị 1 ký tự trên LCD
// a: ký tự muốn hiển thị trên LCD
// line: dòng muốn hiển thị ký tự (line = 1 hoặc 2);
// conti: tùy chọn hiển thị ký tự tại vị trí con trỏ đang đứng
// hoặc trở về đầu dòng (conti = 0 hoặc 1)
void display_char_lcd1602(unsigned char a, int line, int conti) {
    // Kiểm tra xem có cần đặt lại vị trí con trỏ hay không
    if (conti == 0) {
        if (line == 1)
            lcd_cmd(0x80);  // Đặt con trỏ trở về đầu dòng 1
        else if (line == 2)
            lcd_cmd(0xC0);  // Đặt con trỏ trở về đầu dòng 2
        else {
            lcd_cmd(0x01);  // Xóa màn hình LCD
            lcd_cmd(0x80);  // Đặt con trỏ trở về đầu dòng 1
        }
    }
    lcd_bytea(a);
}
//---------------------------------------------------------------------
// Các hàm làm việc với cảm biến nhiệt độ DS18B20

// Hàm trễ dành cho DS18B20 (không quá chính xác)
void delay_us_DS18B20(unsigned int time) {
    // Trễ us
    while (time--)
        ;
}

// Khởi tạo cảm biến
void Init_DS18B20(void) {
    DQ = 1;
    delay_us_DS18B20(8);
    // Đặt lại xung trong một khoảng thời gian tối thiểu.
    DQ = 0;
    delay_us_DS18B20(65);
    DQ = 1;                // Cạnh tăng
    delay_us_DS18B20(20);  // Đợi DS18B20 phản hồi
}

// Đọc 1 byte từ Scratchpad
unsigned char ReadByteFromScratchpad() {
    unsigned char i = 0;
    unsigned char byte = 0;
    for (i = 8; i > 0; i--) {
        DQ = 0;
        byte >>= 1;
        DQ = 1;                // The master bus releases the 1-Wire bus
        if (DQ) byte |= 0x80;  // Read 1? Otherwise, Read 0
        delay_us_DS18B20(4);
    }
    return (byte);
}

// Ghi 1 byte vào Scratchpad
void WriteByteToScratchpad(unsigned char byte) {
    unsigned char i = 0;
    for (i = 8; i > 0; i--) {
        DQ = 0;
        DQ = byte & 0x01;
        delay_us_DS18B20(5);
        DQ = 1;  // The master bus releases the 1-Wire bus
        byte >>= 1;
    }
}

void ReadTemperature(void) {  // Hàm đọc nhiệt độ lưu vào biến temp
    EA = 0;  // Nếu có ngắt thì cấm ngắt để tránh ảnh hưởng quá trình giao tiếp
    unsigned char Byte0 = 0;      // lưu trữ byte 0 của Scratchpad
    unsigned char Byte1 = 0;      // lưu trữ byte 1 của Scratchpad
    Init_DS18B20();               // Khởi tạo cảm biến DS18B20
    WriteByteToScratchpad(0xCC);  // The master issues Skip ROM [CCh] command
    WriteByteToScratchpad(0x44);  // Convert T [44h] command. To initiate a temp
                                  // measurement and A-to-D conversion.
    delay_us_DS18B20(10);
    Init_DS18B20();
    WriteByteToScratchpad(0xCC);  // The master issues Skip ROM [CCh] command
    WriteByteToScratchpad(0xBE);  // Read temp value.
                                  // Read Scratpad [BEh] command.
    delay_us_DS18B20(10);
    Byte0 = ReadByteFromScratchpad();  // Read Byte0 of the Scratchpad (low byte
                                       // of the temp value)
    Byte1 = ReadByteFromScratchpad();  // Read Byte1 of the Scratchpad (high
                                       // byte of the temp value)
    // Calculate the temp value from Byte0 & Byte1 and save it to temp variable
    temp = ((Byte1 * 256 + Byte0) >> 4);

    EA = 1;  // Đọc xong thì cho phép ngắt
}

//---------------------------------------------------------------------
// Các hàm hỗ trợ chuyển đổi

// Chuyển đổi độ C sang độ F
int celsius_to_fahrenheit(float celsius) {
    float fahrenheit = (celsius * 9 / 5) + 32;
    if (((int) (fahrenheit * 10) % 10) >= 5) fahrenheit += 1;
    return fahrenheit;
}

// Chuyển số nguyên thành chuỗi
void convert(int n, char *str) {
    if (isFahrenheit) n = celsius_to_fahrenheit(n);
    sprintf(str, "%d", n);
}

//---------------------------------------------------------------------
void main() {
    lcd_init();        // Khởi tạo LCD
    initilize_int0();  // Khởi tạo ngắt ngoài 0

    // Đọc giá trị lần đầu từ cảm biến và không lấy giá trị
    ReadTemperature();
    delay_ms(1000);

    // Đọc lần 2 và lưu giá trị vào cả 10 phần từ của mảng tempArr
    ReadTemperature();  // Đọc nhiệt độ từ DS18B20
    for (int i = 0; i < 10; i++) {
        currentTempIndex = (currentTempIndex + 1) % 11;
        // Lưu nhiệt độ vào mảng tempArr
        if (temp <= 128)
            tempArr[currentTempIndex] = temp;
        else
            tempArr[currentTempIndex] = temp - 256;
    }

    while (1) {
        currentTempIndex = (currentTempIndex + 1) % 11;
        ReadTemperature();  // Đọc nhiệt độ từ DS18B20
        // Lưu nhiệt độ vào mảng tempArr
        if (temp <= 128)
            tempArr[currentTempIndex] = temp;
        else
            tempArr[currentTempIndex] = temp - 256;

        char str[10];
        // Tính giá trị max và min của nhiệt độ trong 1 phút theo dõi trước đó
        int max = -1000, min = 1000, i;
        for (i = 1; i <= 10; i++) {
            if (tempArr[i] < min) min = tempArr[i];
            if (tempArr[i] > max) max = tempArr[i];
        }

        // Hiển thị dòng 1 của LCD
        display_str_lcd1602("Cur Temp: ", 1, 0);
        // Hiển thị nhiệt độ hiện tại
        convert(tempArr[currentTempIndex], str);
        display_str_lcd1602(str, 2, 1);

        display_char_lcd1602(0xDF, 1, 1);  // Hiển thị ký tự độ
        if (isFahrenheit)
            display_char_lcd1602('F', 1, 1);
        else
            display_char_lcd1602('C', 1, 1);
        display_str_lcd1602("      ", 1, 1);  // Xóa các ký tự thừa ở cuối dòng

        // Hiển thị dòng 2 của LCD
        display_str_lcd1602("L: ", 2, 0);

        // Hiển thị nhiệt độ nhỏ nhất
        convert(min, str);
        display_str_lcd1602(str, 2, 1);
        display_str_lcd1602(" H: ", 2, 1);

        // Hiển thị nhiệt độ lớn nhất
        convert(max, str);
        display_str_lcd1602(str, 2, 1);
        display_str_lcd1602("       ", 2, 1);  // Xóa các ký tự thừa ở cuối dòng
        delay_ms(6000);                        // for every 6s.
    }
}