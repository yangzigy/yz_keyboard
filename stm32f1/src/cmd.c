/*
文件名：cmd.c
时间：2013-3-9
功能：

人机交互命令行库，可以使用超级终端进行命令行通信。
V2.0版本加入功能：
1、echo off功能，可关闭回显
2、实现开放封闭原则，每个应用指令分部写在任务文件中，使用时进行注册，无需修改此文件

*/
#include "cmd.h"
#include "stm32f1_sys.h"

#define MAX_BUFFSIZE	40

const char *HelloString="CMD-Lib V2.0\r\nBJTU-SY0702-YangZi\r\n>";

Queue *cmd_queue;

static CMD_LIST *cmd_list=0;

void fun_help(s8 *para);
void fun_cpu(s8 *para);
void fun_echo(s8 *para);


const CMD_STR cmd_str[]=\
{
	(s8 *)"?",(s8 *)"help",fun_help,
	(s8 *)"echo",(s8 *)"echo 1,0",fun_echo,
	(s8 *)"CPU",(s8 *)"CPU occupy",fun_cpu,	//CPU占用率
};
static CMD_LIST sys_cmd_list={cmd_str,sizeof(cmd_str)/sizeof(CMD_STR),0};
void cmd_ini(Queue *pq)
{
	cmd_queue=pq;
	cmd_reg((CMD_LIST*)&sys_cmd_list);
}

void cmd_reg(CMD_LIST *a)
{
	a->next=cmd_list;
	cmd_list=a;
}

s8 cmd_c=0;		//当前指令
u8 buff_p=0;	//当前指令字符串的末尾（指向\0）
u8 cmd_para=0;	//参数偏移
s8 cmd_buff[MAX_BUFFSIZE]={0};	//命令行字符缓冲

void (*cmd)(void)=cmd_fun;	//任务函数
void void_bin_pro_cb(u8 t){}
void (*bin_pro_cb)(u8 t)=void_bin_pro_cb; //对于二进制数据的处理函数指针

void shell(void);

void cmd_fun(void)	//任务函数
{
	int exflag=0;
	while(1)//返回0说明队列有值
	{
		OS_CLOSE_INT;
		exflag=Queue_get_1((u8*)&cmd_c,cmd_queue);
		OS_OPEN_INT;
		if(exflag!=0) //若取不出来了
		{
			break;
		}
	//命令解析
		if(cmd_c=='\n' || cmd_c=='\r')	//若是回车
		{
			printf("\r\n");	//回显
			cmd_buff[buff_p++]='\0';
			//处理
			shell();
			//处理完成，需要将指针归零
			buff_p=0;
			cmd_para=0;
			printf("\r\n> ");	//提示符
		}
		else if(cmd_c==0x08)	//退格键
		{
			printf("%c %c",cmd_c,cmd_c);	//回显
			buff_p--;
		}
		else		//若是普通字符
		{
			cmd_buff[buff_p++]=cmd_c;
			printf("%c",cmd_c);	//回显
		}
		if(buff_p>=MAX_BUFFSIZE-1)
		{
			buff_p=MAX_BUFFSIZE-2;
		}
	}
}
void cmd_fun_echooff(void)	//任务函数,无回显
{
	int exflag=0;
	while(1)//返回0说明队列有值
	{
		OS_CLOSE_INT;
		exflag=Queue_get_1((u8*)&cmd_c,cmd_queue);
		OS_OPEN_INT;
		if(exflag!=0) //若取不出来了
		{
			break;
		}
	//命令解析
		if(cmd_c=='\n' || cmd_c=='\r')	//若是回车
		{
			//printf("\r\n");	//回显
			cmd_buff[buff_p++]='\0';
			//处理
			shell();
			//处理完成，需要将指针归零
			buff_p=0;
			cmd_para=0;
			//printf("\r\n> ");	//提示符
		}
		else if(cmd_c==0x08)	//退格键
		{
			//printf("%c %c",cmd_c,cmd_c);	//回显
			buff_p--;
		}
		else		//若是普通字符
		{
			cmd_buff[buff_p++]=cmd_c;
			//printf("%c",cmd_c);	//回显
		}
		if(buff_p>=MAX_BUFFSIZE-1)
		{
			buff_p=MAX_BUFFSIZE-2;
		}
	}
}

void shell(void)
{
//提取指令，其参数由指令对应的服务函数完成
	s8 *str,i;
	CMD_LIST *tmp=cmd_list;
	for(i=0;i<buff_p;i++)
	{
		if(cmd_buff[i]!=' ')
		{
			str=&cmd_buff[i];
			for(;i<buff_p;i++)
			{
				if(cmd_buff[i]==' ')
				{
					break ;
				}
			}
			break ;
		}
	}
	cmd_buff[i]='\0';
	if(i==buff_p)	//说明没有参数
	{
		cmd_para=0;
	}
	else
	{
		cmd_para=i+1;
	}
//得到了命令字符str,和参数偏移
	while(tmp)
	{
		for(i=0;i<tmp->num;i++)
		{
			if(strcmp((char const *)tmp->str[i].str,(char const *)str)==0)
			{
				//执行相应命令的服务函数
				if(cmd_para)	//有参数
				{
					tmp->str[i].fun(cmd_buff+cmd_para);
				}
				else	//无参数
				{
					tmp->str[i].fun((s8 *)"");
				}
				return ;
			}
		}
		tmp=tmp->next;
	}
	if(str[0]!=0)
	{
		printf("bad command");
	}
}

////////////////////////////////////////////////////////////////////////
//服务函数
void fun_help(s8 *para)
{
	int i;
	CMD_LIST *tmp=cmd_list;
	//首先将命令模式改为有回显
	//cmd=cmd_fun;
	while(tmp)
	{
		for(i=0;i<tmp->num;i++)
		{
			printf("%s\t%s\r\n",tmp->str[i].str,tmp->str[i].des);
		}
		tmp=tmp->next;
	}
}
void fun_cpu(s8 *para)
{
	printf("%d",SYS_idle_dis);
}
void fun_echo(s8 *para)
{
	u32 t1;
	if(sscanf(para,"%d",&t1)==1)
	{
		if(t1)
		{
			cmd=cmd_fun;
		}
		else
		{
			cmd=cmd_fun_echooff;
		}
	}
}

