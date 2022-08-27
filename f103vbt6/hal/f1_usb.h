#ifndef __USB_REGS_H
#define __USB_REGS_H

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

#define RegBase  (0x40005C00L)  /* USB_IP Peripheral Registers base address */
#define PMAAddr  (0x40006000L)  /* USB_IP Packet Memory Area base address   */

#define USB_BASE            (0x40005C00L)
#define USB                 ((USB_TypeDef *) USB_BASE)
#define USB_EP(i)           (((vu16 *)USB_BASE)[(i)*2]) //通过索引访问端点寄存器

typedef enum 
{
	EP_DBUF_ERR,
	EP_DBUF_OUT,
	EP_DBUF_IN
} EP_DBUF_DIR; //双缓存端点方向

#define ISTR_CTR    (0x8000) /* Correct TRansfer (clear-only bit) */
#define ISTR_DOVR   (0x4000) /* DMA OVeR/underrun (clear-only bit) */
#define ISTR_ERR    (0x2000) /* ERRor (clear-only bit) */
#define ISTR_WKUP   (0x1000) /* WaKe UP (clear-only bit) */
#define ISTR_SUSP   (0x0800) /* SUSPend (clear-only bit) */
#define ISTR_RESET  (0x0400) /* RESET (clear-only bit) */
#define ISTR_SOF    (0x0200) /* Start Of Frame (clear-only bit) */
#define ISTR_ESOF   (0x0100) /* Expected Start Of Frame (clear-only bit) */


#define ISTR_DIR    (0x0010)  /* DIRection of transaction (read-only bit)  */
#define ISTR_EP_ID  (0x000F)  /* EndPoint IDentifier (read-only bit)  */

#define CLR_CTR    (~ISTR_CTR)   /* clear Correct TRansfer bit */
#define CLR_DOVR   (~ISTR_DOVR)  /* clear DMA OVeR/underrun bit*/
#define CLR_ERR    (~ISTR_ERR)   /* clear ERRor bit */
#define CLR_WKUP   (~ISTR_WKUP)  /* clear WaKe UP bit     */
#define CLR_SUSP   (~ISTR_SUSP)  /* clear SUSPend bit     */
#define CLR_RESET  (~ISTR_RESET) /* clear RESET bit      */
#define CLR_SOF    (~ISTR_SOF)   /* clear Start Of Frame bit   */
#define CLR_ESOF   (~ISTR_ESOF)  /* clear Expected Start Of Frame bit */

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

#define EPREG_MASK     ((1<<15)|(1<<11)|(3<<9)|EP_KIND|EP_CTR_TX|EPADDR_FIELD)

/* EP_TYPE[1:0] EndPoint TYPE */
#define EP_TYPE_MASK   (0x0600) /* EndPoint TYPE Mask */
#define EP_BULK        (0x0000) /* EndPoint BULK */
#define EP_CONTROL     (0x0200) /* EndPoint CONTROL */
#define EP_ISOCHRONOUS (0x0400) /* EndPoint ISOCHRONOUS */
#define EP_INTERRUPT   (0x0600) /* EndPoint INTERRUPT */
#define EP_T_MASK      (~EP_T_FIELD & EPREG_MASK)

/* EP_KIND EndPoint KIND */
#define EPKIND_MASK    (~EP_KIND & EPREG_MASK)

/* STAT_TX[1:0] STATus for TX transfer */
#define EP_TX_DIS      (0x0000) /* EndPoint TX DISabled */
#define EP_TX_STALL    (0x0010) /* EndPoint TX STALLed */
#define EP_TX_NAK      (0x0020) /* EndPoint TX NAKed */
#define EP_TX_VALID    (0x0030) /* EndPoint TX VALID */
#define EPTX_DTOG1     (0x0010) /* EndPoint TX Data TOGgle bit1 */
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

/* SetENDPOINT */
#define _SetENDPOINT(i,wRegValue)  (USB_EP(i)= (uint16_t)wRegValue)

/*******************************************************************************
 * Macro Name     : SetEPTxStatus
 * Description    : sets the status for tx transfer (bits STAT_TX[1:0]).
 * Input          : bEpNum: Endpoint Number. 
 *                  wState: new state
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPTxStatus(bEpNum,wState) {\
	register uint16_t _wRegVal;       \
	_wRegVal = USB_EP(bEpNum) & EPTX_DTOGMASK;\
	/* toggle first bit ? */     \
	if((EPTX_DTOG1 & wState)!= 0)      \
	_wRegVal ^= EPTX_DTOG1;        \
	/* toggle second bit ?  */         \
	if((EPTX_DTOG2 & wState)!= 0)      \
	_wRegVal ^= EPTX_DTOG2;        \
	_SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX));    \
} /* _SetEPTxStatus */

