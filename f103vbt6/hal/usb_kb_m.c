#include "stm32f1_sys.h"
#include "usb_core.h"
#include "usb_kb_m.h"

vu16 wIstr;  /* ISTR register last read value */
vu8 bIntPackSOF = 0;  /* SOFs received between 2 consecutive packets */
extern u8	EPindex;
extern u16  wInterrupt_Mask;

void EP1_OUT_Callback(void)
{
	SetEPRxValid(ENDP1);
}
void (*pEpInt_IN[7])(void) =
{
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};

void (*pEpInt_OUT[7])(void) =
{
	EP1_OUT_Callback,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};

vu16 SaveRState;
vu16 SaveTState;

typedef enum _RESUME_STATE
{
	RESUME_EXTERNAL,
	RESUME_INTERNAL,
	RESUME_LATER,
	RESUME_WAIT,
	RESUME_START,
	RESUME_ON,
	RESUME_OFF,
	RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE
{
	UNCONNECTED,
	ATTACHED,
	POWERED,
	SUSPENDED,
	ADDRESSED,
	CONFIGURED
} DEVICE_STATE;

void Suspend(void);
void Resume_Init(void);
void Resume(RESUME_STATE eResumeSetVal);
RESULT PowerOn(void);
RESULT PowerOff(void);
void CTR_LP(void)
{
	vu16 wEPVal = 0;
	/* stay in loop while pending interrupts */
	while (((wIstr = USB->ISTR) & ISTR_CTR) != 0)
	{
		/* extract highest priority endpoint number */
		EPindex = (uint8_t)(wIstr & ISTR_EP_ID);
		if (EPindex == 0)
		{
			/* Decode and service control endpoint interrupt */
			/* calling related service routine */
			/* (Setup0_Process, In0_Process, Out0_Process) */

			/* save RX & TX status */
			/* and set both to NAK */

			SaveRState = _GetENDPOINT(ENDP0);
			SaveTState = SaveRState & EPTX_STAT;
			SaveRState &=  EPRX_STAT;	

			_SetEPRxTxStatus(ENDP0,EP_RX_NAK,EP_TX_NAK);

			/* DIR bit = origin of the interrupt */

			if ((wIstr & ISTR_DIR) == 0)
			{
				/* DIR = 0 */

				/* DIR = 0      => IN  int */
				/* DIR = 0 implies that (EP_CTR_TX = 1) always  */

				_ClearEP_CTR_TX(ENDP0);
				In0_Process();

				/* before terminate set Tx & Rx status */

				_SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
				return;
			}
			else
			{
				/* DIR = 1 */

				/* DIR = 1 & CTR_RX       => SETUP or OUT int */
				/* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */

				wEPVal = _GetENDPOINT(ENDP0);

				if ((wEPVal &EP_SETUP) != 0)
				{
					_ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
					Setup0_Process();
					/* before terminate set Tx & Rx status */

					_SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
					return;
				}

				else if ((wEPVal & EP_CTR_RX) != 0)
				{
					_ClearEP_CTR_RX(ENDP0);
					Out0_Process();
					/* before terminate set Tx & Rx status */

					_SetEPRxTxStatus(ENDP0,SaveRState,SaveTState);
					return;
				}
			}
		}/* if(EPindex == 0) */
		else
		{
			/* Decode and service non control endpoints interrupt  */

			/* process related endpoint register */
			wEPVal = _GetENDPOINT(EPindex);
			if ((wEPVal & EP_CTR_RX) != 0)
			{
				/* clear int flag */
				_ClearEP_CTR_RX(EPindex);

				/* call OUT service function */
				(*pEpInt_OUT[EPindex-1])();

			} /* if((wEPVal & EP_CTR_RX) */

			if ((wEPVal & EP_CTR_TX) != 0)
			{
				/* clear int flag */
				_ClearEP_CTR_TX(EPindex);

				/* call IN service function */
				(*pEpInt_IN[EPindex-1])();
			} /* if((wEPVal & EP_CTR_TX) != 0) */

		}/* if(EPindex == 0) else */

	}/* while(...) */
}
//USB唤醒中断服务函数
void USBWakeUp_IRQHandler(void) 
{
	EXTI->PR|=1<<18;//清除USB唤醒中断挂起位
} 
//USB中断处理函数
void USB_LP_CAN1_RX0_IRQHandler(void) 
{
	wIstr = USB->ISTR;
	if (wIstr & ISTR_RESET & wInterrupt_Mask)
	{
		USB->ISTR=(u16)CLR_RESET;
		Device_Property.Reset();
	}

	if (wIstr & ISTR_ERR & wInterrupt_Mask)
	{
		USB->ISTR=(u16)CLR_ERR;
	}

	if (wIstr & ISTR_WKUP & wInterrupt_Mask)
	{
		USB->ISTR=(u16)CLR_WKUP;
		Resume(RESUME_EXTERNAL);
	}
	if (wIstr & ISTR_SUSP & wInterrupt_Mask)
	{

		/* check if SUSPEND is possible */
		if (fSuspendEnabled)
		{
			Suspend();
		}
		else
		{
			/* if not possible then resume after xx ms */
			Resume(RESUME_LATER);
		}
		/* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
		USB->ISTR=(u16)CLR_SUSP;
	}

	if (wIstr & ISTR_SOF & wInterrupt_Mask)
	{
		USB->ISTR=(u16)CLR_SOF;
		bIntPackSOF++;
	}

	if (wIstr & ISTR_ESOF & wInterrupt_Mask)
	{
		USB->ISTR=(u16)CLR_ESOF;
		/* resume handling timing is made with ESOFs */
		Resume(RESUME_ESOF); /* request without change of the machine state */
	}

	if (wIstr & ISTR_CTR & wInterrupt_Mask)
	{
		/* servicing of the endpoint correct transfer interrupt */
		/* clear of the CTR flag into the sub */
		CTR_LP();
	}
} 

/*******************************************************************************
 * Function Name  : Enter_LowPowerMode.
 * Description    : Power-off system clocks and power while entering suspend mode.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;	  
	/* Request to enter STOP mode with regulator in low power mode */
	//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

/*******************************************************************************
 * Function Name  : Leave_LowPowerMode.
 * Description    : Restores system clocks and power while exiting suspend mode.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}
}

//USB接口配置(配置1.5K上拉电阻,战舰V3不需要配置,恒上拉)
//NewState:DISABLE,不上拉
//         ENABLE,上拉
void USB_Cable_Config(FunctionalState NewState)
{ 
	if (NewState!=DISABLE)
	{
		PAout(8)=1;
		//printf("usb pull up enable\r\n"); 
	}
	else
	{
		PAout(8)=0;
		//printf("usb pull up disable\r\n"); 
	}
}
u32 keyboard_send_pre_0=0;
u32 keyboard_send_pre_1=0; //上次发送值
void keyboard_send(u8 *buf)
{
	if (((u32*)buf)[0]==keyboard_send_pre_0 && ((u32*)buf)[1]==keyboard_send_pre_1) //若相同
	{
		return ;
	}
	keyboard_send_pre_0=((u32*)buf)[0];
	keyboard_send_pre_1=((u32*)buf)[1];
	UserToPMABufferCopy(buf, _GetEPTxAddr(ENDP1), 8);
	SetEPTxValid(ENDP1);
}
u32 mouse_send_pre=0; //上次发送值
void mouse_send(u8 *buf)
{
	if (((u32*)(buf+1))[0]==mouse_send_pre && mouse_send_pre==0) //若相同并且为0
	{
		return ;
	}
	mouse_send_pre=((u32*)(buf+1))[0];
	UserToPMABufferCopy(buf, _GetEPTxAddr(ENDP2), 5);
	SetEPTxValid(ENDP2);
}

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);

	if (Device_Serial0 != 0)
	{
		Joystick_StringSerial[2] = (u8)(Device_Serial0 & 0x000000FF);
		Joystick_StringSerial[4] = (u8)((Device_Serial0 & 0x0000FF00) >> 8);
		Joystick_StringSerial[6] = (u8)((Device_Serial0 & 0x00FF0000) >> 16);
		Joystick_StringSerial[8] = (u8)((Device_Serial0 & 0xFF000000) >> 24);

		Joystick_StringSerial[10] = (u8)(Device_Serial1 & 0x000000FF);
		Joystick_StringSerial[12] = (u8)((Device_Serial1 & 0x0000FF00) >> 8);
		Joystick_StringSerial[14] = (u8)((Device_Serial1 & 0x00FF0000) >> 16);
		Joystick_StringSerial[16] = (u8)((Device_Serial1 & 0xFF000000) >> 24);

		Joystick_StringSerial[18] = (u8)(Device_Serial2 & 0x000000FF);
		Joystick_StringSerial[20] = (u8)((Device_Serial2 & 0x0000FF00) >> 8);
		Joystick_StringSerial[22] = (u8)((Device_Serial2 & 0x00FF0000) >> 16);
		Joystick_StringSerial[24] = (u8)((Device_Serial2 & 0xFF000000) >> 24);
	}
}

//设备描述符，配置描述符，字符串描述符，接口描述符，端点描述符
//USB Standard Device Descriptor设备描述符，只有一个
const u8 Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC] =
{
	0x12,   //bLength */
	USB_DEVICE_DESCRIPTOR_TYPE, //bDescriptorType设备描述符类型．固定为0x01
	0x00,    //bcdUSB,USB规范发布号．USB2.0=0200，1.1=0110
	0x02,
	0x00,  //bDeviceClass类型代码。0:所有接口在配置描述符里，且所有接口独立。1到FE:不同的接口关联。FF:厂商自定义
	0x00,     //bDeviceSubClass子类型代码,若bDeviceClass=0，一定要为0
	0x00,     //bDeviceProtocol协议代码厂商自己定义的可以设置为FF
	0x40,     //bMaxPacketSize端点０最大分组大小（只有8,16,32,64有效）
	0x11,     //idVendor(0x1234)供应商ID
	0x11,
	0x11,     //idProduct = 0x4321产品ID,由供应商ID和产品ID，让OS加载不同的驱动程序
	0x11,
	0x00,     //bcdDevice设备出产编码．由厂家自行设置
	0x02,
	1,        //Index of string descriptor describing manufacturer 厂商描述符字符串索引．索引到对应的字符串描述符没有就是0
	2,        //Index of string descriptor describing product*/
	3,        //Index of string descriptor describing the device serial number */
	0x01      //bNumConfigurations 可能的配置数．指配置字符串的个数
}; // Joystick_DeviceDescriptor */

