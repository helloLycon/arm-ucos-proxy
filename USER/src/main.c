#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
//#include "key.h"
//#include "lwip_comm.h"
#include "LAN8720.h"
//#include "usmart.h"
#include "timer.h"
//#include "lcd.h"
//#include "sram.h"
#include "malloc.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwipopts.h"
#include "tcpip.h"
#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "delay.h"
#include "usart.h"  
#include <stdio.h>
#include "ucos_ii.h" 
#include "lwip/opt.h"
//#include "lwip/sys.h"
#include "lwip/api.h"
#include "ring_buffer.h"
#include "led.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "mainconfig.h"
#include "useriap.h"
#include "uart3_control.h"
#include "snmp_agent.h"
#include "usr_prio.h"
//ALIENTEK 探索者STM32F407开发板
//LWIP LWIP+UCOS操作系统移植实验
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司


struct netif lwip_netif;				//定义一个全局的网络接口

//static int scom3_baudrate;

//LED任务
//任务优先级
//#define LED_TASK_PRIO		9
//任务堆栈大小
#define LED_STK_SIZE		512
//任务堆栈
OS_STK	LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);  


//START任务
//任务优先级
//#define START_TASK_PRIO		10
//任务堆栈大小
#define START_STK_SIZE		64
//任务堆栈
//OS_STK START_TASK_STK[START_STK_SIZE];
OS_STK * START_TASK_STK;
//任务函数
void start_task(void *pdata); 
u8 led0_state=0;
u8 wdi_state=0;

int Main_Port_Wdi_Toggle(void)
{
    wdi_state = !wdi_state;
	Port_Wdi_Toggle(wdi_state);
}

int led0_port_toggle(void)
{
    led0_state = !led0_state;    
    if(led0_state){
    	LED0_ON;
    }else{
    	LED0_OFF;
    }
}



int main(void)
{
	INT8U err;
    u32_t i;
    delay_init(50);       	//延时初始化
    LED_Init();  			//LED初始化
    led0_port_toggle();
	delay_ms(100);
    LED_Init();  			//LED初始化
    Main_Port_Wdi_Toggle();
    led0_port_toggle();
	delay_init(168);       	//延时初始化
	LED_Init();  			//LED初始化
	for(i=0;i<6;i++){
	    led0_port_toggle();
	    Main_Port_Wdi_Toggle();
	    delay_ms(100);	    
        Main_Port_Wdi_Toggle();
    	delay_ms(100);
        Main_Port_Wdi_Toggle();
        led0_port_toggle();
    	delay_ms(100);
        Main_Port_Wdi_Toggle();
    	delay_ms(100);
        Main_Port_Wdi_Toggle();
	}

	
	uart3_init(57600); 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
	
	uart1_init(115200);    	//串口波特率设置
//	usmart_dev.init(84); 	//初始化USMART
	NVIC_DisableIRQ(USART3_IRQn);   //起始禁止串口3中断
    Main_Port_Wdi_Toggle();

//	FSMC_SRAM_Init();		//SRAM初始化	
	mymem_init(SRAMIN);  	//初始化内部内存池
//	mymem_init(SRAMEX);  	//初始化外部内存池
	mymem_init(SRAMCCM); 	//初始化CCM内存池
	SPI_Flash_Init();
	
	
	OSInit(); 					//UCOS初始化
	Main_Port_Wdi_Toggle();
	main_config_init_f();
	Main_Port_Wdi_Toggle();
	while(lwip_comm_init()) 	//lwip初始化
	{
		Main_Port_Wdi_Toggle();
		delay_ms(200);
		printf("lwip_comm_init error\r\n");
	} 
    START_TASK_STK = mymalloc(SRAMCCM,START_STK_SIZE*4);

	/* http daemon init */
	httpd_init();
	/* snmp agent init */
	snmp_agent_init();

	err = OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);

	/* do check */
	if(os_obj_create_check("start_task","main",err)<0){
		for(;;);
	}

	OSStart(); //开启UCOS
	main_config_release_f();
}
//start任务  不能去除，否除os各任务无法启动
void start_task(void *pdata)
{
	INT8U err;
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	Main_Port_Wdi_Toggle();
	OSStatInit();  			//初始化统计任务
	OS_ENTER_CRITICAL();  	//关中断
    scom1_task_create_f();  //创建串口1任务
	user_tcp_create_f(); //创建TCP任务
    scom3_task_create_f();  //创建串口2任务
	ftp_create_f();
	Main_Port_Wdi_Toggle();
	// 2017.9.21
	tftp_client_create_f();  // create tftp client thread
	Main_Port_Wdi_Toggle();
	// 2017.4.15
	telnet_server_create_f();//创建telnet_server任务
	Main_Port_Wdi_Toggle();	
	
	NVIC_EnableIRQ(USART3_IRQn);    //最后再使能串口3中断
	
	err = OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);//创建LED任务
	/* do check */
	os_obj_create_check("led_task","start_task",err);
	
	OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
	OS_EXIT_CRITICAL();  		//开中断
}




