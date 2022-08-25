
#include "stm32f1_sys.h" 

//系统变量
u32 SYS_task=0; //任务标志
u16 SYS_idle=0; //空闲计数
u16 SYS_idle_dis=0; //空闲计数的平均
u32 SYS_time=0; //周期计数
void wait(vu32 i)
{
	while(i--);
}
int void_putchar(int c)
{
	return 0;
}
int (*IO_putchar)(int)=void_putchar;	//初始化的函数
int fputc(int c, FILE *f)
{
	return IO_putchar(c);
}
//设置向量表偏移地址
//NVIC_VectTab:基址
//Offset:偏移量
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{ 	   	 
	SCB->VTOR = NVIC_VectTab|(Offset & (u32)0x1FFFFF80);//设置NVIC的向量表偏移寄存器
	//用于标识向量表是在CODE区还是在RAM区
}
//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//取后三位
	temp1<<=8;
	temp=SCB->AIRCR;  //读取先前的设置
	temp&=0X0000F8FF; //清空先前分组
	temp|=0X05FA0000; //写入钥匙
	temp|=temp1;	   
	SCB->AIRCR=temp;  //设置分组
}
//设置NVIC 
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)
{ 
	u32 temp;
	MY_NVIC_PriorityGroupConfig(NVIC_Group);//设置分组
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;								//取低四位  
	NVIC->ISER[NVIC_Channel/32]|=(1<<NVIC_Channel%32);//使能中断位(要清除的话,相反操作就OK) 
	NVIC->IP[NVIC_Channel]|=temp<<4;		//设置响应优先级和抢断优先级
} 
//外部中断配置函数
//只针对GPIOA~G;不包括PVD,RTC和USB唤醒这三个
//参数:
//GPIOx:0~6,代表GPIOA~G
//BITx:需要使能的位;
//TRIM:触发模式,1,下升沿;2,上降沿;3，任意电平触发
//该函数一次只能配置1个IO口,多个IO口,需多次调用
//该函数会自动开启对应中断,以及屏蔽线
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM) 
{
	u8 EXTADDR;
	u8 EXTOFFSET;
	EXTADDR=BITx/4;//得到中断寄存器组的编号
	EXTOFFSET=(BITx%4)*4; 
	RCC->APB2ENR|=0x01;//使能io复用时钟
	AFIO->EXTICR[EXTADDR]&=~(0x000F<<EXTOFFSET);//清除原来设置！！！
	AFIO->EXTICR[EXTADDR]|=GPIOx<<EXTOFFSET;//EXTI.BITx映射到GPIOx.BITx 
	//自动设置
	EXTI->IMR|=1<<BITx;//  开启line BITx上的中断
	//EXTI->EMR|=1<<BITx;//不屏蔽line BITx上的事件 (如果不屏蔽这句,在硬件上是可以的,但是在软件仿真的时候无法进入中断!)
 	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;//line BITx上事件下降沿触发
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;//line BITx上事件上升降沿触发
}
//不能在这里执行所有外设复位!否则至少引起串口不工作.
//把所有时钟寄存器复位
void MYRCC_DeInit(void)
{
	RCC->APB1RSTR = 0x00000000;//复位结束
	RCC->APB2RSTR = 0x00000000; 

	RCC->AHBENR = 0x00000014;  //睡眠模式闪存和SRAM时钟使能.其他关闭.	  
	RCC->APB2ENR = 0x00000000; //外设时钟关闭.
	RCC->APB1ENR = 0x00000000;   
	RCC->CR |= 0x00000001;     //使能内部高速时钟HSION
	RCC->CFGR &= 0xF8FF0000;   //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]
	RCC->CR &= 0xFEF6FFFF;     //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //复位HSEBYP
	RCC->CFGR &= 0xFF80FFFF;   //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR = 0x00000000;     //关闭所有中断
	//配置向量表
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(0x20000000, 0x0);
#else   
	MY_NVIC_SetVectorTable(0x08000000,0x0);
#endif
}
//OSPEED为速度，0无效，1：10MHz，2：2MHz，3：50MHz
void GPIO_Set(GPIO_TypeDef* GPIOx,u32 pin,u32 MODE,u32 OTYPE,u32 OSPEED,u32 PUPD)
{  
	u32 pinpos=0,pos=0,curpin=0;
	for(pinpos=0;pinpos<16;pinpos++)
	{
		vu32 *pcr;
		pos=1<<pinpos;	//一个个位检查 
		curpin=pin&pos;//检查引脚是否要设置
		pcr=pinpos<8?&(GPIOx->CRL):&(GPIOx->CRH);
		if(curpin==pos)	//需要设置
		{
			u32 regpos=pinpos>=8?pinpos-8:pinpos;
			u32 mask=0x0f<<(regpos*4);
			u32 val=0;
			*pcr&=~(mask);	//先清除原来的设置
			if(MODE==0) val=PUPD?8:4; //输入,若有配置上下拉
			else if(MODE==1) val=OSPEED | (OTYPE<<2);//GPIO输出
			else if(MODE==2) val=OSPEED | (OTYPE<<2) | (1<<3); //AF
			else if(MODE==3) val=0; //模拟输入
			*pcr|=val<<(regpos*4);	//设置新的模式 
			val=1<<pin; //设置io的位
			GPIOx->BSRR=PUPD==0?(val<<16):val; //上下拉
		}
	}
} 
//看门狗开启
//定时器时钟为40k,最大时间：409.6/819.2/1638.4ms
void wtd_ini(u8 div) //按默认0，200~600ms
{
	DBGMCU->CR |= (1<<8); //DBG_IWDG_STOP调试时看门狗不工作
	IWDG->KR=0x5555;
	IWDG->PR=div;//预分频:4/8/16/32……,写之前必须先关写保护
	//IWDG->KR=0x5555;
	//IWDG->RLR=;//重装载值12bit
	IWDG->KR=0xcccc;//启动看门狗	
}
void wtd_tri(void)
{
	IWDG->KR=0xaaaa;
}
//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");
}
//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(u32 addr) 
{
	__ASM volatile("MSR MSP, r0");
	__ASM volatile("BX r14");
}
void switch_2_addr(u32 *pc) //切换到指定的程序指针
{
	u32 i = 0;
	MYRCC_DeInit();

	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0x000007FF;
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0x000007FF;
	for(i = 0; i < 0x0B; i++)
	{
		NVIC->IP[i] = 0x00000000;
	}
	OS_CLOSE_INT;

	MSR_MSP(*pc); //堆栈指针是应用程序起始的第一个u32
	((void (*)(void))*(pc+1))();//入口函数是应用程序起始的第二个u32
}
//进入待机模式	  
void Sys_Standby(void)
{
	SCB->SCR|=1<<2;//使能SLEEPDEEP位 (SYS->CTRL)	   
  	RCC->APB1ENR|=1<<28;     //使能电源时钟	    
 	PWR->CSR|=1<<8;          //设置WKUP用于唤醒
	PWR->CR|=1<<2;           //清除Wake-up 标志
	PWR->CR|=1<<1;           //PDDS置位		  
	WFI_SET();				 //执行WFI指令		 
}	     
//系统软复位   
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 		 
//JTAG模式设置,用于设置JTAG的模式
//mode:jtag,swd模式设置;00,全使能;01,使能SWD;10,全关闭;	   
//#define JTAG_SWD_DISABLE   0X02
//#define SWD_ENABLE         0X01
//#define JTAG_SWD_ENABLE    0X00		  
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟	   
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
} 
//系统时钟初始化函数
//pll:选择的倍频数，从2开始，最大值为16		 
void Stm32_Clock_Init(u8 PLL)
{
	unsigned char temp=0;   
	MYRCC_DeInit();		  //复位并配置向量表
	RCC->CR|=0x00010000;  //外部高速时钟使能HSEON
	while(!(RCC->CR>>17));//等待外部时钟就绪
	RCC->CFGR=0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	PLL-=2;				  //抵消2个单位（因为是从2开始的，设置0就是2）
	RCC->CFGR|=PLL<<18;   //设置PLL值 2~16
	RCC->CFGR|=1<<16;	  //PLLSRC ON 
	FLASH->ACR|=0x32;	  //FLASH 2个延时周期
	RCC->CR|=0x01000000;  //PLLON
	while(!(RCC->CR>>25));//等待PLL锁定
	RCC->CFGR|=0x00000002;//PLL作为系统时钟	 
	while(temp!=0x02)     //等待PLL作为系统时钟设置成功
	{   
		temp=RCC->CFGR>>2;
		temp&=0x03;
	}    
}

