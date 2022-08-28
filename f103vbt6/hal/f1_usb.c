
#include "f1_usb.h"
#include "usb_core.h"

int EP_num=1; //总共多少个端点
void SetEPType(u8 i, u16 d) //设置端点类型
{
	//先清零写0无效的位，再置位写1无效的位
	USB_EP(i)=(USB_EP(i) & (~(EPREG_0_SET & (3<<9)))) | EPREG_1_SET | d;
}
void SetEPTxStatus(u8 i, u16 d) //设置端点发送状态
{
	u16 t;
	t=USB_EP(i) & ((~EPREG_0_SET) | (3<<4)); //先清零写0无效的位，排除需要设置的位
	//t = USB_EP(i) & EPTX_DTOGMASK;
	if(((1<<4) & d)!= 0) t ^= (1<<4);
	if(((1<<5) & d)!= 0) t ^= (1<<5);
	USB_EP(i)= (t | EPREG_1_SET); //再置位写1无效的位
}
void SetEPRxStatus(u8 i, u16 d) //设置端点接收状态
{
	u16 t;
	t=USB_EP(i) & ((~EPREG_0_SET) | (3<<12)); //先清零写0无效的位，排除需要设置的位
	//t = USB_EP(i) & EPTX_DTOGMASK;
	if(((1<<12) & d)!= 0) t ^= (1<<12);
	if(((1<<13) & d)!= 0) t ^= (1<<13);
	USB_EP(i)= (t | EPREG_1_SET); //再置位写1无效的位
}
void SetEPRxTxStatus(u8 i, u16 rx,u16 tx) //设置端点发送、接收状态
{
	u16 t;
	t=USB_EP(i) & ((~EPREG_0_SET) | (3<<4) | (3<<12)); //先清零写0无效的位，排除需要设置的位
	if(((1<<12) & rx)!= 0) t ^= (1<<12);
	if(((1<<13) & rx)!= 0) t ^= (1<<13);
	if(((1<<4) & tx)!= 0) t ^= (1<<4);
	if(((1<<5) & tx)!= 0) t ^= (1<<5);
	USB_EP(i)= (t | EPREG_1_SET); //再置位写1无效的位
}
void ClearDTOG_RX(u8 i)
{
	if((USB_EP(i) & (1<<14)) != 0) //DTOG_RX没法直接给0，只能翻转
	{
		//先清零写0无效的位，再置位写1无效的位
		USB_EP(i)=(USB_EP(i) & EPREG_MASK) | EPREG_1_SET | (1<<14); //DTOG_RX
	}
}
void ClearDTOG_TX(u8 i)
{
	if((USB_EP(i) & (1<<6)) != 0) //DTOG_TX 没法直接给0，只能翻转
	{
		//先清零写0无效的位，再置位写1无效的位
		USB_EP(i)=(USB_EP(i) & EPREG_MASK) | EPREG_1_SET | (1<<6); //DTOG_TX
	}
}
void SetEPRxCount(u8 port, u16 n) //设置接收缓存大小
{
	u16 block_n; //block数量，根据总字节数，有两种块大小
	if(n > 62)
	{
		block_n = n >> 5; //0表示1个块
		if((n & 0x1f) == 0) block_n--;
		USB_BT[port].COUNT_RX = (block_n << 10) | 0x8000; //32字节块
	}
	else
	{
		block_n = n >> 1;
		if((n & 0x1) != 0)
		block_n++;
		USB_BT[port].COUNT_RX = block_n << 10;
	}
}
void UserToPMABufferCopy(u8 *p, u16 ind, u16 n) //将CPU数据复制到USB地址下的缓存
{
	u32 i;
	n = (n + 1) >> 1; //n = (n + 1) / 2 字节数改为字数
	u16 *p_usb_b = (u16 *)(ind * 2 + PMAAddr); //CPU读取USB缓存的地址，16bit字转32bit字的地址
	for(i = 0; i<n; i++)//一个循环写一个字
	{
		p_usb_b[i<<1] = ((u16*)p)[i];
	}
}
void PMAToUserBufferCopy(u8 *p, u16 ind, u16 n)//将USB字复制到CPU地址下
{
	u32 i;
	n = (n + 1) >> 1; //n = (n + 1) / 2 字节数改为字数
	u16 *p_usb_b = (u16 *)(ind * 2 + PMAAddr); //CPU读取USB缓存的地址，16bit字转32bit字的地址
	for(i = 0; i<n; i++)//一个循环写一个字
	{
		((u16*)p)[i] = p_usb_b[i<<1];
	}
}

void usb_hal_ini(void) //usb片上外设初始化
{
	RCC->CFGR &= ~(1<<22); //USBclk=PLLclk/1.5=48Mhz
	RCC->APB1ENR |= RCC_APB1ENR_USBEN; //USB时钟使能

	EXTI->IMR |= 1<<18;//  开启线18上的中断
	EXTI->RTSR |= 1<<18;//line 18上事件上升降沿触发	 
	MY_NVIC_Init(0,0,USB_LP_CAN1_RX0_IRQn,0);
	MY_NVIC_Init(0,0,USBWakeUp_IRQn,0);
}

