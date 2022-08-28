
#include "f1_usb.h"
#include "usb_core.h"

DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb状态
u8 MaxPacketSize=64;

DEVICE_INFO	Device_Info;

#define ValBit(VAR,Place)    (VAR & (1 << Place))
#define SetBit(VAR,Place)    (VAR |= (1 << Place))
#define ClrBit(VAR,Place)    (VAR &= ((1 << Place) ^ 255))

S_U16_U8 StatusInfo;

int Data_Mul_MaxPacketSize = 0;
/* Private function prototypes -----------------------------------------------*/
static void DataStageOut(void);
static void DataStageIn(void);
static void NoData_Setup0(void);
static void Data_Setup0(void);

/*******************************************************************************
 * Function Name  : Standard_GetConfiguration.
 * Description    : Return the current configuration variable address.
 * Input          : Length - How many bytes are needed.
 * Output         : None.
 * Return         : Return 1 , if the request is invalid when "Length" is 0.
 *                  Return "Buffer" if the "Length" is not 0.
 *******************************************************************************/
u8 *Standard_GetConfiguration(u16 Length)
{
	if (Length == 0)
	{
		Device_Info.Ctrl_Info.Usb_wLength =1;
		return 0;
	}
	return 0;
}
/*******************************************************************************
 * Function Name  : Standard_SetConfiguration.
 * Description    : This routine is called to set the configuration value
 *                  Then each class should configure device itself.
 * Input          : None.
 * Output         : None.
 * Return         : Return USB_SUCCESS, if the request is performed.
 *                  Return USB_UNSUPPORT, if the request is invalid.
 *******************************************************************************/
RESULT Standard_SetConfiguration(void)
{
	if ((Device_Info.vals.b.b1 <= 1) && (Device_Info.vals.b.b0 == 0)
			&& (Device_Info.inds.w == 0)) /*call Back usb spec 2.0*/
	{
		Device_Info.Current_Configuration = Device_Info.vals.b.b1;
		return USB_SUCCESS;
	}
	else
	{
		return USB_UNSUPPORT;
	}
}
/*******************************************************************************
 * Function Name  : Standard_GetInterface.
 * Description    : Return the Alternate Setting of the current interface.
 * Input          : Length - How many bytes are needed.
 * Output         : None.
 * Return         : Return 0, if the request is invalid when "Length" is 0.
 *                  Return "Buffer" if the "Length" is not 0.
 *******************************************************************************/
u8 *Standard_GetInterface(u16 Length)
{
	if (Length == 0)
	{
		Device_Info.Ctrl_Info.Usb_wLength = sizeof(Device_Info.Current_AlternateSetting);
		return 0;
	}
	return (u8 *)&Device_Info.Current_AlternateSetting;
}

/*******************************************************************************
 * Function Name  : Standard_SetInterface.
 * Description    : This routine is called to set the interface.
 *                  Then each class should configure the interface them self.
 * Input          : None.
 * Output         : None.
 * Return         : - Return USB_SUCCESS, if the request is performed.
 *                  - Return USB_UNSUPPORT, if the request is invalid.
 *******************************************************************************/
RESULT Standard_SetInterface(void)
{
	RESULT Re;
	/*Test if the specified Interface and Alternate Setting are supported by
	  the application Firmware*/
	Re = (Device_Property.Class_Get_Interface_Setting)(Device_Info.inds.b.b1, Device_Info.vals.b.b1);
	if (Device_Info.Current_Configuration != 0)
	{
		if ((Re != USB_SUCCESS) || (Device_Info.inds.b.b0 != 0)
				|| (Device_Info.vals.b.b0 != 0))
		{
			return  USB_UNSUPPORT;
		}
		else if (Re == USB_SUCCESS)
		{
			Device_Info.Current_Interface = Device_Info.inds.b.b1;
			Device_Info.Current_AlternateSetting = Device_Info.vals.b.b1;
			return USB_SUCCESS;
		}
	}
	return USB_UNSUPPORT;
}
/*******************************************************************************
 * Function Name  : Standard_GetStatus.
 * Description    : Copy the device request data to "StatusInfo buffer".
 * Input          : - Length - How many bytes are needed.
 * Output         : None.
 * Return         : Return 0, if the request is at end of data block,
 *                  or is invalid when "Length" is 0.
 *******************************************************************************/
