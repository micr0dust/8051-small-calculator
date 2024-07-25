#include "Keypad4x4.h"
#include "LED_Display.h"
#include <8051.h>
/*
 █████╗  ██████╗ ███████╗ ██╗    ███████╗███╗   ███╗ █████╗ ██╗     ██╗            
██╔══██╗██╔═████╗██╔════╝███║    ██╔════╝████╗ ████║██╔══██╗██║     ██║            
╚█████╔╝██║██╔██║███████╗╚██║    ███████╗██╔████╔██║███████║██║     ██║            
██╔══██╗████╔╝██║╚════██║ ██║    ╚════██║██║╚██╔╝██║██╔══██║██║     ██║            
╚█████╔╝╚██████╔╝███████║ ██║    ███████║██║ ╚═╝ ██║██║  ██║███████╗███████╗       
 ╚════╝  ╚═════╝ ╚══════╝ ╚═╝    ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝       
                                                                                   
 ██████╗ █████╗ ██╗      ██████╗██╗   ██╗██╗      █████╗ ████████╗ ██████╗ ██████╗ 
██╔════╝██╔══██╗██║     ██╔════╝██║   ██║██║     ██╔══██╗╚══██╔══╝██╔═══██╗██╔══██╗
██║     ███████║██║     ██║     ██║   ██║██║     ███████║   ██║   ██║   ██║██████╔╝
██║     ██╔══██║██║     ██║     ██║   ██║██║     ██╔══██║   ██║   ██║   ██║██╔══██╗
╚██████╗██║  ██║███████╗╚██████╗╚██████╔╝███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║
 ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝
                                                                    made by Microdust
*/
// ──── Timer macros ───────────────────────────────────────────────
#define Fosc 12000000
#define Fclk (Fosc / 12)
#define Fint 500
#define T0VAL (65536 - Fclk / Fint)
#define TH0_R (T0VAL >> 8)
#define TL0_R (T0VAL & 0xff)
// ────┤Keypad macros├──────────────────────────────────────────────
#define KEYPAD_PORT P0
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
// ────┤7-seg-display macros├───────────────────────────────────────
#define SEGMENT_PORT P2                 // segment port
#define DIGIT_PORT P1                   // port for selecting digit
#define _H 23                           // h display 
#define _D 13                           // d display 
#define BLANK 16                        // blank display 
// ────┤constant├───────────────────────────────────────────────────
#define FLOAT_PREC 5                    // float precision
#define DEC 10
#define HEX 16
#define PRE_OVER_FLOW_32_BIT 214748360  // (2**31)/100*10
#define PRE_OVER_FLOW_16_BIT 3276       // (2**15)/10
#define INF (2147483647+1)              // (2**31)+1
#define N_REC (10)
#define HEX_NEG_MASK 268435455          // (16**7)-1
#define MASK_3_BIT (8-1)                // mask 0b111
// ────┤7-seg-display buffer├───────────────────────────────────────
unsigned char display_buffer[] = {16, 16, 16, 16, 16, 16, 16, 16};
unsigned char dot = 9;
// ────┤key buffer├─────────────────────────────────────────────────
signed char key = -2, oldkey = -1;
// ────┤ 鍵盤規劃 ├─────────────────────────────────────────────────
const unsigned char btn[2][16]= {
    {   // 預設鍵盤  (10、16 進位皆通用)
        '7','8','9','x', //│0 │1 │2 │3 │
        '4','5','6','+', //│4 │5 │6 │7 │
        '1','2','3','=', //│8 │9 │10│11│
        '0','^','<','H'  //│12│13│14│15│
    }, {// 切換後鍵盤 (僅 16 進位能切換)
        ' ',' ',' ','x', //│0 │1 │2 │3 │
        'E','F',' ','+', //│4 │5 │6 │7 │
        'B','C','D','=', //│8 │9 │10│11│
        'A','^','<','H'  //│12│13│14│15│
    }
};
/*═════════════════════════════════════╦════════════════════════════════════════════════════════╗
║   [Dec mode] mode = 0                ║  [Hex mode] mode = 1                                   ║
╟──────────────────────────────────────╫────────────────────────────────────────────────────────╢
║   single press     double press      ║  single press     double press        use ^ to switch  ║
║   │7 8 9 x│        │7 8 9 x│         ║  │7 8 9 x│        │7 8 9 x│           │      x│        ║
║   │4 5 6 +│        │4 5 6 - * / % /. ║  │4 5 6 +│        │4 5 6 - * / % /.   │E F   +│        ║
║   │1 2 3 =│        │1 2 3 P│         ║  │1 2 3 =│        │1 2 3 P│           │B C D =│        ║
║   │0 - < h│        │0 - < d│         ║  │0 ^ < d│        │0 ^ < h│           │A ^ < d│        ║
╚══════════════════════════════════════╩═══════════════════════════════════════════════════════*/

