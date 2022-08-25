
#include "led.h"
//ascii转键盘码（键盘数组中的偏移位置）
const u8 ascii_index[256]=
{
0,//Fn
1,//01	SOH(start of headling)	标题开始 用作默认,和分隔符
1,//02	STX (start of text)	正文开始
1,//03	ETX (end of text)	正文结束
1,//04	EOT (end of transmission)	传输结束
1,//05	ENQ (enquiry)	请求
1,//06	ACK (acknowledge)	收到通知
1,//07	BEL (bell)	响铃
1,//08	BS (backspace)	退格
43,//09	HT (horizontal tab)	水平制表符
1,//0A	LF (NL line feed, new line)	换行键
1,//0B	VT (vertical tab)	垂直制表符
1,//0C	FF (NP form feed, new page)	换页键
40,//0D	CR (carriage return)	回车键
1,//0E	SO (shift out)	不用切换
1,//0F	SI (shift in)	启用切换
1,//10	DLE (data link escape)	数据链路转义
1,//11	DC1 (device control 1)	设备控制1
1,//12	DC2 (device control 2)	设备控制2
1,//13	DC3 (device control 3)	设备控制3
1,//14	DC4 (device control 4)	设备控制4
1,//15	NAK (negative acknowledge)	拒绝接收
1,//16	SYN (synchronous idle)	同步空闲
1,//17	ETB (end of trans. block)	传输块结束
1,//18	CAN (cancel)	取消
1,//19	EM (end of medium)	介质中断
1,//1A	SUB (substitute)	替补
1,//1B	ESC (escape)	溢出
1,//1C	FS (file separator)	文件分割符
1,//1D	GS (group separator)	分组符
1,//1E	RS (record separator)	记录分离符
1,//1F	US (unit separator)	单元分隔符
44,//20	(space)	空格
1,//21	!	 
1,//22	"	 
1,//23	#	 
1,//24	$	 
1,//25	%	 
1,//26	&	 
52,//27	'	 
1,//28	(	 
1,//29	)	 
1,//2A	*	 
1,//2B	+	 
54,//2C	,	 
45,//2D	-	 
55,//2E	.	 
56,//2F	/	 
39,//30	0	 
30,//31	1	 
31,//32	2	 
32,//33	3	 
33,//34	4	 
34,//35	5	 
35,//36	6	 
36,//37	7	 
37,//38	8	 
38,//39	9	 
1,//3A	:	 
51,//3B	;	 
1,//3C	<	 
46,//3D	=	 
1,//3E	>	 
1,//3F	?	 
1,//40	@	 
101,//41	A	 用于表示APP
42,//42	B	 用于表示backspace
224,//43	C	用于表示ctrl
1,//44	D	 
41,//45	E	 用于表示esc
1,//46	F	 
227,//47	G	 用于表示GUI
1,//48	H	 
1,//49	I	 
1,//4A	J	 
1,//4B	K	 
226,//4C	L	用于表示LeftAlt
1,//4D	M	 
1,//4E	N	 
1,//4F	O	 
1,//50	P	 
1,//51	Q	 
230,//52	R	用于表示rightAlt
225,//53	S	用于表示shift
43,//54	T	 用于表示TAB
1,//55	U	 
1,//56	V	 
1,//57	W	 
1,//58	X	 
1,//59	Y	 
1,//5A	Z	 
47,//5B	[	 
49,//5C	\|
48,//5D	]	 
1,//5E	^	 
1,//5F	_	 
1,//60	`	 
4,//61	a	 
5,//62	b	 
6,//63	c	 
7,//64	d	 
8,//65	e	 
9,//66	f	 
10,//67	g	 
11,//68	h	 
12,//69	i	 
13,//6A	j	 
14,//6B	k	 
15,//6C	l	 
16,//6D	m	 
17,//6E	n	 
18,//6F	o	 
19,//70	p	 
20,//71	q	 
21,//72	r	 
22,//73	s	 
23,//74	t	 
24,//75	u	 
25,//76	v	 
26,//77	w	 
27,//78	x	 
28,//79	y	 
29,//7A	z	 
1,//7B	{	 
1,//7C	|	 
1,//7D	}	 
53,//7E	~	 
1,//7F	DEL (delete)	删除
};
int led_stat=0; //led状态
int led_delay=0; //led延时计数器
//工具
int led_p=0; //当前执行到哪个位置了
int led_set_pattern(S_LED_LIGHT *pa,int n) //按给的pattern设置,并返回延时数
{
	for(; led_p < n; led_p++)
	{
		if (pa[led_p].key==1) //若是一个组结尾
		{
			return pa[led_p++].light;
		}
		keys[ascii_index[pa[led_p].key]].led=pa[led_p].light; //为了更方便的索引到keys数组中的引脚，通过ASCII码索引
	}
	led_p=0;
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////
//数组以1为分隔符,分组,每组持续单位为10ms,1的亮度为持续时间数
const S_LED_LIGHT pattern_common[]= //分4行快速显示
{
	'E',2, '1',2, '2',2, '3',2, '4',2, '5',2, '6',2, '7',2, '8',2, '9',2, '0',2, '-',2, '=',2, '\\',2, '~',2, 1,1, 
	'T',2, 'q',2, 'w',2, 'e',2, 'r',2, 't',2, 'y',2, 'u',2, 'i',2, 'o',2, 'p',2, '[',2, ']',2, 'B',2, 1,1,
	'C',2, 'a',2, 's',2, 'd',2, 'f',2, 'g',2, 'h',2, 'j',2, 'k',2, 'l',2, ';',2, '\'',2, '\n',2, 1,1,
	'S',2, 'z',2, 'x',2, 'c',2, 'v',2, 'b',2, 'n',2, 'm',2, ',',2, '.',2, '/',2, 
	'G',2, 'L',2, 0,2, ' ',2, 'R',2, 'A',2, 1,1,
};

void led_common(void)
{
	if (led_delay)
	{
		led_delay--;
		return ;
	}
	led_delay=led_set_pattern((S_LED_LIGHT*)pattern_common,sizeof(pattern_common)/sizeof(S_LED_LIGHT));
	led_delay=led_delay*(1+1/(0.25+keys_per_sec_f));
}
////////////////////////////////////////////////////////////////////
void (*led_fun)(void)=led_common;
void led_poll(void) //100Hz
{
	led_fun();
}
