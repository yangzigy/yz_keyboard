#include "stm32f1_sys.h"
#include "usb_core.h"
#include "usb_kb_m.h"

////////////////////////////////////////////////////////////////////////////
//				端点输入、输出的回调函数，除了0
////////////////////////////////////////////////////////////////////////////
void EP1_OUT_Callback(void)
{
	SetEPRxStatus(1, 3<<12); //RX_VALID
}
void (*pEpInt_IN[7])(void) = //端点0没有
{
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};

void (*pEpInt_OUT[7])(void) = //端点0没有
{
	EP1_OUT_Callback,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
	NOP_Process,
};
////////////////////////////////////////////////////////////////////////////
//				挂起处理
////////////////////////////////////////////////////////////////////////////
vu16 SaveRState; //是接收中断与处理函数关于最后端点0状态的接口
vu16 SaveTState; //是接收中断与处理函数关于最后端点0状态的接口

typedef enum _RESUME_STATE
{
	RESUME_EXTERNAL,
	RESUME_INTERNAL,
	RESUME_LATER,
	RESUME_WAIT,
	RESUME_START,
	RESUME_ON,
	RESUME_OFF,
} RESUME_STATE;

void Suspend(void);
void Resume_Init(void);
void Resume(RESUME_STATE eResumeSetVal);
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
		Resume(RESUME_EXTERNAL);
	}
	if (t & (1<<11)) //SUSP挂起
	{
		Suspend();
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
void Enter_LowPowerMode(void)
{
	usb_stat = USB_SUSPENDED;	  
	//进入低功耗模式
}
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;
	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		usb_stat = USB_CONFIGURED; //配置完成
	}
	else
	{
		usb_stat = USB_ATTACHED;
	}
}

void USB_Cable_Config(int e) //0不上拉，1上拉
{ 
	if (e) PAout(8)=1;
	else PAout(8)=0;
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
	UserToPMABufferCopy(buf, USB_BT[1].ADDR_TX, 8);
	SetEPTxStatus(1, 3<<4); //TX_VALID可用于发送
}
u32 mouse_send_pre=0; //上次发送值
void mouse_send(u8 *buf)
{
	if (((u32*)(buf+1))[0]==mouse_send_pre && mouse_send_pre==0) //若相同并且为0
	{
		return ;
	}
	mouse_send_pre=((u32*)(buf+1))[0];
	UserToPMABufferCopy(buf, USB_BT[2].ADDR_TX, 5);
	SetEPTxStatus(2, 3<<4); //TX_VALID可用于发送
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

DEVICE_PROP Device_Property =
{
	Joystick_Data_Setup,
	Joystick_NoData_Setup,
	Joystick_Get_Interface_Setting,
	Joystick_GetDeviceDescriptor,
	Joystick_GetConfigDescriptor,
	Joystick_GetStringDescriptor,
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
void Joystick_Reset(void)
{
	/* Set Joystick_DEVICE as not configured */
	Device_Info.Current_Configuration = 0;
	Device_Info.Current_Interface = 0;/*the default Interface*/

	/* Current Feature initialization */
	Device_Info.Current_Feature = Joystick_ConfigDescriptor[7];

	USB->BTABLE =0;

	//设置端点0
	SetEPType(0, EP_CONTROL); //设置端点0为控制端点
	SetEPTxStatus(0, 1<<4); //TX_STALL
	USB_BT[0].ADDR_RX = (ENDP0_RXADDR & 0xfffe);
	USB_BT[0].ADDR_TX = (ENDP0_TXADDR & 0xfffe);
	USB_EP(0)= EPREG_1_SET | //写1无效的位
			(USB_EP(0) & (EPREG_MASK & (~(1<<8)))); //清除KIND
	SetEPRxCount(0, MaxPacketSize);
	SetEPRxStatus(0, 3<<12); //RX_VALID

	//设置端点1的 In 方向
	SetEPType(1, EP_INTERRUPT); //设置端点1为中断端点类型
	USB_BT[1].ADDR_TX = (ENDP1_TXADDR & 0xfffe); //设置发送数据的地址

	USB_BT[1].COUNT_TX = 8; //设置发送的长度
	SetEPTxStatus(1, EP_TX_NAK); //设置端点处于忙状态

	/* Initialize Endpoint Out 1 */
	USB_BT[1].ADDR_RX = (ENDP1_RXADDR & 0xfffe); //设置接收数据的地址
	SetEPRxCount(1, 2);  //设置接收长度
	SetEPRxStatus(1, EP_RX_VALID); //设置端点有效，可以接收数据

	/* Initialize Endpoint In 2 */
	SetEPType(2, EP_INTERRUPT); //初始化为中断端点类型
	USB_BT[2].ADDR_TX = (ENDP2_TXADDR & 0xfffe); //设置发送数据的地址
	USB_BT[2].COUNT_TX = 5; //设置发送的长度
	SetEPTxStatus(2, EP_TX_NAK); //设置端点处于忙状态

	usb_stat = USB_ATTACHED;

	SetDeviceAddress(0); //设置默认地址
}
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
			&& (Device_Info.inds.b.b1 < 2))
	{
		if (Device_Info.vals.b.b0 == REPORT_DESCRIPTOR)
		{
			if (Device_Info.inds.b.b1 == 0)
				CopyRoutine = KP_GetReportDescriptor;
			else
				CopyRoutine = Mouse_GetReportDescriptor;
		}
		else if (Device_Info.vals.b.b0 == HID_DESCRIPTOR_TYPE)
		{
			if (Device_Info.inds.b.b1 == 0)
				CopyRoutine = KP_GetHIDDescriptor;
			else
				CopyRoutine = Mouse_GetHIDDescriptor;
		}
	}
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

	Device_Info.Ctrl_Info.CopyData = CopyRoutine; //设置好之后的回调
	Device_Info.Ctrl_Info.Usb_wOffset = 0; //设置初始偏置为0
	(*CopyRoutine)(0); //设置len为0
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
	u8 wValue0 = Device_Info.vals.b.b1;
	return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
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
	u8 wValue0 = Device_Info.vals.b.b1;
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
		Device_Info.Ctrl_Info.Usb_wLength = 1;
		return 0;
	}
	else
	{
		return (u8 *)(&ProtocolValue);
	}
}

