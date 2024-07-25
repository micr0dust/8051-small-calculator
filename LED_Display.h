#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H
#include <8051.h>
extern unsigned char display_buffer[];
extern unsigned char pot;
// 七段顯示器的字型設定（共陽極）
extern const unsigned char digits[17];
extern unsigned char i;
extern __bit swich;

void timer0_isr(void) __interrupt(1) __using(1);

#endif // LED_DISPLAY_H