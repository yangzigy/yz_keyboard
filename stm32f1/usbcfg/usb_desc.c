// File Name          : usb_desc.c

#include "usb_lib.h"
#include "usb_desc.h"
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

