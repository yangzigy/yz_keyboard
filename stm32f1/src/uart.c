/*
uart.c
同步方式的串口通信
*/
#include "uart.h"

//串口结构
S_UART uart1=
{
	USART1,
};
static u8 uart1_tx_buf[128];
static u8 uart1_rx_buf[128];
S_UART uart2=
{
	USART2,
};
static u8 uart2_tx_buf[128];
static u8 uart2_rx_buf[128];
S_UART uart3=
{
	USART3,
};
static u8 uart3_tx_buf[128];
static u8 uart3_rx_buf[128];
#ifdef STM32F10X_HD
S_UART uart4=
{
	UART4,
};
static u8 uart4_tx_buf[128];
static u8 uart4_rx_buf[128];
S_UART uart5=
{
	UART5,
};
static u8 uart5_tx_buf[128];
static u8 uart5_rx_buf[128];
#endif

void uart_initial(S_UART *obj,int b)
{
	u32 nvic_num=USART1_IRQn;
	switch((u32)obj->uart)
	{
	case (u32)USART1:
		RCC->APB2ENR|=RCC_APB2Periph_USART1;  //使能串口时钟 
		MGPIOA->CT9=GPIO_AF_PP; //发送
		MGPIOA->CT10=GPIO_IN_PP; //接收
		GPIOA->BSRR=(1<<10); //上拉
		b/=2; //1号串口主频高一倍
		nvic_num=USART1_IRQn;
		Queue_ini(uart1_tx_buf,sizeof(uart1_tx_buf),&(obj->que_tx));
		Queue_ini(uart1_rx_buf,sizeof(uart1_rx_buf),&(obj->que_rx));
		break;
	case (u32)USART2:
		RCC->APB1ENR|=RCC_APB1Periph_USART2;  //使能串口时钟 
		MGPIOA->CT2=GPIO_AF_PP; //发送
		MGPIOA->CT3=GPIO_IN_PP; //接收
		GPIOA->BSRR=(1<<3); //上拉
		nvic_num=USART2_IRQn;
		Queue_ini(uart2_tx_buf,sizeof(uart2_tx_buf),&(obj->que_tx));
		Queue_ini(uart2_rx_buf,sizeof(uart2_rx_buf),&(obj->que_rx));
		break;
	case (u32)USART3:
		RCC->APB1ENR|=RCC_APB1Periph_USART3;  //使能串口时钟 
		MGPIOB->CT10=GPIO_AF_PP; //发送
		MGPIOB->CT11=GPIO_IN_PP; //接收
		GPIOB->BSRR=(1<<11); //上拉
		nvic_num=USART3_IRQn;
		Queue_ini(uart3_tx_buf,sizeof(uart3_tx_buf),&(obj->que_tx));
		Queue_ini(uart3_rx_buf,sizeof(uart3_rx_buf),&(obj->que_rx));
		break;
#ifdef STM32F10X_HD
	case (u32)UART4:
		RCC->APB1ENR|=RCC_APB1Periph_UART4;  //使能串口时钟 
		MGPIOC->CT10=GPIO_AF_PP; //发送
		MGPIOC->CT11=GPIO_IN_PP; //接收
		GPIOC->BSRR=(1<<11); //上拉
		nvic_num=UART4_IRQn;
		Queue_ini(uart4_tx_buf,sizeof(uart4_tx_buf),&(obj->que_tx));
		Queue_ini(uart4_rx_buf,sizeof(uart4_rx_buf),&(obj->que_rx));
		break;
	case (u32)UART5:
		RCC->APB1ENR|=RCC_APB1Periph_UART5;  //使能串口时钟 
		MGPIOC->CT12=GPIO_AF_PP; //发送
		MGPIOD->CT2=GPIO_IN_PP; //接收
		GPIOD->BSRR=(1<<2); //上拉
		nvic_num=UART5_IRQn;
		Queue_ini(uart5_tx_buf,sizeof(uart5_tx_buf),&(obj->que_tx));
		Queue_ini(uart5_rx_buf,sizeof(uart5_rx_buf),&(obj->que_rx));
		break;
#endif
	default:
		break;
	}
	obj->uart->SR=0;
	obj->uart->DR=0;
	obj->uart->BRR=36000000/b;
	//obj->uart->CR1 = (u16)0x206c;	//使能发送中断
	obj->uart->CR1 = (u16)0x202c;
	obj->uart->SR = 0;

	MY_NVIC_Init(0,0,nvic_num,0);
}

void uart_send(u8 *p,int n,S_UART *obj) //发送数据
{
	int i;
	obj->uart->CR1 &= ~(1<<7);//TXE
	for(i=0;i<n;i++)
	{
		Queue_set_1(p[i],&(obj->que_tx));
	}
	obj->uart->CR1 |= (1<<7);//TXE
}
void uart_irq(S_UART *obj)
{
	u16 t;
	if(obj->uart->SR & 0x28)//接收
	{
		t=(u8)(obj->uart->DR); //此时清了接收和过载的中断标志
		Queue_set_1(t,obj->pque_rx);
	}
	if(obj->uart->CR1 & (1<<7) && obj->uart->SR & 0x80)//发送空中断TXE
	{
		u8 tmp;
		if(Queue_get_1(&tmp,&(obj->que_tx))==0)
		{
			obj->uart->DR=tmp;
		}
		else
		{
			obj->uart->CR1 &= ~(1<<7);//TXE
		}
	}
	//这里不清中断，防止此时有其他中断发生
}

void USART1_IRQHandler(void)
{
	uart_irq(&uart1);
}
void USART2_IRQHandler(void)
{
	uart_irq(&uart2);
}
void USART3_IRQHandler(void)
{
	uart_irq(&uart3);
}
#ifdef STM32F10X_HD
void UART4_IRQHandler(void)
{
	uart_irq(&uart4);
}
void UART5_IRQHandler(void)
{
	uart_irq(&uart5);
}
#endif

//C标准库的重定向
//int uart1_putchar(int c)
//{
	//USART1->DR=c;
	//while(!(USART1->SR & 0x80));
	//USART1->SR &= ~0x00000040;
	//return c;	//IAR编译器里这里不能返回0
//}

int uart1_putchar(int c)
{
	u8 t=c;
	uart_send(&t,1,&uart1);
	return c;	//IAR编译器里这里不能返回0
}

