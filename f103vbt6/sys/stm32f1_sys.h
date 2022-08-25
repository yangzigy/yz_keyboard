#ifndef STM32F1_SYS_H
#define STM32F1_SYS_H	  

#include "stm32f10x.h"
#include "main.h"
 
//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define PortSet(p,n,o) p->BSRR=(1<<(n))<<((1-(o))*16) //低16位是1
#define PortGet(p,n) ((p->IDR & (1<<(n))) != 0)
#define PAset(n,o)	PortSet(GPIOA,n,o)
#define PAget(n)	PortGet(GPIOA,n)
#define PBset(n,o)	PortSet(GPIOB,n,o)
#define PBget(n)	PortGet(GPIOB,n)
#define PCset(n,o)	PortSet(GPIOC,n,o)
#define PCget(n)	PortGet(GPIOC,n)
#define PDset(n,o)	PortSet(GPIOD,n,o)
#define PDget(n)	PortGet(GPIOD,n)
#define PEset(n,o)	PortSet(GPIOE,n,o)
#define PEget(n)	PortGet(GPIOE,n)
#define PFset(n,o)	PortSet(GPIOF,n,o)
#define PFget(n)	PortGet(GPIOF,n)
#define PGset(n,o)	PortSet(GPIOG,n,o)
#define PGget(n)	PortGet(GPIOG,n)
/////////////////////////////////////////////////////////////////
//Ex_NVIC_Config专用定义
#define GPIO_A 0
#define GPIO_B 1
#define GPIO_C 2
#define GPIO_D 3
#define GPIO_E 4
#define GPIO_F 5
#define GPIO_G 6 
#define FTIR   1  //下降沿触发
#define RTIR   2  //上升沿触发

//JTAG模式设置定义
#define JTAG_SWD_DISABLE   0X02
#define SWD_ENABLE         0X01
#define JTAG_SWD_ENABLE    0X00	

//RCC
#define RCC_AHBPeriph_DMA1               ((u32)0x00000001)
#define RCC_AHBPeriph_DMA2               ((u32)0x00000002)
#define RCC_AHBPeriph_SRAM               ((u32)0x00000004)
#define RCC_AHBPeriph_FLITF              ((u32)0x00000010)
#define RCC_AHBPeriph_CRC                ((u32)0x00000040)
#define RCC_AHBPeriph_FSMC               ((u32)0x00000100)
#define RCC_AHBPeriph_SDIO               ((u32)0x00000400)

#define RCC_APB2Periph_AFIO              ((u32)0x00000001)
#define RCC_APB2Periph_GPIOA             ((u32)0x00000004)
#define RCC_APB2Periph_GPIOB             ((u32)0x00000008)
#define RCC_APB2Periph_GPIOC             ((u32)0x00000010)
#define RCC_APB2Periph_GPIOD             ((u32)0x00000020)
#define RCC_APB2Periph_GPIOE             ((u32)0x00000040)
#define RCC_APB2Periph_GPIOF             ((u32)0x00000080)
#define RCC_APB2Periph_GPIOG             ((u32)0x00000100)
#define RCC_APB2Periph_ADC1              ((u32)0x00000200)
#define RCC_APB2Periph_ADC2              ((u32)0x00000400)
#define RCC_APB2Periph_TIM1              ((u32)0x00000800)
#define RCC_APB2Periph_SPI1              ((u32)0x00001000)
#define RCC_APB2Periph_TIM8              ((u32)0x00002000)
#define RCC_APB2Periph_USART1            ((u32)0x00004000)
#define RCC_APB2Periph_ADC3              ((u32)0x00008000)
#define RCC_APB2Periph_ALL               ((u32)0x0000FFFD)