const u8 Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC] =
{
	//**************配置描述符***********************/
	0x09,		//bLength字段,描述符大小．固定为0x09
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	//bDescriptorType 配置描述符类型．固定为0x02
	//wTotalLength字段
	JOYSTICK_SIZ_CONFIG_DESC,//全长．此配置返回的配置描述符，接口描述符以及端点描述符的全部大小
	0x00,

	0x02,	//bNumInterfaces配置所支持的接口数
	0x01,	//bConfiguration作为Set Configuration的一个参数选择配置值
	0x00,	//iConfiguration用于描述该配置字符串描述符的索引
	0x80,	//bmAttributes供电模式选择．Bit4-0保留，D7:总线供电，D6:自供电，D5:远程唤醒
	//0x32,	//bMaxPower总线供电的USB设备的最大消耗电流．以2mA为单位
	0xfa,	//bMaxPower总线供电的USB设备的最大消耗电流．以2mA为单位

	//******************第一个接口描述符*********************/
	0x09,	//bLength描述符大小．固定为0x09
	0x04,	//bDescriptorType接口描述符类型．固定为0x04
	0x00,	//bInterfaceNumber该接口的编号
	0x00,	//bAlternateSetting用于为上一个字段选择可供替换的位置．即备用的接口描述符标号
	0x02,	//bNumEndpoints使用的端点数目．端点０除外
	0x03,	//bInterfaceClass 类型代码,3:HID
	0x01,	//bInterfaceSubClass子类型代码1=BOOT, 0=no boot
	0x01,	//bInterfaceProtocol协议代码0=none, 1=keyboard, 2=mouse 
	0x00,	//iConfiguration字符串描述符的索引

	//*****************HID描述符************************/
	0x09,	//bLength字段
	0x21,	//bDescriptorType字段
	0x10,	//bcdHID字段
	0x01,
	0x21,	//bCountyCode字段
	0x01,	//bNumDescriptors字段
	0x22,	//bDescriptorType字段

	//bDescriptorLength字段。
	//下级描述符的长度。下级描述符为键盘报告描述符。
	sizeof(KeyboardReportDescriptor)&0xFF,
	(sizeof(KeyboardReportDescriptor)>>8)&0xFF,

	//*********************输入端点描述符***********************/
	0x07,	//bLength描述符大小．固定为0x07
	0x05,	//bDescriptorType接口描述符类型．固定为0x05
	0x81,	//bEndpointAddressUSB端点地址．Bit7:方向,控制端点可忽略;1/0:IN/OUT．Bit6-4，保留．BIt3-0：端点号
	0x03,	//bmAttributes端点属性．Bit7-2保留．BIt1-0:00控制,01同步,02批量,03中断
	0x10,	//wMaxPacketSize本端点接收或发送的最大信息包大小
	0x00,
	0x0A,	//bInterval轮训数据传送端点的时间间隔．对于批量传送和控制传送的端点忽略．对于同步传送的端点，必须为１，对于中断传送的端点，范围为１－２５５

	//*********************输出端点描述符***********************/
	0x07,	//bLength字段
	0x05,	//bDescriptorType字段
	0x01,	//bEndpointAddress字段
	0x03,	//bmAttributes字段
	0x10,	//wMaxPacketSize字段
	0x00,
	0x0A,	//bInterval字段

	//******************第二个接口描述符*********************/
	0x09,	//bLength字段
	0x04,	//bDescriptorType字段
	0x01,	//bInterfaceNumber字段
	0x00,	//bAlternateSetting字段
	0x01,	//bNumEndpoints字段
	0x03,	//bInterfaceClass字段
	0x01,	//bInterfaceSubClass字段
	0x02,	//bInterfaceProtocol字段
	0x00,	//iConfiguration字段

	//*****************HID描述符************************/
	0x09,	//bLength字段
	0x21,	//bDescriptorType HID的描述符类型为0x21 
	0x10,	//bcdHID HID协议的版本1.1
	0x01,
	0x21,	//bCountyCode国家代号
	0x01,	//bNumDescriptors下级描述符的数量
	0x22,	//bDescriptorType下级描述符的类型
	sizeof(MouseReportDescriptor)&0xFF,		//bDescriptorLength字段
	(sizeof(MouseReportDescriptor)>>8)&0xFF,

	//*********************输入端点描述符***********************/
	0x07,	//bLength字段
	0x05,	//bDescriptorType字段
	0x82,	//bEndpointAddress字段
	0x03,	//bmAttributes字段。D1~D0为端点传输类型选择
	0x40,	//wMaxPacketSize字段
	0x00,
	0x0A 	//bInterval字段
};
////////////////////////配置描述符集合完毕//////////////////////////
//USB键盘报告描述符的定义
//定义了8字节发送,第一字节表示特殊件是否按下：D0:左Ctrl D1:左Shift D2:左Alt D3:左win
//	D4:右Ctrl D5:右Shift D6:右Alt D7:右win
//第二字节保留，值为0
//第三~第八字节:普通键键值的数组,最多能同时按下6个键
//定义了1字节接收:对应键盘上的LED灯,这里只用了两个位。
//D0:Num Lock   D1:Cap Lock   D2:Scroll Lock   D3:Compose   D4:Kana
const u8 KeyboardReportDescriptor[KP_ReportDescriptor_Size]=
{
	0x05, 0x01,  // USAGE_PAGE (Generic Desktop)	//63
	0x09, 0x06,  // USAGE (Keyboard)
	0xa1, 0x01,  // COLLECTION (Application)
	0x05, 0x07,  //   USAGE_PAGE (Keyboard)
	0x19, 0xe0,  //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7,  //   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00,  //   LOGICAL_MINIMUM (0)
	0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,  //   REPORT_SIZE (1)
	0x95, 0x08,  //   REPORT_COUNT (8)
	0x81, 0x02,  //   INPUT (Data,Var,Abs)
	0x95, 0x01,  //   REPORT_COUNT (1)
	0x75, 0x08,  //   REPORT_SIZE (8)
	0x81, 0x03,  //   INPUT (Cnst,Var,Abs)
	0x95, 0x05,  //   REPORT_COUNT (5)
	0x75, 0x01,  //   REPORT_SIZE (1)
	0x05, 0x08,  //   USAGE_PAGE (LEDs)
	0x19, 0x01,  //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,  //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,  //   OUTPUT (Data,Var,Abs)
	0x95, 0x01,  //   REPORT_COUNT (1)
	0x75, 0x03,  //   REPORT_SIZE (3)
	0x91, 0x03,  //   OUTPUT (Cnst,Var,Abs)
	0x95, 0x06,  //   REPORT_COUNT (6)
	0x75, 0x08,  //   REPORT_SIZE (8)
	0x15, 0x00,  //   LOGICAL_MINIMUM (0)
	0x25, 0x65,  //   LOGICAL_MAXIMUM (101)
	0x05, 0x07,  //   USAGE_PAGE (Keyboard)
	0x19, 0x00,  //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65,  //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00,  //   INPUT (Data,Ary,Abs)
	0xc0,         // END_COLLECTION
	//0xc0,
}; // Joystick_ReportDescriptor */
///////////////////////////键盘报告描述符完毕////////////////////////////

