#ifndef __USB_CORE_H
#define __USB_CORE_H

#include "f1_usb.h"

typedef enum _RECIPIENT_TYPE
{
	DEVICE_RECIPIENT,     /* Recipient device */
	INTERFACE_RECIPIENT,  /* Recipient interface */
	ENDPOINT_RECIPIENT,   /* Recipient endpoint */
	OTHER_RECIPIENT
} RECIPIENT_TYPE;
typedef enum _STANDARD_REQUESTS
{
	GET_STATUS = 0,
	CLEAR_FEATURE,
	RESERVED1,
	SET_FEATURE,
	RESERVED2,
	SET_ADDRESS,
	GET_DESCRIPTOR,
	SET_DESCRIPTOR,
	GET_CONFIGURATION,
	SET_CONFIGURATION,
	GET_INTERFACE,
	SET_INTERFACE,
	TOTAL_sREQUEST,  /* Total number of Standard request */
	SYNCH_FRAME = 12
} STANDARD_REQUESTS;
/* Definition of "USBwValue" */
typedef enum _DESCRIPTOR_TYPE
{
	DEVICE_DESCRIPTOR = 1,
	CONFIG_DESCRIPTOR,
	STRING_DESCRIPTOR,
	INTERFACE_DESCRIPTOR,
	ENDPOINT_DESCRIPTOR
} DESCRIPTOR_TYPE;
typedef enum _FEATURE_SELECTOR //Feature selector of a SET_FEATURE or CLEAR_FEATURE
{
	ENDPOINT_STALL,
	DEVICE_REMOTE_WAKEUP
} FEATURE_SELECTOR;

typedef enum _CONTROL_STATE
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
} CONTROL_STATE;    /* The state machine states of a control pipe */

typedef struct OneDescriptor
{
	u8 *Descriptor;
	u16 Descriptor_Size;
}
ONE_DESCRIPTOR, *PONE_DESCRIPTOR;
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


/*-*-*-*-*-*-*-*-*-*-* Definitions for endpoint level -*-*-*-*-*-*-*-*-*-*-*-*/
typedef struct _ENDPOINT_INFO
{
	/* When send data out of the device,
	   CopyData() is used to get data buffer 'Length' bytes data
	   if Length is 0,
	   CopyData() returns the total length of the data
	   if the request is not supported, returns 0
	   (NEW Feature )
	   if CopyData() returns -1, the calling routine should not proceed
	   further and will resume the SETUP process by the class device
	   if Length is not 0,
	   CopyData() returns a pointer to indicate the data location
	   Usb_wLength is the data remain to be sent,
	   Usb_wOffset is the Offset of original data
	   When receive data from the host,
	   CopyData() is used to get user data buffer which is capable
	   of Length bytes data to copy data from the endpoint buffer.
	   if Length is 0,
	   CopyData() returns the available data length,
	   if Length is not 0,
	   CopyData() returns user buffer address
	   Usb_rLength is the data remain to be received,
	   Usb_rPointer is the Offset of data buffer
	   */
	u16  Usb_wLength;
	u16  Usb_wOffset;
	u16  PacketSize;
	u8   *(*CopyData)(u16 Length);
}ENDPOINT_INFO;

extern int Total_Configuration;/* Number of configuration available */

typedef union
{
	u16 w;
	struct BW
	{
		u8 bb1;
		u8 bb0;
	}
	bw;
} uint16_t_uint8_t;

typedef struct _DEVICE_INFO
{
	u8 USBbmRequestType;       /* bmRequestType */
	u8 USBbRequest;            /* bRequest */
	uint16_t_uint8_t USBwValues;         /* wValue */
	uint16_t_uint8_t USBwIndexs;         /* wIndex */
	uint16_t_uint8_t USBwLengths;        /* wLength */

	u8 ControlState;           /* of type CONTROL_STATE */
	u8 Current_Feature;
	u8 Current_Configuration;   /* Selected configuration */
	u8 Current_Interface;       /* Selected interface of current configuration */
	u8 Current_AlternateSetting;/* Selected Alternate Setting of current
								   interface*/

	ENDPOINT_INFO Ctrl_Info;
}DEVICE_INFO;

