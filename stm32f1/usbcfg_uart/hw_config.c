#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_istr.h"
#include "hw_config.h"
#include "usb_pwr.h" 
#include "string.h"	
#include "stdarg.h"		 
#include "stdio.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//USB-hw_config 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/1/28
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
 
_usb_usart_fifo uu_txfifo;					//USB串口发送FIFO结构体 
u8  USART_PRINTF_Buffer[USB_USART_REC_LEN];	//usb_printf发送缓冲区

//用类似串口1接收数据的方法,来处理USB虚拟串口接收到的数据.
u8 USB_USART_RX_BUF[USB_USART_REC_LEN]; 	//接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USB_USART_RX_STA=0;       				//接收状态标记	 

extern LINE_CODING linecoding;							//USB虚拟串口配置信息
/////////////////////////////////////////////////////////////////////////////////
//各USB例程通用部分代码,ST各各USB例程,此部分代码都可以共用.
//此部分代码一般不需要修改!

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

//USB时钟配置函数,USBclk=48Mhz@HCLK=72Mhz
void Set_USBClock(void)
{
 	RCC->CFGR&=~(1<<22); //USBclk=PLLclk/1.5=48Mhz	    
	RCC->APB1ENR|=1<<23; //USB时钟使能					 
} 

//USB进入低功耗模式
//当USB进入suspend模式时,MCU进入低功耗模式
//需自行添加低功耗代码(比如关时钟等)
void Enter_LowPowerMode(void)
{
 	printf("usb enter low power mode\r\n");
	bDeviceState=SUSPENDED;
} 

//USB退出低功耗模式
//用户可以自行添加相关代码(比如重新配置时钟等)
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo=&Device_Info;
	printf("leave low power mode\r\n"); 
	if (pInfo->Current_Configuration!=0)bDeviceState=CONFIGURED; 
	else bDeviceState = ATTACHED; 
} 

//USB中断配置
void USB_Interrupts_Config(void)
{ 
	EXTI->IMR|=1<<18;	//开启线18上的中断
 	EXTI->RTSR|=1<<18;	//line 18上事件上升降沿触发	 
	MY_NVIC_Init(1,0,USB_LP_CAN1_RX0_IRQn,2);	//组2，优先级次之 
	MY_NVIC_Init(0,0,USBWakeUp_IRQn,2);     	//组2，优先级最高	 	 
}	

//USB接口配置(配置1.5K上拉电阻,战舰V3不需要配置,恒上拉)
//NewState:DISABLE,不上拉
//         ENABLE,上拉
void USB_Cable_Config (FunctionalState NewState)
{ 
	if (NewState!=DISABLE)printf("usb pull up enable\r\n"); 
	else printf("usb pull up disable\r\n"); 
}

//USB使能连接/断线
//enable:0,断开
//       1,允许连接	   
void USB_Port_Set(u8 enable)
{
	RCC->APB2ENR|=1<<2;    //使能PORTA时钟	   	 
	if(enable)_SetCNTR(_GetCNTR()&(~(1<<1)));//退出断电模式
	else
	{	  
		_SetCNTR(_GetCNTR()|(1<<1));  // 断电模式
		GPIOA->CRH&=0XFFF00FFF;
		GPIOA->CRH|=0X00033000;
		PAout(8)=0;	    		  
	}
}  

//获取STM32的唯一ID
//用于USB配置信息
void Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;
	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);
	Device_Serial0 += Device_Serial2;
	if (Device_Serial0 != 0)
	{
		IntToUnicode (Device_Serial0,&Virtual_Com_Port_StringSerial[2] , 8);
		IntToUnicode (Device_Serial1,&Virtual_Com_Port_StringSerial[18], 4);
	}
} 

//将32位的值转换成unicode.
//value,要转换的值(32bit)
//pbuf:存储地址
//len:要转换的长度
void IntToUnicode (u32 value , u8 *pbuf , u8 len)
{
	u8 idx = 0;
	for( idx = 0 ; idx < len ; idx ++)
	{
		if( ((value >> 28)) < 0xA )
		{
			pbuf[ 2* idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2* idx] = (value >> 28) + 'A' - 10; 
		} 
		value = value << 4; 
		pbuf[ 2* idx + 1] = 0;
	}
}
/////////////////////////////////////////////////////////////////////////////////
 
//USB COM口的配置信息,通过此函数打印出来. 
bool USART_Config(void)
{
	uu_txfifo.readptr=0;	//清空读指针
	uu_txfifo.writeptr=0;	//清空写指针
	USB_USART_RX_STA=0;		//USB USART接收状态清零
	printf("linecoding.format:%d\r\n",linecoding.format);
  	printf("linecoding.paritytype:%d\r\n",linecoding.paritytype);
	printf("linecoding.datatype:%d\r\n",linecoding.datatype);
	printf("linecoding.bitrate:%d\r\n",linecoding.bitrate);
	return (TRUE);
}
 
//处理从USB虚拟串口接收到的数据
//databuffer:数据缓存区
//Nb_bytes:接收到的字节数.
void USB_To_USART_Send_Data(u8* data_buffer, u8 Nb_bytes)
{ 
	u8 i;
	u8 res;
	for(i=0;i<Nb_bytes;i++)
	{  
		res=data_buffer[i]; 
		if((USB_USART_RX_STA&0x8000)==0)		//接收未完成
		{
			if(USB_USART_RX_STA&0x4000)			//接收到了0x0d
			{
				if(res!=0x0a)USB_USART_RX_STA=0;//接收错误,重新开始
				else USB_USART_RX_STA|=0x8000;	//接收完成了 
			}else //还没收到0X0D
			{	
				if(res==0x0d)USB_USART_RX_STA|=0x4000;
				else
				{
					USB_USART_RX_BUF[USB_USART_RX_STA&0X3FFF]=res;
					USB_USART_RX_STA++;
					if(USB_USART_RX_STA>(USB_USART_REC_LEN-1))USB_USART_RX_STA=0;//接收数据错误,重新开始接收	
				}					
			}
		}   
	}  
} 

//发送一个字节数据到USB虚拟串口
void USB_USART_SendData(u8 data)
{
	uu_txfifo.buffer[uu_txfifo.writeptr]=data;
	uu_txfifo.writeptr++;
	if(uu_txfifo.writeptr==USB_USART_TXFIFO_SIZE)//超过buf大小了,归零.
	{
		uu_txfifo.writeptr=0;
	} 
}

//usb虚拟串口,printf 函数
//确保一次发送数据不超USB_USART_REC_LEN字节
void usb_printf(char* fmt,...)  
{  
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART_PRINTF_Buffer,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART_PRINTF_Buffer);//此次发送数据的长度
	for(j=0;j<i;j++)//循环发送数据
	{
		USB_USART_SendData(USART_PRINTF_Buffer[j]); 
	}
} 






















