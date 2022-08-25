
#ifndef USB_KB_M_H
#define USB_KB_M_H

#include "stm32f1_sys.h"
#include "usb_core.h"

#define EP_NUM     (3) //总端点数

/* buffer table base address */
/* buffer table base address */
#define BTABLE_ADDRESS      (0x00)

/* EP0  */
/* rx/tx buffer base address */
#define ENDP0_RXADDR        (0x18)
#define ENDP0_TXADDR        (0x58)

/* EP1  */
/* tx buffer base address */
#define ENDP1_TXADDR        (0x100)
#define ENDP1_RXADDR		(0x110)
/* EP2  */
/* tx buffer base address */
#define ENDP2_TXADDR        (0x120)

/* mask defining which events has to be handled */
/* by the device application software */
#define IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM \
                 | CNTR_ESOFM | CNTR_RESETM )

/* CTR service routines */
/* associated to defined endpoints */
#define  EP1_IN_Callback   NOP_Process
#define  EP2_IN_Callback   NOP_Process
#define  EP3_IN_Callback   NOP_Process
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

//#define  EP1_OUT_Callback   NOP_Process
#define  EP2_OUT_Callback   NOP_Process
#define  EP3_OUT_Callback   NOP_Process
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process

void keyboard_send(u8 *buf);
void mouse_send(u8 *buf);

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
extern  vu32 bDeviceState; /* USB device status */
extern vu32 fSuspendEnabled;  /* true when suspend is possible */

typedef enum _HID_REQUESTS
{
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,

	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

void Joystick_init(void);
void Joystick_Reset(void);
void Joystick_SetConfiguration(void);
void Joystick_SetDeviceAddress (void);
void Joystick_Status_In (void);
void Joystick_Status_Out (void);
RESULT Joystick_Data_Setup(u8);
RESULT Joystick_NoData_Setup(u8);
RESULT Joystick_Get_Interface_Setting(u8 Interface, u8 AlternateSetting);
u8 *Joystick_GetDeviceDescriptor(u16 );
u8 *Joystick_GetConfigDescriptor(u16);
u8 *Joystick_GetStringDescriptor(u16);
RESULT Joystick_SetProtocol(void);
u8 *Joystick_GetProtocolValue(u16 Length);
RESULT Joystick_SetProtocol(void);
u8 *Joystick_GetReportDescriptor(u16 Length);
u8 *Joystick_GetHIDDescriptor(u16 Length);
/***********************************************/
u8 *Mouse_GetReportDescriptor(u16 Length);
u8 *KP_GetReportDescriptor(u16 Length);
u8 *Mouse_GetHIDDescriptor(u16 Length);
u8 *KP_GetHIDDescriptor(u16 Length);
/***********************************************/

/* Exported define -----------------------------------------------------------*/
#define Joystick_GetConfiguration          NOP_Process
//#define Joystick_SetConfiguration          NOP_Process
#define Joystick_GetInterface              NOP_Process
#define Joystick_SetInterface              NOP_Process
#define Joystick_GetStatus                 NOP_Process
#define Joystick_ClearFeature              NOP_Process
#define Joystick_SetEndPointFeature        NOP_Process
#define Joystick_SetDeviceFeature          NOP_Process
//#define Joystick_SetDeviceAddress          NOP_Process

#define REPORT_DESCRIPTOR                  0x22

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01//设备描述符类型．固定为0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02//配置描述符类型．固定为0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define JOYSTICK_SIZ_HID_DESC                   0x09

#define JOYSTICK_SIZ_DEVICE_DESC                18//设备描述符，只有一个长度固定
/*********************************************************/
#define KP_OFF_HID_DESC							18
#define Mouse_OFF_HID_DESC						50

#define JOYSTICK_SIZ_CONFIG_DESC                66
#define KP_ReportDescriptor_Size				63
#define Mouse_ReportDescriptor_Size				54
/*********************************************************/
#define JOYSTICK_SIZ_STRING_LANGID              4
#define JOYSTICK_SIZ_STRING_VENDOR              38
#define JOYSTICK_SIZ_STRING_PRODUCT             30
#define JOYSTICK_SIZ_STRING_SERIAL              26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

/* Exported functions ------------------------------------------------------- */
/*******************************************************************************/
extern const u8 KeyboardReportDescriptor[KP_ReportDescriptor_Size];
extern const u8 MouseReportDescriptor[Mouse_ReportDescriptor_Size];
/*******************************************************************************/

extern const u8 Joystick_DeviceDescriptor[JOYSTICK_SIZ_DEVICE_DESC];
extern const u8 Joystick_ConfigDescriptor[JOYSTICK_SIZ_CONFIG_DESC];
extern const u8 Joystick_StringLangID[JOYSTICK_SIZ_STRING_LANGID];
extern const u8 Joystick_StringVendor[JOYSTICK_SIZ_STRING_VENDOR];
extern const u8 Joystick_StringProduct[JOYSTICK_SIZ_STRING_PRODUCT];
extern u8 Joystick_StringSerial[JOYSTICK_SIZ_STRING_SERIAL];

#endif 
