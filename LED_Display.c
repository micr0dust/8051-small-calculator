#include <8051.h>
#define Fosc 12000000
#define Fclk (Fosc / 12)
#define Fint 500
#define T0VAL (65536 - Fclk / Fint)
#define TH0_R (T0VAL >> 8)
#define TL0_R (T0VAL & 0xff)
#define SEGMENT_PORT P2 // 七段顯示器的段端口
#define DIGIT_PORT P1   // 七段顯示器的位端口
extern unsigned char display_buffer[];
extern unsigned char dot;
extern __bit swich;

// 七段顯示器的字型設定（共陽極）
const unsigned char digits[] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b01011000, // c
    0b01011110, // d
    0b01111001, // E
    0b01110001, // F
    0b00000000, // 空白
    0b01000110, //+
    0b01000000, //-
    0b01110110, //*
    0b01010010, ///
    0b01010100, //%
    0b11010010, ///
    0b01110100, // h 23
};
unsigned char i;

void timer0_isr(void) __interrupt(1) __using(1)
{
    TH0 = TH0_R;
    TL0 = TL0_R;
    
    SEGMENT_PORT = digits[display_buffer[i]]; // 設置段
    if(i==dot){ // 小於 1 的小數，小數點前補 0
        if(!SEGMENT_PORT)
            SEGMENT_PORT=digits[0];
        SEGMENT_PORT |= 0x80;
    }
    if(swich && i==0)
        SEGMENT_PORT |= 0x80;
    DIGIT_PORT = 7-i; // 選擇位置
    i++;
    if(i>7)
        i = 0;
}