/*******************************************************************************
 * Macro Name     : SetEPRxStatus
 * Description    : sets the status for rx transfer (bits STAT_TX[1:0])
 * Input          : bEpNum: Endpoint Number. 
 *                  wState: new state.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPRxStatus(bEpNum,wState) {\
	register uint16_t _wRegVal;   \
	\
	_wRegVal = USB_EP(bEpNum) & EPRX_DTOGMASK;\
	/* toggle first bit ? */  \
	if((EPRX_DTOG1 & wState)!= 0) \
	_wRegVal ^= EPRX_DTOG1;  \
	/* toggle second bit ? */  \
	if((EPRX_DTOG2 & wState)!= 0) \
	_wRegVal ^= EPRX_DTOG2;  \
	_SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX)); \
} /* _SetEPRxStatus */

/*******************************************************************************
 * Macro Name     : SetEPRxTxStatus
 * Description    : sets the status for rx & tx (bits STAT_TX[1:0] & STAT_RX[1:0])
 * Input          : bEpNum: Endpoint Number. 
 *                  wStaterx: new state.
 *                  wStatetx: new state.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPRxTxStatus(bEpNum,wStaterx,wStatetx) {\
	register uint32_t _wRegVal;   \
	\
	_wRegVal = USB_EP(bEpNum) & (EPRX_DTOGMASK |EPTX_STAT) ;\
	/* toggle first bit ? */  \
	if((EPRX_DTOG1 & wStaterx)!= 0) \
	_wRegVal ^= EPRX_DTOG1;  \
	/* toggle second bit ? */  \
	if((EPRX_DTOG2 & wStaterx)!= 0) \
	_wRegVal ^= EPRX_DTOG2;  \
	/* toggle first bit ? */     \
	if((EPTX_DTOG1 & wStatetx)!= 0)      \
	_wRegVal ^= EPTX_DTOG1;        \
	/* toggle second bit ?  */         \
	if((EPTX_DTOG2 & wStatetx)!= 0)      \
	_wRegVal ^= EPTX_DTOG2;        \
	_SetENDPOINT(bEpNum, _wRegVal | EP_CTR_RX|EP_CTR_TX);    \
} /* _SetEPRxTxStatus */
/*******************************************************************************
 * Macro Name     : GetEPTxStatus / GetEPRxStatus 
 * Description    : gets the status for tx/rx transfer (bits STAT_TX[1:0]
 *                  /STAT_RX[1:0])
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : status .
 *******************************************************************************/
#define _GetEPTxStatus(bEpNum) ((uint16_t)USB_EP(bEpNum) & EPTX_STAT)

#define _GetEPRxStatus(bEpNum) ((uint16_t)USB_EP(bEpNum) & EPRX_STAT)

/*******************************************************************************
 * Macro Name     : GetTxStallStatus / GetRxStallStatus.
 * Description    : checks stall condition in an endpoint.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : TRUE = endpoint in stall condition.
 *******************************************************************************/
#define _GetTxStallStatus(bEpNum) (_GetEPTxStatus(bEpNum) \
		== EP_TX_STALL)
#define _GetRxStallStatus(bEpNum) (_GetEPRxStatus(bEpNum) \
		== EP_RX_STALL)

