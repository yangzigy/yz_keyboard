
#ifndef USB_KB_M_H
#define USB_KB_M_H

#include "stm32f1_sys.h"
#include "usb_core.h"

/* EP0  */
/* rx/tx buffer base address */
#define ENDP0_RXADDR        (0x18 & 0xfffe)
#define ENDP0_TXADDR        (0x58 & 0xfffe)

/* EP1  */
/* tx buffer base address */
#define ENDP1_TXADDR        (0x100 & 0xfffe)
#define ENDP1_RXADDR		(0x110 & 0xfffe)
/* EP2  */
/* tx buffer base address */
#define ENDP2_TXADDR        (0x120 & 0xfffe)

void keyboard_send(u8 *buf);
void mouse_send(u8 *buf);

typedef enum _HID_REQUESTS
{
	GET_REPORT = 1,
	GET_IDLE,
	GET_PROTOCOL,

	SET_REPORT = 9,
	SET_IDLE,
	SET_PROTOCOL
} HID_REQUESTS;

void usb_ini(void);

#define REPORT_DESCRIPTOR                  0x22

#define HID_DESCRIPTOR_TYPE                     0x21
#define YZKB_SIZ_HID_DESC                   0x09

#define YZKB_SIZ_DEVICE_DESC                18//设备描述符，只有一个长度固定
/*********************************************************/
#define KP_OFF_HID_DESC							18
#define Mouse_OFF_HID_DESC						50

#define YZKB_SIZ_CONFIG_DESC                66
#define KP_ReportDescriptor_Size				63
#define Mouse_ReportDescriptor_Size				54
/*********************************************************/
#define YZKB_SIZ_STRING_LANGID              4
#define YZKB_SIZ_STRING_VENDOR              38
#define YZKB_SIZ_STRING_PRODUCT             30
#define YZKB_SIZ_STRING_SERIAL              26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09
#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#endif 
