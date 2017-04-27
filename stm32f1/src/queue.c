/*
文件名：queue.c
作者：北京交通大学 思源0702 杨孜
时间：2011-1-22
修改时间：2016-3-1

功能：
	实现PC与单片机通用的环形队列。
*/

#include "queue.h"

/////////////////////////////////////////////////////////////////////////
//工具函数
#define S (*q) //让指针操作有对象风格
#define MAX(a,b) ( ((a)>(b)) ? (a):(b) )
#define MIN(a,b) ( ((a)>(b)) ? (b):(a) )

static void write_cb(t_maxs n,Queue *q) //写队列回调
{
	//从写指针开始写，长度n，写完修改各指针
	memcpy(S.q_data+S.w,S.p_in,n);
	S.dlen+=n;
	S.empty=S.buflen-S.dlen; //空的长度
}
static void read_cb(t_maxs n,Queue *q) //读队列回调
{
	//从读指针开始读，长度n，写完修改各指针
	memcpy(S.p_in,S.q_data+S.r,n);
	S.dlen-=n;
	S.empty=S.dlen; //空的长度
}
//分两种情况：w<r r<w	/为数据，~为空
//	|/////////////w->~~~~~~~~~~~~~~~~~~~~r->//////////////|
//	|~~~~~~~~~~~~~r->////////////////////w->~~~~~~~~~~~~~~|
//在队列中运转,输入静止偏移变量的指针，运动偏移变量的指针,数量
static void ring(t_maxs *sta,t_maxs *mot,Queue *q,void (*cb)(t_maxs n,Queue *q)) 
{
	t_maxs cplen=0; //复制的长度
	while(1)
	{
		cplen=0;
		if(*mot<*sta) //若是情况1
		{
			cplen=MIN(S.empty,S.n_in); //取空白和待写数据中比较小的
		}
		else if(*mot>*sta || S.empty!=0) //若是情况2
		{
			t_maxs n_end=S.buflen-*mot; //至尾长度
			cplen=MIN(n_end,S.n_in); //取至尾长度和待写数据中比较小的
		}
		if (cplen==0)
		{
			return;
		}
		cb(cplen,q); //回调函数
		*mot+=cplen;
		if (*mot>=S.buflen)
		{
			*mot=0;
		}
		else if(*mot<0)
		{
			*mot+=S.buflen;
		}
		S.p_in+=cplen;
		S.n_in-=cplen;
	}
}
///////////////////////////////////////////////////////////////////
//队列功能操作：
void Queue_ini(u8 *d,t_maxs num,Queue *q) //输入缓冲地址，缓冲长度
{
	S.q_data=d;
	S.buflen=num;
	S.r=0;
	S.w=0;
	S.dlen=0;//数据的长度
	S.p_in=0;
	S.n_in=0;
	S.empty=0;
}
t_maxs Queue_set(u8 *p,t_maxs n,Queue *q)	//返回入队的个数
{
	S.p_in=p;
	S.n_in=n;
	S.empty=S.buflen-S.dlen; //空的长度
	ring(&S.r,&S.w,q,write_cb);
	return n-S.n_in;
}
t_maxs Queue_get(u8 *p,t_maxs n,Queue *q) //返回出队的个数
{
	S.p_in=p;
	S.n_in=n;
	S.empty=S.dlen; //数据的长度
	ring(&S.w,&S.r,q,read_cb);
	return n-S.n_in;
}
t_maxs Queue_rseek(t_maxs n,Queue *q) //调整读指针，返回是否出错,出错则尽可能调整
{
	t_maxs total=n;
	if(n>S.dlen)  //首先对输入限幅
	{
		n=S.dlen;
	}
	else if (n<S.dlen-S.buflen)
	{
		n=S.dlen-S.buflen;
	}
	S.r+=n;
	if (S.r<0)
	{
		S.r+=S.buflen;
	}
	S.r=S.r%S.buflen;
	S.dlen-=n;
	return total-n;
}
t_maxs Queue_wseek(t_maxs n,Queue *q) //调整写指针，返回是否出错,出错则尽可能调整
{
	t_maxs total=n;
	if(n>S.buflen-S.dlen)  //首先对输入限幅
	{
		n=S.buflen-S.dlen;
	}
	else if (n<-S.dlen)
	{
		n=-S.dlen;
	}
	S.w+=n;
	if (S.w<0)
	{
		S.w+=S.buflen;
	}
	S.w=S.w%S.buflen;
	S.dlen+=n;
	return total-n;
}
void Queue_set_over(u8 *p,t_maxs n,Queue *q)	//覆盖最老的数据
{//有意义的写长度只是一个buflen，所以大于这个值时，取后一个buflen的长度
	int l;
	if (n>S.buflen)
	{
		p+=n-S.buflen;
		n=S.buflen;
	}
	l=Queue_set(p,n,q); //有l个入队
	p+=l;
	l=n-l; //剩下l个没有入队
	Queue_rseek(l,q);
	Queue_set(p,l,q);
}

//为了提高效率，编写单字节操作函数
t_maxs Queue_set_1(u8 p,Queue *q)	//入队成功返回0，失败返回1
{
	if(S.dlen<S.buflen) //若是空的可以写
	{
		S.q_data[S.w]=p;
		S.w++;
		if(S.w>=S.buflen)
		{
			S.w=0;
		}
		S.dlen++;
		return 0;
	}
	return 1;
}
t_maxs Queue_get_1(u8 *p,Queue *q) //出队成功返回0，失败返回非零
{
	if (S.dlen>0) //若有数据可以读
	{
		*p=S.q_data[S.r];
		S.r++;
		if(S.r>=S.buflen)
		{
			S.r=0;
		}
		S.dlen--;
		return 0;
	}
	return 1;
}