//USB鼠标报告描述符的定义
//共4字节：第一字节D0: 左键；D1: 右键；D2: 中键
const u8 MouseReportDescriptor[Mouse_ReportDescriptor_Size]=
{
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x02, // USAGE (Mouse)
	0xa1, 0x01, // COLLECTION (Application)
	0x85, 0x01, //Report ID (1)
	0x09, 0x01, //   USAGE (Pointer)
	0xa1, 0x00, //   COLLECTION (Physical)
	0x05, 0x09, //     USAGE_PAGE (Button)
	0x19, 0x01, //     USAGE_MINIMUM (Button 1)
	0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00, //     LOGICAL_MINIMUM (0)
	0x25, 0x01, //     LOGICAL_MAXIMUM (1)
	0x95, 0x03, //     REPORT_COUNT (3)
	0x75, 0x01, //     REPORT_SIZE (1)
	0x81, 0x02, //     INPUT (Data,Var,Abs)
	0x95, 0x01, //     REPORT_COUNT (1)
	0x75, 0x05, //     REPORT_SIZE (5)
	0x81, 0x03, //     INPUT (Cnst,Var,Abs)
	0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30, //     USAGE (X)
	0x09, 0x31, //     USAGE (Y)
	0x09, 0x38, //     USAGE (Wheel)
	0x15, 0x81, //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
	0x75, 0x08, //     REPORT_SIZE (8)
	0x95, 0x03, //     REPORT_COUNT (3)
	0x81, 0x06, //     INPUT (Data,Var,Rel)
	0xc0,       //   END_COLLECTION
	0xc0        // END_COLLECTION
};
///////////////////////////报告描述符完毕////////////////////////////
// USB String Descriptors (optional) */
const u8 Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID] =
{
	JOYSTICK_SIZ_STRING_LANGID,
	USB_STRING_DESCRIPTOR_TYPE,
	0x09,
	0x04
}
; // LangID = 0x0409: U.S. English */

