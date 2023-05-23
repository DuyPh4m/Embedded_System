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
void delay_ms(int ms) {
   while(ms--) {
      TMOD = 0x01;
      TH0= 0xfc;
      TL0=  0x18;
      TR0 = 1;
      while(!TF0);
      TF0 =0;
      TR0= 0;
   }
}

// ham hien thi cho led 7 thanh dau vao la mins, secs va n voi n la tuy chon de tinh delay co ham
void display(unsigned char mins, unsigned char secs, int n) {
   unsigned char tens0, tens1, units0, units1, i;
   tens0 = mins/10; units0 = mins%10;
   tens1 = secs/10; units1 = secs%10;
   for(i = 0 ; i<n; i++) {
      Q4=0; P1= so[tens0]; delay_ms(5); Q4 = 1;
      Q3=0; P1= so[units0]; delay_ms(5); Q3 = 1;
      Q2=0; P1= so[tens1]; delay_ms(5); Q2 = 1;
      Q1=0; P1= so[units1]; delay_ms(5); Q1 = 1;
   }
}
void main() {
   
   unsigned char mins = 0; 
   unsigned char secs = 0;
   
   while(1){
   beep = 0;
  display(mins, secs, 1);
   if(bt3==0) {mins++;display(mins, secs, 7);}
   if(bt1==0) {
   while(secs||mins) {
      display(mins, secs, 49); // delay 1s, chon 49 de bot sai so
      if(secs==0) {secs=60 ;mins--;}
      secs--;
      }
      for(int j =0; j < 3; j++) {
	 beep = 0;
	   delay_ms(1000);
	   for (int i = 0; i< 25;i++)
	   {
		   beep = 1;
		   delay_ms(40);	   
	   }
      }
   } 	   
   };
     
   
}	