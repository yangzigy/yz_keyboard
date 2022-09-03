
#include "f1_usb.h"
#include "usb_core.h"

u8 MaxPacketSize=64;

DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb设备状态，失去连接时要更新
S_USB_CTRL_STATE usb_ctrl_stat=IN_DATA; //usb控制状态
DEVICE_INFO	Device_Info;

int Data_Mul_MaxPacketSize = 0;
/* Private function prototypes -----------------------------------------------*/
static void DataStageOut(void);
static void DataStageIn(void);
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
		Device_Info.Ctrl_Info.Usb_wLength = pDesc->Descriptor_Size - Device_Info.Ctrl_Info.Usb_wOffset;
		return 0;
	}
	return pDesc->Descriptor + Device_Info.Ctrl_Info.Usb_wOffset;
}

/*******************************************************************************
 * Function Name  : DataStageOut.
 * Description    : Data stage of a Control Write Transfer.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void DataStageOut(void)
{
	u32 save_rLength;
	save_rLength = Device_Info.Ctrl_Info.Usb_wLength;
	if (Device_Info.Ctrl_Info.CopyData && save_rLength)
	{
		u8 *Buffer;
		u32 Length;

		Length = Device_Info.Ctrl_Info.PacketSize; //总数据长度
		if (Length > save_rLength)
		{
			Length = save_rLength;
		}

		Buffer = (Device_Info.Ctrl_Info.CopyData)(Length);
		Device_Info.Ctrl_Info.Usb_wLength -= Length;
		Device_Info.Ctrl_Info.Usb_wOffset += Length;
		PMAToUserBufferCopy(Buffer, USB_BT[0].ADDR_RX, Length);
	}

	if (Device_Info.Ctrl_Info.Usb_wLength != 0)
	{
		SaveRState=EP_RX_VALID;/* re-enable for next data reception */
		USB_BT[0].COUNT_TX = 0;
		SaveTState=EP_TX_VALID;/* Expect the host to abort the data OUT stage */
	}
	/* Set the next State*/
	if (Device_Info.Ctrl_Info.Usb_wLength >= Device_Info.Ctrl_Info.PacketSize)
	{
		usb_ctrl_stat = OUT_DATA;
	}
	else
	{
		if (Device_Info.Ctrl_Info.Usb_wLength > 0)
		{
			usb_ctrl_stat = LAST_OUT_DATA;
		}
		else if (Device_Info.Ctrl_Info.Usb_wLength == 0)
		{
			usb_ctrl_stat = WAIT_STATUS_IN;
			//USB_StatusIn();
			USB_BT[0].COUNT_TX = 0;
			SaveTState=EP_TX_VALID; 
		}
	}
}
void DataStageIn(void) //控制端点的发送数据 
{
	u8 *DataBuffer;
	u32 Length;

	if ((Device_Info.Ctrl_Info.Usb_wLength == 0) && (usb_ctrl_stat == LAST_IN_DATA)) //如果是最后一包
	{
		if(Data_Mul_MaxPacketSize) //没有数据，发送空包
		{
			USB_BT[0].COUNT_TX = 0;
			SaveTState=EP_TX_VALID; 

			usb_ctrl_stat = LAST_IN_DATA;
			Data_Mul_MaxPacketSize = 0;
		}
		else //没有数据了
		{
			usb_ctrl_stat = WAIT_STATUS_OUT;
			SaveTState=(1<<4); //TX_STALL
		}
		return ;
	}
	Length = Device_Info.Ctrl_Info.PacketSize; //按全长赋值
	usb_ctrl_stat = (Device_Info.Ctrl_Info.Usb_wLength <= Length) ? LAST_IN_DATA : IN_DATA; //确定是否是最后一包

	if (Length > Device_Info.Ctrl_Info.Usb_wLength) Length = Device_Info.Ctrl_Info.Usb_wLength;
	DataBuffer = (Device_Info.Ctrl_Info.CopyData)(Length); //调用发送回调，准备数据

	UserToPMABufferCopy(DataBuffer, USB_BT[0].ADDR_TX, Length);

	USB_BT[0].COUNT_TX = Length;

	Device_Info.Ctrl_Info.Usb_wLength -= Length;
	Device_Info.Ctrl_Info.Usb_wOffset += Length;
	SaveTState=EP_TX_VALID;

	SaveRState = EP_RX_VALID; //USB_StatusOut//期望host退出data IN
}
/*******************************************************************************
 * Function Name  : Data_Setup0.
 * Description    : Proceed the processing of setup request with data stage.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Data_Setup0(void)
{
	u8 *(*CopyRoutine)(u16)=0;
	RESULT r;
	if (Device_Info.req == GET_DESCRIPTOR) //获取描述符
	{
		if(Device_Info.type.s.rx_type==0 && Device_Info.type.s.req_type==0) //标准请求，接收者为设备
		{ //这个函数必须是进这三个之一，否则后边就没意义
			u8 wValue1 = Device_Info.vals.b.b0;
			if (wValue1 == DEVICE_DESCRIPTOR)
			{
				CopyRoutine = Device_Property.GetDeviceDescriptor;
			}
			else if (wValue1 == CONFIG_DESCRIPTOR)
			{
				CopyRoutine = Device_Property.GetConfigDescriptor;
			}
			else if (wValue1 == STRING_DESCRIPTOR)
			{
				CopyRoutine = Device_Property.GetStringDescriptor;
			}
		}
	}
	if (CopyRoutine)
	{
		Device_Info.Ctrl_Info.Usb_wOffset = 0;
		Device_Info.Ctrl_Info.CopyData = CopyRoutine;
		/* sb in the original the cast to word was directly */
		/* now the cast is made step by step */
		(*CopyRoutine)(0);
		r = USB_SUCCESS;
	}
	else
	{
		r = (Device_Property.Class_Data_Setup)(Device_Info.req); //调用设备的处理
		if (r == USB_NOT_READY)
		{
			usb_ctrl_stat = PAUSE;
			return;
		}
	}
	if (Device_Info.Ctrl_Info.Usb_wLength == 0xFFFF)
	{
		usb_ctrl_stat = PAUSE; //等数据准备好
		return;
	}
	if ((r == USB_UNSUPPORT) || (Device_Info.Ctrl_Info.Usb_wLength == 0))
	{ //不支持的操作
		usb_ctrl_stat = STALLED;
		return;
	}
	if(Device_Info.type.s.dir) //1设备到主机
	{
		/* Restrict the data length to be the one host asks for */
		if (Device_Info.Ctrl_Info.Usb_wLength > Device_Info.lens)
		{
			Device_Info.Ctrl_Info.Usb_wLength = Device_Info.lens;
		}
		else if (Device_Info.Ctrl_Info.Usb_wLength < Device_Info.lens)
		{
			if (Device_Info.Ctrl_Info.Usb_wLength < MaxPacketSize)
			{
				Data_Mul_MaxPacketSize = 0;
			}
			else if ((Device_Info.Ctrl_Info.Usb_wLength % MaxPacketSize) == 0)
			{
				Data_Mul_MaxPacketSize = 1;
			}
		}   
		Device_Info.Ctrl_Info.PacketSize = MaxPacketSize;
		DataStageIn();
	}
	else //0主机到设备
	{
		usb_ctrl_stat = OUT_DATA;
		SaveRState=EP_RX_VALID; /* enable for next data reception */
	}
	return;
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
//端点0的接收缓存，换算成CPU的32bit地址
//看做SETUP0的接收结构
#define S0 (*(S_SETUP0_RX*)(PMAAddr + (u8 *)(USB_BT[0].ADDR_RX * 2))) 
void Setup0_Process(void)
{
	Device_Info.type.b = S0.type;
	Device_Info.req = S0.req;
	Device_Info.vals.w = CHANGE_END16(S0.vals.w);
	Device_Info.inds.w = CHANGE_END16(S0.inds.w);
	Device_Info.lens = S0.lens; 

	usb_ctrl_stat = SETTING_UP;
	if (Device_Info.lens == 0)
	{
		//USB_StatusIn
		USB_BT[0].COUNT_TX = 0;
		SaveTState=EP_TX_VALID;
		usb_ctrl_stat = WAIT_STATUS_IN;
	}
	else Data_Setup0();

	SetEPRxCount(0, MaxPacketSize);
	if (usb_ctrl_stat == STALLED)
	{
		SaveRState=(1<<12);
		SaveTState=(1<<4); //STALL
	}
}
void In0_Process(void)
{
	if ((usb_ctrl_stat == IN_DATA) || (usb_ctrl_stat == LAST_IN_DATA))
	{
		DataStageIn();
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
	else
	{
		usb_ctrl_stat = STALLED;
	}
	SetEPRxCount(0, MaxPacketSize);
	if (usb_ctrl_stat == STALLED)
	{
		SaveRState=(1<<12);
		SaveTState=(1<<4); //STALL
	}
}
void Out0_Process(void)
{
	if ((usb_ctrl_stat == OUT_DATA) || (usb_ctrl_stat == LAST_OUT_DATA))
	{
		DataStageOut();
	}
	else //未定义状态，设置STALL
	{
		usb_ctrl_stat = STALLED;
	}
	SetEPRxCount(0, MaxPacketSize);
	if (usb_ctrl_stat == STALLED)
	{
		SaveRState=(1<<12);
		SaveTState=(1<<4); //STALL
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
vu16 SaveRState; //是接收中断与处理函数关于最后端点0状态的接口
vu16 SaveTState; //是接收中断与处理函数关于最后端点0状态的接口

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
				// save RX & TX status  and set both to NAK 
				SaveRState = USB_EP(0);
				SaveTState = SaveRState & EPTX_STAT;
				SaveRState &=  EPRX_STAT;	
				SetEPRxTxStatus(0,EP_RX_NAK,EP_TX_NAK); //先保存了收发状态，设置成NAK，再恢复
				/* DIR bit = origin of the interrupt */
				if ((t & (1<<4)) == 0) //DIR 0:IN, 1:OUT
				{
					/* DIR = 0      => IN  int */
					/* DIR = 0 implies that (EP_CTR_TX = 1) always  */
					USB_EP(0)=USB_EP(0) & (~(1<<7)) & EPREG_MASK; //清除CTR_TX
					In0_Process();
					/* before terminate set Tx & Rx status */
					SetEPRxTxStatus(0,SaveRState,SaveTState);
					return;
				}
				else
				{
					/* DIR = 1 & CTR_RX       => SETUP or OUT int */
					/* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
					ep_reg = USB_EP(0);
					if ((ep_reg & EP_SETUP) != 0)
					{
						USB_EP(0)=USB_EP(0) & (~(1<<15)) & EPREG_MASK; //清除CTR_RX
						Setup0_Process();
						/* before terminate set Tx & Rx status */
						SetEPRxTxStatus(0,SaveRState,SaveTState);
						return;
					}
					else if ((ep_reg & (1<<15)) != 0) //CTR_RX正确接收
					{
						USB_EP(0)=USB_EP(0) & (~(1<<15)) & EPREG_MASK; //清除CTR_RX
						Out0_Process();
						/* before terminate set Tx & Rx status */
						SetEPRxTxStatus(0,SaveRState,SaveTState);
						return;
					}
				}
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

