//
// LY096BG30 : 0.96" OLED
// 
#define LCD_I2C_SLA        0x78
#define LCD_I2C_PORT       I2C1

#define LCD_Xmax 128
#define LCD_Ymax 64

extern void Init_LCD(void);
extern void clear_LCD(void);
extern void print_LCD(unsigned char *buffer);