u8 *Standard_GetStatus(u16 Length)
{
	if (Length == 0)
	{
		Device_Info.Ctrl_Info.Usb_wLength = 2;
		return 0;
	}
	/* Reset Status Information */
	StatusInfo.w = 0;
	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
	{
		/*Get Device Status */
		u8 Feature = Device_Info.Current_Feature;
		/* Remote Wakeup enabled */
		if (ValBit(Feature, 5))
		{
			SetBit(StatusInfo.b.b0, 1);
		}
		else
		{
			ClrBit(StatusInfo.b.b0, 1);
		}      

		/* Bus-powered */
		if (ValBit(Feature, 6))
		{
			SetBit(StatusInfo.b.b0, 0);
		}
		else /* Self-powered */
		{
			ClrBit(StatusInfo.b.b0, 0);
		}
	}
	/*Interface Status*/
	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
	{
		return (u8 *)&StatusInfo;
	}
	/*Get EndPoint Status*/
	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
	{
		u8 Related_Endpoint;
		u8 wIndex0 = Device_Info.inds.b.b1;

		Related_Endpoint = (wIndex0 & 0x0f);
		if (ValBit(wIndex0, 7))
		{
			/* IN endpoint */
			if (USB_EP(Related_Endpoint)==(1<<4)) //TX_STALL
			{
				SetBit(StatusInfo.b.b0, 0); /* IN Endpoint stalled */
			}
		}
		else //OUT
		{
			if ((USB_EP(Related_Endpoint) & (3<<12)) == (1<<12))//RX_STALL
			{
				SetBit(StatusInfo.b.b0, 0); /* OUT Endpoint stalled */
			}
		}

	}
	else return 0;
	return (u8 *)&StatusInfo;
}

/*******************************************************************************
 * Function Name  : Standard_ClearFeature.
 * Description    : Clear or disable a specific feature.
 * Input          : None.
 * Output         : None.
 * Return         : - Return USB_SUCCESS, if the request is performed.
 *                  - Return USB_UNSUPPORT, if the request is invalid.
 *******************************************************************************/