__bit mode = 0;  // 進位模式 (dec or hex)
__bit swich = 0; // 鍵盤模式切換 (僅用於16進位)
__bit NB = 0;    // 目前編輯數值 (第一數值 或 第二數值)

// 歷史數值小數點位儲存
unsigned char floatpos = 0;
// 歷史數值儲存
long history[] = {0,INF,INF,INF,INF,INF,INF,INF,INF,INF};

// 目前 第一數值小數點位、第二數值小數點位
unsigned int history_dot[] = {0,0};
// 目前 第一數值、第二數值
long value[]={0,0};

// 狀態
unsigned char state = 0;
/*  0: [第一數值輸入]
    1: [運算子選擇]
    2: [第二數值輸入]
*/
// 運算子
unsigned char oper = 0;
/*  0: +  (加)
    1: -  (減)
    2: *  (乘)
    3: // (整除)
    4: %  (mod)
    5: /  (小數除法)
*/
// 顯示位移
unsigned char shift = 0;
// 進位
unsigned char carry = DEC;
// 歷史資料指標
signed char browse = 0;

// 小數無條件捨去
void to_int(){
    while (floatpos){
        value[0]/=10;
        floatpos--;
    }
}

// 小數清除非必要的 0
void float_trim(){
    value[1]=value[0];
    while (floatpos&&value[1]/10*10==value[0])
    {
        value[0]/=10;
        value[1]=value[0];
        floatpos--;
    }
}

// 運算子計算
void calculate(){
    // 小數加減法預處理
    if(mode==0 && oper<=1){
        for (unsigned char i = 0; i < floatpos; i++)
            value[1]*=10;
    }
    // 除法、mod 僅接受整數當輸入
    if(mode==0 && (oper>=3))
        to_int();
    if(oper==0)        // 加法
        value[0]+=value[1];
    else if(oper==1)   // 減法
        value[0]-=value[1];
    else if(oper==2){  // 乘法
        value[0]*=value[1];
        if(mode==0)
            float_trim();
    }else if(oper==3){ // 整除
        value[0]/=value[1];
    }else if(oper==4)  // mod
        value[0]=value[0]%value[1];
    else if(oper==5){  // 小數除法
        // 如有負號先提出
        __bit neg=0;
        float ftmp;
        if(value[0]<0) {ftmp=-value[0]; neg=1;}
        else ftmp=value[0];
        // 浮點數運算
        ftmp/=(float)value[1];
        // overflow 檢查，取小數點後幾位
        for (unsigned char i = FLOAT_PREC; ftmp<PRE_OVER_FLOW_16_BIT && i--;){
            ftmp*=10;
            floatpos+=1;
        }
        // 如有負號放回
        if(neg) value[0]=-(int)ftmp;
        else value[0]=(int)ftmp;
        float_trim();
    }
    // 儲存計算結果到歷史資料
    history[browse]=value[0];
    // 儲存 3 bit 小數點位置到歷史資料
    unsigned int tmp;
    __bit fpflag = browse>=5;
    value[1]=fpflag?(browse-5)*3:(browse)*3;
    tmp=~(MASK_3_BIT<<(value[1]));
    history_dot[fpflag]&=tmp;
    tmp=floatpos&MASK_3_BIT;
    tmp<<=value[1];
    history_dot[fpflag]|=tmp;
    value[1]=0;
    // 更新到歷史資料指標
    if((++browse)>=N_REC) browse=0;
}


