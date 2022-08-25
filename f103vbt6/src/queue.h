/*
文件名：queue.h
时间：2011-1-22
修改时间：2016-3-1
版本：	V1.0			2018-11-21 13:56:31
版本：	V1.1			2019-06-12 12:14:44

功能：
	实现PC与单片机通用的环形队列。
	队列提供按字节的存储，并可块操作以提高效率
	可以选择队满时入队失败或者覆盖最老数据两种入队方式
	支持读写指针的调整,考虑数据长度

在使用时，需要将整块数据入队出队时使用块操作函数，而对于单字节数据使用单字节函数
*/
#ifndef QUEUE_H
#define QUEUE_H

#include "main.h"

#ifdef __cplusplus
extern "C" //防止在C++文件中引用
{
#endif

typedef struct 
{
	u8 *q_data; //数据指针
	t_maxs buflen; //缓冲区长度
	t_maxs r; //队首(r)指向还没读的那个字节
	t_maxs w;//对尾(w)指向即将写的那个字节
	t_maxs dlen;//数据的长度

	u8 *p_in; //输入的数据指针(为了节省堆栈)
	t_maxs n_in; //输入的数据长度
	t_maxs empty; //空的长度，每次计算时更新
} Queue;

void Queue_ini(u8 *p,t_maxs num,Queue *q);

t_maxs Queue_set(u8 *p,t_maxs n,Queue *q);	//入队成功返回0，失败返回未入队的个数
void Queue_set_over(u8 *p,t_maxs n,Queue *q);	//覆盖最老的数据
t_maxs Queue_get(u8 *p,t_maxs n,Queue *q); //出队成功返回0，失败返回非零
t_maxs Queue_rseek(t_maxs n,Queue *q); //调整读指针，返回是否出错,出错则尽可能调整
t_maxs Queue_wseek(t_maxs n,Queue *q); //调整写指针，返回是否出错,出错则尽可能调整

//为了提高效率，编写单字节操作函数
t_maxs Queue_set_1(u8 p,Queue *q);	//队成功返回0，失败返回1
t_maxs Queue_get_1(u8 *p,Queue *q); //出队成功返回0，失败返回非零

#ifdef __cplusplus
} //extern "C"
#endif

#endif
