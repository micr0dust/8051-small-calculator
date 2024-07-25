# 定義編譯器和相關選項
CC = sdcc
AR = sdar
CFLAGS = -mmcs51 -I.

# 定義目標文件和靜態庫
OBJS = main.rel Keypad4x4.rel LED_Display.rel
LIBS = myIO.lib

# 預設目標：生成可執行文件
all: main.ihx main.hex


main.hex: main.ihx 
	packihx $< > $@

# 生成可執行文件
main.ihx: $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# 編譯main.c
main.rel: main.c
	$(CC) $(CFLAGS) -c $<

# 編譯Keypad4x4.c
Keypad4x4.rel: Keypad4x4.c
	$(CC) $(CFLAGS) -c $<

# 編譯LED_Display.c
LED_Display.rel: LED_Display.c
	$(CC) $(CFLAGS) -c $<

# 打包Keypad4x4.rel和LED_Display.rel成靜態庫myIO.lib
myIO.lib: Keypad4x4.rel LED_Display.rel
	$(AR) r $@ $^

# 清理生成的文件
clean:
	del -f *.asm *.lst *.map *.rel *.sym *.lk *.ihx *.lib *.rst *.mem *.hex

.PHONY: all clean




