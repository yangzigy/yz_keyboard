//文件名： key.h
#ifndef KEY_H
#define KEY_H

#include "main.h"
#include "stm32f1_sys.h" 	

#define OFFSET_FN	0 //Fn键的偏移位置
//0  00  Reserved (no event indicated)
//1  01  Keyboard ErrorRollOver
//2  02  Keyboard POSTFail
//3  03  Keyboard ErrorUndefined
#define KEY_a	4 //4 04  Keyboard a and A 
#define KEY_b	5 //5 05  Keyboard b and B
#define KEY_c	6 //6 06  Keyboard c and C 
#define KEY_d	7 //7 07  Keyboard d and D
#define KEY_e	8 //8 08  Keyboard e and E
#define KEY_f	9 //9 09  Keyboard f and F
#define KEY_g	10	//10  0A  Keyboard g and G
#define KEY_h	11	//11  0B  Keyboard h and H
#define KEY_i	12	//12  0C  Keyboard i and I
#define KEY_j	13	//13  0D  Keyboard j and J
#define KEY_k	14	//14  0E  Keyboard k and K
#define KEY_l	15	//15  0F  Keyboard l and L
#define KEY_m	16	//16  10  Keyboard m and M
#define KEY_n	17	//17  11  Keyboard n and N
#define KEY_o	18	//18  12  Keyboard o and O
#define KEY_p	19	//19  13  Keyboard p and P
#define KEY_q	20	//20  14  Keyboard q and Q
#define KEY_r	21	//21  15  Keyboard r and R
#define KEY_s	22	//22  16  Keyboard s and S
#define KEY_t	23	//23  17  Keyboard t and T
#define KEY_u	24	//24  18  Keyboard u and U
#define KEY_v	25	//25  19  Keyboard v and V
#define KEY_w	26	//26  1A  Keyboard w and W
#define KEY_x	27	//27  1B  Keyboard x and X
#define KEY_y	28	//28  1C  Keyboard y and Y
#define KEY_z	29	//29  1D  Keyboard z and Z
#define KEY_1	30	//30  1E  Keyboard 1 and !
#define KEY_2	31	//31  1F  Keyboard 2 and @
#define KEY_3	32	//32  20  Keyboard 3 and #
#define KEY_4	33	//33  21  Keyboard 4 and $
#define KEY_5	34	//34  22  Keyboard 5 and %
#define KEY_6	35	//35  23  Keyboard 6 and ^
#define KEY_7	36	//36  24  Keyboard 7 and &
#define KEY_8	37	//37  25  Keyboard 8 and *
#define KEY_9	38	//38  26  Keyboard 9 and (
#define KEY_0	39	//39  27  Keyboard 0 and )
#define KEY_ENTER 	40  //40	28  Keyboard Return (ENTER) 
#define KEY_ESCAPE 	41  //41	29  Keyboard ESCAPE  
#define KEY_BackSpace 	42  //42	2A  Keyboard DELETE (Backspace) 
#define KEY_Tab 	43  //43	2B  Keyboard Tab 
#define KEY_Space 	44  //44	2C  Keyboard Spacebar 
#define KEY_Minus 	45  //45	2D  Keyboard - and (underscore) 
#define KEY_Equal 	46  //46	2E  Keyboard = and + 
#define KEY_Bracket1 	47  //47	2F  Keyboard [ and { 
#define KEY_Bracket2 	48  //48	30  Keyboard ] and } 
#define KEY_BackSlash 	49  //49	31  Keyboard \ and | 
//#define KEY_Wave 	50  //50	32  Keyboard Non-US # and ~ 
#define KEY_Semicolon 	51  //51	33  Keyboard ; and : 
#define KEY_Quotes 	52  //52	34  Keyboard ‘ and “ 
//#define KEY_ 	53  //53	35  Keyboard Grave Accent and Tilde 
#define KEY_Wave 	53  //53	35  Keyboard Grave Accent and Tilde 
#define KEY_Comma 	54  //54	36  Keyboard, and < 
#define KEY_StopPoint 	55  //55	37  Keyboard . and > 
#define KEY_QuestionMark 	56  //56	38  Keyboard / and ? 
#define KEY_Caps 	57  //57	39  Keyboard Caps Lock
#define KEY_F1 	58  //58	3A  Keyboard F1  
#define KEY_F2 	59  //59	3B  Keyboard F2  
#define KEY_F3 	60  //60	3C  Keyboard F3  
#define KEY_F4 	61  //61	3D  Keyboard F4  
#define KEY_F5 	62  //62	3E  Keyboard F5  
#define KEY_F6 	63  //63	3F  Keyboard F6  
#define KEY_F7 	64  //64	40  Keyboard F7  
#define KEY_F8 	65  //65	41  Keyboard F8  
#define KEY_F9 	66  //66	42  Keyboard F9  
#define KEY_F10 	67  //67	43  Keyboard F10  
#define KEY_F11 	68  //68	44  Keyboard F11
#define KEY_F12 	69  //69	45  Keyboard F12
#define KEY_PrintScree 	70	//70  46  Keyboard PrintScree n 
#define KEY_Scroll 	71	//71  47  Keyboard Scroll Lock
#define KEY_Pause 	72	//72  48  Keyboard Pause 
#define KEY_Insert 	73	//73  49  Keyboard Insert
#define KEY_Home 	74	//74  4A  Keyboard Home
#define KEY_PageUp 	75	//75  4B  Keyboard PageUp
#define KEY_Delete 	76	//76  4C  Keyboard Delete Forward
#define KEY_End 	77	//77  4D  Keyboard End
#define KEY_PageDown 	78	//78  4E  Keyboard PageDown
#define KEY_RightArrow 	79	//79  4F  Keyboard RightArrow
#define KEY_LeftArrow 	80	//80  50  Keyboard LeftArrow
#define KEY_DownArrow 	81	//81  51  Keyboard DownArrow
#define KEY_UpArrow 	82	//82  52  Keyboard UpArrow
#define KEY_NumLock 	83	//83  53  Keypad Num Lock and Clear
//84  54  Keypad /
//85  55  Keypad * 
//86  56  Keypad - 
//87  57  Keypad + 
//88  58  Keypad ENTER
//89  59  Keypad 1 and End
//90  5A  Keypad 2 and Down Arrow
//91  5B  Keypad 3 and PageDn 
//92  5C  Keypad 4 and Left Arrow
//93  5D  Keypad 5
//94  5E  Keypad 6 and Right Arrow 
//95  5F  Keypad 7 and Home
//96  60  Keypad 8 and Up Arrow
//97  61  Keypad 9 and PageUp 
//98  62  Keypad 0 and Insert
//99  63  Keypad . and Delete 
//100  64  Keyboard Non-US \ and | 3;
#define KEY_cfg	101	//101	65  Keyboard Application 
//102  66  Keyboard Power
//103  67  Keypad =  
//104  68  Keyboard F13
//105  69  Keyboard F14
//106  6A  Keyboard F15
//107  6B  Keyboard F16 
//108  6C  Keyboard F17 
//109  6D  Keyboard F18 
//110  6E  Keyboard F19 
//111  6F  Keyboard F20 
//112  70  Keyboard F21 
//113  71  Keyboard F22 
//114  72  Keyboard F23 
//115  73  Keyboard F24 
//116  74  Keyboard Execute  √ 
//117  75  Keyboard Help  √ 
//#define KEY_cfg	118 //118 76  Keyboard Menu  √ 
//119  77  Keyboard Select  √ 
//120  78  Keyboard Stop  √ 
//121  79  Keyboard Again  √ 
//122  7A  Keyboard Undo  √ 
//123  7B  Keyboard Cut  √ 
//124  7C  Keyboard Copy  √ 
//125  7D  Keyboard Paste  √ 
//126  7E  Keyboard Find  √ 
//127  7F  Keyboard Mute  √ 
//128  80  Keyboard Volume Up  √ 
//129  81  Keyboard Volume Down  √ 
//130  82  Keyboard Locking Caps Lock 12 √ 
//131  83  Keyboard Locking Num Lock 12 √ 
//132  84  Keyboard Locking Scroll Lock 12 √ 
//133  85  Keypad Comma 27 107 
//134  86  Keypad Equal Sign 29
//135  87  Keyboard International1 15,28 56 
//136  88  Keyboard International2 16
//137  89  Keyboard International3 17
//138  8A  Keyboard International4 18
//139  8B  Keyboard International5 19
//140  8C  Keyboard International6 20
//141  8D  Keyboard International7 21
//142  8E  Keyboard International8 22
//143  8F  Keyboard International9 22
//144  90  Keyboard LANG1 25
//145  91  Keyboard LANG2 26
//146  92  Keyboard LANG3 30
//147  93  Keyboard LANG4 31
//148  94  Keyboard LANG5 32
//149  95  Keyboard LANG6 8
//150  96  Keyboard LANG7 8
//151  97  Keyboard LANG8 8
//152  98  Keyboard LANG9 8
//153  99  Keyboard Alternate Erase 7
//154  9A  Keyboard SysReq/Attention 1
//155  9B  Keyboard Cancel 
//156  9C  Keyboard Clear 
//157  9D  Keyboard Prior 
//158  9E  Keyboard Return 
//159  9F  Keyboard Separator 
//160  A0  Keyboard Out 
//161  A1  Keyboard Oper 
//162  A2  Keyboard Clear/Again 
//163  A3  Keyboard CrSel/Props 
//164  A4  Keyboard ExSel 
//165-175  A5-CF  Reserved 
//176  B0  Keypad 00 
//177  B1  Keypad 000 
//178  B2  Thousands Separator  33
//179  B3  Decimal Separator  33
//180  B4  Currency Unit  34
//181  B5  Currency Sub-unit  34
//182  B6  Keypad ( 
//183  B7  Keypad ) 
//184  B8  Keypad { 
//185  B9  Keypad } 
//186  BA  Keypad Tab 
//187  BB  Keypad Backspace 
//188  BC  Keypad A 
//189  BD  Keypad B 
//190  BE  Keypad C 
//191  BF  Keypad D 
//192  C0  Keypad E 
//193  C1  Keypad F 
//194  C2  Keypad XOR 
//195  C3  Keypad ^ 
//196  C4  Keypad % 
//197  C5  Keypad < 
//198  C6  Keypad > 
//199  C7  Keypad & 
//200  C8  Keypad && 
//201  C9  Keypad | 
//202  CA  Keypad || 
//203  CB  Keypad : 
//204  CC  Keypad # 
//205  CD  Keypad Space 
//206  CE  Keypad @ 
//207  CF  Keypad ! 
//208  D0  Keypad Memory Store 
//209  D1  Keypad Memory Recall 
//210  D2  Keypad Memory Clear 
//211  D3  Keypad Memory Add 
//212  D4  Keypad Memory Subtract 
//213  D5  Keypad Memory Multiply 
//214  D6  Keypad Memory Divide 
//215  D7  Keypad +/- 
//216  D8  Keypad Clear 
//217  D9  Keypad Clear Entry 
//218  DA  Keypad Binary 
//219  DB  Keypad Octal 
//220  DC  Keypad Decimal 
//221  DD  Keypad Hexadecimal 
//222-223  DE-DF  Reserved 
#define LeftControl	224 //224 	E0  Keyboard LeftControl  
#define LeftShift	225 //225 	E1  Keyboard LeftShift  
#define LeftAlt		226 //226 	E2  Keyboard LeftAlt  
#define LeftGUI		227 //227 	E3  Keyboard Left GUI 
#define RightControl	228 //228 	E4  Keyboard RightControl
#define RightShift	229 //229 	E5  Keyboard RightShift  
#define RightAlt	230 //230 	E6  Keyboard RightAlt
#define RightGUI	231 //231 	E7  Keyboard Right GUI
//232-65535 E8-FFFF  Reserved 

#pragma pack(1)
typedef struct
{
	u32 tick; //记录上次按下的tick值
	MGPIO_TypeDef *port; //按键的端口
	u8 pin; //按键的引脚
	u8 state; //按键状态
	u8 led; //亮度0~20
} BUTTON_STATE; //记录按键状态的结构
#pragma pack()
extern int keys_per_sec; //每秒按键数
extern float keys_per_sec_f; //每秒按键数滤波结果
extern BUTTON_STATE keys[232];
void key_ini(void);
void key_poll(void); //任务函数

#endif

