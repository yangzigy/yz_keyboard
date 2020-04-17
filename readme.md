yz_keyboard
========

diy机械键盘，类似HHKB的布局，修改fn的位置，增加功能组合键定义，并加入鼠标功能。可实现动态led图案。  
![image](https://github.com/yangzigy/yz_keyboard/raw/master/pictures/IMG_20170524_213813_HHT.jpg)  
说明地址：https://www.cnblogs.com/yangzifb/p/11485313.html#4425981
**设计思想：**  
1. 优先使用大拇指完成任务，实现组合键  
2. 减轻小拇指负担，将Ctrl移动到caplock处，取消caplock  
3. 方向键使用vim风格，home end pageup pagedown也使用hjkl实现  
4. 使用键盘模拟鼠标，用于连贯动作，减少键鼠切换，主要鼠标操作仍通过实体鼠标实现  
5. 缩小键盘尺寸，降低空间占用，减少手的移动范围  
6. 使用组合键并通过按键时长实现光标鼠标移动以及滚屏的连续控制，所以需要无段落感的轴  
7. 使用力量小的轴体以减缓手部疲劳。所以主要使用红轴  
8. 使用消音胶垫，进一步让感觉变软、变轻，降低噪音  
**实现参考：** http://tieba.baidu.com/p/4592747695  
后期可以考虑加入的功能：  
  1. 录制宏  
  2. 通过统计按键风格做一些智能动作,例如操作者识别，操作者工作状态、工作类别统计  
使用stm32f103vbt6，驱动键盘扫描、led和USB通信，没有使用固件库。  

## 设计  
### 按键设计  
仿HHKB的设计，使用59键，通过fn键复用按键。将fn位置放在左手大拇指处  
app键也作为fn使用，方便单手操作。只有APP键单独按下或无有效组合时用作APP键。  
复用按键：(失去所有复用按键的组合功能)  
1. F1~F12: 通过第一排按键实现  
2. del: 通过backspace和~实现,其中fn+~为shift+del  
3. 方向键： 通过vim风格的hjkl键实现  
4. home、end、pgup、pgdn：fn+a+hjkl(失去a+上下左右功能)  
5. home、end：fn+< > 方便单手操作  
6. insert、prtscren：fn+i,fn+p  
7. 上一级（alt+up）: fn+u  
8. 鼠标上下左右滚轮：fn+s+hjklnm(失去s+上下左右功能)  
9. 鼠标大幅上下左右：fn+a+s+hjklnm  
10. 鼠标按键：fn+dfgnm：左键中键右键,滚轮  
11. 鼠标拖动：fn+dfg+hjklnm  
12. 鼠标大幅拖动：fn+a+dfg+hjklnm  
### 布局  
使用键位编辑网站Keyboard-layout-editor编辑键盘的布局：http://www.keyboard-layout-editor.com/  
使用定位板生成网站http://builder.swillkb.com/  生成dxf  
将生成的文件导入Altium designer后，作为机械层辅助布局  

## 硬件实现  
### 电路部分  
为了实现全键任意组合，单片机的每一个引脚连接一个按键，单独进行扫描。  
通过传感引脚控制led的亮度，led设在按键上拉电阻后，成为上拉回路中的一部分。按键按下时，led最大亮度。按键松开时，单片机控制引脚为输出模式，低电平亮灯；设置为输入上拉模式时，灯灭。通过占空比实现亮度调节。  
扫描和灯的控制共用0.5ms的时隙，10ms为一组，每个时间组首先扫描按键，扫描完成后按照灯的亮度设定设置引脚是否需要输出。10ms后重新设置为输入模式扫描按键。所以按键扫描频率为100Hz，led的PWM频率为100Hz，占空比为20档。  
USB需申请500mA电流，使用100mA时，键盘容易掉。  
### 焊接  
led使用压降2~3V的高亮led，限流电阻1K，由MCU引脚直接驱动，驱动电流约1ma，全亮时MCU电流在可承载范围内，且1ma的驱动电流已经足够亮了。  
usb预留了miniUSB接口，但实际焊接时，由于线长吃力，容易掉。所以改成通过焊接USB线，或焊接miniUSB转接板的方式出线。  
### 键帽/外壳  
外壳使用GH60塑料外壳；  
卫星轴使用PCB卫星轴，3个X2的；  
键帽为一套标准键帽，其中右shift键安装在空格位置（可购买无刻2.75X R1），ctrl键购买无刻1.75X R2，反斜杠键购买无刻1X R4，退格键购买无刻1.5X R3  
## 程序结构  
### 文件组织  
<pre>
|~stm32f1/			单片机程序
| |~app/				应用代码
| | |-key.c					按键扫描和定义文件
| | |-key.h
| | |-led.c					led与图形定义
| | `-led.h
| |+DebugConfig/
| |+obj/
| |~src/				单片机基础源文件
| | |-cmd.c
| | |-cmd.h
| | |-main.c
| | |-main.h
| | |-queue.c
| | |-queue.h
| | |-uart.c
| | `-uart.h
| |~sys/				单片机系统源文件
| | |-core_cm3.h
| | |-core_cmFunc.h
| | |-core_cmInstr.h
| | |-startup_stm32f10x_hd.s
| | |-stm32f10x.h [RO]
| | |-stm32f1_sys.c
| | |-stm32f1_sys.h
| | `-system_stm32f10x.h [RO]
| |+usbcfg/				USB描述符、处理程序等
| |+usblib/				USB库
| |-JLinkSettings.ini
| |-keilkill.bat*
| |-test.uvoptx
| |-test.uvprojx
| `-test_Target 1.dep
|+参考/				本项目的参考资料
|+pictures/			本项目过程照片
|+电路/				电路设计
|+电路库/			电路库文件
|-.gitignore
|-keyboard-layout.json	键盘的布局设计
|-readme.md			说明文件
|-效果.JPG			设计效果图
`-生成的CAD图.dxf	生成的键盘布局图，用于PCB布局
</pre>  

### MCU系统部分  
不使用stm32固件库，而是将MCU相关的宏定义、必须的函数集中在sys/stm32f1_sys.c .h文件对中，引用stm32f10x.h、core_cm3.h等少数文件作为单片机系统配置。  
在stm32f1_sys模块中定义MGPIO_TypeDef，代替原来固件库中的GPIO定义。在MGPIO_TypeDef中使用位段的方式定义控制寄存器，代替原来的通过函数配置GPIO。  
使用事件驱动方式实现并发，定义SYS_task变量用于存储系统中待执行的任务，其中每一bit代表一个任务需要执行。定义了START_TASK宏，用于启动一个任务（将代表此任务的bit置一）  
程序入口为main.c文件，main函数中首先配置系统时钟、周期中断、调试串口、调试指令模块，然后初始化USB和键盘模块，最后进入主循环。  
系统周期中断频率为2kHz，调试串口256000bps，系统主频72MHz。  
系统中定义了SYS_time变量作为系统时钟计数器，可以通过对此变量的求余，实现分频。  
在主循环中，不断扫描是否有任务需要执行，若有任务，则首先清除任务标志位，然后再执行任务。若先执行任务再清标志位，会导致在任务执行过程中产生的任务无法执行。  
在主循环中通过SYS_idle变量统计CPU占用，原理是每隔100ms就在定时中断中更新SYS_idle_dis变量，并清零SYS_idle变量。也就是说，SYS_idle在100ms内被主循环一直累加，100ms后被周期读取，得到的计数值是主循环的循环次数。在0负荷状态下，100ms内主循环运行约162000次，而系统带负载时，用于轮询主循环的时间线性降低，以比例计算出CPU占用率。所以在主循环中写了for(i=0;i<1620;i++)，让SYS_idle变量的值在100ms时大约为100，则带负载时输出SYS_idle变量的值大约就是CPU空闲百分比。  

### 串口调试指令  
使用USart1作为调试串口，与SWD调试线使用同一接口引出。用此串口实现一个简易的命令行（shell），实现代码为src/cmd.c .h文件对。cmd模块中，接收到的串口数据判断是否为回车换行，若是，则将缓冲中的文本作为指令处理，若不是，则给串口回显，并压入缓冲备用。这样，就可以通过超级终端等串口调试助手对单片机进行命令行控制。  
cmd模块的命令通过cmd_reg((CMD_LIST*)&bl_sys_cmd_list);函数实现注册，定义时将命令描述放在bl_sys_cmd_list数组中：  
(s8 *)"gk",(s8 *)"display keybuf",cmdfun_getkeybuf,  
第一列为命令名称，第二列为命令的帮助信息，第三列为命令的执行函数。命令接受参数，以空格分割。命令执行函数的定义：  
void cmdfun_ledtest(s8 *para) //设置led亮度  
输入para为参数，通过sscanf函数读取  
串口通过统一的收发函数进行操作，收发均使用中断方式，均使用队列解耦。接收时接收中断和过载中断同时响应。  

### 按键扫描模块  
按键扫描的入口函数为key_poll，由主循环调用，频率2kHz。函数首先区分当前周期是扫描按键的周期还是led控制的周期，分别执行不同的功能。  
为了方便引脚的访问，提高运行效率，定义了一个数组：BUTTON_STATE keys[232]
keys数组是以按键号为索引的数组，存储了按键的GPIO的port、pin，状态、时间和亮度信息。由于键盘的按键号有很多保留，所以数组中有很多0，规定以port为0表示数组中的一个位置为保留。  
定义keys_index数组，用于高效的访问非零的按键数据。  
当进行按键扫描时，首先遍历所有非零按键位，将引脚配置为输入上拉模式。然后调用key_scan函数进行按键扫描，在此函数中调用check_press函数判断按键是否按下，并以一定的间隔时间防抖。key_scan函数在扫描完成后开始判断发送值，首先为特殊按键赋值，然后判断Fn组合键，然后判断正常按键。  
发送指令时，keyboard_send和mouse_send函数判断是否有变化，若无变化且鼠标速度为0，则不再发送。若以查询式一直发送键鼠数据，则在开机BIOS中无法通过键盘选择菜单。  
### led模块  
led模块定义了led数组const u8 ascii_index[256]，此数组作为索引，将按键的ascii值转换为按键扫描模块keys数组的索引（按键码）。  
led模块以led_poll函数为入口，调用具体图案函数执行，在图案函数中，设置图案数组，调用led_set_pattern函数进行led亮度赋值。图案数组为S_LED_LIGHT结构的数组，第一个域为按键的ascii表示，对于特殊按键的表示见ascii_index数组的注释，若为1，则为一个图形的结束。第二个域为亮度。若第一个域为1，则第二个域表示这个图形的持续时间。led_set_pattern函数只返回持续时间，延时由图案函数执行。延时的单位为led_poll的周期，100Hz。  

