#ifndef USB_CORE_H
#define USB_CORE_H

#include "main.h"

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

typedef enum 
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

typedef enum
{
	USB_SUCCESS = 0,    //
	USB_ERROR,
	USB_UNSUPPORT,
	USB_NOT_READY //处理未完成, 端点要发NAK
} RESULT;

typedef enum
{
	USB_EP_DIS     =(0),
	USB_EP_STALL   =(1),
	USB_EP_NAK     =(2),
	USB_EP_VALID   =(3)
} USB_EP_TXRX_STAT; //端点的收发状态，VALID准备好了

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
	union
	{
		u8 b;
		struct
		{
			u8 rx_type	:5; //接收者，0设备，1接口，2端点，3其他
			u8 req_type	:2; //请求类型，0标准，1类，2厂商，3保留
			u8 dir		:1; //0主机到设备，1设备到主机
		} s;
	} type; //请求类型
	u8 req;
	//u16 res0;

	S_U16_U8 vals;
	//u16 res1;

	S_U16_U8 inds;
	//u16 res2;

	u16 lens;
	//u16 res3;
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
typedef struct //由于需要复制，所以不需要改内存布局，不是真正的结构
{
	S_USB_REQ_TYPE type;
	u8 req;
	S_U16_U8 vals;
	S_U16_U8 inds;
	u16 lens;
} S_USB_REQ_DATA; //请求结构

void Setup0_Process(void); //接收到setup后的data0，放在端点0的接收缓存，处理
void In0_Process(void); //收发状态的接口要用

extern int usb_max_packet; //最大包长
extern S_SETUP0_RX usb_req_rxbuf; //usb端口0的标准请求接收缓存
extern RESULT (*class_data_setup)(void); //setup的处理回调函数
extern void (*usb_reset_fun)(void); //usb设备reset回调函数

extern u16 p0_len; //剩余发送数
extern u16 p0_ind; //发送时的分批发送的位置
extern u8 *p0_p; //收发指针
extern volatile USB_EP_TXRX_STAT usb_rx_stat; //接收控制端点保存之前的状态，可对其进行设置
extern volatile USB_EP_TXRX_STAT usb_tx_stat; //接收控制端点保存之前的状态，可对其进行设置

#endif

