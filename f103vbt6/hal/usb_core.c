
#include "usb_core.h"

//////////////////////////////////////////////////////////////////
//			外部提供的接口
//////////////////////////////////////////////////////////////////
extern void usb_ep0_send_prepare(u8 *p,int n); //端点0发送准备，给枚举流程用
	//if(p!=0) UserToPMABufferCopy(p, USB_BT[0].ADDR_TX, n);
	//USB_BT[0].COUNT_TX = n;

extern void usb_get_req_rxbuf(u8 *p,int n); //枚举时，从端点0缓冲区获取下发的请求
	//PMAToUserBufferCopy(p,USB_BT[0].ADDR_RX,n);

extern void SetDeviceAddress(u8 Val); //设置设备地址和端点地址

//////////////////////////////////////////////////////////////////
//			对外提供的接口
//////////////////////////////////////////////////////////////////
DEVICE_STATE usb_stat=USB_UNCONNECTED; //usb设备状态，指示是否连接上了
RESULT (*class_data_setup)(void)=0; //setup的处理回调函数
//数据收发的长度和位置
u16 p0_len=0; //剩余发送数
u16 p0_ind=0; //发送时的分批发送的位置
u8 *p0_p=0; //收发指针

//////////////////////////////////////////////////////////////////
//			基础配置逻辑
//////////////////////////////////////////////////////////////////
int usb_max_packet=64; //最大包长
S_USB_CTRL_STATE usb_ctrl_stat=IN_DATA; //usb控制状态，是初始化流程的状态
//收发部分
S_SETUP0_RX usb_req_rxbuf; //usb端口0的标准请求接收缓存

//由于中断处理的时候需要设置NAK，所以要保存之前的状态
volatile USB_EP_TXRX_STAT usb_rx_stat; //接收控制端点保存之前的状态，可对其进行设置
volatile USB_EP_TXRX_STAT usb_tx_stat; //接收控制端点保存之前的状态，可对其进行设置
static int need_send_void = 0; //大于等于最大包数，且是整数包，需要发送空包

static void p0_send_poll(void) //控制端点的发送数据 
{
	u32 n;
	if((p0_len == 0) && (usb_ctrl_stat == LAST_IN_DATA)) //如果是最后一包
	{
		if(need_send_void) //大于等于最大包数，且是整数包，发送空包
		{
			usb_ep0_send_prepare(0,0);
			usb_tx_stat=USB_EP_VALID; //可以发送 
			need_send_void = 0;
		}
		else //没有数据了
		{
			usb_ctrl_stat = WAIT_STATUS_OUT;
			usb_tx_stat=USB_EP_STALL;
		}
		p0_p=0; //恢复指针
		return ;
	}
	n = usb_max_packet; //先假设发最大包长
	usb_ctrl_stat = (p0_len <= n) ? LAST_IN_DATA : IN_DATA; //是否是最后一包
	if (n > p0_len) n = p0_len; //发送不超过包长

	if(p0_p) //若是测试
	{
		usb_ep0_send_prepare(p0_p,n);
		p0_len -= n;
		p0_p+=n;
	}
	usb_tx_stat = USB_EP_VALID;
	usb_rx_stat = USB_EP_VALID; //USB_StatusOut//期望host退出data IN
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
void Setup0_Process(void) //接收到setup后的data0，放在端点0的接收缓存，处理
{
	RESULT r;
	//将接收缓存中的数据复制到CPU区域的变量中
	usb_get_req_rxbuf((u8*)&usb_req_rxbuf,sizeof(usb_req_rxbuf));
	usb_req_rxbuf.vals.w = CHANGE_END16(usb_req_rxbuf.vals.w);
	usb_req_rxbuf.inds.w = CHANGE_END16(usb_req_rxbuf.inds.w);

	usb_ctrl_stat = SETTING_UP; //接收到setup
	if(usb_req_rxbuf.lens == 0) //长度为0，不发东西
	{
		usb_ep0_send_prepare(0,0);
		usb_tx_stat=USB_EP_VALID;
		usb_ctrl_stat = WAIT_STATUS_IN;
		return ;
	}
	r = class_data_setup(); //调用设备的处理，给回复数据的地址和长度
	if (r == USB_NOT_READY)
	{
		usb_ctrl_stat = PAUSE;
		return ;
	}
	if ((r == USB_UNSUPPORT) || (p0_len == 0)) //不支持的操作
	{
		usb_ctrl_stat = STALLED;
		usb_rx_stat=USB_EP_STALL;
		usb_tx_stat=USB_EP_STALL;
		return ;
	}
	if(usb_req_rxbuf.type.s.dir) //1设备到主机
	{
		if (p0_len > usb_req_rxbuf.lens) //若比主机请求的还长，按主机的
		{
			p0_len = usb_req_rxbuf.lens;
		}
		need_send_void = 0;
		if (p0_len >= usb_max_packet &&
			(p0_len % usb_max_packet) == 0) //大于等于最大包数，且是整数包
		{
			need_send_void = 1;
		}
		p0_send_poll();
	}
	else //0主机到设备
	{
		usb_ctrl_stat = OUT_DATA;
		usb_rx_stat=USB_EP_VALID;
	}
}
void In0_Process(void) //收发状态的接口要用
{
	if ((usb_ctrl_stat == IN_DATA) || (usb_ctrl_stat == LAST_IN_DATA))
	{
		p0_send_poll();
	}
	else if(usb_ctrl_stat == WAIT_STATUS_IN)
	{
		if((usb_req_rxbuf.req == SET_ADDRESS) &&
			(usb_req_rxbuf.type.s.rx_type==0 && usb_req_rxbuf.type.s.req_type==0))
			//标准请求，接收者为设备
		{
			SetDeviceAddress(usb_req_rxbuf.vals.b.b1);
			usb_stat = USB_ATTACHED; //设置地址后，认为已经链接了
		}
		usb_ctrl_stat = STALLED;
	}
	else usb_ctrl_stat = STALLED;
	if (usb_ctrl_stat == STALLED)
	{
		usb_rx_stat=USB_EP_STALL;
		usb_tx_stat=USB_EP_STALL; //STALL
	}
}

