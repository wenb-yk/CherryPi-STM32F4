#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "lcd.h"
#include "sram.h"
#include "malloc.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwipopts.h"
#include "udp_demo.h"
#include "GUI.h"
#include "sdio_sdcard.h"    
#include "malloc.h"     
#include "ff.h"  
#include "exfuns.h"   
#include "ws281x.h"
/************************************************
 ALIENTEK 探索者STM32F407网络实验9
 基于NETCONN API的UDP实验(UCOSIII版本)
 
 UCOSIII中以下优先级用户程序不能使用，ALIENTEK
 将这些优先级分配给了UCOSIII的5个系统内部任务
 优先级0：中断服务服务管理任务 OS_IntQTask()
 优先级1：时钟节拍任务 OS_TickTask()
 优先级2：定时任务 OS_TmrTask()
 优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
 优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com  
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//KEY任务
#define KEY_TASK_PRIO 		8
//任务堆栈大小
#define KEY_STK_SIZE		128	
//任务控制块
OS_TCB KeyTaskTCB;
//任务堆栈
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);   

//LED任务
//任务优先级
#define LED_TASK_PRIO		9
//任务堆栈大小
#define LED_STK_SIZE		128
//任务控制块
OS_TCB LedTaskTCB;
//任务堆栈
CPU_STK	LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);  

//在LCD上显示地址信息任务
//任务优先级
#define DISPLAY_TASK_PRIO	10
//任务堆栈大小
#define DISPLAY_STK_SIZE	128
//任务控制块
OS_TCB DisplayTaskTCB;
//任务堆栈
CPU_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];
//任务函数
void display_task(void *pdata);

//START任务
//任务优先级
#define START_TASK_PRIO		11
//任务堆栈大小
#define START_STK_SIZE		128
//任务堆栈
OS_TCB StartTaskTCB;
//任务堆栈
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata); 

//在LCD上显示地址信息
//mode:1 显示DHCP获取到的地址
//	  其他 显示静态地址
void show_address(u8 mode)
{
//	u8 buf[30];
//	if(mode==2)
//	{
//		sprintf((char*)buf,"DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
//		Show_Str(30,170,210,16,16,buf); 
//		sprintf((char*)buf,"DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
//		Show_Str(30,190,210,16,16,buf); 
//		sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
//		Show_Str(30,210,210,16,16,buf); 
//		Show_Str(30,230,210,16,16,"Port:8089!"); 
//	}
//	else 
//	{
//		sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
//		Show_Str(30,170,210,16,16,buf); 
//		sprintf((char*)buf,"Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
//		Show_Str(30,190,210,16,16,buf); 
//		sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
//		Show_Str(30,210,210,16,16,buf); 
//		Show_Str(30,230,210,16,16,"Port:8089!"); 
//	}	
}



int main(void)
{
	uint8_t wheelPos;
    u32 total,free;
//	OS_ERR err;
//	CPU_SR_ALLOC();
	
	delay_init(168);       	//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
//	uart_init(115200);    	//串口波特率设置
//    rs485_init(115200);    	//串口波特率设置
//	usmart_dev.init(84); 	//初始化USMART
//	LED_Init();  			//LED初始化
//	LCD_Init();  			//LCD初始化
	WS281x_Init();
	while(1)
	{
//		WS281x_ColorWipe(0x00FF00, 50);
//		WS281x_Rainbow(30);
        WS281x_RainbowCycle(5);
//		WS281x_TheaterChase(0x00FF00, 50);
//		WS281x_TheaterChaseRainbow(50);

//        delay_ms(1000);
	}
#if 0
	
//	FSMC_SRAM_Init();		//SRAM初始化
	
	mymem_init(SRAMIN);  	//初始化内部内存池
//	mymem_init(SRAMEX);  	//初始化外部内存池
//	mymem_init(SRAMCCM); 	//初始化CCM内存池
	
//    Gui_StrCenter(0,90,RED,BLUE,(uint8_t *)"3.95\" ST7796S 320X480",16,1);

    
//    while(SD_Init())//检测不到SD卡
//	{
//		delay_ms(500);
//		LED0=!LED0;//DS0闪烁
//	}
// 	exfuns_init();							//为fatfs相关变量申请内存				 
//  	f_mount(fs[0],"0:",1); 					//挂载SD卡 
//    while(exf_getfree("0",&total,&free))	//得到SD卡的总容量和剩余容量
//	{
//        Gui_StrCenter(0,90,RED,BLUE,"SD Card Fatfs Error!",16,1);
//		delay_ms(200);
//		LED0=!LED0;//DS0闪烁
//	}													  			       
//    Gui_StrCenter(0,100,RED,BLUE,"FATFS OK!",16,1);    	
// 	LCD_ShowNum(0,120,total>>10,5,16);				//显示SD卡总容量 MB
// 	LCD_ShowNum(0,140,free>>10,5,16);					//显示SD卡剩余容量 MB		
    
	
	OSInit(&err); 					//UCOSIII初始化
	while(lwip_comm_init()) 		//lwip初始化
	{
		delay_ms(500); 
	}
	while(udp_demo_init()) 										//初始化udp_demo(创建udp_demo线程)
	{
		delay_ms(500);
	}
//	Show_Str(30,150,200,20,16,"UDP Success!"); 			//udp创建成功
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err); 		//开启UCOS
    #endif
}

//start任务
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif

	OS_CRITICAL_ENTER();	//进入临界区
	//创建LED任务
	OSTaskCreate((OS_TCB 	* )&LedTaskTCB,		
				 (CPU_CHAR	* )"led0 task", 		
                 (OS_TASK_PTR )led_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED_TASK_PRIO,     
                 (CPU_STK   * )&LED_TASK_STK[0],	
                 (CPU_STK_SIZE)LED_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
			 
	//创建显示任务
	OSTaskCreate((OS_TCB 	* )&DisplayTaskTCB,		
				 (CPU_CHAR	* )"display task", 		
                 (OS_TASK_PTR )display_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )DISPLAY_TASK_PRIO,     	
                 (CPU_STK   * )&DISPLAY_TASK_STK[0],	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE/10,	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);				 			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}


//显示地址等信息
void display_task(void *pdata)
{
	OS_ERR err;
	while(1)
	{ 
#if LWIP_DHCP									//当开启DHCP的时候
//		if(lwipdev.dhcpstatus != 0) 			//开启DHCP
//		{
//			show_address(lwipdev.dhcpstatus );	//显示地址信息
//			OS_TaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);		//挂起自身任务	
//		}
#else
//		show_address(0); 						//显示静态地址
//		OSTaskSuspend(NULL,&err); 				//显示完地址信息后挂起自身任务
#endif //LWIP_DHCP


//		ledSingleShow();

		OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_HMSM_STRICT,&err); //延时
	}
}


//led任务
void led_task(void *pdata)
{
	OS_ERR err;
	while(1)
	{
		LED0 = !LED0;
        OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
        LED1 = !LED1;
        OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
        LED2 = !LED2;
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
 	}
}