/*******************************************************************************
 * Macro Name     : SetEP_KIND / ClearEP_KIND.
 * Description    : set & clear EP_KIND bit.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEP_KIND(bEpNum)    (_SetENDPOINT(bEpNum, \
			(EP_CTR_RX|EP_CTR_TX|((USB_EP(bEpNum) | EP_KIND) & EPREG_MASK))))
#define _ClearEP_KIND(bEpNum)  (_SetENDPOINT(bEpNum, \
			(EP_CTR_RX|EP_CTR_TX|(USB_EP(bEpNum) & EPKIND_MASK))))

/*******************************************************************************
 * Macro Name     : ClearEP_CTR_RX / ClearEP_CTR_TX.
 * Description    : Clears bit CTR_RX / CTR_TX in the endpoint register.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _ClearEP_CTR_RX(bEpNum)   (_SetENDPOINT(bEpNum,\
			USB_EP(bEpNum) & 0x7FFF & EPREG_MASK))
#define _ClearEP_CTR_TX(bEpNum)   (_SetENDPOINT(bEpNum,\
			USB_EP(bEpNum) & 0xFF7F & EPREG_MASK))

/*******************************************************************************
 * Macro Name     : ToggleDTOG_RX / ToggleDTOG_TX .
 * Description    : Toggles DTOG_RX / DTOG_TX bit in the endpoint register.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _ToggleDTOG_RX(bEpNum)    (_SetENDPOINT(bEpNum, \
			EP_CTR_RX|EP_CTR_TX|EP_DTOG_RX | (USB_EP(bEpNum) & EPREG_MASK)))
#define _ToggleDTOG_TX(bEpNum)    (_SetENDPOINT(bEpNum, \
			EP_CTR_RX|EP_CTR_TX|EP_DTOG_TX | (USB_EP(bEpNum) & EPREG_MASK)))

/*******************************************************************************
 * Macro Name     : ClearDTOG_RX / ClearDTOG_TX.
 * Description    : Clears DTOG_RX / DTOG_TX bit in the endpoint register.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _ClearDTOG_RX(bEpNum)  if((USB_EP(bEpNum) & EP_DTOG_RX) != 0)\
																		 _ToggleDTOG_RX(bEpNum)
#define _ClearDTOG_TX(bEpNum)  if((USB_EP(bEpNum) & EP_DTOG_TX) != 0)\
																		 _ToggleDTOG_TX(bEpNum)

#define _pEPTxAddr(bEpNum) ((uint32_t *)((USB->BTABLE + bEpNum*8  )*2 + PMAAddr))
#define _pEPTxCount(bEpNum) ((uint32_t *)((USB->BTABLE + bEpNum*8+2)*2 + PMAAddr))
#define _pEPRxAddr(bEpNum) ((uint32_t *)((USB->BTABLE + bEpNum*8+4)*2 + PMAAddr))
#define _pEPRxCount(bEpNum) ((uint32_t *)((USB->BTABLE + bEpNum*8+6)*2 + PMAAddr))

/*******************************************************************************
 * Macro Name     : SetEPTxAddr / SetEPRxAddr.
 * Description    : sets address of the tx/rx buffer.
 * Input          : bEpNum: Endpoint Number.
 *                  wAddr: address to be set (must be word aligned).
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPTxAddr(bEpNum,wAddr) (*_pEPTxAddr(bEpNum) = ((wAddr >> 1) << 1))
#define _SetEPRxAddr(bEpNum,wAddr) (*_pEPRxAddr(bEpNum) = ((wAddr >> 1) << 1))

/*******************************************************************************
 * Macro Name     : GetEPTxAddr / GetEPRxAddr.
 * Description    : Gets address of the tx/rx buffer.
 * Input          : bEpNum: Endpoint Number.
 * Output         : None.
 * Return         : address of the buffer.
 *******************************************************************************/
#define _GetEPTxAddr(bEpNum) ((uint16_t)*_pEPTxAddr(bEpNum))
#define _GetEPRxAddr(bEpNum) ((uint16_t)*_pEPRxAddr(bEpNum))

/*******************************************************************************
 * Macro Name     : SetEPCountRxReg.
 * Description    : Sets counter of rx buffer with no. of blocks.
 * Input          : pdwReg: pointer to counter.
 *                  wCount: Counter.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _BlocksOf32(dwReg,wCount,wNBlocks) {\
	wNBlocks = wCount >> 5;\
	if((wCount & 0x1f) == 0)\
	wNBlocks--;\
	*pdwReg = (uint32_t)((wNBlocks << 10) | 0x8000);\
}/* _BlocksOf32 */

#define _BlocksOf2(dwReg,wCount,wNBlocks) {\
	wNBlocks = wCount >> 1;\
	if((wCount & 0x1) != 0)\
	wNBlocks++;\
	*pdwReg = (uint32_t)(wNBlocks << 10);\
}/* _BlocksOf2 */

#define _SetEPCountRxReg(dwReg,wCount)  {\
	uint16_t wNBlocks;\
	if(wCount > 62){_BlocksOf32(dwReg,wCount,wNBlocks);}\
	else {_BlocksOf2(dwReg,wCount,wNBlocks);}\
}/* _SetEPCountRxReg */



