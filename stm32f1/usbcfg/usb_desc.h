// File Name          : usb_desc.h
#ifndef __USB_DESC_H
#define __USB_DESC_H

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

#endif /* __USB_DESC_H */

