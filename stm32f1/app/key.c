
#include "key.h"
#include "led.h"
#include "usb_conf.h"

int keys_per_sec=0; //每秒按键数
float keys_per_sec_f=0; //每秒按键数滤波结果
BUTTON_STATE keys[232]=
{
	0,MGPIOD,9,0,0, //Fn
	0,0,0,0,0, //1
	0,0,0,0,0, //2
	0,0,0,0,0, //3
	0,MGPIOE, 9,0,0, //4 KEY_a
	0,MGPIOE,15,0,0, //5 KEY_b
	0,MGPIOB,11,0,0, //6 KEY_c
	0,MGPIOE, 7,0,0, //7 KEY_d
	0,MGPIOA, 1,0,0, //8 KEY_e
	0,MGPIOB, 1,0,0, //9 KEY_f
	0,MGPIOB, 0,0,0, //10 KEY_g
	0,MGPIOC, 5,0,0, //11 KEY_h
	0,MGPIOE, 6,0,0, //12 KEY_i
	0,MGPIOC, 4,0,0, //13 KEY_j
	0,MGPIOA, 7,0,0, //14 KEY_k
	0,MGPIOA, 6,0,0, //15 KEY_l
	0,MGPIOE,13,0,0, //16 KEY_m
	0,MGPIOE,14,0,0, //17 KEY_n
	0,MGPIOE, 5,0,0, //18 KEY_o
	0,MGPIOE, 4,0,0, //19 KEY_p
	0,MGPIOA, 3,0,0, //20 KEY_q
	0,MGPIOA, 0,0,0, //21 KEY_r
	0,MGPIOE, 8,0,0, //22 KEY_s
	0,MGPIOC, 3,0,0, //23 KEY_t
	0,MGPIOC, 1,0,0, //24 KEY_u
	0,MGPIOB,10,0,0, //25 KEY_v
	0,MGPIOA, 2,0,0, //26 KEY_w
	0,MGPIOB,12,0,0, //27 KEY_x
	0,MGPIOC, 2,0,0, //28 KEY_y
	0,MGPIOB,13,0,0, //29 KEY_z
	0,MGPIOE, 1,0,0, //30 KEY_1
	0,MGPIOE, 0,0,0, //31 KEY_2
	0,MGPIOB, 9,0,0, //32 KEY_3
	0,MGPIOB, 8,0,0, //33 KEY_4
	0,MGPIOB, 7,0,0, //34 KEY_5
	0,MGPIOB, 6,0,0, //35 KEY_6
	0,MGPIOB, 5,0,0, //36 KEY_7
	0,MGPIOD, 7,0,0, //37 KEY_8
	0,MGPIOD, 6,0,0, //38 KEY_9
	0,MGPIOD, 5,0,0, //39 KEY_0
	0,MGPIOC,12,0,0, //40 KEY_ENTER
	0,MGPIOE, 2,0,0, //41 KEY_ESCAPE
	0,MGPIOC,11,0,0, //42 KEY_BackSpace
	0,MGPIOA, 4,0,0, //43 KEY_Tab
	0,MGPIOD,10,0,0, //44 KEY_Space
	0,MGPIOD, 3,0,0, //45 KEY_Minus
	0,MGPIOD, 2,0,0, //46 KEY_Equal
	0,MGPIOD, 4,0,0, //47 KEY_Bracket1 [ and { 
	0,MGPIOD, 1,0,0, //48 KEY_Bracket2 ] and } 
	0,MGPIOD, 0,0,0, //49 KEY_BackSlash \ and | 
	//0,MGPIOC,10,0,0, //50 KEY_Wave ~
	0,0,0,0,0, //50 KEY_Wave ~
	0,MGPIOC, 0,0,0, //51 KEY_Semicolon ; and : 
	0,MGPIOE, 3,0,0, //52 KEY_Quotes ‘ and “ 
	0,MGPIOC,10,0,0, //53 e KEY_  KEY_Wave
	0,MGPIOE,12,0,0, //54 KEY_Comma , and < 
	0,MGPIOE,11,0,0, //55 KEY_StopPoint . and > 
	0,MGPIOA, 5,0,0, //56 KEY_QuestionMark / and ? 
	0,0,0,0,0, //57 KEY_Caps
	0,0,0,0,0, //58 KEY_F1
	0,0,0,0,0, //59 KEY_F2
	0,0,0,0,0, //60 KEY_F3
	0,0,0,0,0, //61 KEY_F4
	0,0,0,0,0, //62 KEY_F5
	0,0,0,0,0, //63 KEY_F6
	0,0,0,0,0, //64 KEY_F7
	0,0,0,0,0, //65 KEY_F8
	0,0,0,0,0, //66 KEY_F9
	0,0,0,0,0, //67 KEY_F10
	0,0,0,0,0, //68 KEY_F11
	0,0,0,0,0, //69 KEY_F12
	0,0,0,0,0, //70 KEY_PrintScree
	0,0,0,0,0, //71 KEY_Scroll
	0,0,0,0,0, //72 KEY_Pause
	0,0,0,0,0, //73 KEY_Insert
	0,0,0,0,0, //74 KEY_Home
	0,0,0,0,0, //75 KEY_PageUp
	0,0,0,0,0, //76 KEY_Delete
	0,0,0,0,0, //77 KEY_End
	0,0,0,0,0, //78 KEY_PageDown
	0,0,0,0,0, //79 KEY_RightArrow
	0,0,0,0,0, //80 KEY_LeftArrow
	0,0,0,0,0, //81 KEY_DownArrow
	0,0,0,0,0, //82 KEY_UpArrow
	0,0,0,0,0, //83 KEY_NumLock
	0,0,0,0,0, //84
	0,0,0,0,0, //85
	0,0,0,0,0, //86
	0,0,0,0,0, //87
	0,0,0,0,0, //88
	0,0,0,0,0, //89
	0,0,0,0,0, //90
	0,0,0,0,0, //91
	0,0,0,0,0, //92
	0,0,0,0,0, //93
	0,0,0,0,0, //94
	0,0,0,0,0, //95
	0,0,0,0,0, //96
	0,0,0,0,0, //97
	0,0,0,0,0, //98
	0,0,0,0,0, //99
	0,0,0,0,0, //100
	0,MGPIOD,12,0,0, //101 KEY_APP
	0,0,0,0,0, //102
	0,0,0,0,0, //103
	0,0,0,0,0, //104
	0,0,0,0,0, //105
	0,0,0,0,0, //106
	0,0,0,0,0, //107
	0,0,0,0,0, //108
	0,0,0,0,0, //109
	0,0,0,0,0, //110
	0,0,0,0,0, //111
	0,0,0,0,0, //112
	0,0,0,0,0, //113
	0,0,0,0,0, //114
	0,0,0,0,0, //115
	0,0,0,0,0, //116
	0,0,0,0,0, //117
	0,0,0,0,0, //118 KEY_cfg
	0,0,0,0,0, //119
	0,0,0,0,0, //120
	0,0,0,0,0, //121
	0,0,0,0,0, //122
	0,0,0,0,0, //123
	0,0,0,0,0, //124
	0,0,0,0,0, //125
	0,0,0,0,0, //126
	0,0,0,0,0, //127
	0,0,0,0,0, //128
	0,0,0,0,0, //129
	0,0,0,0,0, //130
	0,0,0,0,0, //131
	0,0,0,0,0, //132
	0,0,0,0,0, //133
	0,0,0,0,0, //134
	0,0,0,0,0, //135
	0,0,0,0,0, //136
	0,0,0,0,0, //137
	0,0,0,0,0, //138
	0,0,0,0,0, //139
	0,0,0,0,0, //140
	0,0,0,0,0, //141
	0,0,0,0,0, //142
	0,0,0,0,0, //143
	0,0,0,0,0, //144
	0,0,0,0,0, //145
	0,0,0,0,0, //146
	0,0,0,0,0, //147
	0,0,0,0,0, //148
	0,0,0,0,0, //149
	0,0,0,0,0, //150
	0,0,0,0,0, //151
	0,0,0,0,0, //152
	0,0,0,0,0, //153
	0,0,0,0,0, //154
	0,0,0,0,0, //155
	0,0,0,0,0, //156
	0,0,0,0,0, //157
	0,0,0,0,0, //158
	0,0,0,0,0, //159
	0,0,0,0,0, //160
	0,0,0,0,0, //161
	0,0,0,0,0, //162
	0,0,0,0,0, //163
	0,0,0,0,0, //164
	0,0,0,0,0, //165
	0,0,0,0,0, //166
	0,0,0,0,0, //167
	0,0,0,0,0, //168
	0,0,0,0,0, //169
	0,0,0,0,0, //170
	0,0,0,0,0, //171
	0,0,0,0,0, //172
	0,0,0,0,0, //173
	0,0,0,0,0, //174
	0,0,0,0,0, //175
	0,0,0,0,0, //176
	0,0,0,0,0, //177
	0,0,0,0,0, //178
	0,0,0,0,0, //179
	0,0,0,0,0, //180
	0,0,0,0,0, //181
	0,0,0,0,0, //182
	0,0,0,0,0, //183
	0,0,0,0,0, //184
	0,0,0,0,0, //185
	0,0,0,0,0, //186
	0,0,0,0,0, //187
	0,0,0,0,0, //188
	0,0,0,0,0, //189
	0,0,0,0,0, //190
	0,0,0,0,0, //191
	0,0,0,0,0, //192
	0,0,0,0,0, //193
	0,0,0,0,0, //194
	0,0,0,0,0, //195
	0,0,0,0,0, //196
	0,0,0,0,0, //197
	0,0,0,0,0, //198
	0,0,0,0,0, //199
	0,0,0,0,0, //200
	0,0,0,0,0, //201
	0,0,0,0,0, //202
	0,0,0,0,0, //203
	0,0,0,0,0, //204
	0,0,0,0,0, //205
	0,0,0,0,0, //206
	0,0,0,0,0, //207
	0,0,0,0,0, //208
	0,0,0,0,0, //209
	0,0,0,0,0, //210
	0,0,0,0,0, //211
	0,0,0,0,0, //212
	0,0,0,0,0, //213
	0,0,0,0,0, //214
	0,0,0,0,0, //215
	0,0,0,0,0, //216
	0,0,0,0,0, //217
	0,0,0,0,0, //218
	0,0,0,0,0, //219
	0,0,0,0,0, //220
	0,0,0,0,0, //221
	0,0,0,0,0, //222
	0,0,0,0,0, //223
	0,MGPIOE,10,0,0, //224 LeftControl
	0,MGPIOB,14,0,0, //225 LeftShift
	0,MGPIOD, 8,0,0, //226 LeftAlt
	0,MGPIOB,15,0,0, //227 LeftGUI
	0,0,0,0,0, //228 RightControl
	0,0,0,0,0, //229 RightShift
	0,MGPIOD,11,0,0, //230 RightAlt
	0,0,0,0,0, //231 RightGUI
};
u8 keys_index[232];
int keys_none0=0; //非零个数