#define RCC_APB1Periph_TIM2              ((u32)0x00000001)
#define RCC_APB1Periph_TIM3              ((u32)0x00000002)
#define RCC_APB1Periph_TIM4              ((u32)0x00000004)
#define RCC_APB1Periph_TIM5              ((u32)0x00000008)
#define RCC_APB1Periph_TIM6              ((u32)0x00000010)
#define RCC_APB1Periph_TIM7              ((u32)0x00000020)
#define RCC_APB1Periph_WWDG              ((u32)0x00000800)
#define RCC_APB1Periph_SPI2              ((u32)0x00004000)
#define RCC_APB1Periph_SPI3              ((u32)0x00008000)
#define RCC_APB1Periph_USART2            ((u32)0x00020000)
#define RCC_APB1Periph_USART3            ((u32)0x00040000)
#define RCC_APB1Periph_UART4             ((u32)0x00080000)
#define RCC_APB1Periph_UART5             ((u32)0x00100000)
#define RCC_APB1Periph_I2C1              ((u32)0x00200000)
#define RCC_APB1Periph_I2C2              ((u32)0x00400000)
#define RCC_APB1Periph_USB               ((u32)0x00800000)
#define RCC_APB1Periph_CAN1              ((u32)0x02000000)
#define RCC_APB1Periph_CAN2              ((u32)0x04000000)
#define RCC_APB1Periph_BKP               ((u32)0x08000000)
#define RCC_APB1Periph_PWR               ((u32)0x10000000)
#define RCC_APB1Periph_DAC               ((u32)0x20000000)
#define RCC_APB1Periph_ALL               ((u32)0x3AFEC83F)

typedef struct
{
	u32 CT0	:4;
	u32 CT1	:4;
	u32 CT2	:4;
	u32 CT3	:4;
	u32 CT4	:4;
	u32 CT5	:4;
	u32 CT6	:4;
	u32 CT7	:4;
	u32 CT8	:4;
	u32 CT9	:4;
	u32 CT10	:4;
	u32 CT11	:4;
	u32 CT12	:4;
	u32 CT13	:4;
	u32 CT14	:4;
	u32 CT15	:4;
	u32 IDR;
	u32 ODR;
	u32 BSRR;
	u32 BRR;
	u32 LCKR;
} MGPIO_TypeDef;
#define MGPIOA ((MGPIO_TypeDef*)GPIOA)
#define MGPIOB ((MGPIO_TypeDef*)GPIOB)
#define MGPIOC ((MGPIO_TypeDef*)GPIOC)
#define MGPIOD ((MGPIO_TypeDef*)GPIOD)
#define MGPIOE ((MGPIO_TypeDef*)GPIOE)
#define MGPIOF ((MGPIO_TypeDef*)GPIOF)
#define MGPIOG ((MGPIO_TypeDef*)GPIOG)

#define GPIO_OUT_PP	((0<<2)|3)
#define GPIO_OUT_OD	((1<<2)|3)
#define GPIO_AF_PP	((2<<2)|3)
#define GPIO_AF_OD	((3<<2)|3)
#define GPIO_IN_AN	((0<<2)|0)
#define GPIO_IN_F	((1<<2)|0)
#define GPIO_IN_PP	((2<<2)|0)

//IO：
//寄存器：CRL，CRH，IDR，ODR，BSRR，BRR，LCKR
//CR的结构：4位一个端口，CNF[1:0]MODE[1:0]
//MODE=11:输出
//CNF1=0:IO，1:复用	 CNF0=0:PP，1:OD
//MODE=00:输入
//CNF1=0:不拉，1:上下拉	 CNF0=0:模拟，1:浮空
//上下拉使用ODR控制
#define GPIO_MODE_IN    	0		//普通输入模式
#define GPIO_MODE_OUT		1		//普通输出模式
#define GPIO_MODE_AF		2		//AF功能模式
#define GPIO_MODE_AIN		3		//模拟输入模式

#define  GPIO_SPEED_LOW      (0x00000001U)  //10MHz
#define  GPIO_SPEED_MEDIUM   (0x00000002U)  //2MHz
#define  GPIO_SPEED_HIGH     (0x00000003U)  //50MHz

