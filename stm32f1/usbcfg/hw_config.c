// File Name          : hw_config.c
#include "stm32f1_sys.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "platform_config.h"
#include "usb_pwr.h"
#include "usb_istr.h"

//USB唤醒中断服务函数
void USBWakeUp_IRQHandler(void) 
{
	EXTI->PR|=1<<18;//清除USB唤醒中断挂起位
} 

//USB中断处理函数
void USB_LP_CAN1_RX0_IRQHandler(void) 
{
	USB_Istr();
} 

//配置USB时钟,USBclk=48Mhz
void Set_USBClock(void)
{
	RCC->CFGR&=~(1<<22); //USBclk=PLLclk/1.5=48Mhz
	RCC->APB1ENR|=1<<23; //USB时钟使能
}

/*******************************************************************************
 * Function Name  : Enter_LowPowerMode.
 * Description    : Power-off system clocks and power while entering suspend mode.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;	  
	/* Request to enter STOP mode with regulator in low power mode */
	//PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

/*******************************************************************************
 * Function Name  : Leave_LowPowerMode.
 * Description    : Restores system clocks and power while exiting suspend mode.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}
}

//USB中断配置
void USB_Interrupts_Config(void)
{
	EXTI->IMR|=1<<18;//  开启线18上的中断
	EXTI->RTSR|=1<<18;//line 18上事件上升降沿触发	 
	MY_NVIC_Init(0,0,USB_LP_CAN1_RX0_IRQn,0);
	MY_NVIC_Init(0,0,USBWakeUp_IRQn,0);
}
//USB接口配置(配置1.5K上拉电阻,战舰V3不需要配置,恒上拉)
//NewState:DISABLE,不上拉
//         ENABLE,上拉
void USB_Cable_Config (FunctionalState NewState)
{ 
	if (NewState!=DISABLE)
	{
		PAout(8)=1;
		//printf("usb pull up enable\r\n"); 
	}
	else
	{
		PAout(8)=0;
		//printf("usb pull up disable\r\n"); 
	}
}
u32 keyboard_send_pre_0=0;
u32 keyboard_send_pre_1=0; //上次发送值
void keyboard_send(u8 *buf)
{
	if (((u32*)buf)[0]==keyboard_send_pre_0 && ((u32*)buf)[1]==keyboard_send_pre_1) //若相同
	{
		return ;
	}
	keyboard_send_pre_0=((u32*)buf)[0];
	keyboard_send_pre_1=((u32*)buf)[1];
	UserToPMABufferCopy(buf, GetEPTxAddr(ENDP1), 8);
	SetEPTxValid(ENDP1);
}
u32 mouse_send_pre=0; //上次发送值
void mouse_send(u8 *buf)
{
	if (((u32*)(buf+1))[0]==mouse_send_pre && mouse_send_pre==0) //若相同并且为0
	{
		return ;
	}
	mouse_send_pre=((u32*)(buf+1))[0];
	UserToPMABufferCopy(buf, GetEPTxAddr(ENDP2), 5);
	SetEPTxValid(ENDP2);
}

/*******************************************************************************
 * Function Name  : Get_SerialNum.
 * Description    : Create the serial number string descriptor.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);

	if (Device_Serial0 != 0)
	{
		Joystick_StringSerial[2] = (u8)(Device_Serial0 & 0x000000FF);
		Joystick_StringSerial[4] = (u8)((Device_Serial0 & 0x0000FF00) >> 8);
		Joystick_StringSerial[6] = (u8)((Device_Serial0 & 0x00FF0000) >> 16);
		Joystick_StringSerial[8] = (u8)((Device_Serial0 & 0xFF000000) >> 24);

		Joystick_StringSerial[10] = (u8)(Device_Serial1 & 0x000000FF);
		Joystick_StringSerial[12] = (u8)((Device_Serial1 & 0x0000FF00) >> 8);
		Joystick_StringSerial[14] = (u8)((Device_Serial1 & 0x00FF0000) >> 16);
		Joystick_StringSerial[16] = (u8)((Device_Serial1 & 0xFF000000) >> 24);

		Joystick_StringSerial[18] = (u8)(Device_Serial2 & 0x000000FF);
		Joystick_StringSerial[20] = (u8)((Device_Serial2 & 0x0000FF00) >> 8);
		Joystick_StringSerial[22] = (u8)((Device_Serial2 & 0x00FF0000) >> 16);
		Joystick_StringSerial[24] = (u8)((Device_Serial2 & 0xFF000000) >> 24);
	}
}

