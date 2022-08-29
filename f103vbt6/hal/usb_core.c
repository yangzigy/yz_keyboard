
#include "f1_usb.h"
#include "usb_core.h"

DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb状态
u8 MaxPacketSize=64;

DEVICE_INFO	Device_Info;

int Data_Mul_MaxPacketSize = 0;
/* Private function prototypes -----------------------------------------------*/
static void DataStageOut(void);
static void DataStageIn(void);
static void NoData_Setup0(void);
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
		Device_Info.ControlState = OUT_DATA;
	}
	else
	{
		if (Device_Info.Ctrl_Info.Usb_wLength > 0)
		{
			Device_Info.ControlState = LAST_OUT_DATA;
		}
		else if (Device_Info.Ctrl_Info.Usb_wLength == 0)
		{
			Device_Info.ControlState = WAIT_STATUS_IN;
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

	if ((Device_Info.Ctrl_Info.Usb_wLength == 0) && (Device_Info.ControlState == LAST_IN_DATA)) //如果是最后一包
	{
		if(Data_Mul_MaxPacketSize) //没有数据，发送空包
		{
			USB_BT[0].COUNT_TX = 0;
			SaveTState=EP_TX_VALID; 

			Device_Info.ControlState = LAST_IN_DATA;
			Data_Mul_MaxPacketSize = 0;
		}
		else //没有数据了
		{
			Device_Info.ControlState = WAIT_STATUS_OUT;
			SaveTState=(1<<4); //TX_STALL
		}
		return ;
	}
	Length = Device_Info.Ctrl_Info.PacketSize; //按全长赋值
	Device_Info.ControlState = (Device_Info.Ctrl_Info.Usb_wLength <= Length) ? LAST_IN_DATA : IN_DATA; //确定是否是最后一包

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
 * Function Name  : NoData_Setup0.
 * Description    : Proceed the processing of setup request without data stage.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void NoData_Setup0(void)
{
	if(Device_Info.type.s.req_type) goto UNSUPPORT; //不是标准请求
	if(Device_Info.type.s.rx_type==0) //接收者为设备
	{
		if (Device_Info.req == SET_ADDRESS) //设置地址
		{
			if ((Device_Info.vals.b.b1 > 127) || (Device_Info.vals.b.b0 != 0)
					|| (Device_Info.inds.w != 0))
				/* Device Address should be 127 or less*/
			{
				Device_Info.ControlState = STALLED;
				return ;
			}
		}
	}
UNSUPPORT: //认为一定成功，那个r没用
	//USB_StatusIn
	USB_BT[0].COUNT_TX = 0;
	SaveTState=EP_TX_VALID;
	Device_Info.ControlState = WAIT_STATUS_IN;
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
	u8 *(*CopyRoutine)(u16);
	RESULT r;

	u32 Related_Endpoint, Reserved;
	u32 Status;
	CopyRoutine = 0;
	/*GET DESCRIPTOR*/
	if (Device_Info.req == GET_DESCRIPTOR)
	{
		if(Device_Info.type.s.rx_type==0 && Device_Info.type.s.req_type==0) //标准请求，接收者为设备
		{
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
			Device_Info.ControlState = PAUSE;
			return;
		}
	}
	if (Device_Info.Ctrl_Info.Usb_wLength == 0xFFFF)
	{
		/* Data is not ready, wait it */
		Device_Info.ControlState = PAUSE;
		return;
	}
	if ((r == USB_UNSUPPORT) || (Device_Info.Ctrl_Info.Usb_wLength == 0))
	{ //不支持的操作
		Device_Info.ControlState = STALLED;
		return;
	}
	if(Device_Info.type.s.dir) //1设备到主机
	{
		vu32 wLength = Device_Info.lens;
		/* Restrict the data length to be the one host asks for */
		if (Device_Info.Ctrl_Info.Usb_wLength > wLength)
		{
			Device_Info.Ctrl_Info.Usb_wLength = wLength;
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
		Device_Info.ControlState = OUT_DATA;
		SaveRState=EP_RX_VALID; /* enable for next data reception */
	}
	return;
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Function Name  : Setup0_Process
 * Description    : Get the device request data and dispatch to individual process.
 * Input          : None.
 * Output         : None.
 * Return         : Post0_Process.
 *******************************************************************************/
//端点0的接收缓存，换算成CPU的32bit地址
//看做SETUP0的接收结构
#define S0 (*(S_SETUP0_RX*)(PMAAddr + (u8 *)(USB_BT[0].ADDR_RX * 2))) 
u8 Setup0_Process(void)
{
	Device_Info.type.b = S0.type;
	Device_Info.req = S0.req;
	Device_Info.vals.w = CHANGE_END16(S0.vals.w);
	Device_Info.inds.w = CHANGE_END16(S0.inds.w);
	Device_Info.lens = S0.lens; 

	Device_Info.ControlState = SETTING_UP;
	if (Device_Info.lens == 0) NoData_Setup0();
	else Data_Setup0();

	return Post0_Process();
}

/*******************************************************************************
 * Function Name  : In0_Process
 * Description    : Process the IN token on all default endpoint.
 * Input          : None.
 * Output         : None.
 * Return         : Post0_Process.
 *******************************************************************************/
u8 In0_Process(void)
{
	u32 ControlState = Device_Info.ControlState;

	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA))
	{
		DataStageIn();
		ControlState = Device_Info.ControlState; //外部可能改变
	}
	else if(ControlState == WAIT_STATUS_IN)
	{
		if((Device_Info.req == SET_ADDRESS) &&
			(Device_Info.type.s.rx_type==0 && Device_Info.type.s.req_type==0))
			//标准请求，接收者为设备
		{
			SetDeviceAddress(Device_Info.vals.b.b1);
		}
		ControlState = STALLED;
	}
	else
	{
		ControlState = STALLED;
	}
	Device_Info.ControlState = ControlState;
	return Post0_Process();
}
/*******************************************************************************
 * Function Name  : Out0_Process
 * Description    : Process the OUT token on all default endpoint.
 * Input          : None.
 * Output         : None.
 * Return         : Post0_Process.
 *******************************************************************************/
u8 Out0_Process(void)
{
	u32 ControlState = Device_Info.ControlState;

	if ((ControlState == IN_DATA) || (ControlState == LAST_IN_DATA))
	{
		ControlState = STALLED; //host关闭传输
	}
	else if ((ControlState == OUT_DATA) || (ControlState == LAST_OUT_DATA))
	{
		DataStageOut();
		ControlState = Device_Info.ControlState; /* may be changed outside the function */
	}
	else if (ControlState == WAIT_STATUS_OUT)
	{
		ControlState = STALLED;
	}
	else //未定义状态，设置STALL
	{
		ControlState = STALLED;
	}
	Device_Info.ControlState = ControlState;
	return Post0_Process();
}

/*******************************************************************************
 * Function Name  : Post0_Process
 * Description    : Stall the Endpoint 0 in case of error.
 * Input          : None.
 * Output         : None.
 * Return         : - 0 if the control State is in PAUSE
 *                  - 1 if not.
 *******************************************************************************/
u8 Post0_Process(void)
{
	SetEPRxCount(0, MaxPacketSize);
	if (Device_Info.ControlState == STALLED)
	{
		SaveRState=(1<<12);
		SaveTState=(1<<4); //STALL
	}
	return (Device_Info.ControlState == PAUSE);
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