int flash_erase(void *pf) //擦FLASH扇区,返回本扇区大小
{
	//解锁：
	FLASH->KEYR=0x45670123;
	FLASH->KEYR=0xCDEF89AB;
	while(FLASH->SR & 1); //bsy
	FLASH->CR |= (1<<1); //PER
	FLASH->AR=(u32)pf;
	FLASH->CR |= (1<<6); //START
	while(FLASH->SR & 1); //bsy
	FLASH->CR &= ~(1<<1); //PER
	//上锁
	FLASH->CR |= (1<<7); //LOCK 
	return FLASH_PAGE; //
}
int flash_write(u8 *p,int n,void *addr) //直接写入(按2字节)
{
	int i,t=(n+1)/2; //t为写操作的次数
	//解锁：
	FLASH->KEYR=0x45670123;
	FLASH->KEYR=0xCDEF89AB;

	while(FLASH->SR & 1); //bsy
	FLASH->CR |= 1; //PG
	for(i = 0; i < t; i++)
	{
		((u16*)addr)[i]=((u16*)p)[i];
		while(FLASH->SR & 1); //bsy
	}
	FLASH->CR &= ~1; //PG
	//上锁
	FLASH->CR |= (1<<7); //LOCK 
	return 0;
}
int flash_eraseAwrite(u8 *p,int n,void *pf) //擦除并写入,输入数据指针，数据长度(单位字节)，FLASH地址
{
	int i;
	flash_erase(pf); //擦除
	flash_write(p,n,pf); //写入
	for(i = 0; i < n; i++) //校验
	{
		if(p[i]!=((u8*)pf)[i]) return 1;
	}
	return 0;
}
