#ifndef _OLED_H_
#define _OLED_H_

#include "ARMCM0.h"

//设备ID，这是7位ID，如果写操作正在发出的ID是 0x19<<1+1=0x33  读操作就是 0x19<<1=0x32
#define I2C_ID_OLED 0x3c

#define X_WIDTH 128
#define Y_WIDTH 64

#define oled_rest   BIT21

extern void oled_init(void);
extern void oled_6x8str(uint8_t x, uint8_t y,uint8_t ch[]);
extern uint8_t oled_8x16str(uint8_t x, uint8_t y,uint8_t ch[]);
extern void oled_set_pos(uint8_t x, uint8_t y) ;
extern void oled_printf(uint8_t x, uint8_t y,char *format,...);
void oled_hexdump(uint8_t x, uint8_t y,char *title, uint8_t *buf, uint16_t sz);
void PutGB1616(unsigned short x, unsigned short  y, unsigned char *s);

#endif

