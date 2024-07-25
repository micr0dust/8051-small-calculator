#ifndef KEYPAD4X4_H
#define KEYPAD4X4_H
#include <8051.h>
extern signed char key, oldkey;
extern signed char read;
void external_isr(void) __interrupt(0) __using(1);

#endif // KEYPAD4X4_H