void main(void)
{

    TMOD = 0x01; // Set Timer0 to mode 1 (16-bit)
    TH0 = TH0_R; // Set high byte
    TL0 = TL0_R; // Set low byte
    ET0 = 1;     // Enable Timer0 interrupt
    EA = 1;      // Enable global interrupt

    IT0 = 1; // 設置為邊緣觸發
    EX0 = 1; // 啟用外部中斷0
    // Start Timer0
    TR0 = 1;
    P3 = 0b00000100;
    P0 = 0xf0; // 設置P0以啟用interupt監測

    unsigned char last_op = ' ';
    display_buffer[0] = 0;
    display_buffer[7] = _D;
    while (1)
    {
        P3 = 0b00000100;
        P0 = 0xf0;// 重啟interupt監測

        // 按鍵偵測
        if (key==oldkey)
        {
            // 取得按鍵指令
            unsigned char op = btn[swich][key];
            // 過濾空按鍵
            if(op!=' '){
                // 顯示位移自動復位
                if(op!='<')
                    shift=0;
                // 依指令執行
                if(op>='0' && op<='9'){ // 0~9 輸入
                    if(state==1){
                        NB=1;
                        state=2;
                    }
                    // 檢查輸入會不會 overflow (小數只有16bit、整數32bit)
                    if(NB==1 || !floatpos){
                        if(value[NB]<PRE_OVER_FLOW_32_BIT)
                            value[NB] = value[NB]*carry+(int)(op-'0');
                    }else if(value[NB]<PRE_OVER_FLOW_16_BIT && floatpos<FLOAT_PREC){
                        floatpos++;
                        value[NB] = value[NB]*carry+(int)(op-'0');
                    }
                }else if (op>='A' && op<='F') // A~F 輸入
                {
                    // 從[運算子選擇狀態]切到[第二數值輸入]狀態
                    if(state==1){
                        NB=1;
                        state=2;
                    }
                    // 檢查輸入會不會 overflow
                    if(value[NB]<PRE_OVER_FLOW_32_BIT)
                        value[NB] = value[NB]*carry+(int)(op-'A'+10);
                }else if (op=='x'){ // 清除目前數值
                    floatpos=oper=NB=value[0]=value[1]=state=0;
                }else if (op=='='){
                    if(last_op=='='){ // 連續2次(含)以上的=，觸發歷史紀錄瀏覽
                        // 找上一個歷史紀錄
                        do{
                            if((--browse)<0) browse=N_REC-1;
                            value[0]=history[browse];
                        }while(value[0]==INF);
                        // 載入歷史紀錄小數點
                        __bit fpflag = browse>=5;
                        value[1]=fpflag?(browse-5)*3:(browse)*3;
                        floatpos=(history_dot[fpflag]&(MASK_3_BIT<<value[1]))>>value[1];
                        if(mode&&floatpos){
                            while (floatpos--)
                                value[0]/=10;
                        }
                        value[1]=0;
                    }else if(state==2){ // 第一次按=，純粹計算結果
                        calculate();
                        swich=NB=oper=state=0;
                    }
                }else if (op=='H'){ // 10、16 進位轉換
                    swich=0;
                    mode=!mode;
                    carry=mode?HEX:DEC;
                    if(mode&&oper==5)
                        oper=0;
                    // 16 進位只接受整數，無條件捨去小數
                    if(mode)
                        to_int();
                    else floatpos=0;
                }else if (op=='<'){ // 數字位移查看
                    if(display_buffer[6]==BLANK) shift=0;
                    else shift++;
                }else if (op=='^'){ // 10、16 進位轉換
                    if(mode){
                        swich=!swich;
                    }else value[NB]*=-1;
                }else if (op=='+'){ // 運算子切換
                    if(state==0) state=1; // 切換到[運算子選擇]狀態
                    else if(state==2){ // 如果目前是第二數值輸入，直接省略=計算結果
                        calculate();   // 並以結果當第一數值繼續計算
                        state=1;
                        NB=oper=0;
                    }else{ // 運算子選擇
                        oper++;
                        if(oper>5 || (mode&&oper==5))
                            oper=0;
                    }
                    // 目前運算子顯示
                    display_buffer[7]=17+oper;
                }
                // 紀錄上個輸入
                last_op = op;
            }
            // 目前進位制顯示
            if(state!=1)
                display_buffer[7]=(mode?_H:_D);
            
            // 負數顯示處理
            __bit neg=0;
            long display_val;
            display_val=value[NB];
            if(display_val<0){
                if(mode==0){ // 10 進位顯示負號
                    neg=1;
                    display_val=-display_val;
                }else{       // 16 進位顯示補數
                    display_val=display_val&(HEX_NEG_MASK);
                }
            }
            // 小數點顯示
            if(NB==0 && floatpos) dot=floatpos;
            else dot=9;
            // 目前位移顯示數值
            unsigned shift_display=shift;
            // 特別處理 0 的顯示
            if(!display_val) {
                for (unsigned char i = 1; i < 7; i++)
                    display_buffer[i] = 16;
                display_buffer[0] = 0;
            }else{
                // 位移顯示的數值處理
                while (shift_display--)
                    display_val/=carry;
                // 7 個燈號顯數處理
                for (unsigned char i = 0; i < 7; i++)
                {
                    if(display_val){ // 數值顯示
                        display_buffer[i] = display_val-((display_val/carry)*carry);
                        display_val/=carry;
                    }else if(neg && (NB==1||i>floatpos)){   // 負號顯示
                        neg=0;
                        display_buffer[i]=18;
                    }else if(mode==0 && NB==0 && i<floatpos) // 小數點後補 0
                        display_buffer[i] = 0;
                    else display_buffer[i] = BLANK; // 預設不顯示
                }
            }
            oldkey = -1;
        }
    }
}
