#ifndef USB_CORE_H
#define USB_CORE_H

#include "f1_usb.h"

typedef enum
{
	USB_UNCONNECTED, //未连接
	USB_ATTACHED, //连接
	USB_POWERED, //
	USB_SUSPENDED,
	USB_ADDRESSED,
	USB_CONFIGURED
} DEVICE_STATE; //设备状态

extern DEVICE_STATE usb_stat; //usb状态

typedef enum _DESCRIPTOR_TYPE
{
	DEVICE_DESCRIPTOR = 1,
	CONFIG_DESCRIPTOR,
	STRING_DESCRIPTOR,
	INTERFACE_DESCRIPTOR,
	ENDPOINT_DESCRIPTOR
} DESCRIPTOR_TYPE;

typedef enum 
{
	WAIT_SETUP,       /* 0 */
	SETTING_UP,       /* 1 */
	IN_DATA,          /* 2 */
	OUT_DATA,         /* 3 */
	LAST_IN_DATA,     /* 4 */
	LAST_OUT_DATA,    /* 5 */
	WAIT_STATUS_IN,   /* 7 */
	WAIT_STATUS_OUT,  /* 8 */
	STALLED,          /* 9 */
	PAUSE             /* 10 */
} S_USB_CTRL_STATE;    //usb控制状态

typedef struct OneDescriptor
{
	u8 *Descriptor;
	u16 Descriptor_Size;
} ONE_DESCRIPTOR, *PONE_DESCRIPTOR;
/* All the request process routines return a value of this type
   If the return value is not SUCCESS or NOT_READY,
   the software will STALL the correspond endpoint */
typedef enum _RESULT
{
	USB_SUCCESS = 0,    /* Process successfully */
	USB_ERROR,
	USB_UNSUPPORT,
	USB_NOT_READY       /* The process has not been finished, endpoint will be
						   NAK to further request */
} RESULT;
typedef struct 
{
	/* 设备发送数据时
	   CopyData() is used to get data buffer 'Length' bytes data
	   Length为0时：CopyData() 返回数据总数
	   if the request is not supported, returns 0
	   if CopyData() returns -1, the calling routine should not proceed
	   further and will resume the SETUP process by the class device
	   Length非0时：CopyData() 返回数据位置
	   Usb_wOffset is the Offset of original data
	   设备接收数据时 CopyData() 用于获得数据缓存，要求长度Length字节
	   */
	u16  Usb_wLength; //Usb_wLength 剩余发送数
	u16  Usb_wOffset; //发送时的分批发送的位置
	u16  PacketSize;
	u8   *(*CopyData)(u16 Length); //len为0时准备数据总数
}ENDPOINT_INFO;

typedef union
{
	u16 w;
	struct
	{
		u8 b0;
		u8 b1;
	} b;
} S_U16_U8;

typedef enum 
{
	GET_STATUS = 0,
	CLEAR_FEATURE,
	USB_RESERVED1,
	SET_FEATURE,
	USB_RESERVED2,
	SET_ADDRESS,
	GET_DESCRIPTOR,
	SET_DESCRIPTOR,
	GET_CONFIGURATION,
	SET_CONFIGURATION,
	GET_INTERFACE,
	SET_INTERFACE,
	SYNCH_FRAME = 12
} USB_REQUESTS; //标准请求定义
#pragma pack(1)
typedef struct //在USB缓存中，4个字节有2个空
{
	u8 type;
	u8 req;
	u16 res0;

	S_U16_U8 vals;
	u16 res1;

	S_U16_U8 inds;
	u16 res2;

	u16 lens;
	u16 res3;
} S_SETUP0_RX; //setup0接收结构 标准设备请求结构
#pragma pack()
typedef union
{
	u8 b;
	struct
	{
		u8 rx_type	:5; //接收者，0设备，1接口，2端点，3其他
		u8 req_type	:2; //请求类型，0标准，1类，2厂商，3保留
		u8 dir		:1; //0主机到设备，1设备到主机
	} s;
} S_USB_REQ_TYPE; //请求类型
typedef struct 
{
	S_USB_REQ_TYPE type;
	u8 req;
	S_U16_U8 vals;
	S_U16_U8 inds;
	u16 lens;

	//u8 ControlState;           /* of type CONTROL_STATE */
	ENDPOINT_INFO Ctrl_Info;
}DEVICE_INFO;

typedef struct _DEVICE_PROP
{
	RESULT (*Class_Data_Setup)(u8 RequestNo);

	u8* (*GetDeviceDescriptor)(u16 Length);
	u8* (*GetConfigDescriptor)(u16 Length);
	u8* (*GetStringDescriptor)(u16 Length);

}DEVICE_PROP;

void Setup0_Process(void);
void Out0_Process(void);
void In0_Process(void);

u8 *Standard_GetDescriptorData(u16 Length, PONE_DESCRIPTOR pDesc);

void SetDeviceAddress(u8);
void NOP_Process(void);

extern u8 MaxPacketSize;
extern DEVICE_PROP Device_Property;
extern DEVICE_INFO Device_Info;

/* cells saving status during interrupt servicing */
extern vu16 SaveRState;
extern vu16 SaveTState;

#endif