vu32 EP[8];
struct
{
	__IO RESUME_STATE eState;
	vu8 bESOFcnt;
} ResumeS;

vu32 remotewakeupon=0;
void Suspend(void)
{
	u16 i,t;

	for(i=0;i<8;i++) EP[i] = USB_EP(i); //保存端点寄存器的值
	t = USB->CNTR; //读-修改-写操作，缓存以后只需读一次 
	t |= (1<<10); USB->CNTR=t; //RESETM：复位中断使能
	t |= (1<<0);  USB->CNTR=t; //FRES：强制复位
	t &= ~(1<<0); USB->CNTR=t; //需要人工去掉复位状态

	while((USB->ISTR & (1<<10)) == 0); //RESET标志
	USB->ISTR = ~(1<<10); //清除RESET标志

	for (i=0;i<8;i++) USB_EP(i)= EP[i]; //恢复保存的端点寄存器。写1翻转的也行？
	//现在能安全的进入强制挂起
	t |= (1<<3); //FSUSP强制挂起
	USB->CNTR=t;

	/* force low-power mode in the macrocell */
	//t = USB->CNTR;
	t |= (1<<2); //LPMODE低功耗模式
	USB->CNTR=t;

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
	u16 t;
	/* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
	/* restart the clocks */
	/* CNTR_LPMODE = 0 */
	t = USB->CNTR;
	t &= (~CNTR_LPMODE);
	USB->CNTR=t;
	/* restore full power */
	/* ... on connected devices */
	Leave_LowPowerMode();
	/* reset FSUSP bit */
	USB->CNTR=CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM | CNTR_ESOFM | CNTR_RESETM;
	/* reverse suspend preparation */
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
	u16 wCNTR;

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

	Device_Info.ControlState = 2;
	//初始化设备
	u32 t0, t1, t2;
	t0 = *(u32*)(0x1FFFF7E8);
	t1 = *(u32*)(0x1FFFF7EC);
	t2 = *(u32*)(0x1FFFF7F0);
	Joystick_StringSerial[2] = (u8)(t0 & 0x000000FF);
	Joystick_StringSerial[4] = (u8)((t0 & 0x0000FF00) >> 8);
	Joystick_StringSerial[6] = (u8)((t0 & 0x00FF0000) >> 16);
	Joystick_StringSerial[8] = (u8)((t0 & 0xFF000000) >> 24);

	Joystick_StringSerial[10] = (u8)(t1 & 0x000000FF);
	Joystick_StringSerial[12] = (u8)((t1 & 0x0000FF00) >> 8);
	Joystick_StringSerial[14] = (u8)((t1 & 0x00FF0000) >> 16);
	Joystick_StringSerial[16] = (u8)((t1 & 0xFF000000) >> 24);

	Joystick_StringSerial[18] = (u8)(t2 & 0x000000FF);
	Joystick_StringSerial[20] = (u8)((t2 & 0x0000FF00) >> 8);
	Joystick_StringSerial[22] = (u8)((t2 & 0x00FF0000) >> 16);
	Joystick_StringSerial[24] = (u8)((t2 & 0xFF000000) >> 24);

	Device_Info.Current_Configuration = 0;
	/* Connect the device */
	USB->CNTR=1; //重启
	USB->CNTR=0;
	USB->ISTR=0; //清除中断
	USB->CNTR=CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM; //设置中断
	/* USB interrupts initialization */
	USB->ISTR=0;               /* clear pending interrupts */
	USB->CNTR= CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM | CNTR_ESOFM | CNTR_RESETM; /* set interrupts mask */

	usb_stat = USB_UNCONNECTED;
	USB_Cable_Config(1);
}

