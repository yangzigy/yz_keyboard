#ifndef F1_USB_H
#define F1_USB_H

#include "stm32f1_sys.h"

typedef struct
{
	vu16 EP0R;          //USB Endpoint 0 register,              offset: 0x00
	vu16 RESERVED0;
	vu16 EP1R;          //USB Endpoint 1 register,              offset: 0x04
	vu16 RESERVED1;
	vu16 EP2R;          //USB Endpoint 2 register,              offset: 0x08
	vu16 RESERVED2;
	vu16 EP3R;          //USB Endpoint 3 register,              offset: 0x0C
	vu16 RESERVED3;
	vu16 EP4R;          //USB Endpoint 4 register,              offset: 0x10
	vu16 RESERVED4;
	vu16 EP5R;          //USB Endpoint 5 register,              offset: 0x14
	vu16 RESERVED5;
	vu16 EP6R;          //USB Endpoint 6 register,              offset: 0x18
	vu16 RESERVED6;
	vu16 EP7R;          //USB Endpoint 7 register,              offset: 0x1C
	vu16 RESERVED7[17];
	vu16 CNTR;          //Control register,                     offset: 0x40
	vu16 RESERVED8;
	vu16 ISTR;          //Interrupt status register,            offset: 0x44
	vu16 RESERVED9;
	vu16 FNR;           //Frame number register,                offset: 0x48
	vu16 RESERVEDA;
	vu16 DADDR;         //Device address register,              offset: 0x4C
	vu16 RESERVEDB;
	vu16 BTABLE;        //Buffer Table address register,        offset: 0x50
	vu16 RESERVEDC;
} USB_TypeDef;

typedef struct
{
	vu16 ADDR_TX;
	vu16 RESERVED0;
	vu16 COUNT_TX;
	vu16 RESERVED1;
	vu16 ADDR_RX;
	vu16 RESERVED2;
	vu16 COUNT_RX;
	vu16 RESERVED3;
} USB_BUF_TAB_ITEM; //缓存表项结构，USB看是8字节，CPU看是16字节

//缓冲区512字节，256个16bit字 CPU按32bit访问，用低2字节，所以CPU这边看是1KB
//每个端点有两个分组缓冲，一个收一个发。可位于缓冲区的任意位置，由缓冲区表描述，缓冲区表也在缓冲区中。
//缓冲区表每一项都是4个16bit字，TX相对地址、长度；RX相对地址、长度

#define USB_BASE            (0x40005C00L)
#define PMAAddr				(0x40006000L)  //缓冲区地址
#define USB                 ((USB_TypeDef *) USB_BASE)
#define USB_EP(i)           (((vu16 *)USB_BASE)[(i)*2]) //通过索引访问端点寄存器
#define USB_BT				((USB_BUF_TAB_ITEM *)PMAAddr) //USB缓存表数组

/******************************************************************************/
/*             CNTR control register bits definitions                         */
/******************************************************************************/
#define CNTR_CTRM   (0x8000) /* Correct TRansfer Mask */
#define CNTR_DOVRM  (0x4000) /* DMA OVeR/underrun Mask */
#define CNTR_ERRM   (0x2000) /* ERRor Mask */
#define CNTR_WKUPM  (0x1000) /* WaKe UP Mask */
#define CNTR_SUSPM  (0x0800) /* SUSPend Mask */
#define CNTR_RESETM (0x0400) /* RESET Mask   */
#define CNTR_SOFM   (0x0200) /* Start Of Frame Mask */
#define CNTR_ESOFM  (0x0100) /* Expected Start Of Frame Mask */


#define CNTR_RESUME (0x0010) /* RESUME request */
#define CNTR_FSUSP  (0x0008) /* Force SUSPend */
#define CNTR_LPMODE (0x0004) /* Low-power MODE */
#define CNTR_PDWN   (0x0002) /* Power DoWN */
#define CNTR_FRES   (0x0001) /* Force USB RESet */