//上电后识别是否有在线升级，有则发送升级结果
#define MAX_SEND_UPDATE_RESULT_CNT 3

void control_if_send_update_result(void)
{
    struct tagUpdateConfigS update_cfg;  
    //struct tagGlobalConfigS  global_cfg;
    u32_t i;
    //main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS)); 
    main_config_read_update_f((uint8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));
	//update_cfg.boot_just_update_flag = 1;
    if(update_cfg.boot_just_update_flag == 1){            //是否需要升级
       
		Main_Port_Wdi_Toggle();
        update_cfg.boot_just_update_flag = 0;
        printf("boot_update_result=%x\n", update_cfg.boot_update_result);
        for(i=0;i<MAX_SEND_UPDATE_RESULT_CNT;i++){      
            ftp0_isp_result_send(update_cfg.boot_update_result);    
            OSTimeDly(100);
        }        
        
        main_config_write_update_f((uint8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));
        //main_config_read_update_f((uint8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));
    }
}
//int control_get_som3_baudrate(void)
//{
//    return scom3_baudrate; 
//}

uint32_t led_task_count;
uint32_t spitestid = 0;
uint32_t flag_control_if_send_update_result=0;
//led任务
void led_task(void *pdata)
{
    uint32_t physts;    
    uint32_t spiid;
//	scom3_baudrate = 0xff;
//	led_task_cnt = 0;
	led_task_count = 0;
	//#ifdef STM32XX_BOOT    
    //    control_is_need_update();
    //#endif   
	while(1)
	{	
	    /* Call the PHY status update state machine once in a while
		   to keep the link status up-to-date */
		physts = lpc_phy_sts_sm();		
		
		Main_Port_Wdi_Toggle();
	    
		//spitestid=SPI_Flash_ReadID();	//读取FLASH ID.		
		OSTimeDly(200);
		led_task_count++;
		if(led_task_count%5==0){
		    control_check_send_cmd();
	    }
		led0_port_toggle();
		
		if(flag_control_if_send_update_result==0){
		    if(led_task_count >= 5){
		    
    		    control_if_send_update_result();    
				flag_control_if_send_update_result = 1;
    		}
		}
		//spiid=SPI_Flash_ReadID();	//读取FLASH ID.		
		//OSTimeDly(200);
	    //#ifdef STM32XX_BOOT        //在bootloader阶段，跳转到APP应程序里去执行
        //    iap_load_app(FLASH_APP_ADDR);  		  
		//#endif
 	}
}


int os_obj_create_check(const char * obj,const char * caller,INT8U err){
	if( OS_ERR_NONE != err){
		printf("\r\n*******************COLLAPSE*******************\r\n");
		printf("Failed to create \"%s\" in \"%s\"(errcode=%d)\r\n" ,obj,caller,err );
		return -1;
	}
	printf("CHECK: SUCCEED to create \"%s\" in \"%s\"\r\n" ,obj,caller );
	return 0;
}







