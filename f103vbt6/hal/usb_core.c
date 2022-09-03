
#include "f1_usb.h"
#include "usb_core.h"

u8 MaxPacketSize=64;

DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb设备状态，失去连接时要更新
S_USB_CTRL_STATE usb_ctrl_stat=IN_DATA; //usb控制状态
DEVICE_INFO	Device_Info;

RESULT (*Class_Data_Setup)(void)=0; //setup的处理回调函数

//数据收发的长度和位置
u16 p0_len=0; //剩余发送数
u16 p0_ind=0; //发送时的分批发送的位置
u8 *p0_p=0; //收发指针

//由于中断处理的时候需要设置NAK，所以要保存之前的状态
static vu16 rx_stat; //是接收中断与处理函数关于最后端点0状态的接口
static vu16 tx_stat; //是接收中断与处理函数关于最后端点0状态的接口

static int need_send_void = 0; //大于等于最大包数，且是整数包，需要发送空包
/* Private function prototypes -----------------------------------------------*/
static void p0_send_poll(void);
static void Data_Setup0(void);

/*******************************************************************************
 * Function Name  : Standard_GetDescriptorData.
 * Description    : Standard_GetDescriptorData is used for descriptors transfer.
 *                : This routine is used for the descriptors resident in Flash
 *                  or RAM
 *                  pDesc can be in either Flash or RAM
 *                  The purpose of this routine is to have a versatile way to
 *                  response descriptors request. It allows user to generate
 *                  certain descriptors with software or read descriptors from
 *                  external storage part by part.
 * Input          : - Length - Length of the data in this transfer.
 *                  - pDesc - A pointer points to descriptor struct.
 *                  The structure gives the initial address of the descriptor and
 *                  its original size.
 * Output         : None.
 * Return         : Address of a part of the descriptor pointed by the Usb_
 *                  wOffset The buffer pointed by this address contains at least
 *                  Length bytes.
 *******************************************************************************/
u8 *Standard_GetDescriptorData(u16 Length, ONE_DESCRIPTOR *pDesc)
{
	if (Length == 0)
	{
		p0_len = pDesc->Descriptor_Size - p0_ind;
		return 0;
	}
	return pDesc->Descriptor + p0_ind;
}
static void p0_send_poll(void) //控制端点的发送数据 
{
	u8 *DataBuffer;
	u32 Length;

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
	Length = MaxPacketSize; //先假设发最大包长
	usb_ctrl_stat = (p0_len <= Length) ? LAST_IN_DATA : IN_DATA; //是否是最后一包

	if (Length > p0_len) Length = p0_len; //发送不超过包长

	if(p0_p) //若是测试
	{
		UserToPMABufferCopy(p0_p, USB_BT[0].ADDR_TX, Length);
		USB_BT[0].COUNT_TX = Length;
		p0_len -= Length;
		p0_p+=Length;
	}
	else //
	{
		DataBuffer = (Device_Info.Ctrl_Info.CopyData)(Length); //调用发送回调，准备数据
		UserToPMABufferCopy(DataBuffer, USB_BT[0].ADDR_TX, Length);
		USB_BT[0].COUNT_TX = Length;
		p0_len -= Length;
		p0_ind += Length;
	}

	tx_stat=EP_TX_VALID;
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
	Device_Info.type.b = S0.type;
	Device_Info.req = S0.req;
	Device_Info.vals.w = CHANGE_END16(S0.vals.w);
	Device_Info.inds.w = CHANGE_END16(S0.inds.w);
	Device_Info.lens = S0.lens;

	usb_ctrl_stat = SETTING_UP; //接收到setup
	if(Device_Info.lens == 0) //长度为0，不发东西
	{
		//USB_StatusIn
		USB_BT[0].COUNT_TX = 0;
		tx_stat=EP_TX_VALID;
		usb_ctrl_stat = WAIT_STATUS_IN;
	}
	else
	{
		RESULT r;
		r = Class_Data_Setup(); //调用设备的处理
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
		if(Device_Info.type.s.dir) //1设备到主机
		{
			if (p0_len > Device_Info.lens) //若比主机请求的还长，按主机的
			{
				p0_len = Device_Info.lens;
			}
			need_send_void = 0;
			if (p0_len >= MaxPacketSize &&
				(p0_len % MaxPacketSize) == 0) //大于等于最大包数，且是整数包
			{
				need_send_void = 1;
			}
			p0_send_poll();
		}
		else //0主机到设备
		{
			usb_ctrl_stat = OUT_DATA;
			rx_stat=EP_RX_VALID; /* enable for next data reception */
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
		if((Device_Info.req == SET_ADDRESS) &&
			(Device_Info.type.s.rx_type==0 && Device_Info.type.s.req_type==0))
			//标准请求，接收者为设备
		{
			SetDeviceAddress(Device_Info.vals.b.b1);
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
void Out0_Process(void)
{
	if ((usb_ctrl_stat == OUT_DATA) || (usb_ctrl_stat == LAST_OUT_DATA))
	{
		u32 save_rLength;
		save_rLength = p0_len;
		if (Device_Info.Ctrl_Info.CopyData && save_rLength)
		{
			u8 *Buffer;
			u32 Length;

			Length = MaxPacketSize; //总数据长度
			if (Length > save_rLength)
			{
				Length = save_rLength;
			}

			Buffer = (Device_Info.Ctrl_Info.CopyData)(Length);
			p0_len -= Length;
			p0_ind += Length;
			PMAToUserBufferCopy(Buffer, USB_BT[0].ADDR_RX, Length);
		}

		if (p0_len != 0)
		{
			rx_stat=EP_RX_VALID;/* re-enable for next data reception */
			USB_BT[0].COUNT_TX = 0;
			tx_stat=EP_TX_VALID;/* Expect the host to abort the data OUT stage */
		}
		/* Set the next State*/
		if (p0_len >= MaxPacketSize)
		{
			usb_ctrl_stat = OUT_DATA;
		}
		else
		{
			if (p0_len > 0)
			{
				usb_ctrl_stat = LAST_OUT_DATA;
			}
			else if (p0_len == 0)
			{
				usb_ctrl_stat = WAIT_STATUS_IN;
				//USB_StatusIn();
				USB_BT[0].COUNT_TX = 0;
				tx_stat=EP_TX_VALID; 
			}
		}
	}
	else //未定义状态，设置STALL
	{
		usb_ctrl_stat = STALLED;
	}
	if (usb_ctrl_stat == STALLED)
	{
		rx_stat=(1<<12);
		tx_stat=(1<<4); //STALL
	}
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
extern void Joystick_Reset(void);
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
		Joystick_Reset();
	}
	if (t & (1<<13)) USB->ISTR=(u16)~(1<<13); //ERR 主要用于开发阶段，测量传输质量
	if (t & (1<<12)) //WKUP唤醒
	{
		USB->ISTR=(u16)~(1<<12);
	}
	if (t & (1<<11)) //SUSP挂起
	{
		USB->ISTR=(u16)~(1<<11); //必须在CNTR_FSUSP之后
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
						Out0_Process();
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