#define _SetEPRxDblBuf0Count(bEpNum,wCount) {\
	uint32_t *pdwReg = _pEPTxCount(bEpNum); \
	_SetEPCountRxReg(pdwReg, wCount);\
}
/*******************************************************************************
 * Macro Name     : SetEPTxCount / SetEPRxCount.
 * Description    : sets counter for the tx/rx buffer.
 * Input          : bEpNum: endpoint number.
 *                  wCount: Counter value.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPTxCount(bEpNum,wCount) (*_pEPTxCount(bEpNum) = wCount)
#define _SetEPRxCount(bEpNum,wCount) {\
	uint32_t *pdwReg = _pEPRxCount(bEpNum); \
	_SetEPCountRxReg(pdwReg, wCount);\
}
/*******************************************************************************
 * Macro Name     : GetEPTxCount / GetEPRxCount.
 * Description    : gets counter of the tx buffer.
 * Input          : bEpNum: endpoint number.
 * Output         : None.
 * Return         : Counter value.
 *******************************************************************************/
#define _GetEPTxCount(bEpNum)((uint16_t)(*_pEPTxCount(bEpNum)) & 0x3ff)
#define _GetEPRxCount(bEpNum)((uint16_t)(*_pEPRxCount(bEpNum)) & 0x3ff)

/*******************************************************************************
 * Macro Name     : SetEPDblBuf0Addr / SetEPDblBuf1Addr.
 * Description    : Sets buffer 0/1 address in a double buffer endpoint.
 * Input          : bEpNum: endpoint number.
 *                : wBuf0Addr: buffer 0 address.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPDblBuf0Addr(bEpNum,wBuf0Addr) {_SetEPTxAddr(bEpNum, wBuf0Addr);}
#define _SetEPDblBuf1Addr(bEpNum,wBuf1Addr) {_SetEPRxAddr(bEpNum, wBuf1Addr);}

/*******************************************************************************
 * Macro Name     : SetEPDblBuffAddr.
 * Description    : Sets addresses in a double buffer endpoint.
 * Input          : bEpNum: endpoint number.
 *                : wBuf0Addr: buffer 0 address.
 *                : wBuf1Addr = buffer 1 address.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPDblBuffAddr(bEpNum,wBuf0Addr,wBuf1Addr) { \
	_SetEPDblBuf0Addr(bEpNum, wBuf0Addr);\
	_SetEPDblBuf1Addr(bEpNum, wBuf1Addr);\
} /* _SetEPDblBuffAddr */

/*******************************************************************************
 * Macro Name     : GetEPDblBuf0Addr / GetEPDblBuf1Addr.
 * Description    : Gets buffer 0/1 address of a double buffer endpoint.
 * Input          : bEpNum: endpoint number.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _GetEPDblBuf0Addr(bEpNum) (_GetEPTxAddr(bEpNum))
#define _GetEPDblBuf1Addr(bEpNum) (_GetEPRxAddr(bEpNum))

/*******************************************************************************
 * Macro Name     : SetEPDblBuffCount / SetEPDblBuf0Count / SetEPDblBuf1Count.
 * Description    : Gets buffer 0/1 address of a double buffer endpoint.
 * Input          : bEpNum: endpoint number.
 *                : bDir: endpoint dir  EP_DBUF_OUT = OUT 
 *                                      EP_DBUF_IN  = IN 
 *                : wCount: Counter value    
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
#define _SetEPDblBuf0Count(bEpNum, bDir, wCount)  { \
	if(bDir == EP_DBUF_OUT)\
	/* OUT endpoint */ \
	{_SetEPRxDblBuf0Count(bEpNum,wCount);} \
	else if(bDir == EP_DBUF_IN)\
	/* IN endpoint */ \
	*_pEPTxCount(bEpNum) = (uint32_t)wCount;  \
} /* SetEPDblBuf0Count*/

#define _SetEPDblBuf1Count(bEpNum, bDir, wCount)  { \
	if(bDir == EP_DBUF_OUT)\
	/* OUT endpoint */ \
	{_SetEPRxCount(bEpNum,wCount);}\
	else if(bDir == EP_DBUF_IN)\
	/* IN endpoint */\
	*_pEPRxCount(bEpNum) = (uint32_t)wCount; \
} /* SetEPDblBuf1Count */

#define _SetEPDblBuffCount(bEpNum, bDir, wCount) {\
	_SetEPDblBuf0Count(bEpNum, bDir, wCount); \
	_SetEPDblBuf1Count(bEpNum, bDir, wCount); \
} /* _SetEPDblBuffCount  */

extern vu16 wIstr;  /* ISTR register last read value */
extern int EP_num;
void usb_hal_ini(void);

