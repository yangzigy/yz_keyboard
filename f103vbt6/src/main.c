#include "f1_uart.h"
#include "cmd.h"
#include "key.h"
#include "usb_kb_m.h"

int get_que_data(u8 *p,Queue *q);
#define TASK_POOL_2K 2
u32 led_div=512-1; //led分频系数

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
	MGPIOD->CT13=GPIO_OUT_PP; //LED
	uart_initial(&uart1,115200);
	uart_initial(&uart2,115200);

	IO_putchar=uart1_putchar;
	cmd_ini();
	cmd_reg((CMD_LIST*)&bl_sys_cmd_list);
	//printf(HelloString);
	usb_ini();
	key_ini();
	while(1)
	{
		int i;
		for(i=0;i<1620;i++)
		{
			u8 tt;//队列实参
			if(uart1.que_rx.dlen>0) //这个只读不用锁
			{ 
				while(get_que_data(&tt,&(uart1.que_rx))==0)
				{
					cmd_fun(tt);
				}
			}
			if(SYS_task & TASK_POOL_2K) //2000Hz任务
			{
				SYS_task &= ~TASK_POOL_2K;
				key_poll();
				if(usb_stat==USB_ATTACHED) led_div=511; //加入USB连接指示
				else led_div=4095;
				if((SYS_time & led_div)==0) PDout(13)^=1; //LED分频后，让IO反转
			}
		}
		SYS_idle++;
	}
}
void SysTick_Handler(void) //2000Hz
{
	SYS_time++;
	START_TASK(TASK_POOL_2K);
	if(SYS_time%200==1)
	{
		SYS_idle_dis=(SYS_idle_dis+SYS_idle)/2;
		SYS_idle=0;
	}
}
int get_que_data(u8 *p,Queue *q)
{
	int rst=1;
	OS_CLOSE_INT;
	rst=Queue_get_1(p,q);
	OS_OPEN_INT;
	return rst;
}
//////////////////////////////////////////////////////////////////////////
//			指令
extern u8 keybuf[8];
void cmdfun_getkeybuf(char *para) //显示发送缓冲
{
	int i;
	for(i=0;i<sizeof(keybuf);i++)
	{
		printf("%02X ",keybuf[i]);
	}
	printf("\r\n");
}
extern u8 mousebuf[5]; //鼠标发送缓存
extern void mouse_send(u8 *buf);
void cmdfun_mousetest(char *para) //显示鼠标发送缓冲
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
void cmdfun_ledtest(char *para) //设置led亮度
{
	int b;
	char c;
	if(sscanf(para,"%c %d",&c,&b)==2)
	{
		extern const u8 ascii_index[256];
		keys[ascii_index[c]].led=b;
	}
}
static const CMD_STR blcmd_str[]=\
{
	"gk","display keybuf",cmdfun_getkeybuf,//显示发送缓冲
	"m", "mouse test",cmdfun_mousetest, //显示鼠标发送缓冲
	"led","led test",cmdfun_ledtest, //设置led亮度
};
CMD_LIST bl_sys_cmd_list={blcmd_str,sizeof(blcmd_str)/sizeof(CMD_STR),0};

