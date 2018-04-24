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
//ALIENTEK ̽����STM32F407������
//LWIP LWIP+UCOS����ϵͳ��ֲʵ��
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾


struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

//static int scom3_baudrate;

//LED����
//�������ȼ�
//#define LED_TASK_PRIO		9
//�����ջ��С
#define LED_STK_SIZE		512
//�����ջ
OS_STK	LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);  


//START����
//�������ȼ�
//#define START_TASK_PRIO		10
//�����ջ��С
#define START_STK_SIZE		64
//�����ջ
//OS_STK START_TASK_STK[START_STK_SIZE];
OS_STK * START_TASK_STK;
//������
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
    delay_init(50);       	//��ʱ��ʼ��
    LED_Init();  			//LED��ʼ��
    led0_port_toggle();
	delay_ms(100);
    LED_Init();  			//LED��ʼ��
    Main_Port_Wdi_Toggle();
    led0_port_toggle();
	delay_init(168);       	//��ʱ��ʼ��
	LED_Init();  			//LED��ʼ��
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	
	uart1_init(115200);    	//���ڲ���������
//	usmart_dev.init(84); 	//��ʼ��USMART
	NVIC_DisableIRQ(USART3_IRQn);   //��ʼ��ֹ����3�ж�
    Main_Port_Wdi_Toggle();

//	FSMC_SRAM_Init();		//SRAM��ʼ��	
	mymem_init(SRAMIN);  	//��ʼ���ڲ��ڴ��
//	mymem_init(SRAMEX);  	//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM); 	//��ʼ��CCM�ڴ��
	SPI_Flash_Init();
	
	
	OSInit(); 					//UCOS��ʼ��
	Main_Port_Wdi_Toggle();
	main_config_init_f();
	Main_Port_Wdi_Toggle();
	while(lwip_comm_init()) 	//lwip��ʼ��
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

	OSStart(); //����UCOS
	main_config_release_f();
}
//start����  ����ȥ�������os�������޷�����
void start_task(void *pdata)
{
	INT8U err;
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	Main_Port_Wdi_Toggle();
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
    scom1_task_create_f();  //��������1����
	user_tcp_create_f(); //����TCP����
    scom3_task_create_f();  //��������2����
	ftp_create_f();
	Main_Port_Wdi_Toggle();
	// 2017.9.21
	tftp_client_create_f();  // create tftp client thread
	Main_Port_Wdi_Toggle();
	// 2017.4.15
	telnet_server_create_f();//����telnet_server����
	Main_Port_Wdi_Toggle();	
	
	NVIC_EnableIRQ(USART3_IRQn);    //�����ʹ�ܴ���3�ж�
	
	err = OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);//����LED����
	/* do check */
	os_obj_create_check("led_task","start_task",err);
	
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  		//���ж�
}




//�ϵ��ʶ���Ƿ��������������������������
#define MAX_SEND_UPDATE_RESULT_CNT 3

void control_if_send_update_result(void)
{
    struct tagUpdateConfigS update_cfg;  
    //struct tagGlobalConfigS  global_cfg;
    u32_t i;
    //main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS)); 
    main_config_read_update_f((uint8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));
	//update_cfg.boot_just_update_flag = 1;
    if(update_cfg.boot_just_update_flag == 1){            //�Ƿ���Ҫ����
       
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
//led����
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
	    
		//spitestid=SPI_Flash_ReadID();	//��ȡFLASH ID.		
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
		//spiid=SPI_Flash_ReadID();	//��ȡFLASH ID.		
		//OSTimeDly(200);
	    //#ifdef STM32XX_BOOT        //��bootloader�׶Σ���ת��APPӦ������ȥִ��
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