/* Exported functions ------------------------------------------------------- */
void SetCNTR(uint16_t /*wRegValue*/);
void SetISTR(uint16_t /*wRegValue*/);
void SetDADDR(uint16_t /*wRegValue*/);
uint16_t GetCNTR(void);
uint16_t GetISTR(void);
uint16_t GetFNR(void);
uint16_t GetDADDR(void);
uint16_t GetBTABLE(void);
void SetENDPOINT(uint8_t /*bEpNum*/, uint16_t /*wRegValue*/);
uint16_t GetENDPOINT(uint8_t /*bEpNum*/);
void SetEPType(uint8_t /*bEpNum*/, uint16_t /*wType*/);
uint16_t GetEPType(uint8_t /*bEpNum*/);
void SetEPTxStatus(uint8_t /*bEpNum*/, uint16_t /*wState*/);
void SetEPRxStatus(uint8_t /*bEpNum*/, uint16_t /*wState*/);
void SetDouBleBuffEPStall(uint8_t /*bEpNum*/, uint8_t bDir);
uint16_t GetEPTxStatus(uint8_t /*bEpNum*/);
uint16_t GetEPRxStatus(uint8_t /*bEpNum*/);
void SetEPTxValid(uint8_t /*bEpNum*/);
void SetEPRxValid(uint8_t /*bEpNum*/);
uint16_t GetTxStallStatus(uint8_t /*bEpNum*/);
uint16_t GetRxStallStatus(uint8_t /*bEpNum*/);
void SetEP_KIND(uint8_t /*bEpNum*/);
void ClearEP_KIND(uint8_t /*bEpNum*/);
void Set_Status_Out(uint8_t /*bEpNum*/);
void Clear_Status_Out(uint8_t /*bEpNum*/);
void SetEPDoubleBuff(uint8_t /*bEpNum*/);
void ClearEPDoubleBuff(uint8_t /*bEpNum*/);
void ClearEP_CTR_RX(uint8_t /*bEpNum*/);
void ClearEP_CTR_TX(uint8_t /*bEpNum*/);
void ToggleDTOG_RX(uint8_t /*bEpNum*/);
void ToggleDTOG_TX(uint8_t /*bEpNum*/);
void ClearDTOG_RX(uint8_t /*bEpNum*/);
void ClearDTOG_TX(uint8_t /*bEpNum*/);
void SetEPAddress(uint8_t /*bEpNum*/, uint8_t /*bAddr*/);
uint8_t GetEPAddress(uint8_t /*bEpNum*/);
void SetEPTxAddr(uint8_t /*bEpNum*/, uint16_t /*wAddr*/);
void SetEPRxAddr(uint8_t /*bEpNum*/, uint16_t /*wAddr*/);
uint16_t GetEPTxAddr(uint8_t /*bEpNum*/);
uint16_t GetEPRxAddr(uint8_t /*bEpNum*/);
void SetEPCountRxReg(uint32_t * /*pdwReg*/, uint16_t /*wCount*/);
void SetEPTxCount(uint8_t /*bEpNum*/, uint16_t /*wCount*/);
void SetEPRxCount(uint8_t /*bEpNum*/, uint16_t /*wCount*/);
uint16_t GetEPTxCount(uint8_t /*bEpNum*/);
uint16_t GetEPRxCount(uint8_t /*bEpNum*/);
void SetEPDblBuf0Addr(uint8_t /*bEpNum*/, uint16_t /*wBuf0Addr*/);
void SetEPDblBuf1Addr(uint8_t /*bEpNum*/, uint16_t /*wBuf1Addr*/);
void SetEPDblBuffAddr(uint8_t /*bEpNum*/, uint16_t /*wBuf0Addr*/, uint16_t /*wBuf1Addr*/);
uint16_t GetEPDblBuf0Addr(uint8_t /*bEpNum*/);
uint16_t GetEPDblBuf1Addr(uint8_t /*bEpNum*/);
void SetEPDblBuffCount(uint8_t /*bEpNum*/, uint8_t /*bDir*/, uint16_t /*wCount*/);
void SetEPDblBuf0Count(uint8_t /*bEpNum*/, uint8_t /*bDir*/, uint16_t /*wCount*/);
void SetEPDblBuf1Count(uint8_t /*bEpNum*/, uint8_t /*bDir*/, uint16_t /*wCount*/);
uint16_t GetEPDblBuf0Count(uint8_t /*bEpNum*/);
uint16_t GetEPDblBuf1Count(uint8_t /*bEpNum*/);
EP_DBUF_DIR GetEPDblBufDir(uint8_t /*bEpNum*/);
void FreeUserBuffer(uint8_t bEpNum/*bEpNum*/, uint8_t bDir);
uint16_t ByteSwap(uint16_t);

void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);

#endif /* __USB_REGS_H */

