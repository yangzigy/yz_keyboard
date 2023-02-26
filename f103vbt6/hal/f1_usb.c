
#include "f1_usb.h"

int EP_num=1; //一共多少个端点
void (*usb_reset_fun)(void); //usb设备reset回调函数
void NOP_Process(void) { }
void (*pEpInt_IN[7])(void) = //端点0没有
{
	NOP_Process, NOP_Process, NOP_Process, NOP_Process,
	NOP_Process, NOP_Process, NOP_Process,
};
void (*pEpInt_OUT[7])(void) = //端点0没有
{
	NOP_Process, NOP_Process, NOP_Process, NOP_Process,
	NOP_Process, NOP_Process, NOP_Process,
};

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
void usb_ep0_send_prepare(u8 *p,int n) //端点0发送准备，给枚举流程用
{
	if(p!=0) UserToPMABufferCopy(p, USB_BT[0].ADDR_TX, n);
	USB_BT[0].COUNT_TX = n;
}
void usb_get_req_rxbuf(u8 *p,int n) //枚举时，从端点0缓冲区获取下发的请求
{
	PMAToUserBufferCopy(p,USB_BT[0].ADDR_RX,n);
}
void SetDeviceAddress(u8 Val) //设置设备地址和端点地址
{
	u32 i;
	for (i = 0; i < EP_num; i++) //设置每个端点的id，EP0R等
	{
		//_SetEPAddress((u8)i, (u8)i);
		USB_EP(i)=(1<<15) | //CTR_RX，正确接收，写1无效
					(1<<7) | //CTR_TX，正确发送，写1无效
					(USB_EP(i) & (0xf80)) | i; //当前值有不能写1的(写0无效)
	}
	USB->DADDR=Val | (1<<7); //设置设备地址，并使能
}

void usb_hal_ini(void) //usb片上外设初始化
{
	RCC->CFGR &= ~(1<<22); //USBclk=PLLclk/1.5=48Mhz
	RCC->APB1ENR |= RCC_APB1ENR_USBEN; //USB时钟使能

	USB->CNTR=1; //重启
	USB->CNTR=0;
	USB->ISTR=0; //清除中断
	USB->CNTR=CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM; //设置中断
	/* USB interrupts initialization */
	USB->ISTR=0;               /* clear pending interrupts */
	USB->CNTR= CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM | CNTR_ESOFM | CNTR_RESETM; /* set interrupts mask */

	EXTI->IMR |= 1<<18;//  开启线18上的中断
	EXTI->RTSR |= 1<<18;//line 18上事件上升降沿触发	 
	MY_NVIC_Init(0,0,USB_LP_CAN1_RX0_IRQn,0);
	MY_NVIC_Init(0,0,USBWakeUp_IRQn,0);
}

//USB唤醒中断服务函数
void USBWakeUp_IRQHandler(void) 
{
	EXTI->PR|=1<<18;//清除USB唤醒中断挂起位
} 
//USB中断处理函数
void USB_LP_CAN1_RX0_IRQHandler(void) 
{
	vu16 t;
	t = USB->ISTR;
	if (t & (1<<10)) //RESET
	{
		USB->ISTR=(u16)~(1<<10);
		usb_reset_fun();
	}
	if (t & (1<<13)) USB->ISTR=(u16)~(1<<13); //ERR 主要用于开发阶段，测量传输质量
	if (t & (1<<12)) //WKUP唤醒
	{
		USB->ISTR=(u16)~(1<<12);
	}
	if (t & (1<<11)) //SUSP挂起
	{
		USB->ISTR=(u16)~(1<<11); //必须在CNTR_FSUSP之后
		usb_stat=USB_UNCONNECTED;
	}
	if (t & (1<<9)) USB->ISTR=(u16)~(1<<9); //SOF帧首
	if (t & (1<<8)) //ESOF期望帧首，没收到1ms SOF
	{
		USB->ISTR=(u16)~(1<<8);
	}
	if (t & (1<<15)) //CTR端点正确传输
	{
		u16 ep_reg = 0; //端点寄存器值
		while (((t = USB->ISTR) & (1<<15)) != 0) //处理完所有中断
		{
			u32 port = t & 0x0f; //最高优先级端点
			if (port == 0) //0端点的处理
			{
				usb_rx_stat = (USB_EP(0) & EPRX_STAT)>>12; //只保存了发送状态，没有其他bit
				usb_tx_stat = (USB_EP(0) & EPTX_STAT)>>4;
				SetEPRxTxStatus(0,EP_RX_NAK,EP_TX_NAK); //必须！！先保存了收发状态，设置成NAK，让主机等待重试，再恢复
				/* DIR bit = origin of the interrupt */
				if ((t & (1<<4)) == 0) //DIR 0:IN, 1:OUT
				{
					USB_EP(0)=USB_EP(0) & (~(1<<7)) & EPREG_MASK; //清除CTR_TX
					In0_Process();
					//printf("rx: %d -> %d  tx: %d -> %d\n",
						//rx_stat>>12,(USB_EP(0) & EPRX_STAT)>>12,
						//tx_stat>>4,(USB_EP(0) & EPTX_STAT)>>4);
				}
				else //1：out
				{
					ep_reg = USB_EP(0);
					if ((ep_reg & EP_SETUP) != 0)
					{
						USB_EP(0)=USB_EP(0) & (~(1<<15)) & EPREG_MASK; //清除CTR_RX
						Setup0_Process();
					}
					else if ((ep_reg & (1<<15)) != 0) //CTR_RX正确接收
					{
						USB_EP(0)=USB_EP(0) & (~(1<<15)) & EPREG_MASK; //清除CTR_RX
						usb_rx_stat=USB_EP_STALL;
						usb_tx_stat=USB_EP_STALL; //STALL
					}
				}
				SetEPRxTxStatus(0,usb_rx_stat<<12,usb_tx_stat<<4); //恢复收发状态
			}
			else //非控制端点中断
			{
				ep_reg = USB_EP(port);
				if ((ep_reg & (1<<15)) != 0) //CTR_RX正确接收
				{
					USB_EP(port)=USB_EP(port) & (~(1<<15)) & EPREG_MASK; //清除CTR_RX
					(*pEpInt_OUT[port-1])(); //端点输出回调
				}
				if ((ep_reg & (1<<7)) != 0) //正确发送
				{
					USB_EP(port)=USB_EP(port) & (~(1<<7)) & EPREG_MASK; //清除CTR_TX
					(*pEpInt_IN[port-1])();
				}
			}
		}
	}
} 