const u8 Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR] =
{
	JOYSTICK_SIZ_STRING_VENDOR, // Size of Vendor string */
	USB_STRING_DESCRIPTOR_TYPE,  // bDescriptorType*/
	// Manufacturer: "STMicroelectronics" */
	'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
	'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
	'c', 0, 's', 0
};

const u8 Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT] =
{
	JOYSTICK_SIZ_STRING_PRODUCT,          // bLength */
	USB_STRING_DESCRIPTOR_TYPE,        // bDescriptorType */
	'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'J', 0,
	'o', 0, 'y', 0, 's', 0, 't', 0, 'i', 0, 'c', 0, 'k', 0
};
u8 Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL] =
{
	JOYSTICK_SIZ_STRING_SERIAL,           // bLength */
	USB_STRING_DESCRIPTOR_TYPE,        // bDescriptorType */
	'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, '1', 0, '0', 0
};

u32 ProtocolValue;
extern DEVICE_INFO *pInformation;
extern u16  wInterrupt_Mask;

DEVICE_PROP Device_Property =
{
	Joystick_init,
	Joystick_Reset,
	Joystick_Status_In,
	Joystick_Status_Out,
	Joystick_Data_Setup,
	Joystick_NoData_Setup,
	Joystick_Get_Interface_Setting,
	Joystick_GetDeviceDescriptor,
	Joystick_GetConfigDescriptor,
	Joystick_GetStringDescriptor,
	0,
	0x40 /*MAX PACKET SIZE*/
};
USER_STANDARD_REQUESTS User_Standard_Requests =
{
	Joystick_GetConfiguration,
	Joystick_SetConfiguration,
	Joystick_GetInterface,
	Joystick_SetInterface,
	Joystick_GetStatus,
	Joystick_ClearFeature,
	Joystick_SetEndPointFeature,
	Joystick_SetDeviceFeature,
	Joystick_SetDeviceAddress
};

