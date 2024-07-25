#include <8051.h>
#define KEYPAD_PORT P0 // Assuming keypad is connected to Port 0
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define KEYPAD_DEBOUNCE 10
extern signed char key, oldkey;

void external_isr(void) __interrupt(0) __using(1)
{
    //掃描鍵盤
    unsigned char row, col;
    for (col = 0; col < KEYPAD_COLS; col++)
    {
        KEYPAD_PORT = ~(0x10 << col);

        for (row = 0; row < KEYPAD_ROWS; row++)
        {
            if ((KEYPAD_PORT & (0x01 << row)) == 0){
                key = col*4+row;
                oldkey = key;
                return;
            }
        }
    }
}
