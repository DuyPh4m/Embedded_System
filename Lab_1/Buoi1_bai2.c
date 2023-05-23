#include <mcs51/8052.h>

#define led P1
int i, j, k;
int effect_index = 1; // 3 hiệu ứng tương ứng với 1, 2 và 3
int delayms = 500;         // Thời gian trễ giữa các lần sáng led

// Tạo độ trễ chính xác (ms) sử dụng Timer 0
void delay_ms(long ms)
{
    ms *= 1000; // Tính số chu kì máy (1000ckm = 1ms)
    while (ms > 0)
    {
        // Xóa thiết lập cũ của Timer0
        TMOD = TMOD & 0xF0;
        // Thiết lập Mode 1, 16-bit Timer/Counter
        TMOD = TMOD | 0x01;
        ET0 = 0; // Che ngắt Timer 0
        // Kiểm tra xem số ckm còn lại có lớn hơn 65536
        if (ms >= 65536)
        {
            // Nếu lớn hơn thiết lập giá trị thanh ghi về 0
            TH0 = 0;
            TL0 = 0;
            ms -= 65536;
        }
        else
        {
            // Nếu nhỏ hơn thiết lập giá trị thanh ghi về 65536 - ms
            ms = 65536 - ms;
            TH0 = ms / 256;
            TL0 = ms % 256;
            ms = 0;
        }
        TF0 = 0; // Xóa cờ tràn Timer 0
        TR0 = 1; // Khởi động Timer 0
        while (TF0 == 0)
            ;    // Chờ đến khi tràn số
        TR0 = 0; // Dừng Timer 0
    }
}

// Hiệu ứng 1
void effect_1()
{
    led = 0xFF; // Tắt tất cả led
    delay_ms(delayms);

    unsigned char effect_1 = 0x03; // Cho 2 led đầu sáng
    // Cho 2 led sáng chạy từ đầu đến cuối
    for (j = 0; j < 7; j++)
    {
        // Kiểm tra xem hiệu ứng đã thay đổi hay chưa
        if (effect_index != 1)
            return;

        led = ~effect_1;
        delay_ms(delayms);
        effect_1 = effect_1 << 1;
    }

    // Cho 2 led sáng chạy từ cuói đến đầu
    effect_1 = 0xC0; // Cho 2 led cuối sáng
    for (j = 0; j < 7; j++)
    {
        // Kiểm tra xem hiệu ứng đã thay đổi hay chưa
        if (effect_index != 1)
            return;

        led = ~effect_1;
        delay_ms(delayms);
        effect_1 = effect_1 >> 1;
    }
}

// Hiệu ứng 2
void effect_2()
{
    led = 0xFF; // Tắt tất cả led
    delay_ms(delayms);

    // effect_stt lưu trữ trạng thái sáng của các led đã được dịch đến cuối
    unsigned char effect_stt = 0x00;
    // countled là số vòng lặp để dịch led (giảm dần)
    int countled = 8;

    // Hiệu ứng sáng dần
    for (j = 0; j < 8; j++)
    {
        unsigned char effect_2 = 0x01; // bật sáng led đầu tiên
        // Bắt đầu vòng lặp dịch led đưa led về cuối
        for (k = 0; k < countled; k++)
        {
            // Kiểm tra xem hiệu ứng đã thay đổi hay chưa
            if (effect_index != 2)
                return;

            // Kết hợp trạng thái sáng của led đang bị dịch và các led đã ở cuối
            led = ~(effect_2 | effect_stt);
            delay_ms(delayms);
            // Kiểm tra xem đã đến cuối vòng dịch led chưa
            // Nếu chưa thì tiếp tục dịch led
            if (k != countled - 1)
                effect_2 = effect_2 << 1;
        }
        // Khi led bị dịch đã ở cuối lưu lại trạng thái sáng của các led ở cuối
        effect_stt = effect_2 | effect_stt;
        countled--; // Giảm vòng lặp dịch led
    }

    // Hiệu ứng tắt dần
    for (j = 0; j < 8; j++)
    {
        // Kiểm tra xem hiệu ứng đã thay đổi hay chưa
        if (effect_index != 2)
            return;
        // Lúc này tất cả các led đã sáng nên effect_stt = 0xFF
        led = ~effect_stt;
        delay_ms(delayms);
        // Dịch từng bit của effect_stt sang phải để tắt led
        effect_stt = effect_stt >> 1;
    }
}

// Hiệu ứng 3
void effect_3()
{
    led = 0xFF; // Tắt tất cả các led
    delay_ms(delayms);

    unsigned char effect_3_1 = 0x01; // Bật led đầu tiên
    unsigned char effect_3_2 = 0x80; // Bật led cuối cùng
    for (j = 0; j < 8; j++)
    {
        // Kiểm tra xem hiệu ứng đã thay đổi hay chưa
        if (effect_index != 3)
            return;
        led = ~(effect_3_1 | effect_3_2);
        // Kiểm tra led 4, 5 có đang bị sáng lặp lại do dịch bit đối xứng không
        // Nếu không thì cho phép trễ
        if (effect_3_1 != 0x08)
            delay_ms(delayms);
        // Dịch 2 led dần dần sáng vào trong và rồi sáng lại ra phía ngoài
        effect_3_1 = effect_3_1 << 1;
        effect_3_2 = effect_3_2 >> 1;
    }
}

// Khởi tạo ngắt ngoài 0 và 1
void initExtInterrupt()
{
    P3_2 = 1;
    P3_3 = 1;
    IT0 = 1;
    IT1 = 1;
    IE = 0x85;
}

// Xử lý ngắt ngoài 0
void ISR0() __interrupt(0)
{
    EA = 0; // Che toàn bộ các ngắt
    // Chuyển sang hiệu ứng trước
    if (effect_index == 1)
        effect_index = 3;
    else
        effect_index--;
    EA = 1; // Cho phép ngắt toàn cục
}

// Xử lý ngắt ngoài 1
void ISR1() __interrupt(2)
{
    EA = 0; // Che toàn bộ các ngắt
    // Chuyển sang hiệu ứng sau
    if (effect_index == 3)
        effect_index = 1;
    else
        effect_index++;
    EA = 1; // Cho phép ngắt toàn cục
}

int main()
{
    // Khởi tạo 2 ngắt ngoài 0 và 1
    initExtInterrupt();
    while (1)
    {
        // Kiểm tra hiệu ứng hiện tại và cho chạy hiệu ứng đó
        if (effect_index == 1)
        {
            effect_1();
        }
        else if (effect_index == 2)
        {
            effect_2();
        }
        else if (effect_index == 3)
        {
            effect_3();
        }
    }
}