ONE_DESCRIPTOR Device_Descriptor =
{
	(u8*)Joystick_DeviceDescriptor,
	JOYSTICK_SIZ_DEVICE_DESC
};

ONE_DESCRIPTOR Config_Descriptor =
{
	(u8*)Joystick_ConfigDescriptor,
	JOYSTICK_SIZ_CONFIG_DESC
};
/*******************************************************************/
ONE_DESCRIPTOR KP_Report_Descriptor =							   	//
{																	//
	(u8 *)KeyboardReportDescriptor,									//
	KP_ReportDescriptor_Size										//
};																//
//
ONE_DESCRIPTOR KP_Hid_Descriptor =									//
{																	//
	(u8*)Joystick_ConfigDescriptor + KP_OFF_HID_DESC,				//
	JOYSTICK_SIZ_HID_DESC											//
};																//
//
ONE_DESCRIPTOR Mouse_Report_Descriptor =							//
{																	//
	(u8 *)MouseReportDescriptor,									//
	Mouse_ReportDescriptor_Size										//
};																//
//
ONE_DESCRIPTOR Mouse_Hid_Descriptor =								//
{																	//
	(u8*)Joystick_ConfigDescriptor + Mouse_OFF_HID_DESC,			//
	JOYSTICK_SIZ_HID_DESC											//
};																//
/*******************************************************************/

ONE_DESCRIPTOR String_Descriptor[4] =
{
	{(u8*)Joystick_StringLangID, JOYSTICK_SIZ_STRING_LANGID},
	{(u8*)Joystick_StringVendor, JOYSTICK_SIZ_STRING_VENDOR},
	{(u8*)Joystick_StringProduct, JOYSTICK_SIZ_STRING_PRODUCT},
	{(u8*)Joystick_StringSerial, JOYSTICK_SIZ_STRING_SERIAL}
};

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
 * Function Name  : Joystick_init.
 * Description    : Joystick Mouse init routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_init(void)
{

	/* Update the serial number string descriptor with the data from the unique
	   ID*/
	Get_SerialNum();

	pInformation->Current_Configuration = 0;
	/* Connect the device */
	PowerOn();
	/* USB interrupts initialization */
	USB->ISTR=0;               /* clear pending interrupts */
	wInterrupt_Mask = IMR_MSK;
	USB->CNTR=wInterrupt_Mask; /* set interrupts mask */

	bDeviceState = UNCONNECTED;
}

