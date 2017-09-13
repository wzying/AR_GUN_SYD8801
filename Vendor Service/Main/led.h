#ifndef _LED_H_
#define _LED_H_

#include "ARMCM0.h"

//#define LED0 BIT16
#define LED1 BIT17
#define LED2 BIT18
#define LED0 BIT19
#define LEDALL ( LED0 | LED1 | LED2)

void led_config(uint32_t led);
void led_open(uint32_t led);
void led_close(uint32_t led);
void led_turn(uint32_t led);

#endif
