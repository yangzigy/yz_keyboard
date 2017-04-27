//文件名： led.h
#ifndef LED_H
#define LED_H

#include "key.h"

typedef struct
{
	char key; //键
	u8 light; //亮度
} S_LED_LIGHT; //led亮度结构
void led_poll(void);

#endif