typedef struct _DEVICE_PROP
{
	void (*Init)(void);        /* Initialize the device */
	void (*Reset)(void);       /* Reset routine of this device */

	/* Device dependent process after the status stage */
	void (*Process_Status_IN)(void);
	void (*Process_Status_OUT)(void);

	/* Procedure of process on setup stage of a class specified request with data stage */
	/* All class specified requests with data stage are processed in Class_Data_Setup
	   Class_Data_Setup()
	   responses to check all special requests and fills ENDPOINT_INFO
	   according to the request
	   If IN tokens are expected, then wLength & wOffset will be filled
	   with the total transferring bytes and the starting position
	   If OUT tokens are expected, then rLength & rOffset will be filled
	   with the total expected bytes and the starting position in the buffer

	   If the request is valid, Class_Data_Setup returns SUCCESS, else UNSUPPORT

CAUTION:
Since GET_CONFIGURATION & GET_INTERFACE are highly related to
the individual classes, they will be checked and processed here.
*/
	RESULT (*Class_Data_Setup)(u8 RequestNo);

	/* Procedure of process on setup stage of a class specified request without data stage */
	/* All class specified requests without data stage are processed in Class_NoData_Setup
	   Class_NoData_Setup
	   responses to check all special requests and perform the request

CAUTION:
Since SET_CONFIGURATION & SET_INTERFACE are highly related to
the individual classes, they will be checked and processed here.
*/
	RESULT (*Class_NoData_Setup)(u8 RequestNo);

	/*Class_Get_Interface_Setting
	  This function is used by the file usb_core.c to test if the selected Interface
	  and Alternate Setting (u8 Interface, u8 AlternateSetting) are supported by
	  the application.
	  This function is writing by user. It should return "SUCCESS" if the Interface
	  and Alternate Setting are supported by the application or "UNSUPPORT" if they
	  are not supported. */

	RESULT  (*Class_Get_Interface_Setting)(u8 Interface, u8 AlternateSetting);

	u8* (*GetDeviceDescriptor)(u16 Length);
	u8* (*GetConfigDescriptor)(u16 Length);
	u8* (*GetStringDescriptor)(u16 Length);

	/* This field is not used in current library version. It is kept only for 
	   compatibility with previous versions */
	void* RxEP_buffer;

	u8 MaxPacketSize;

}DEVICE_PROP;

typedef struct _USER_STANDARD_REQUESTS
{
	void (*User_GetConfiguration)(void);       /* Get Configuration */
	void (*User_SetConfiguration)(void);       /* Set Configuration */
	void (*User_GetInterface)(void);           /* Get Interface */
	void (*User_SetInterface)(void);           /* Set Interface */
	void (*User_GetStatus)(void);              /* Get Status */
	void (*User_ClearFeature)(void);           /* Clear Feature */
	void (*User_SetEndPointFeature)(void);     /* Set Endpoint Feature */
	void (*User_SetDeviceFeature)(void);       /* Set Device Feature */
	void (*User_SetDeviceAddress)(void);       /* Set Device Address */
}
USER_STANDARD_REQUESTS;

#define REQUEST_TYPE      0x60  /* Mask to get request type */
#define STANDARD_REQUEST  0x00  /* Standard request */
#define CLASS_REQUEST     0x20  /* Class request */
#define VENDOR_REQUEST    0x40  /* Vendor request */

#define RECIPIENT         0x1F  /* Mask to get recipient */
/* Exported constants --------------------------------------------------------*/
#define Type_Recipient (pInformation->USBbmRequestType & (REQUEST_TYPE | RECIPIENT))

#define Usb_rLength Usb_wLength
#define Usb_rOffset Usb_wOffset

#define USBwValue USBwValues.w
#define USBwValue0 USBwValues.bw.bb0
#define USBwValue1 USBwValues.bw.bb1
#define USBwIndex USBwIndexs.w
#define USBwIndex0 USBwIndexs.bw.bb0
#define USBwIndex1 USBwIndexs.bw.bb1
#define USBwLength USBwLengths.w
#define USBwLength0 USBwLengths.bw.bb0
#define USBwLength1 USBwLengths.bw.bb1

u8 Setup0_Process(void);
u8 Post0_Process(void);
u8 Out0_Process(void);
u8 In0_Process(void);

RESULT Standard_SetEndPointFeature(void);
RESULT Standard_SetDeviceFeature(void);

u8 *Standard_GetConfiguration(u16 Length);
RESULT Standard_SetConfiguration(void);
u8 *Standard_GetInterface(u16 Length);
RESULT Standard_SetInterface(void);
u8 *Standard_GetDescriptorData(u16 Length, PONE_DESCRIPTOR pDesc);

u8 *Standard_GetStatus(u16 Length);
RESULT Standard_ClearFeature(void);
void SetDeviceAddress(u8);
void NOP_Process(void);

extern DEVICE_PROP Device_Property;
extern  USER_STANDARD_REQUESTS User_Standard_Requests;
extern DEVICE_INFO Device_Info;

/* cells saving status during interrupt servicing */
extern vu16 SaveRState;
extern vu16 SaveTState;

#endif /* __USB_CORE_H */