/*******************************************************************************
 * Function Name  : Joystick_Reset.
 * Description    : Joystick Mouse reset routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_Reset(void)
{
	/* Set Joystick_DEVICE as not configured */
	pInformation->Current_Configuration = 0;
	pInformation->Current_Interface = 0;/*the default Interface*/

	/* Current Feature initialization */
	pInformation->Current_Feature = Joystick_ConfigDescriptor[7];

	USB->BTABLE =0;

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_STALL);
	SetEPRxAddr(ENDP0, ENDP0_RXADDR);
	SetEPTxAddr(ENDP0, ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);

	/* Initialize Endpoint In 1 */
	SetEPType(ENDP1, EP_INTERRUPT); //初始化为中断端点类型
	SetEPTxAddr(ENDP1, ENDP1_TXADDR); //设置发送数据的地址
	SetEPTxCount(ENDP1, 8); //设置发送的长度
	SetEPTxStatus(ENDP1, EP_TX_NAK); //设置端点处于忙状态

	/* Initialize Endpoint Out 1 */
	SetEPRxAddr(ENDP1, ENDP1_RXADDR); //设置接收数据的地址
	SetEPRxCount(ENDP1, 2);  //设置接收长度
	SetEPRxStatus(ENDP1, EP_RX_VALID); //设置端点有效，可以接收数据

	/* Initialize Endpoint In 2 */
	SetEPType(ENDP2, EP_INTERRUPT); //初始化为中断端点类型
	SetEPTxAddr(ENDP2, ENDP2_TXADDR); //设置发送数据的地址
	SetEPTxCount(ENDP2, 5); //设置发送的长度
	SetEPTxStatus(ENDP2, EP_TX_NAK); //设置端点处于忙状态

	bDeviceState = ATTACHED;

	/* Set this device to response on default address */
	SetDeviceAddress(0);
}
/*******************************************************************************
 * Function Name  : Joystick_SetConfiguration.
 * Description    : Udpade the device state to configured.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_SetConfiguration(void)
{
}
/*******************************************************************************
 * Function Name  : Joystick_SetConfiguration.
 * Description    : Udpade the device state to addressed.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_SetDeviceAddress (void)
{
}
/*******************************************************************************
 * Function Name  : Joystick_Status_In.
 * Description    : Joystick status IN routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_Status_In(void)
{}

/*******************************************************************************
 * Function Name  : Joystick_Status_Out
 * Description    : Joystick status OUT routine.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Joystick_Status_Out (void)
{}

/*******************************************************************************
 * Function Name  : Joystick_Data_Setup
 * Description    : Handle the data class specific requests.
 * Input          : Request Nb.
 * Output         : None.
 * Return         : USB_UNSUPPORT or USB_SUCCESS.
 *******************************************************************************/
RESULT Joystick_Data_Setup(u8 RequestNo)
{
	u8 *(*CopyRoutine)(u16);

	CopyRoutine = 0;
	if ((RequestNo == GET_DESCRIPTOR)
			&& (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
			&& (pInformation->USBwIndex0 < 2))
	{

		if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
		{
			if (pInformation->USBwIndex0 == 0)
				CopyRoutine = KP_GetReportDescriptor;
			else
				CopyRoutine = Mouse_GetReportDescriptor;
		}
		else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
		{
			if (pInformation->USBwIndex0 == 0)
				CopyRoutine = KP_GetHIDDescriptor;
			else
				CopyRoutine = Mouse_GetHIDDescriptor;
		}

	} /* End of GET_DESCRIPTOR */

	/*** GET_PROTOCOL ***/
	else if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			&& RequestNo == GET_PROTOCOL)
	{
		CopyRoutine = Joystick_GetProtocolValue;
	}
	if (CopyRoutine == 0)
	{
		return USB_UNSUPPORT;
	}

	pInformation->Ctrl_Info.CopyData = CopyRoutine;
	pInformation->Ctrl_Info.Usb_wOffset = 0;
	(*CopyRoutine)(0);
	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Joystick_NoData_Setup
 * Description    : handle the no data class specific requests
 * Input          : Request Nb.
 * Output         : None.
 * Return         : USB_UNSUPPORT or USB_SUCCESS.
 *******************************************************************************/
RESULT Joystick_NoData_Setup(u8 RequestNo)
{
	if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			&& (RequestNo == SET_PROTOCOL))
	{
		return Joystick_SetProtocol();
	}

	else
	{
		return USB_UNSUPPORT;
	}
}

/*******************************************************************************
 * Function Name  : Joystick_GetDeviceDescriptor.
 * Description    : Gets the device descriptor.
 * Input          : Length
 * Output         : None.
 * Return         : The address of the device descriptor.
 *******************************************************************************/
u8 *Joystick_GetDeviceDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
 * Function Name  : Joystick_GetConfigDescriptor.
 * Description    : Gets the configuration descriptor.
 * Input          : Length
 * Output         : None.
 * Return         : The address of the configuration descriptor.
 *******************************************************************************/