/******************************************************************************/
/*                            Endpoint register                               */
/******************************************************************************/
/* bit positions */
#define EP_CTR_RX      (0x8000) /* EndPoint Correct TRansfer RX */
#define EP_DTOG_RX     (0x4000) /* EndPoint Data TOGGLE RX */
#define EPRX_STAT      (0x3000) /* EndPoint RX STATus bit field */
#define EP_SETUP       (0x0800) /* EndPoint SETUP */
#define EP_T_FIELD     (0x0600) /* EndPoint TYPE */
#define EP_KIND        (0x0100) /* EndPoint KIND */
#define EP_CTR_TX      (0x0080) /* EndPoint Correct TRansfer TX */
#define EP_DTOG_TX     (0x0040) /* EndPoint Data TOGGLE TX */
#define EPTX_STAT      (0x0030) /* EndPoint TX STATus bit field */
#define EPADDR_FIELD   (0x000F) /* EndPoint ADDRess FIELD */

//端点寄存器中，写0无效的位
#define EPREG_0_SET    ((1<<14) | (3<<12) | (1<<6) | (3<<4))
//端点寄存器中，写1无效的位
#define EPREG_1_SET    ((1<<15) | (1<<7))
//访问端点寄存器时，需要保留的位
#define EPREG_MASK     ((1<<15)|(1<<11)|(3<<9)|(1<<8)|(1<<7)|0x0f)

/* EP_TYPE[1:0] EndPoint TYPE */
#define EP_TYPE_MASK   (0x0600) /* EndPoint TYPE Mask */
#define EP_BULK        (0x0000) /* EndPoint BULK */
#define EP_CONTROL     (0x0200) /* EndPoint CONTROL */
#define EP_ISOCHRONOUS (0x0400) /* EndPoint ISOCHRONOUS */
#define EP_INTERRUPT   (0x0600) /* EndPoint INTERRUPT */
#define EP_T_MASK      (~EP_T_FIELD & EPREG_MASK)

/* STAT_TX[1:0] STATus for TX transfer */
#define EP_TX_DIS      (0x0000) /* EndPoint TX DISabled */
#define EP_TX_STALL    (0x0010) /* EndPoint TX STALLed */
#define EP_TX_NAK      (0x0020) /* EndPoint TX NAKed */
#define EP_TX_VALID    (0x0030) /* EndPoint TX VALID */
//#define EPTX_DTOG1     (0x0010) /* EndPoint TX Data TOGgle bit1 */
#define EPTX_DTOG2     (0x0020) /* EndPoint TX Data TOGgle bit2 */
#define EPTX_DTOGMASK  (EPTX_STAT|EPREG_MASK)

/* STAT_RX[1:0] STATus for RX transfer */
#define EP_RX_DIS      (0x0000) /* EndPoint RX DISabled */
#define EP_RX_STALL    (0x1000) /* EndPoint RX STALLed */
#define EP_RX_NAK      (0x2000) /* EndPoint RX NAKed */
#define EP_RX_VALID    (0x3000) /* EndPoint RX VALID */
#define EPRX_DTOG1     (0x1000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOG2     (0x2000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOGMASK  (EPRX_STAT|EPREG_MASK)

extern int EP_num;

void usb_hal_ini(void);

void SetEPType(u8 i, u16 wType); //设置端点类型
void SetEPTxStatus(u8 i, u16 d); //设置端点发送状态
void SetEPRxStatus(u8 i, u16 d); //设置端点接收状态
void SetEPRxTxStatus(u8 i, u16 rx,u16 tx); //设置端点发送、接收状态
void ClearDTOG_RX(u8 i);
void ClearDTOG_TX(u8 i);
void SetEPRxCount(u8 port, u16 n); //设置接收缓存大小

uint16_t GetEPTxCount(uint8_t /*bEpNum*/);
uint16_t GetEPRxCount(uint8_t /*bEpNum*/);
void FreeUserBuffer(uint8_t bEpNum/*bEpNum*/, uint8_t bDir);

void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);

#endif

