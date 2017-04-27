/*
uart.h
同步方式的串口通信
*/
#ifndef UART_H
#define UART_H

#include "main.h"
#include "stm32f1_sys.h"
#include "queue.h"

typedef struct
{
	USART_TypeDef *uart;
	Queue que_tx;
	Queue que_rx;
} S_UART; //串口设备结构

extern S_UART uart1;
extern S_UART uart2;
extern S_UART uart3;
extern S_UART uart4;
extern S_UART uart5;

void uart_initial(S_UART *obj,int b);

int uart1_putchar(int c);

#endif