u8 *Joystick_GetConfigDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
 * Function Name  : Joystick_GetStringDescriptor
 * Description    : Gets the string descriptors according to the needed index
 * Input          : Length
 * Output         : None.
 * Return         : The address of the string descriptors.
 *******************************************************************************/
u8 *Joystick_GetStringDescriptor(u16 Length)
{
	u8 wValue0 = pInformation->USBwValue0;
	//  if (wValue0 > 4)
	//  {
	//    return NULL;
	//  }
	//  else
	//  {
	return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
	//  }
}
u8 INIT_OK = 0;
/*******************************************************************************
 * Function Name  : Joystick_GetReportDescriptor.
 * Description    : Gets the HID report descriptor.
 * Input          : Length
 * Output         : None.
 * Return         : The address of the configuration descriptor.
 *******************************************************************************/
u8 *KP_GetReportDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &KP_Report_Descriptor);
}

u8 *Mouse_GetReportDescriptor(u16 Length)
{
	INIT_OK = 1;
	return Standard_GetDescriptorData(Length, &Mouse_Report_Descriptor);
}

/*******************************************************************************
 * Function Name  : Joystick_GetHIDDescriptor.
 * Description    : Gets the HID descriptor.
 * Input          : Length
 * Output         : None.
 * Return         : The address of the configuration descriptor.
 *******************************************************************************/
u8 *KP_GetHIDDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &KP_Hid_Descriptor);
}
u8 *Mouse_GetHIDDescriptor(u16 Length)
{
	return Standard_GetDescriptorData(Length, &Mouse_Hid_Descriptor);
}

/*******************************************************************************
 * Function Name  : Joystick_Get_Interface_Setting.
 * Description    : tests the interface and the alternate setting according to the
 *                  supported one.
 * Input          : - Interface : interface number.
 *                  - AlternateSetting : Alternate Setting number.
 * Output         : None.
 * Return         : USB_SUCCESS or USB_UNSUPPORT.
 *******************************************************************************/