#define GPIO_PUPD_NONE		0		//不带上下拉
#define GPIO_PUPD_PU		1		//上拉
#define GPIO_PUPD_PD		2		//下拉

#define GPIO_OTYPE_PP		0		//推挽输出
#define GPIO_OTYPE_OD		1		//开漏输出 
//GPIO引脚编号定义
#define PIN0				(1<<0)
#define PIN1				(1<<1)
#define PIN2				(1<<2)
#define PIN3				(1<<3)
#define PIN4				(1<<4)
#define PIN5				(1<<5)
#define PIN6				(1<<6)
#define PIN7				(1<<7)
#define PIN8				(1<<8)
#define PIN9				(1<<9)
#define PIN10				(1<<10)
#define PIN11				(1<<11)
#define PIN12				(1<<12)
#define PIN13				(1<<13)
#define PIN14				(1<<14)
#define PIN15				(1<<15)
void GPIO_Set(GPIO_TypeDef* GPIOx,u32 BITx,u32 MODE,u32 OTYPE,u32 OSPEED,u32 PUPD);//GPIO设置函数  

/////////////////////////////////////////////////////////////////  
void MYRCC_DeInit(void);
void Stm32_Clock_Init(u8 PLL);  //时钟初始化  
void Sys_Soft_Reset(void);      //系统软复位
void Sys_Standby(void);         //待机模式 	
void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset);//设置偏移地址
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);//设置NVIC分组
void MY_NVIC_Init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group);//设置中断
void Ex_NVIC_Config(u8 GPIOx,u8 BITx,u8 TRIM);//外部中断配置函数(只对GPIOA~G)
void JTAG_Set(u8 mode);
void wtd_ini(u8 div); //按默认0，200~600ms
void wtd_tri(void);
//////////////////////////////////////////////////////////////////////////////
//以下为汇编函数
void WFI_SET(void);		//执行WFI指令
#define OS_OPEN_INT		__ASM volatile("cpsie i")//开启所有中断
#define OS_CLOSE_INT	__ASM volatile("cpsid i")//关闭所有中断
void MSR_MSP(u32 addr);	//设置堆栈地址
void switch_2_addr(u32 *pc); //切换到指定的程序指针

//stm32f105rct6是中容量产品，Flash为256k，SRAM为64k
//按照不同容量，存储器组织成32个1K字节/页(小容量)、128个1K字节/页(中容量)、128个2K字节/页(互联型)、256个2K字节/页(大容量)的主存储器块和一个信息块
//起始：0x0800 0000 互联的终止：0x0803 FFFF

//stm32f103c8t6是中容量产品，Flash为64k，SRAM为20k
//按照不同容量，存储器组织成32个1K字节/页(小容量)、128个1K字节/页(中容量)、128个2K字节/页(互联型)、256个2K字节/页(大容量)的主存储器块和一个信息块
//起始：0x0800 0000 中容量的终止：0x0801 FFFF
#ifdef STM32F10X_CL
#define FLASH_PAGE (2048)
#else
#define FLASH_PAGE 1024
#endif
int flash_erase(void *pf); //擦MFLASH扇区,返回本扇区大小
int flash_write(u8 *p,int n,void *addr); //直接写入
int flash_eraseAwrite(u8 *p,int n,void *pf); //擦除并写入,输入数据指针，数据长度，MFLASH地址
//////////////////////////////////////////////////////////////////////////////

//系统变量
extern u32 SYS_task; //任务标志
extern u16 SYS_idle; //空闲计数
extern u16 SYS_idle_dis; //空闲计数的平均
extern u32 SYS_time; //周期计数

#define START_TASK(x)       SYS_task |= x

#define TASK_POOL_100			1	//周期任务

//#define TASK_                 //必须是2的幂次
void wait(vu32 i);
int void_putchar(int c);
extern int (*IO_putchar)(int);	//初始化的函数

#endif

