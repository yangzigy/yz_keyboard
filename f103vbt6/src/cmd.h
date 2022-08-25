/*
文件名：cmd.h
时间：2017-8-22
版本：	V2.1

功能：实现字符型人机交互

使用案例：
extern CMD_LIST bl_sys_cmd_list;
void con_task_ini()
{
	cmd_ini();
	cmd_reg((CMD_LIST*)&bl_sys_cmd_list);
}
//////////////////////////////////////////////////////////////
//			指令
void dir_calibrat_cmd(s8 *para)  //目标定位标定值
{
}
void srtm_z_offset_cmd(s8 *para)  //设置高程库的偏移
{
}
static const CMD_STR blcmd_str[]=\
{
	(s8 *)"dir_calibrat_cmd",(s8 *)"目标定位标定值",dir_calibrat_cmd,
	(s8 *)"srtm_z_offset_cmd",(s8 *)"设置高程库的偏移",srtm_z_offset_cmd,
};
CMD_LIST bl_sys_cmd_list={blcmd_str,sizeof(blcmd_str)/sizeof(CMD_STR),0};

*/
#ifndef CMD_H
#define CMD_H

#include "main.h"

typedef struct
{
	const char *str;	//命令名
	const char *des;	//描述
	void (*fun)(char *para);
} CMD_STR;

typedef struct _tag_CMD_LIST//命令列表，作为命令初始化的结构
{
	CMD_STR const *str;
	u8 num;//此数组中指令个数
	struct _tag_CMD_LIST *next;
} CMD_LIST;

extern const char *HelloString;

void cmd_ini(void);
void cmd_reg(CMD_LIST *a);//注册命令列表
void cmd_fun(u8);	//自身命令函数

void dft_cmd_out(const char *p); //printf定向
void void_cmd_out(const char *p); //空输出
extern void (*cmd_out)(const char *p);	//输出函数

#endif