void set_gpio(MGPIO_TypeDef *port,u8 pin,u32 type)
{
	switch(pin)
	{
	case 0: port->CT0=type; break;
	case 1: port->CT1=type; break;
	case 2: port->CT2=type; break;
	case 3: port->CT3=type; break;
	case 4: port->CT4=type; break;
	case 5: port->CT5=type; break;
	case 6: port->CT6=type; break;
	case 7: port->CT7=type; break;
	case 8: port->CT8=type; break;
	case 9: port->CT9=type; break;
	case 10: port->CT10=type; break;
	case 11: port->CT11=type; break;
	case 12: port->CT12=type; break;
	case 13: port->CT13=type; break;
	case 14: port->CT14=type; break;
	case 15: port->CT15=type; break;
	break;
	}
}
void key_ini(void)
{
	//认为时钟已经打开
	int i;
	for(i = 0; i < sizeof(keys)/sizeof(BUTTON_STATE); i++)
	{
		if(keys[i].port==0) continue;
		set_gpio(keys[i].port,keys[i].pin,GPIO_IN_PP);
		keys[i].port->BSRR=(1<<keys[i].pin); //输入上拉
		//初始化index
		keys_index[keys_none0]=i;
		keys_none0++;
	}
}
s8 mousebuf[5]={1,0,0,0,0}; //鼠标发送缓存
u8 keybuf[8]={0,0,0,0,0,0,0,0}; //按键发送缓存
int keybufp=2; //位置
void putkey(u32 key) //发现一个键按下
{
	if(keybufp==8) //若已经超过6个了
	{
		//发送0xffff;
		memset(keybuf,0xff,8);
		keybufp++;
		return ;
	}
	else if(keybufp>8)
	{
		return ;
	}
	keybuf[keybufp++]=key;
}
void check_press(BUTTON_STATE *bts) //判断按键是否按下
{
	if(bts->port==0)
	{
		return ;
	}
	if(!(bts->port->IDR & (1<<(bts->pin))))
	{
		bts->tick=SYS_time;
		bts->state=1;
	}
	else //若释放了
	{
		if (SYS_time-bts->tick>5) //并且tick值在5个以上
		{
			bts->state=0;
		}
	}
}
int key_cfg_stat=0; //app键状态，0释放，1按下，2有组合
float mouse_xdelta=0; //鼠标实际移动量
float mouse_ydelta=0; //鼠标实际移动量
float mouse_zdelta=0; //鼠标实际移动量
int mouse_delta_0=3; //鼠标移动量0
int mouse_delta_1=15; //鼠标移动量1
void key_scan(void) //100Hz
{
	int i;
	keybufp=2; //位置
	memset(keybuf,0,8);
	//首先轮询所有位置
	for(i = 0; i < keys_none0; i++)
	{
		check_press(keys+keys_index[i]);
	}
	//特殊键直接置位
	if(keys[LeftControl].state) { keybuf[0] |= (1<<0); }
	if(keys[LeftShift].state) { keybuf[0] |= (1<<1); }
	if(keys[LeftAlt].state) { keybuf[0] |= (1<<2); }
	if(keys[LeftGUI].state) { keybuf[0] |= (1<<3); }
	if(keys[RightAlt].state) { keybuf[0] |= (1<<6); }
	//fn
	mousebuf[0]=1;
	mousebuf[1]=0;
	if(keys[OFFSET_FN].state || keys[KEY_cfg].state) //Fn按下，组合键需要将state置零，防止后续重新添加
	{
		int delta=mouse_delta_0;
		int x=0,y=0,z=0; //鼠标速度
		if(keys[KEY_a].state) //鼠标速度
		{
			delta=mouse_delta_1;
		}
		else
		{
			delta=mouse_delta_0;
		}
		//先判断是否是鼠标操作
		if(keys[KEY_d].state) {mousebuf[1] |= (1<<0);keys[KEY_d].state=0;}
		if(keys[KEY_f].state) {mousebuf[1] |= (1<<2);keys[KEY_f].state=0;}
		if(keys[KEY_g].state) {mousebuf[1] |= (1<<1);keys[KEY_g].state=0;}
		if(keys[KEY_n].state) {z=delta; keys[KEY_n].state=0;}
		if(keys[KEY_m].state) {z=-delta;keys[KEY_m].state=0;}
		if(keys[KEY_s].state || mousebuf[1] || z) //鼠标上下左右：fn+s+hjkl
		{
			static u32 mouse_tick=0;
			mouse_tick++;
			if(keys[KEY_h].state) {x=-delta;}
			if(keys[KEY_j].state) {y=delta;}
			if(keys[KEY_k].state) {y=-delta;}
			if(keys[KEY_l].state) {x=delta;}
			mouse_xdelta=mouse_xdelta*0.94f + x*0.06f;
			mouse_ydelta=mouse_ydelta*0.94f + y*0.06f;
			//mouse_zdelta=z;
			mouse_zdelta=mouse_zdelta*0.94f + z*0.06f;
			mousebuf[2]=mouse_xdelta;
			mousebuf[3]=mouse_ydelta;
			mousebuf[4]=(mouse_tick&7)==0?mouse_zdelta+0.6:0;
			mouse_send(mousebuf);
		}
		else //非鼠标操作
		{
			mouse_xdelta=0; mouse_ydelta=0; mouse_zdelta=0; //将鼠标移动速度清零
			mousebuf[2]=mouse_xdelta;
			mousebuf[3]=mouse_ydelta;
			mousebuf[4]=mouse_zdelta;
			mouse_send(mousebuf);
			if(keys[KEY_a].state) //若是home等
			{
				if(keys[KEY_k].state) { putkey(KEY_PageUp); }
				if(keys[KEY_j].state) { putkey(KEY_PageDown); }
				if(keys[KEY_h].state) { putkey(KEY_Home); }
				if(keys[KEY_l].state) { putkey(KEY_End); }
			}
			else
			{
				if(keys[KEY_h].state) { putkey(KEY_LeftArrow); }
				if(keys[KEY_j].state) { putkey(KEY_DownArrow); }
				if(keys[KEY_k].state) { putkey(KEY_UpArrow); }
				if(keys[KEY_l].state) { putkey(KEY_RightArrow); }
			}
			if(keys[KEY_Comma].state) { putkey(KEY_Home); keys[KEY_Comma].state=0;}
			if(keys[KEY_StopPoint].state) { putkey(KEY_End); keys[KEY_StopPoint].state=0;}
			if(keys[KEY_1].state) { putkey(KEY_F1); keys[KEY_1].state=0;}
			if(keys[KEY_2].state) { putkey(KEY_F2); keys[KEY_2].state=0;}
			if(keys[KEY_3].state) { putkey(KEY_F3); keys[KEY_3].state=0;}
			if(keys[KEY_4].state) { putkey(KEY_F4); keys[KEY_4].state=0;}
			if(keys[KEY_5].state) { putkey(KEY_F5); keys[KEY_5].state=0;}
			if(keys[KEY_6].state) { putkey(KEY_F6); keys[KEY_6].state=0;}
			if(keys[KEY_7].state) { putkey(KEY_F7); keys[KEY_7].state=0;}
			if(keys[KEY_8].state) { putkey(KEY_F8); keys[KEY_8].state=0;}
			if(keys[KEY_9].state) { putkey(KEY_F9); keys[KEY_9].state=0;}
			if(keys[KEY_0].state) { putkey(KEY_F10);keys[KEY_0].state=0; }
			if(keys[KEY_Minus].state) { putkey(KEY_F11); keys[KEY_Minus].state=0; }
			if(keys[KEY_Equal].state) { putkey(KEY_F12); keys[KEY_Equal].state=0; }
			if(keys[KEY_i].state) { putkey(KEY_Insert); keys[KEY_i].state=0; }
			if(keys[KEY_p].state) { putkey(KEY_PrintScree); keys[KEY_p].state=0; }
			if(keys[KEY_BackSpace].state) { putkey(KEY_Delete); keys[KEY_BackSpace].state=0; }
			if(keys[KEY_Wave].state) { keybuf[0] |= (1<<1);  putkey(KEY_Delete); keys[KEY_Wave].state=0; }
			if(keys[KEY_u].state) { putkey(KEY_UpArrow); keybuf[0] |= (1<<2);keys[KEY_u].state=0; }
			if(keybufp>2 && keys[KEY_cfg].state) //APP键按下且有组合
			{
				key_cfg_stat=2;
			}
		}
		//发送
		//keyboard_send(keybuf); //最后发送缓冲
		//return ;
		if(keys[KEY_h].state) {keys[KEY_h].state=0;} //将按键置零，等后边发
		if(keys[KEY_j].state) {keys[KEY_j].state=0;}
		if(keys[KEY_k].state) {keys[KEY_k].state=0;}
		if(keys[KEY_l].state) {keys[KEY_l].state=0;}
		if(keys[KEY_s].state) {keys[KEY_s].state=0;}
		if(keys[KEY_a].state) {keys[KEY_a].state=0;}
	}
	else
	{
		mouse_xdelta=0; mouse_ydelta=0; mouse_zdelta=0; //将鼠标移动速度清零
		mousebuf[2]=mouse_xdelta;
		mousebuf[3]=mouse_ydelta;
		mousebuf[4]=mouse_zdelta;
		mouse_send(mousebuf);
	}
	if(keys[KEY_cfg].state==0) //cfg键释放事件
	{
		keys[KEY_cfg].state=key_cfg_stat==1?1:0;
		key_cfg_stat=0;
	}
	else //cfg键按下事件
	{
		key_cfg_stat=key_cfg_stat==2?2:1;
		keys[KEY_cfg].state=0;
	}
	//普通键追加到buf
	for(i = 0; i < keys_none0; i++)
	{
		int j=keys_index[i];
		if(keys[j].state)
		{
			putkey(j);
		}
	}
	keyboard_send(keybuf); //最后发送缓冲
	//统计按键个数
	if(keybuf[2]) keys_per_sec++;
}
void key_poll(void) //任务函数2000Hz
{
	static u32 tick=0;
	int i;
	if(tick==0) //100Hz
	{
		//配置输入上拉
		for(i = 0; i < keys_none0; i++) //判断每个按键，是否亮
		{
			int j=keys_index[i];
			set_gpio(keys[j].port,keys[j].pin,GPIO_IN_PP);
			keys[j].port->BSRR=(1<<keys[j].pin); //输入上拉
		}
		key_scan(); //100Hz
	}
	for(i = 0; i < keys_none0; i++) //判断每个按键，是否亮
	{
		int j=keys_index[i];
		//判断，设置输出0
		if(20-keys[j].led==tick) //若有在这个周期需要亮的led
		{
			set_gpio(keys[j].port,keys[j].pin,GPIO_OUT_PP);
			keys[j].port->BRR=(1<<keys[j].pin);
		}
	}
	tick++;
	if(tick>=20) //100Hz
	{
		static u32 tick1=0;
		tick=0;
		tick1++;
		if(tick1%100==0) //1s统计按键频率
		{
			float k=0.95;
			if (keys_per_sec>keys_per_sec_f) { k=0.9; }
			keys_per_sec_f=keys_per_sec_f*k+(1-k)*keys_per_sec;
			keys_per_sec=0;
		}
		led_poll();
	}
}

