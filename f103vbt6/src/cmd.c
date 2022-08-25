/*
文件名：cmd.c
时间：2013-3-9

V2.0版本加入功能：
1、echo off功能，可关闭回显
2、实现开放封闭原则，每个应用指令分部写在任务文件中，使用时进行注册，无需修改此文件
版本：	V2.1
版本：	V2.2	2022-07-02更改类型

功能：
人机交互命令行库，可以使用超级终端进行命令行通信。
*/
#include "cmd.h"

#define MAX_BUFFSIZE	80

const char *HelloString="CMD-Lib V2.2\r\nstart\r\n>";

static CMD_LIST *cmd_list=0;

void fun_help(char *para);
void fun_echo(char *para);

const CMD_STR cmd_str[]=\
{
	"?","help",fun_help,
	"echo","echo 1/0",fun_echo,
};
static CMD_LIST sys_cmd_list={cmd_str,sizeof(cmd_str)/sizeof(CMD_STR),0};
void cmd_ini(void)
{
	cmd_reg((CMD_LIST*)&sys_cmd_list);
	//cmd_out((char*)HelloString);
}

void cmd_reg(CMD_LIST *a)
{
	a->next=cmd_list;
	cmd_list=a;
}

static u8 buff_p=0;	//当前指令字符串的末尾（指向\0）
static u8 cmd_para=0;	//参数偏移
static char cmd_buff[MAX_BUFFSIZE]={0};	//命令行字符缓冲

void dft_cmd_out(const char *p){printf("%s",p);} //printf定向
void void_cmd_out(const char *p){} //空输出
void (*cmd_out)(const char *p)=dft_cmd_out;	//输出函数

void shell(void);

void cmd_fun(u8 cmd_c)	//任务函数
{
//命令解析
	if(cmd_c=='\n' || cmd_c=='\r')	//若是回车
	{
		cmd_out("\r\n");	//回显
		cmd_buff[buff_p++]='\0';
		//处理
		shell();
		//处理完成，需要将指针归零
		buff_p=0;
		cmd_para=0;
		cmd_out("\r\n> ");	//提示符
	}
	else if(cmd_c==0x08)	//退格键
	{
		cmd_out("\b \b");	//回显
		buff_p--;
	}
	else		//若是普通字符
	{
		cmd_buff[buff_p]=cmd_c;
		cmd_buff[buff_p+1]=0;
		cmd_out(cmd_buff+buff_p);	//回显
		buff_p++;
	}
	if(buff_p>=MAX_BUFFSIZE-1)
	{
		buff_p=MAX_BUFFSIZE-2;
	}
}

void shell(void)
{
//提取指令，其参数由指令对应的服务函数完成
	char *str,i;
	CMD_LIST *tmp=cmd_list;
	for(i=0;i<buff_p;i++)
	{
		if(cmd_buff[i]!=' ') //找第一个非空字符，作为指令
		{
			str=&cmd_buff[i];
			for(;i<buff_p;i++) //找指令后的空格，作为分割
			{
				if(cmd_buff[i]==' ') break ;
			}
			break ;
		}
	}
	cmd_buff[i]='\0'; //指令后的空格给0，分割字符串
	if(i==buff_p) cmd_para=0;	//说明没有参数
	else cmd_para=i+1;
//得到了命令字符str,和参数偏移
	while(tmp)
	{
		for(i=0;i<tmp->num;i++) //遍历本命令列表
		{
			if(strcmp(tmp->str[i].str,str)==0)
			{
				//执行相应命令的服务函数
				if(cmd_para)	//有参数
				{
					tmp->str[i].fun(cmd_buff+cmd_para);
				}
				else	//无参数
				{
					tmp->str[i].fun("");
				}
				return ;
			}
		}
		tmp=tmp->next;
	}
	if(str[0]!=0) cmd_out("bad command");
}
////////////////////////////////////////////////////////////////////////
//服务函数
void fun_help(char *para)
{
	int i;
	CMD_LIST *tmp=cmd_list;
	//首先将命令模式改为有回显
	while(tmp)
	{
		for(i=0;i<tmp->num;i++)
		{
			cmd_out(tmp->str[i].str);
			cmd_out("\t");
			cmd_out(tmp->str[i].des);
			cmd_out("\r\n");
		}
		tmp=tmp->next;
	}
}
void fun_echo(char *para)
{
	u32 t1;
	if(sscanf((const char *)para,"%d",&t1)==1)
	{
		if(t1) cmd_out=dft_cmd_out;
		else cmd_out=void_cmd_out;
	}
}

