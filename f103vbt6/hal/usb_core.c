
#include "f1_usb.h"
#include "usb_core.h"

int EP_num=1; //一共多少个端点
int usb_max_packet=64; //最大包长

DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb设备状态，失去连接时要更新（当前无效）
S_USB_CTRL_STATE usb_ctrl_stat=IN_DATA; //usb控制状态

RESULT (*class_data_setup)(void)=0; //setup的处理回调函数

//收发部分
S_USB_REQ_DATA usb_req_rxbuf; //usb端口0的标准请求接收缓存
//数据收发的长度和位置
u16 p0_len=0; //剩余发送数
u16 p0_ind=0; //发送时的分批发送的位置
u8 *p0_p=0; //收发指针

//由于中断处理的时候需要设置NAK，所以要保存之前的状态
static vu16 rx_stat; //是接收中断与处理函数关于最后端点0状态的接口
static vu16 tx_stat; //是接收中断与处理函数关于最后端点0状态的接口
static int need_send_void = 0; //大于等于最大包数，且是整数包，需要发送空包

static void p0_send_poll(void) //控制端点的发送数据 
{
	u32 n;
	if((p0_len == 0) && (usb_ctrl_stat == LAST_IN_DATA)) //如果是最后一包
	{
		if(need_send_void) //大于等于最大包数，且是整数包，发送空包
		{
			USB_BT[0].COUNT_TX = 0;
			tx_stat=EP_TX_VALID; 
			need_send_void = 0;
			//SetEPTxStatus(0,EP_TX_VALID); //恢复收发状态
		}
		else //没有数据了
		{
			usb_ctrl_stat = WAIT_STATUS_OUT;
			tx_stat=(1<<4); //TX_STALL
			//SetEPTxStatus(0,EP_TX_STALL); //恢复收发状态
		}
		p0_p=0; //恢复指针
		//SetEPRxStatus(0,EP_RX_VALID); //加上这句就不对了
		return ;
	}
	n = usb_max_packet; //先假设发最大包长
	usb_ctrl_stat = (p0_len <= n) ? LAST_IN_DATA : IN_DATA; //是否是最后一包
	if (n > p0_len) n = p0_len; //发送不超过包长

	if(p0_p) //若是测试
	{
		UserToPMABufferCopy(p0_p, USB_BT[0].ADDR_TX, n);
		USB_BT[0].COUNT_TX = n;
		p0_len -= n;
		p0_p+=n;
	}
	tx_stat = EP_TX_VALID;
	rx_stat = EP_RX_VALID; //USB_StatusOut//期望host退出data IN
	//SetEPRxTxStatus(0,EP_RX_VALID,EP_TX_VALID); //恢复收发状态
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
//端点0的接收缓存，换算成CPU的32bit地址
//看做SETUP0的接收结构
#define S0 (*(S_SETUP0_RX*)(PMAAddr + (u8 *)(USB_BT[0].ADDR_RX * 2))) 
void Setup0_Process(void) //接收到setup后的data0，放在端点0的接收缓存，处理
{
	//将接收缓存中的数据复制到CPU区域的变量中
	usb_req_rxbuf.type.b = S0.type;
	usb_req_rxbuf.req = S0.req;
	usb_req_rxbuf.vals.w = CHANGE_END16(S0.vals.w);
	usb_req_rxbuf.inds.w = CHANGE_END16(S0.inds.w);
	usb_req_rxbuf.lens = S0.lens;

	usb_ctrl_stat = SETTING_UP; //接收到setup
	if(usb_req_rxbuf.lens == 0) //长度为0，不发东西
	{
		//USB_StatusIn
		USB_BT[0].COUNT_TX = 0;
		tx_stat=EP_TX_VALID;
		usb_ctrl_stat = WAIT_STATUS_IN;
	}
	else
	{
		RESULT r;
		r = class_data_setup(); //调用设备的处理
		if (r == USB_NOT_READY)
		{
			usb_ctrl_stat = PAUSE;
			goto END;
		}
		if ((r == USB_UNSUPPORT) || (p0_len == 0))
		{ //不支持的操作
			usb_ctrl_stat = STALLED;
			goto END;
		}
		if(usb_req_rxbuf.type.s.dir) //1设备到主机
		{
			if (p0_len > usb_req_rxbuf.lens) //若比主机请求的还长，按主机的
			{
				p0_len = usb_req_rxbuf.lens;
			}
			need_send_void = 0;
			if (p0_len >= usb_max_packet &&
				(p0_len % usb_max_packet) == 0) //大于等于最大包数，且是整数包
			{
				need_send_void = 1;
			}
			p0_send_poll();
		}
		else //0主机到设备
		{
			usb_ctrl_stat = OUT_DATA;
			rx_stat=EP_RX_VALID;
		}
	}
END:
	if (usb_ctrl_stat == STALLED)
	{
		rx_stat=(1<<12);
		tx_stat=(1<<4); //STALL
	}
}
void In0_Process(void) //收发状态的接口要用
{
	if ((usb_ctrl_stat == IN_DATA) || (usb_ctrl_stat == LAST_IN_DATA))
	{
		p0_send_poll();
	}
	else if(usb_ctrl_stat == WAIT_STATUS_IN)
	{
		if((usb_req_rxbuf.req == SET_ADDRESS) &&
			(usb_req_rxbuf.type.s.rx_type==0 && usb_req_rxbuf.type.s.req_type==0))
			//标准请求，接收者为设备
		{
			SetDeviceAddress(usb_req_rxbuf.vals.b.b1);
			usb_stat = USB_ATTACHED; //设置地址后，认为已经链接了
		}
		usb_ctrl_stat = STALLED;
	}
	else usb_ctrl_stat = STALLED;
	if (usb_ctrl_stat == STALLED)
	{
		rx_stat=(1<<12);
		tx_stat=(1<<4); //STALL
		//SetEPRxTxStatus(0,EP_RX_STALL,EP_TX_STALL); //恢复收发状态
	}
	//else SetEPRxStatus(0,EP_RX_VALID); //加上这句就不对了
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
void NOP_Process(void)
{
}
///////////////////////////////////////////////////////////////////////////////
//					中断部分
///////////////////////////////////////////////////////////////////////////////
extern void (*pEpInt_OUT[7])(void); //端点0没有
extern void (*pEpInt_IN[7])(void); //端点0没有
void (*usb_reset_fun)(void); //usb设备reset回调函数
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
				rx_stat = USB_EP(0) & EPRX_STAT; //只保存了发送状态，没有其他bit
				tx_stat = USB_EP(0) & EPTX_STAT;
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
						rx_stat=(1<<12);
						tx_stat=(1<<4); //STALL
					}
				}
				SetEPRxTxStatus(0,rx_stat,tx_stat); //恢复收发状态
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

