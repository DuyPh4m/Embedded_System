#include <mcs51/8052.h>

#define Q4 P2_0
#define Q3 P2_1
#define Q2 P2_2
#define Q1 P2_3

#define bt3 P3_4
#define bt1 P3_5
#define beep P3_6

char so[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90}; // mang tu 0 den 9

// ham delay dung timer delay theo ms
void delay_ms(int ms)
{
   while (ms--)
   {
      TMOD = 0x01;
      TH0 = 0xfc;
      TL0 = 0x18;
      TR0 = 1;
      while (!TF0)
         ;
      TF0 = 0;
      TR0 = 0;
   }
}

// ham hien thi cho led 7 thanh dau vao la mins, secs va n voi n la tuy chon de tinh delay co ham
void display(unsigned char mins, unsigned char secs, int n)
{
   unsigned char seg0, seg1, unit0, unit1, i;
   seg0 = mins / 10;
   unit0 = mins % 10;
   seg1 = secs / 10;
   unit1 = secs % 10;
   for (i = 0; i < n; i++)
   {
      Q1 = 0;
      P1 = so[seg0];
      delay_ms(10);
      Q1 = 1;
      Q2 = 0;
      P1 = so[unit0];
      delay_ms(10);
      Q2 = 1;
      Q3 = 0;
      P1 = so[seg1];
      delay_ms(10);
      Q3 = 1;
      Q4 = 0;
      P1 = so[unit1];
      delay_ms(10);
      Q4 = 1;
   }
}
void main()
{

   unsigned char mins = 0;
   unsigned char secs = 0;

   while (1)
   {
      beep = 0;
      display(mins, secs, 1);
      if (bt3 == 0)
      {
         mins++;
         display(mins, secs, 7);
      }
      if (bt1 == 0)
      {
         while (secs || mins)
         {
            display(mins, secs, 24); // delay 1s, chon 24 de bot sai so
            if (secs == 0)
            {
               secs = 60;
               mins--;
            }
            secs--;
         }
         for (int j = 0; j < 3; j++)
         {
            beep = 0;
            delay_ms(1000);
            for (int i = 0; i < 25; i++)
            {
               beep = 1;
               delay_ms(40);
            }
         }
      }
   };
}