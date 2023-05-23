#include <mcs51/8052.h>

#define LED P1

void delay(int i)
{
	while (i--)
		;
}

// xoay bit P1 sang trai 1 bit
void sequentialblinks()
{
	LED = LED << 1 | LED >> 7; // dich trai và or bit dau voi bit 0 duoc them vao
}

// xoay bit P1 sang trai 1 bit
void sequentialblinkss()
{
	LED = LED >> 1 | LED << 7; // dich phai và or bit cuoi voi bit 0 duoc them vao
}

// 1.1 nhay cap led
void ledblinks()
{
	LED = 0b11111111;
	delay(100000);
	LED = 0b11111100;
	delay(100000);

	for (int i = 0; i < 6; i++)
	{
		sequentialblinks();
		;
		delay(100000);
	}
	LED = 0b11111111;
	delay(100000);
	LED = 0b10011111;
	delay(100000);
	for (int i = 0; i < 5; i++)
	{
		sequentialblinkss();
		delay(100000);
	}
	LED = 0b11111111;
}

// 1.2 bat lan luot led don tu trai qua phai roi tat lan luot tu phai qua trai
void donled()
{
	LED = 0b11111111;
	delay(10000);
	LED = 0b11111110;
	delay(10000);
	for (int i = 0; i < 7; i++)
	{ // bat den lan dau
		sequentialblinks();
		delay(10000);
	}
	int a = 0b11111110;
	LED = a & 0b01111111; // thuc hien phep and de giu lai led cuoi sang
	delay(10000);
	for (int i = 0; i < 6; i++)
	{						  // bat den lan hai chi can nhay 6 lan do den cuoi giu sang
		a = a << 1 | a >> 7;  // xoay bit a
		a = a & 0b11111111;	  // a co 16 bit can xoay 8 bit nen phai and de xoa bo cac 8 bit cao
		LED = a & 0b01111111; // gan a cho led va giu lai den led cuoi sang
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00111111;
	delay(10000);
	for (int i = 0; i < 5; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00111111;
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00011111;
	delay(10000);
	for (int i = 0; i < 4; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00011111;
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00001111;
	delay(10000);
	for (int i = 0; i < 3; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00001111;
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00000111;
	delay(10000);
	for (int i = 0; i < 2; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00000111;
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00000011;
	delay(10000);
	for (int i = 0; i < 1; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00000011;
		delay(10000);
	}
	a = 0b11111110;
	LED = a & 0b00000001;
	delay(10000);
	for (int i = 0; i < 0; i++)
	{
		a = a << 1 | a >> 7;
		a = a & 0b11111111;
		LED = a & 0b00000001;
		delay(10000);
	}
	// Tat lan luot cac den bang mang
	LED = 0b10000000;
	int array[7] = {0b11000000, 0b11100000, 0b11110000, 0b11111000, 0b11111100, 0b11111110, 0b11111111};
	for (int i = 0; i < 7; i++)
	{
		LED = array[i];
		delay(10000);
	}
}

// 1.3 bat tat led doi xung bang mang
void symmetricalledblinks()
{
	int arrayscb[10] = {255, 126, 189, 219, 231, 231, 219, 189, 126, 255};
	for (int i = 0; i < 10; i++)
	{
		LED = arrayscb[i];
		delay(100000);
	}
}

// --------------------------------------------------

void main()
{

	for (int i = 0; i < 2; i++)
	{
		ledblinks();
	}
	for (int i = 0; i < 2; i++)
	{
		donled();
	}
	for (int i = 0; i < 2; i++)
	{
		symmetricalledblinks();
	}

	while (1)
		;
}