RESULT Standard_ClearFeature(void)
{
	u32     Type_Rec = Type_Recipient;
	u32     Status;
	if (Type_Rec == (STANDARD_REQUEST | DEVICE_RECIPIENT))
	{/*Device Clear Feature*/
		ClrBit(Device_Info.Current_Feature, 5);
		return USB_SUCCESS;
	}
	else if (Type_Rec == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
	{/*EndPoint Clear Feature*/
		u32 Related_Endpoint;
		u32 wIndex0;
		u32 rEP;

		if ((Device_Info.vals.w != ENDPOINT_STALL)
				|| (Device_Info.inds.b.b0 != 0))
		{
			return USB_UNSUPPORT;
		}

		wIndex0 = Device_Info.inds.b.b1;
		rEP = wIndex0 & ~0x80;
		Related_Endpoint = 0 + rEP;

		if (ValBit(Device_Info.inds.b.b1, 7))
		{
			/*Get Status of endpoint & stall the request if the related_ENdpoint
			  is Disabled*/
			Status = USB_EP(Related_Endpoint) & (3<<4);
		}
		else
		{
			Status = USB_EP(Related_Endpoint) & (3<<12);
		}

		if ((rEP >= EP_num) || (Status == 0)
				|| (Device_Info.Current_Configuration == 0))
		{
			return USB_UNSUPPORT;
		}
		if (wIndex0 & 0x80) //IN
		{
			if (USB_EP(Related_Endpoint)==(1<<4)) //TX_STALL
			{
				ClearDTOG_TX(Related_Endpoint);
				SetEPTxStatus(Related_Endpoint, EP_TX_VALID);
			}
		}
		else //OUT
		{
			if ((USB_EP(Related_Endpoint) & (3<<12)) == (1<<12))//RX_STALL
			{
				if (Related_Endpoint == 0)
				{
					/* After clear the STALL, enable the default endpoint receiver */
					SetEPRxCount(Related_Endpoint, MaxPacketSize);
					SetEPRxStatus(Related_Endpoint, (3<<12));
				}
				else
				{
					ClearDTOG_RX(Related_Endpoint);
					SetEPRxStatus(Related_Endpoint, (3<<12));
				}
			}
		}
		return USB_SUCCESS;
	}
	return USB_UNSUPPORT;
}

/*******************************************************************************
 * Function Name  : Standard_SetEndPointFeature
 * Description    : Set or enable a specific feature of EndPoint
 * Input          : None.
 * Output         : None.
 * Return         : - Return USB_SUCCESS, if the request is performed.
 *                  - Return USB_UNSUPPORT, if the request is invalid.
 *******************************************************************************/
RESULT Standard_SetEndPointFeature(void)
{
	u32    wIndex0;
	u32    Related_Endpoint;
	u32    rEP;
	u32    Status;

	wIndex0 = Device_Info.inds.b.b1;
	rEP = wIndex0 & ~0x80;
	Related_Endpoint = 0 + rEP;

	if (ValBit(Device_Info.inds.b.b1, 7))
	{
		/* get Status of endpoint & stall the request if the related_ENdpoint
		   is Disabled*/
		Status = USB_EP(Related_Endpoint) & (3<<4);
	}
	else
	{
		Status = USB_EP(Related_Endpoint) & (3<<12);
	}

	if (Related_Endpoint >= EP_num
			|| Device_Info.vals.w != 0 || Status == 0
			|| Device_Info.Current_Configuration == 0)
	{
		return USB_UNSUPPORT;
	}
	else
	{
		if (wIndex0 & 0x80)
		{
			/* IN endpoint */
			SetEPTxStatus(Related_Endpoint, (1<<4)); //TX_STALL
		}
		else
		{
			/* OUT endpoint */
			SetEPRxStatus(Related_Endpoint, (1<<12)); //RX_STALL
		}
	}
	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Standard_SetDeviceFeature.
 * Description    : Set or enable a specific feature of Device.
 * Input          : None.
 * Output         : None.
 * Return         : - Return USB_SUCCESS, if the request is performed.
 *                  - Return USB_UNSUPPORT, if the request is invalid.
 *******************************************************************************/
RESULT Standard_SetDeviceFeature(void)
{
	SetBit(Device_Info.Current_Feature, 5);
	return USB_SUCCESS;
}
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
/*******************************************************************************
 * Function Name  : DataStageIn.
 * Description    : Data stage of a Control Read Transfer.
 *******************************************************************************/
void DataStageIn(void)
{
	u32 save_wLength = Device_Info.Ctrl_Info.Usb_wLength;
	u32 ControlState = Device_Info.ControlState;

	u8 *DataBuffer;
	u32 Length;

	if ((save_wLength == 0) && (ControlState == LAST_IN_DATA))
	{
		if(Data_Mul_MaxPacketSize) //没有数据，发送空包
		{
			USB_BT[0].COUNT_TX = 0;
			SaveTState=EP_TX_VALID; 

			ControlState = LAST_IN_DATA;
			Data_Mul_MaxPacketSize = 0;
		}
		else //没有数据了
		{
			ControlState = WAIT_STATUS_OUT;
			SaveTState=(1<<4); //TX_STALL
		}
		goto Expect_Status_Out;
	}
	Length = Device_Info.Ctrl_Info.PacketSize;
	ControlState = (save_wLength <= Length) ? LAST_IN_DATA : IN_DATA;

	if (Length > save_wLength) Length = save_wLength;
	DataBuffer = (Device_Info.Ctrl_Info.CopyData)(Length);

	UserToPMABufferCopy(DataBuffer, USB_BT[0].ADDR_TX, Length);

	USB_BT[0].COUNT_TX = Length;

	Device_Info.Ctrl_Info.Usb_wLength -= Length;
	Device_Info.Ctrl_Info.Usb_wOffset += Length;
	SaveTState=EP_TX_VALID;

	SaveRState = EP_RX_VALID; //USB_StatusOut//期望host退出data IN
Expect_Status_Out:
	Device_Info.ControlState = ControlState;
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
	RESULT r = USB_UNSUPPORT;
	u32 RequestNo = Device_Info.USBbRequest;
	u32 ControlState;

	if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
	{
		/* Device Request*/
		/* SET_CONFIGURATION*/
		if (RequestNo == SET_CONFIGURATION)
		{
			r = Standard_SetConfiguration();
		}
		/*SET ADDRESS*/
		else if (RequestNo == SET_ADDRESS)
		{
			if ((Device_Info.vals.b.b1 > 127) || (Device_Info.vals.b.b0 != 0)
					|| (Device_Info.inds.w != 0)
					|| (Device_Info.Current_Configuration != 0))
				/* Device Address should be 127 or less*/
			{
				ControlState = STALLED;
				goto exit_NoData_Setup0;
			}
			else
			{
				r = USB_SUCCESS;
			}
		}
		/*SET FEATURE for Device*/
		else if (RequestNo == SET_FEATURE)
		{
			if ((Device_Info.vals.b.b1 == DEVICE_REMOTE_WAKEUP) \
					&& (Device_Info.inds.w == 0))
			{
				r = Standard_SetDeviceFeature();
			}
			else
			{
				r = USB_UNSUPPORT;
			}
		}
		/*Clear FEATURE for Device */
		else if (RequestNo == CLEAR_FEATURE)
		{
			if (Device_Info.vals.b.b1 == DEVICE_REMOTE_WAKEUP
					&& Device_Info.inds.w == 0
					&& ValBit(Device_Info.Current_Feature, 5))
			{
				r = Standard_ClearFeature();
			}
			else
			{
				r = USB_UNSUPPORT;
			}
		}
	}
	/* Interface Request*/
	else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
	{
		/*SET INTERFACE*/
		if (RequestNo == SET_INTERFACE)
		{
			r = Standard_SetInterface();
		}
	}
	/* EndPoint Request*/
	else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
	{
		/*CLEAR FEATURE for EndPoint*/
		if (RequestNo == CLEAR_FEATURE)
		{
			r = Standard_ClearFeature();
		}
		/* SET FEATURE for EndPoint*/
		else if (RequestNo == SET_FEATURE)
		{
			r = Standard_SetEndPointFeature();
		}
	}
	else
	{
		r = USB_UNSUPPORT;
	}


	if (r != USB_SUCCESS)
	{
		r = (Device_Property.Class_NoData_Setup)(RequestNo);
		if (r == USB_NOT_READY)
		{
			ControlState = PAUSE;
			goto exit_NoData_Setup0;
		}
	}

	if (r != USB_SUCCESS)
	{
		ControlState = STALLED;
		goto exit_NoData_Setup0;
	}

	ControlState = WAIT_STATUS_IN;/* After no data stage SETUP */

	//USB_StatusIn
	USB_BT[0].COUNT_TX = 0;
	SaveTState=EP_TX_VALID;
exit_NoData_Setup0:
	Device_Info.ControlState = ControlState;
	return;
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
	u32 Request_No = Device_Info.USBbRequest;

	u32 Related_Endpoint, Reserved;
	u32 Status;
	CopyRoutine = 0;
	/*GET DESCRIPTOR*/
	if (Request_No == GET_DESCRIPTOR)
	{
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
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
	/*GET STATUS*/
	else if ((Request_No == GET_STATUS) && (Device_Info.vals.w == 0)
			&& (Device_Info.lens.w == 0x0002)
			&& (Device_Info.inds.b.b0 == 0))
	{
		/* GET STATUS for Device*/
		if ((Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
				&& (Device_Info.inds.w == 0))
		{
			CopyRoutine = Standard_GetStatus;
		}

		/* GET STATUS for Interface*/
		else if (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
		{
			if (((Device_Property.Class_Get_Interface_Setting)(Device_Info.inds.b.b1, 0) == USB_SUCCESS)
					&& (Device_Info.Current_Configuration != 0))
			{
				CopyRoutine = Standard_GetStatus;
			}
		}

		/* GET STATUS for EndPoint*/
		else if (Type_Recipient == (STANDARD_REQUEST | ENDPOINT_RECIPIENT))
		{
			Related_Endpoint = (Device_Info.inds.b.b1 & 0x0f);
			Reserved = Device_Info.inds.b.b1 & 0x70;

			if (ValBit(Device_Info.inds.b.b1, 7))
			{
				/*Get Status of endpoint & stall the request if the related_ENdpoint
				  is Disabled*/
				Status = USB_EP(Related_Endpoint) & (3<<4);
			}
			else
			{
				Status = USB_EP(Related_Endpoint) & (3<<12);
			}

			if ((Related_Endpoint < EP_num) && (Reserved == 0)
					&& (Status != 0))
			{
				CopyRoutine = Standard_GetStatus;
			}
		}

	}
	/*GET CONFIGURATION*/
	else if (Request_No == GET_CONFIGURATION)
	{
		if (Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT))
		{
			CopyRoutine = Standard_GetConfiguration;
		}
	}
	/*GET INTERFACE*/
	else if (Request_No == GET_INTERFACE)
	{
		if ((Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
				&& (Device_Info.Current_Configuration != 0) && (Device_Info.vals.w == 0)
				&& (Device_Info.inds.b.b0 == 0) && (Device_Info.lens.w == 0x0001)
				&& ((Device_Property.Class_Get_Interface_Setting)(Device_Info.inds.b.b1, 0) == USB_SUCCESS))
		{
			CopyRoutine = Standard_GetInterface;
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
		r = (Device_Property.Class_Data_Setup)(Device_Info.USBbRequest);
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
	{
		/* Unsupported request */
		Device_Info.ControlState = STALLED;
		return;
	}


	if (ValBit(Device_Info.USBbmRequestType, 7))
	{
		/* Device ==> Host */
		vu32 wLength = Device_Info.lens.w;
		/* Restrict the data length to be the one host asks for */
		if (Device_Info.Ctrl_Info.Usb_wLength > wLength)
		{
			Device_Info.Ctrl_Info.Usb_wLength = wLength;
		}

		else if (Device_Info.Ctrl_Info.Usb_wLength < Device_Info.lens.w)
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
	else
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
	Device_Info.USBbmRequestType = S0.USBbmRequestType;
	Device_Info.USBbRequest = S0.USBbRequest;
	Device_Info.vals.w = CHANGE_END16(S0.vals.w);
	Device_Info.inds.w = CHANGE_END16(S0.inds.w);
	Device_Info.lens.w = S0.lens.w; 

	Device_Info.ControlState = SETTING_UP;
	if (Device_Info.lens.w == 0)
	{
		/* Setup with no data stage */
		NoData_Setup0();
	}
	else
	{
		/* Setup with data stage */
		Data_Setup0();
	}
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
		/* ControlState may be changed outside the function */
		ControlState = Device_Info.ControlState;
	}
	else if (ControlState == WAIT_STATUS_IN)
	{
		if ((Device_Info.USBbRequest == SET_ADDRESS) &&
				(Type_Recipient == (STANDARD_REQUEST | DEVICE_RECIPIENT)))
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
		/* host aborts the transfer before finish */
		ControlState = STALLED;
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
	/* Unexpect state, STALL the endpoint */
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

