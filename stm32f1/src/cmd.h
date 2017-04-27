/*
文件名：cmd.h
作者：北京交通大学 思源0702 杨孜
时间：2013-3-9
功能：

使用案例：

	void fun_a(s8 *para);
	void fun_s(s8 *para);

	static const CMD_STR cmd_str[]=\
	{
		(s8 *)"a",(s8 *)"取AD值",fun_a,
		(s8 *)"s",(s8 *)"IO输出",fun_s,
	};
	static CMD_LIST sys_cmd_list={cmd_str,sizeof(cmd_str)/sizeof(CMD_STR),0};

	void con_task_ini()
	{
		cmd_reg((CMD_LIST*)&sys_cmd_list);
	}

*/
#ifndef CMD_H
#define CMD_H

#include "main.h"
#include "queue.h"

typedef struct
{
	s8 *str;	//命令名
	s8 *des;	//描述
	void (*fun)(s8 *para);
} CMD_STR;

typedef struct _tag_CMD_LIST//命令列表，作为命令初始化的结构
{
	CMD_STR const *str;
	u8 num;//此数组中指令个数
	struct _tag_CMD_LIST *next;
} CMD_LIST;

extern const char *HelloString;

extern void (*cmd)(void);	//任务函数
extern void (*bin_pro_cb)(u8 t); //对于二进制数据的处理函数指针

void cmd_ini(Queue *pq);
void cmd_reg(CMD_LIST *a);//注册命令列表

void cmd_fun(void);	//自身命令函数
void cmd_fun_echooff(void);	//外部通话转发函数

#endif
