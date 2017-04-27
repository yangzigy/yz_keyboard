#include "main.h"
#include "stm32f1_sys.h" 	
#include "uart.h"
#include "cmd.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "key.h"

//系统变量
u32 SYS_task=0;
u16 SYS_idle=0;
u16 SYS_idle_dis=0;
u32 SYS_time=0;

void systime_initial(u32 frq)//输入为中断频率
{
	//输入频率默认HCLK/8=9MHz=9000000
	SysTick->LOAD=9000000/frq;
	SysTick->CTRL |= 2;//开中断
	SysTick->CTRL |= 1;
}
extern CMD_LIST bl_sys_cmd_list;
int main(void)
{
	Stm32_Clock_Init(9);//系统时钟,所有都必须在此函数后
	RCC->APB2ENR|=0x1ff;   //使能所有PORT口时钟
	systime_initial(2000);//Hz
	MGPIOC->CT13=GPIO_OUT_PP; //LED
	uart_initial(&uart1,256000);
	uart_initial(&uart2,115200);
	cmd_ini(&(uart1.que_rx)); //使用串口1作为命令串口
	cmd_reg((CMD_LIST*)&bl_sys_cmd_list);
	//printf(HelloString);
	MGPIOA->CT8=GPIO_OUT_PP; //USB
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
	key_ini();
	while(1)
	{
		int i;
		for(i=0;i<1620;i++)
		{
			if(uart1.que_rx.dlen>0) //这个只读不用锁
			{
				SYS_task &= ~TASK_CMD;
				cmd();
			}
			if(SYS_task & TASK_POOL_100) //这个只读不用锁
			{
				SYS_task &= ~TASK_POOL_100;
				key_poll();
			}
		}
		SYS_idle++;
	}
}

void SysTick_Handler(void)
{
	SYS_time++;
	START_TASK(TASK_POOL_100);
	if(SYS_time%200==1)
	{
		SYS_idle_dis=(SYS_idle_dis+SYS_idle)/2;
		SYS_idle=0;
		//PCo^=1<<13;
		//Joystick_Send(0,4);
		//Joystick_Send(1,4);
	}
}

void wait(vu32 i)
{
	while(i--);
}

int void_putchar(int c)
{
	return 0;
}

int (*IO_putchar)(int)=uart1_putchar;	//初始化的函数

int fputc(int c, FILE *f)
{
	return IO_putchar(c);
}

//////////////////////////////////////////////////////////////////////////
//			指令
extern u8 keybuf[8];
void cmdfun_getkeybuf(s8 *para) //显示发送缓冲
{
	int i;
	for(i=0;i<sizeof(keybuf);i++)
	{
		printf("%02X ",keybuf[i]);
	}
	printf("\r\n");
}
extern u8 mousebuf[5]; //鼠标发送缓存
void cmdfun_mousetest(s8 *para)
{
	int i;
	mousebuf[0]=1;
	mousebuf[1] = (1<<0);
	mouse_send(mousebuf);
	for(i=0;i<sizeof(mousebuf);i++)
	{
		printf("%02X ",mousebuf[i]);
	}
	printf("\r\n");
}
void cmdfun_ledtest(s8 *para) //显示发送缓冲
{
	int a,b;
	if(sscanf(para,"%d %d",&a,&b)==2)
	{
		keys[a].led=b;
	}
}
static const CMD_STR blcmd_str[]=\
{
	(s8 *)"gk",(s8 *)"display keybuf",cmdfun_getkeybuf,
	(s8 *)"m",(s8 *)"mouse test",cmdfun_mousetest,
	(s8 *)"led",(s8 *)"led test",cmdfun_ledtest,
};
CMD_LIST bl_sys_cmd_list={blcmd_str,sizeof(blcmd_str)/sizeof(CMD_STR),0};