RESULT Joystick_Get_Interface_Setting(u8 Interface, u8 AlternateSetting)
{
	if (AlternateSetting > 0)
	{
		return USB_UNSUPPORT;
	}
	else if (Interface > 0)
	{
		return USB_UNSUPPORT;
	}
	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Joystick_SetProtocol
 * Description    : Joystick Set Protocol request routine.
 * Input          : None.
 * Output         : None.
 * Return         : USB SUCCESS.
 *******************************************************************************/
RESULT Joystick_SetProtocol(void)
{
	u8 wValue0 = pInformation->USBwValue0;
	ProtocolValue = wValue0;
	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Joystick_GetProtocolValue
 * Description    : get the protocol value
 * Input          : Length.
 * Output         : None.
 * Return         : address of the protcol value.
 *******************************************************************************/
u8 *Joystick_GetProtocolValue(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = 1;
		return 0;
	}
	else
	{
		return (u8 *)(&ProtocolValue);
	}
}

vu32 bDeviceState = UNCONNECTED; /* USB device status */
vu32 fSuspendEnabled = 1;  /* true when suspend is possible */
vu32 EP[8];
extern u16  wInterrupt_Mask;
struct
{
	__IO RESUME_STATE eState;
	vu8 bESOFcnt;
} ResumeS;

vu32 remotewakeupon=0;

/*******************************************************************************
 * Function Name  : PowerOn
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : USB_SUCCESS.
 *******************************************************************************/
RESULT PowerOn(void)
{
	u16 wRegVal;

	/*** cable plugged-in ? ***/
	USB_Cable_Config(ENABLE);

	/*** CNTR_PWDN = 0 ***/
	wRegVal = CNTR_FRES;
	USB->CNTR=wRegVal;

	/*** CNTR_FRES = 0 ***/
	wInterrupt_Mask = 0;
	USB->CNTR=wInterrupt_Mask;
	/*** Clear pending interrupts ***/
	USB->ISTR=0;
	/*** Set interrupt mask ***/
	wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
	USB->CNTR=wInterrupt_Mask;

	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : PowerOff
 * Description    : handles switch-off conditions
 * Input          : None.
 * Output         : None.
 * Return         : USB_SUCCESS.
 *******************************************************************************/
RESULT PowerOff()
{
	/* disable all interrupts and force USB reset */
	USB->CNTR=CNTR_FRES;
	/* clear interrupt status register */
	USB->ISTR=0;
	/* Disable the Pull-Up*/
	USB_Cable_Config(DISABLE);
	/* switch-off device */
	USB->CNTR=CNTR_FRES + CNTR_PDWN;
	/* sw variables reset */

	return USB_SUCCESS;
}

/*******************************************************************************
 * Function Name  : Suspend
 * Description    : sets suspend mode operating conditions
 * Input          : None.
 * Output         : None.
 * Return         : USB_SUCCESS.
 *******************************************************************************/
void Suspend(void)
{
	u32 i =0;
	u16 wCNTR; 
	vu32 savePWR_CR=0;
	/* suspend preparation */
	/* ... */

	/*Store CNTR value */
	wCNTR = USB->CNTR;  

	/* This a sequence to apply a force RESET to handle a robustness case */

	/*Store endpoints registers status */
	for (i=0;i<8;i++) EP[i] = _GetENDPOINT(i);

	/* unmask RESET flag */
	wCNTR|=CNTR_RESETM;
	USB->CNTR=wCNTR;

	/*apply FRES */
	wCNTR|=CNTR_FRES;
	USB->CNTR=wCNTR;

	/*clear FRES*/
	wCNTR&=~CNTR_FRES;
	USB->CNTR=wCNTR;

	/*poll for RESET flag in ISTR*/
	while((USB->ISTR & ISTR_RESET) == 0);

	/* clear RESET flag in ISTR */
	USB->ISTR=((uint16_t)CLR_RESET);

	/*restore Enpoints*/
	for (i=0;i<8;i++)
		_SetENDPOINT(i, EP[i]);

	/* Now it is safe to enter macrocell in suspend mode */
	wCNTR |= CNTR_FSUSP;
	USB->CNTR=wCNTR;

	/* force low-power mode in the macrocell */
	wCNTR = USB->CNTR;
	wCNTR |= CNTR_LPMODE;
	USB->CNTR=wCNTR;

	Enter_LowPowerMode();
}

/*******************************************************************************
 * Function Name  : Resume_Init
 * Description    : Handles wake-up restoring normal operations
 * Input          : None.
 * Output         : None.
 * Return         : USB_SUCCESS.
 *******************************************************************************/
void Resume_Init(void)
{
	u16 wCNTR;

	/* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
	/* restart the clocks */
	/* ...  */

	/* CNTR_LPMODE = 0 */
	wCNTR = USB->CNTR;
	wCNTR &= (~CNTR_LPMODE);
	USB->CNTR=wCNTR;

	/* restore full power */
	/* ... on connected devices */
	Leave_LowPowerMode();

	/* reset FSUSP bit */
	USB->CNTR=IMR_MSK;

	/* reverse suspend preparation */
	/* ... */ 

}

/*******************************************************************************
 * Function Name  : Resume
 * Description    : This is the state machine handling resume operations and
 *                 timing sequence. The control is based on the Resume structure
 *                 variables and on the ESOF interrupt calling this subroutine
 *                 without changing machine state.
 * Input          : a state machine value (RESUME_STATE)
 *                  RESUME_ESOF doesn't change ResumeS.eState allowing
 *                  decrementing of the ESOF counter in different states.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Resume(RESUME_STATE eResumeSetVal)
{
	uint16_t wCNTR;

	if (eResumeSetVal != RESUME_ESOF)
		ResumeS.eState = eResumeSetVal;
	switch (ResumeS.eState)
	{
		case RESUME_EXTERNAL:
			if (remotewakeupon ==0)
			{
				Resume_Init();
				ResumeS.eState = RESUME_OFF;
			}
			else /* RESUME detected during the RemoteWAkeup signalling => keep RemoteWakeup handling*/
			{
				ResumeS.eState = RESUME_ON;
			}
			break;
		case RESUME_INTERNAL:
			Resume_Init();
			ResumeS.eState = RESUME_START;
			remotewakeupon = 1;
			break;
		case RESUME_LATER:
			ResumeS.bESOFcnt = 2;
			ResumeS.eState = RESUME_WAIT;
			break;
		case RESUME_WAIT:
			ResumeS.bESOFcnt--;
			if (ResumeS.bESOFcnt == 0)
				ResumeS.eState = RESUME_START;
			break;
		case RESUME_START:
			wCNTR = USB->CNTR;
			wCNTR |= CNTR_RESUME;
			USB->CNTR=wCNTR;
			ResumeS.eState = RESUME_ON;
			ResumeS.bESOFcnt = 10;
			break;
		case RESUME_ON:    
			ResumeS.bESOFcnt--;
			if (ResumeS.bESOFcnt == 0)
			{
				wCNTR = USB->CNTR;
				wCNTR &= (~CNTR_RESUME);
				USB->CNTR=wCNTR;
				ResumeS.eState = RESUME_OFF;
				remotewakeupon = 0;
			}
			break;
		case RESUME_OFF:
		case RESUME_ESOF:
		default:
			ResumeS.eState = RESUME_OFF;
			break;
	}
}

void usb_ini(void)
{
	MGPIOA->CT8=GPIO_OUT_PP; //USB线的使能引脚
	EP_num=3; //总共多少个端点
	usb_hal_ini(); //初始化硬件
}

