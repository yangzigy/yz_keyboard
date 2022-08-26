
#include "f1_usb.h"
#include "usb_core.h"

int EP_num=1; //总共多少个端点
void SetEPType(u8 bEpNum, u16 wType)
{
	_SetEPType(bEpNum, wType);
}

void SetEPTxStatus(u8 bEpNum, u16 wState)
{
	_SetEPTxStatus(bEpNum, wState);
}

/*******************************************************************************
 * Function Name  : SetEPRxStatus
 * Description    : Set the status of Rx endpoint.
 * Input          : bEpNum: Endpoint Number. 
 *                  wState: new state.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void SetEPRxStatus(u8 bEpNum, u16 wState)
{
	_SetEPRxStatus(bEpNum, wState);
}
void SetEPTxValid(u8 bEpNum)
{
	_SetEPTxStatus(bEpNum, EP_TX_VALID);
}

/*******************************************************************************
 * Function Name  : SetEPRxValid
 * Description    : Valid the endpoint Rx Status.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void SetEPRxValid(u8 bEpNum)
{
	_SetEPRxStatus(bEpNum, EP_RX_VALID);
}

void Clear_Status_Out(u8 bEpNum)
{
	_ClearEP_KIND(bEpNum);
}
void ClearDTOG_RX(u8 bEpNum)
{
	_ClearDTOG_RX(bEpNum);
}
/*******************************************************************************
 * Function Name  : ClearDTOG_TX.
 * Description    : Clear the DTOG_TX bit.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void ClearDTOG_TX(u8 bEpNum)
{
	_ClearDTOG_TX(bEpNum);
}
void SetEPTxAddr(u8 bEpNum, u16 wAddr)
{
	_SetEPTxAddr(bEpNum, wAddr);
}
/*******************************************************************************
 * Function Name  : SetEPRxAddr
 * Description    : Set the endpoint Rx buffer address.
 * Input          : bEpNum: Endpoint Number.
 *                  wAddr: new address.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void SetEPRxAddr(u8 bEpNum, u16 wAddr)
{
	_SetEPRxAddr(bEpNum, wAddr);
}
/*******************************************************************************
 * Function Name  : GetEPRxAddr.
 * Description    : Returns the endpoint Rx buffer address.
 * Input          : bEpNum: Endpoint Number. 
 * Output         : None.
 * Return         : Rx buffer address.
 *******************************************************************************/
u16 GetEPRxAddr(u8 bEpNum)
{
	return(_GetEPRxAddr(bEpNum));
}
/*******************************************************************************
 * Function Name  : SetEPTxCount.
 * Description    : Set the Tx count.
 * Input          : bEpNum: Endpoint Number.
 *                  wCount: new count value.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void SetEPTxCount(u8 bEpNum, u16 wCount)
{
	_SetEPTxCount(bEpNum, wCount);
}
void SetEPRxCount(u8 bEpNum, u16 wCount)
{
	_SetEPRxCount(bEpNum, wCount);
}
u16 ByteSwap(u16 wSwW)
{
	u8 bTemp;
	u16 wRet;
	bTemp = (u8)(wSwW & 0xff);
	wRet =  (wSwW >> 8) | ((u16)bTemp << 8);
	return(wRet);
}

/*******************************************************************************
 * Function Name  : UserToPMABufferCopy
 * Description    : Copy a buffer from user memory area to packet memory area (PMA)
 * Input          : - pbUsrBuf: pointer to user memory area.
 *                  - wPMABufAddr: address into PMA.
 *                  - wNBytes: no. of bytes to be copied.
 * Output         : None.
 * Return         : None	.
 *******************************************************************************/
void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
	uint32_t n = (wNBytes + 1) >> 1;   /* n = (wNBytes + 1) / 2 */
	uint32_t i, temp1, temp2;
	uint16_t *pdwVal;
	pdwVal = (uint16_t *)(wPMABufAddr * 2 + PMAAddr);
	for (i = n; i != 0; i--)
	{
		temp1 = (uint16_t) * pbUsrBuf;
		pbUsrBuf++;
		temp2 = temp1 | (uint16_t) * pbUsrBuf << 8;
		*pdwVal++ = temp2;
		pdwVal++;
		pbUsrBuf++;
	}
}

/*******************************************************************************
 * Function Name  : PMAToUserBufferCopy
 * Description    : Copy a buffer from user memory area to packet memory area (PMA)
 * Input          : - pbUsrBuf    = pointer to user memory area.
 *                  - wPMABufAddr = address into PMA.
 *                  - wNBytes     = no. of bytes to be copied.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
	uint32_t n = (wNBytes + 1) >> 1;/* /2*/
	uint32_t i;
	uint32_t *pdwVal;
	pdwVal = (uint32_t *)(wPMABufAddr * 2 + PMAAddr);
	for (i = n; i != 0; i--)
	{
		*(uint16_t*)pbUsrBuf++ = *pdwVal++;
		pbUsrBuf++;
	}
}

/*  The number of current endpoint, it will be used to specify an endpoint */
u8	EPindex;
/*  The number of current device, it is an index to the Device_Table */
/* u8	Device_no; */
/*  Points to the DEVICE_INFO structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_INFO *pInformation;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_PROP *pProperty;
/*  Temporary save the state of Rx & Tx status. */
/*  Whenever the Rx or Tx state is changed, its value is saved */
/*  in this variable first and will be set to the EPRB or EPRA */
/*  at the end of interrupt process */
u16	SaveState ;
u16  wInterrupt_Mask;
DEVICE_INFO	Device_Info;
USER_STANDARD_REQUESTS  *pUser_Standard_Requests;

void usb_hal_ini(void)
{
	RCC->CFGR&=~(1<<22); //USBclk=PLLclk/1.5=48Mhz
	RCC->APB1ENR|=1<<23; //USB时钟使能

	EXTI->IMR|=1<<18;//  开启线18上的中断
	EXTI->RTSR|=1<<18;//line 18上事件上升降沿触发	 
	MY_NVIC_Init(0,0,USB_LP_CAN1_RX0_IRQn,0);
	MY_NVIC_Init(0,0,USBWakeUp_IRQn,0);

	pInformation = &Device_Info;
	pInformation->ControlState = 2;
	pProperty = &Device_Property;
	pUser_Standard_Requests = &User_Standard_Requests;
	/* Initialize devices one by one */
	pProperty->Init();
}

