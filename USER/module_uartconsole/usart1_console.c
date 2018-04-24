#include "sys.h"
#include "usart.h"	
#include "usart1.h"	
#include "malloc.h"
#include "uart3_control.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/udp.h"
#include "types.h"
#include "err.h"
#include "lwip_comm.h"
#include "uart3_control.h"
#include "mainconfig.h"	
#include "ping.h"
#include "usr_prio.h"
#include "webserver_reboot.h"
#include "webserver_control_comm.h"
#include "validation.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4探索者开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/6/10
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持
////////////////////////////////////////////////////////////////////////////////// 	  
 


//LED任务
//任务优先级
//#define USERSCOM1_TASK_PRIORITY		11
//任务堆栈大小
#define USERSCOM1_STK_SIZE		512
//任务堆栈
//OS_STK	USERSCOM1_TASK_STK[USERSCOM1_STK_SIZE];

OS_STK	* USERSCOM1_TASK_STK;
void * shell_reply_buf;

//任务函数
void user_scom1_thread(void *arg);  

extern Scom3TxLink_S * hScom3TxLink;
extern Scom3RxLink_S * hScom3RxLink;

static OS_EVENT * Scom1_TxMutex;
int   cur_debug_level = DEFAULT_DBG_LEVEL;
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif


int PutChar(int ch)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
//u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
//u16 USART_RX_STA=0;       //接收状态标记	

Msg_t R_msg;
u8 ReceiveCommand;
//初始化IO 串口1 
//bound:波特率
void uart1_init(u32 bound){
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(USART1, ENABLE);  //使能串口1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	
#if EN_USART1_RX	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif
	
}

static void input_arrow_handler(u8 charac){
	switch(charac){
		case In_LEFT_ARROW:{
			// clear previous 0x1b5b
			putchar('a');
			if( R_msg.CurrentPos>2 )
				R_msg.CurrentPos -= 2;
			else
				R_msg.CurrentPos = 0;
			break;
		}
		case In_RIGHT_ARROW:{
			// clear previous 0x1b5b
			putchar('a');
			if( R_msg.CurrentPos>2 )
				R_msg.CurrentPos -= 2;
			else
				R_msg.CurrentPos = 0;
			break;
		}
		case In_UP_ARROW:{
			// clear previous 0x1b5b
			putchar('a');
			if( R_msg.CurrentPos>2 )
				R_msg.CurrentPos -= 2;
			else
				R_msg.CurrentPos = 0;
			break;
		}
		case In_DOWN_ARROW:{
			// clear previous 0x1b5b
			putchar('a');
			if( R_msg.CurrentPos>2 )
				R_msg.CurrentPos -= 2;
			else
				R_msg.CurrentPos = 0;
			break;
		}
	}
}

void clr_line_insert(char * p ,const char * ist){
	static const char pfx[]={0x08,0x08,0x08,0x0d,0x00};
	static const char sfx[]={0x1b,0x5b,0x4a,0x00};
	strcpy(p , pfx);
	strcat(p,ist);
	strcat(p,sfx);
}

static int shell_tab_handler(char * recv ,u8 * p_recvlen, const CMD_STRUC * cmd_list ){
	static const char tab_prefix[]={0x08,0x08,0x08,0x0d,0x00};
	static const char tab_suffix[]={0x1b,0x5b,0x4a,0x00};
	int i;
	if( !(*p_recvlen) )
		return ERROR_T;
	for(i=0 ; cmd_list[i].cmd ;++i){
		if( !strncmp(cmd_list[i].cmd , recv , *p_recvlen)){
			int cmdlen = strlen(cmd_list[i].cmd);
			strcpy(recv , cmd_list[i].cmd);
			recv[cmdlen] = ' ';
			*p_recvlen = cmdlen+1;

			/* "clr_line_insert" is supposed to use here */
			fputs(tab_prefix,NULL);
			PutMsg("# %s " , cmd_list[i].cmd);
			fputs(tab_suffix,NULL);
			return OK_T;
		}
	}
	return ERROR_T;
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	static u8 ochar[2] = {0};
	u8 Res;
#if SYSTEM_SUPPORT_UCOS  //使用UCOS操作系统
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
//		if((isalpha(Res))&&(islower(Res))) 		//将大小写不一致的命令更换成一致
//			Res=tolower(Res);
//		putchar(Res);
	
        switch(Res)
        { 
		/* "ENTER" */
        case In_EOL:
            ReceiveCommand=1;
            R_msg.msg[R_msg.CurrentPos]='\0';
            
            PutMsg("\r\n");
            OSSemPost(Scom1_TxMutex);
            break;
        case In_SKIP:  
            R_msg.CurrentPos=0; ReceiveCommand=0;
            PutMsg(Out_SKIP); 
            break;
		case In_TAB:
			shell_tab_handler((char*)R_msg.msg,&R_msg.CurrentPos,CMD_INNER);
			break;
        case In_BKSPE: 
			putchar(Res);
            if(R_msg.CurrentPos>0){
            	R_msg.CurrentPos=R_msg.CurrentPos-1; 
            	PutChar(' ');
            	PutMsg(Out_DEL);
            }else 
                PutChar(' '); 
            break;
		/* we might not get a arrow according to ONE character */
		case In_LEFT_ARROW:
		case In_RIGHT_ARROW:
		case In_UP_ARROW:
		case In_DOWN_ARROW:
			/* ATTENTION: goto default if condition not match */
			if( In_ARROW_PREFIX == *((u16*)ochar) ){
				input_arrow_handler(Res);
				break;
			}
		//something useful ,it does't start from the'"0"address
        default: 
        	putchar(Res);
			R_msg.msg[R_msg.CurrentPos]=Res; 
			R_msg.CurrentPos=R_msg.CurrentPos+1;   
			R_msg.CurrentPos &= 0x3f;
			break;
		}				
	} 
	/* save old characters */
	ochar[0] = ochar[1];
	ochar[1] = Res;
	
 #if SYSTEM_SUPPORT_UCOS  
	OSIntExit();    	//退出中断
#endif
} 
#endif	

static int CMD_HELP(send_t send_method,struct netconn *conn);
static int cmd_reboot(send_t send_method,struct netconn *conn);
static int cmd_info(send_t send_method , struct netconn * conn);
static int cmd_ifconfig(send_t send_method,struct netconn *conn);
static int cmd_hjip(send_t send_method,struct netconn *conn,char argc, char *argv[]);
static int cmd_counter(send_t send_method,struct netconn *conn);
static int cmd_mem_free(send_t send_method,struct netconn *conn);
static int cmd_ping(send_t send_method,struct netconn *conn,int argc,const char ** argv);
static int cmd_debug(send_t send_method,struct netconn *conn,int argc, const char *argv[]);
static int cmd_port(send_t send_method,struct netconn *conn,int argc,const char ** argv);

//static int cmd_tftp_test(void);





//对于没有显式调用的函数在函数体内定义变量时一定要定义成xdata，否则会data区溢出。
/*const CMD_STRUC CMD_INNER[] = { 
	{"?",        "?        ---- show help \r\n", CMD_HELP},	
	{"help",     "help     ---- show help \r\n", CMD_HELP},
	{"reboot",   "reboot   ---- reboot the system\r\n",cmd_reboot},
	{"ifconfig", "ifconfig ---- show ip addr \r\n", cmd_ifconfig},
	{"info"    , "info     ---- show informations of device\r\n" , cmd_info},
	{"free",     "free     ---- memory free\r\n", cmd_mem_free},
	{"debug",    "debug xx ---- set debug level\r\n"\
	             "              0: critical\r\n"\
	             "              1: error\r\n"\
	             "              2: warning\r\n"\
	             "              3: informational\r\n"\
	             "              4: debug\r\n", cmd_debug},
	{"hjip",     "hjip [xx.xx.xx.xx.xx.xx] ---- set ip addr,mac addr,netmask\r\n", cmd_hjip},
	{"ping",     "ping <ip_addr>           ---- ping another host\r\n",cmd_ping},
	{NULL},
};*/
const CMD_STRUC CMD_INNER[] = { 
	{"?",        "?\r\n    show help \r\n", CMD_HELP},	
	{"help",     "help\r\n    show help \r\n", CMD_HELP},
	{"reboot",   "reboot\r\n    reboot the system\r\n",cmd_reboot},
	{"ifconfig", "ifconfig\r\n    show ip addr \r\n", cmd_ifconfig},
	{"info"    , "info\r\n    show informations of device\r\n" , cmd_info},
	{"free",     "free\r\n    memory free\r\n", cmd_mem_free},
	{"debug",    "debug [level]\r\n"
	             "    read/set debug level\r\n"
	             "    0: critical\r\n"
	             "    1: error\r\n"
	             "    2: warning\r\n"
	             "    3: informational\r\n"
	             "    4: debug\r\n", cmd_debug},
	{"hjip",     "hjip [<ip-addr>|<netmask>|<MAC-addr>]\r\n    read/set ip addr,mac addr,netmask\r\n", cmd_hjip},
	{"ping",     "ping <ip-addr>\r\n    ping another host\r\n",cmd_ping},
	{"port",     "port <opt|eth|e1>\r\n    show opt/eth/e1 port status\r\n",cmd_port},
	{NULL},
};

/*
void shell_printf(enum file_desc fd , const char* fmt,...){
	char buf[256];
	va_list ap;
	va_start(ap , fmt);
	vsnprintf(buf,sizeof(buf),fmt,ap);
	if(FD_UART1 == fd){
		PutMsg(buf);
	}
	else if(FD_TELNET == fd){
		
	}
	va_end(ap);
}
*/

static int CMD_HELP(send_t send_method, struct netconn *conn)
{
	int  i,os;
	char buf[512];
	for(i=0 ,os=0; CMD_INNER[i].cmd!=NULL; i++)
	{
		if(CMD_INNER[i].hlp!=NULL)
		{
			if( os > (sizeof(buf)-16) )
				break;
			os += snprintf(buf+os,sizeof(buf)-os,CMD_INNER[i].hlp);
			//send_method(CMD_INNER[i].hlp , conn);
		}
	}
	send_method(buf,conn);
	return 0;
}

static int cmd_reboot(send_t send_method, struct netconn *conn){
	//printf("system will reboot in seconds...\r\n");
	//OSTimeDlyHMSM(0,0,3,0);
	//const char * msg="system will reboot in several seconds...\r\n";
	send_method("system will reboot in several seconds...\r\n",conn);
	
	control_set_sysreset();
	return 0;
}

const char * version_ntoa(u8 ver , char * p){
/*
#define  A_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) ((x)>>5)
#define  B_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) (0x3&((x)>>3))
#define  C_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) (0x7&(x))
*/
	sprintf(p , "%d.%d%d" , ver>>5 , 0x3&(ver>>3) , 0x7&ver);
	return p;
}

static int cmd_info(send_t send_method , struct netconn * conn){
	ip_addr_t tmpip;
	int os = 0;
	char buf[256] , inet_buf[32];
	u32_t host_ip , host_port , mask,gateway , devid,nm_ip,nm_port;
	const u8 * pmac;
	host_ip = main_config_get_main_host_ip();
	host_port = main_config_get_main_host_port();
	mask = main_config_get_main_host_mask();
	gateway = main_config_get_main_gateway();
	pmac = main_config_get_main_mac();
//	devid = spi_control_get_deviceid();
	nm_ip = main_config_get_jump_ip();
	nm_port = main_config_get_jump_port();

	tmpip.addr = htonl(host_ip);
	os += sprintf(buf+os , "host-ip   : %s\r\n" , inet_ntoa_r(tmpip, inet_buf, sizeof(inet_buf)));
	os += sprintf(buf+os , "host-port : %d\r\n" ,host_port);
	tmpip.addr = htonl(mask);
	os += sprintf(buf+os , "mask      : %s\r\n" , inet_ntoa_r(tmpip, inet_buf, sizeof(inet_buf)));
	tmpip.addr = htonl(gateway);
	os += sprintf(buf+os , "gateway   : %s\r\n" , inet_ntoa_r(tmpip, inet_buf, sizeof(inet_buf)));
	os += sprintf(buf+os , "mac       : %02x:%02x:%02x:%02x:%02x:%02x\r\n" ,pmac[0],pmac[1],pmac[2],pmac[3],pmac[4],pmac[5]);
	os += sprintf(buf+os , "version   : %s\r\n" , version_ntoa(ARM_VERSION,inet_buf));
//	os += sprintf(buf+os , "device-id : %d\r\n" , devid);
	tmpip.addr = htonl(nm_ip);
	os += sprintf(buf+os , "nm-ip     : %s\r\n" , inet_ntoa_r(tmpip, inet_buf, sizeof(inet_buf)));
	os += sprintf(buf+os , "nm-port   : %d\r\n" ,nm_port);

	send_method(buf , conn);
	return 0;
}


static int cmd_ifconfig(send_t send_method,struct netconn *conn)
{
	uint32_t ipaddr,netmask;
	uint8_t buf[128];
    uint8_t * pmac;
    /* The first time the function is called after the command has been
		entered just a header string is returned. */        
        ipaddr = main_config_get_main_host_ip();
        //ipaddr = 0xc0a80101;
        //printf("ipaddr=%x\n", ipaddr);
        netmask = main_config_get_main_host_mask();
        pmac = (uint8_t *)main_config_get_main_mac();
        
        
        snprintf( (char*)buf,sizeof(buf), "eth0:\r\n ip address: %d.%d.%d.%d\r\n net mask:   %d.%d.%d.%d\r\n mac addr:   %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
            (ipaddr>>24)&0xff, (ipaddr>>16)&0xff, (ipaddr>>8)&0xff, ipaddr&0xff,
            (netmask>>24)&0xff, (netmask>>16)&0xff, (netmask>>8)&0xff, netmask&0xff,
             pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
        
        send_method((char*)buf,conn);

	return 0;
}


static int cmd_counter(send_t send_method,struct netconn *conn)
{        
	char buf[128];
    snprintf(buf,sizeof(buf),
		"hScom3Tx:tx_wr_index=%x\r\n"
		"hScom3Tx:tx_rd_index=%x\r\n"
		"hScom3Rx:rx_wr_index=%x\r\n"
		"hScom3Rx:rx_rd_index=%x\r\n",
		hScom3TxLink->tx_wr_index,
		hScom3TxLink->tx_rd_index,
		hScom3RxLink->rx_wr_index,
		hScom3RxLink->rx_rd_index); 
	send_method(buf,conn);
//	snprintf(buf,sizeof(buf),"hScom3Tx:tx_rd_index=%x\r\n",hScom3TxLink->tx_rd_index); 
//	send_method(buf,conn);
//    snprintf(buf,sizeof(buf),"hScom3Rx:rx_wr_index=%x\r\n",hScom3RxLink->rx_wr_index); 
//	send_method(buf,conn);
//    snprintf(buf,sizeof(buf),"hScom3Rx:rx_rd_index=%x\r\n",hScom3RxLink->rx_rd_index); 
//	send_method(buf,conn);
	return 0;
}

static int cmd_mem_free(send_t send_method,struct netconn *conn)
{
//	uint32_t  i ;
	char buf[64];
	
//	i = mem_perused(SRAMCCM);
	snprintf(buf,sizeof(buf),
		"SRAMCCM is %d%%\r\n"
		"SRAMIN  is %d%%\r\n"
		"SRAMEX  is %d%%\r\n",
		mem_perused(SRAMCCM),
		mem_perused(SRAMIN),
		mem_perused(SRAMEX));
	/* 用回调函数打印不了百分号 */
	if( uart1_send == send_method ){
		printf("SRAMCCM is %d%%\r\n"
		       "SRAMIN  is %d%%\r\n"
		       "SRAMEX  is %d%%\r\n",
		       mem_perused(SRAMCCM),
		       mem_perused(SRAMIN),
		       mem_perused(SRAMEX));
		return 0;
	}
	send_method(buf,conn);
	
//	i = mem_perused(SRAMIN);
//	snprintf(buf,sizeof(buf),"SRAMIN is %d%%\r\n",i);
//	send_method(buf,conn);
//	
//	i = mem_perused(SRAMEX);
//	snprintf(buf,sizeof(buf),"SRAMEX is %d%%\r\n",i);
//	send_method(buf,conn);
	return 0;
}



static int cmd_hjip(send_t send_method,struct netconn *conn,char argc, char *argv[])
{
	char  i;	
	uint32_t ipaddr,netmask;
	/*uint8_t setmac[6];*/ 
	/* it should be more than 9 cause' sscanf regards it as a "int*" type */
	uint8_t setmac[16];
	char buf[256];    
    uint32_t ip[4];
    uint8_t mac[16];
    uint8_t * pmac = &setmac[0];
    uint32_t host_ip/*,host_mask, gateway*/;
    static struct tagGlobalConfigS  global_cfg;
    if(argc < 1){		//参数错误时返回 2
//    	send_method("Arguments Error!\r\n",conn);
//    	send_method("hjip xx.xx.xx.xx.xx.xx\r\n",conn);
    	send_method("Arguments Error!\r\nhjip xx.xx.xx.xx.xx.xx\r\n",conn);
		return 2;
    }    

    if ((2 == argc) && (4 == sscanf(argv[1], "%d.%d.%d.%d", ip, ip + 1, ip + 2, ip + 3) )) {

        for (i = 0, host_ip = 0; i < 4; ++i)  {
            host_ip <<= 8;
            host_ip += ip[i];
        }
        main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS)); 
//		snprintf(buf,sizeof buf,"host_ip=%x\n",host_ip); 
//		send_method(buf,conn);
		
        if((host_ip>>24)==0xff){    //当为0xff时表示设置子网掩码
//            snprintf(buf,sizeof buf,"mask1111=%x，ip =%x\n",host_ip, global_cfg.host_ip);
//			send_method(buf,conn);
			if(ip_msk_gw_validation(global_cfg.host_ip,host_ip,IPV4_COMB(192,168,1,1))){
				main_config_set_main_host_mask(host_ip); 
	            cmd_set_ipaddr(global_cfg.host_ip);
	            cmd_set_netmask(host_ip);   
			}else
				return 1;
        }else{
//            snprintf(buf,sizeof buf,"host_ip2222=%x\n",host_ip);
//			send_method(buf,conn);
			if(ip_msk_gw_validation(host_ip,global_cfg.netmask,IPV4_COMB(192,168,1,1))){
	            main_config_set_main_host_ip(host_ip);    
	            cmd_set_ipaddr(host_ip);
	            cmd_set_netmask(global_cfg.netmask);   
			}else
				return 1;
        }   
        cmd_set_gw(global_cfg.gateway);
        main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS)); 
        send_method("hjip set ok\r\n",conn);  
    }
	else if((2 == argc) && (6 == sscanf(argv[1], "%x:%x:%x:%x:%x:%x", (int*)setmac,(int*)(setmac+1),(int*)(setmac+2),(int*)(setmac+3),(int*)(setmac+4),(int*)(setmac+5)))){
		if(setmac[0] & 1){
			printf("error: mac address can not be multicast/broadcast\r\n");
			return 1;
		}
        for(i=0;i<6;i++){
            mac[i] = setmac[i];
//            snprintf(buf,sizeof buf,"mac[%x]=%x\r\n",i, mac[i]);
//			send_method(buf,conn);
        }
        main_config_set_main_mac(mac);

        send_method("hjip set ok\r\n",conn);       
    }else{

        ipaddr = main_config_get_main_host_ip();

        netmask = main_config_get_main_host_mask();
        pmac = (uint8_t *)main_config_get_main_mac();
        
        
        snprintf( (char*)buf,sizeof buf, "eth0:\r\n ip address: %d.%d.%d.%d\r\n net mask:   %d.%d.%d.%d\r\n mac addr:   %02x:%02x:%02x:%02x:%02x:%02x\r\n", 
            (ipaddr>>24)&0xff, (ipaddr>>16)&0xff, (ipaddr>>8)&0xff, ipaddr&0xff,
            (netmask>>24)&0xff, (netmask>>16)&0xff, (netmask>>8)&0xff, netmask&0xff,
             pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
        
        send_method((char*)buf,conn);       

    }

	return 0;
}


static int cmd_ping(send_t send_method,struct netconn *conn,int argc,const char ** argv){
	ip_addr_t remote_ip;
	const char * prompt="usage: ping <ip_addr>\r\n";
	if( 2 != argc ){
		send_method(prompt,conn);
		return -1;
	}
	/* 1 on success, 0 on failure */
	if( !inet_aton(argv[1], &remote_ip)){
		send_method(prompt,conn);
		return -1;
	}
	return ping_thread(send_method ,conn, remote_ip);
}

int cmd_port_opt(send_t send_method,struct netconn *conn,char * buf){
	int os = 0 , lineno;
	const struct web_opt * opt = web_control->opt;
	if(web_read_opt()<0){
		send_method("error: failed to read opt ports\r\n",conn);
		return -1;
	}
	for(lineno = 1;lineno<=32;++lineno){
		int board,portno;
		union web_opt_config cfg;
		if(opt_get_msg_according_to_lineno(lineno,&board,&portno)){
			os += sprintf(buf+os,"name:\t\t");
			os += opt_column1_portname(buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nposition:\t");
			os += opt_column2_port_pos(buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nswitch:\t\t");
			os += sprintf(buf+os,opt[board].config[portno].bit.enable?"on":"off");
			os += sprintf(buf+os,"\r\nloop:\t\t");
			cfg = web_control->opt[board].config[portno];
			if(cfg.bit.circ_loop && cfg.bit.dev_loop)
				os += sprintf(buf+os,"double");
			else if(cfg.bit.circ_loop)
				os += sprintf(buf+os,"circuit");
			else if(cfg.bit.dev_loop)
				os += sprintf(buf+os,"device");
			else
				os += sprintf(buf+os,"normal");
			os += sprintf(buf+os,"\r\ntrap-mask:\t");
			os += sprintf(buf+os,opt[board].config[portno].bit.trap_mask?"mask":"no-mask");
			os += sprintf(buf+os,"\r\nLOF:\t\t");
			os += sprintf(buf+os,opt[board].state[portno].bit.lof?"yes":"no");
			os += sprintf(buf+os,"\r\nNOP:\t\t");
			os += sprintf(buf+os,opt[board].state[portno].bit.nop?"yes":"no");
			os += sprintf(buf+os,"\r\nLOOP:\t\t");
			os += sprintf(buf+os,opt[board].state[portno].bit.loop?"yes":"no");
			os += sprintf(buf+os,"\r\n--------------------------\r\n");
		}
	}
	send_method(buf,conn);
	return 0;
}

int cmd_port_eth(send_t send_method,struct netconn *conn,char * buf){
	int os = 0 , lineno;
	const struct web_eth * eth = web_control->eth;
	int board,portno;
	union web_eth_config cfg;
	if(web_read_eth()<0){
		send_method("error: failed to read eth ports\r\n",conn);
		return -1;
	}
	for(lineno = 1;lineno<=32;++lineno){
		if(eth_get_msg_according_to_lineno(1,lineno,&board,&portno)){
			os += sprintf(buf+os,"name:\t\t");
			os += eth_column2_portname(1,buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nposition:\t");
			os += eth_column3_port_pos(1,buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nswitch:\t\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.enable?"on":"off");
			os += sprintf(buf+os,"\r\nlink:\t\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.link?"linked":"unlinked");
			os += sprintf(buf+os,"\r\nmode:\t\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.auto_nego?"auto-nego":"force");
			os += sprintf(buf+os,"\r\nspeed:\t\t");
			cfg = web_control->eth[board].ge[portno];
			switch(cfg.bit.bandwidth){
				case 0:
				default:
					os += sprintf(buf+os,"10M");
					break;
				case 1:
					os += sprintf(buf+os,"100M");
					break;
				case 2:
					os += sprintf(buf+os,"1000M");
					break;
			}
			os += sprintf(buf+os,"\r\nduplex:\t\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.full_duplex?"full-duplex":"half-duplex");
			os += sprintf(buf+os,"\r\nflow-ctrl:\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.flow_ctrl?"on":"off");
			os += sprintf(buf+os,"\r\ntrap-mask:\t");
			os += sprintf(buf+os,eth[board].ge[portno].bit.trap_mask?"mask":"no-mask");
			os += sprintf(buf+os,"\r\n--------------------------\r\n");
		}
	}
	for(lineno = 1;lineno<=32;++lineno){
		if(eth_get_msg_according_to_lineno(0,lineno,&board,&portno)){
			os += sprintf(buf+os,"name:\t\t");
			os += eth_column2_portname(0,buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nposition:\t");
			os += eth_column3_port_pos(0,buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nswitch:\t\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.enable?"on":"off");
			os += sprintf(buf+os,"\r\nlink:\t\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.link?"linked":"unlinked");
			os += sprintf(buf+os,"\r\nmode:\t\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.auto_nego?"auto-nego":"force");
			os += sprintf(buf+os,"\r\nspeed:\t\t");
			cfg = web_control->eth[board].fe[portno];
			switch(cfg.bit.bandwidth){
				case 0:
				default:
					os += sprintf(buf+os,"10M");
					break;
				case 1:
					os += sprintf(buf+os,"100M");
					break;
			}
			os += sprintf(buf+os,"\r\nduplex:\t\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.full_duplex?"full-duplex":"half-duplex");
			os += sprintf(buf+os,"\r\nflow-ctrl:\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.flow_ctrl?"on":"off");
			os += sprintf(buf+os,"\r\ntrap-mask:\t");
			os += sprintf(buf+os,eth[board].fe[portno].bit.trap_mask?"mask":"no-mask");
			os += sprintf(buf+os,"\r\n--------------------------\r\n");
		}
	}
	send_method(buf,conn);
	return 0;
}

int cmd_port_e1(send_t send_method,struct netconn *conn,char * buf){
	int os = 0 , lineno;
	const struct web_e1 * e1 = web_control->e1;
	if(web_read_e1()<0){
		send_method("error: failed to read e1 ports\r\n",conn);
		return -1;
	}
	for(lineno = 1;lineno<=32;++lineno){
		int board,portno;
		union web_e1_config cfg;
		if(e1_get_msg_according_to_lineno(lineno,&board,&portno)){
			os += sprintf(buf+os,"name:\t\t");
			os += e1_column1_portname(buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nposition:\t");
			os += e1_column2_port_pos(buf+os,lineno,board,portno)-1;
			os += sprintf(buf+os,"\r\nswitch:\t\t");
			os += sprintf(buf+os,e1[board].config[portno].bit.enable?"on":"off");
			os += sprintf(buf+os,"\r\nloop:\t\t");
			cfg = web_control->e1[board].config[portno];
			if(cfg.bit.circ_loop && cfg.bit.dev_loop)
				os += sprintf(buf+os,"double");
			else if(cfg.bit.circ_loop)
				os += sprintf(buf+os,"circuit");
			else if(cfg.bit.dev_loop)
				os += sprintf(buf+os,"device");
			else
				os += sprintf(buf+os,"normal");
			os += sprintf(buf+os,"\r\ntrap-mask:\t");
			os += sprintf(buf+os,e1[board].config[portno].bit.trap_mask?"mask":"no-mask");
			os += sprintf(buf+os,"\r\nLOS:\t\t");
			os += sprintf(buf+os,e1[board].state[portno].bit.los?"yes":"no");
			os += sprintf(buf+os,"\r\nAIS:\t\t");
			os += sprintf(buf+os,e1[board].state[portno].bit.ais?"yes":"no");
			os += sprintf(buf+os,"\r\nLOOP:\t\t");
			os += sprintf(buf+os,e1[board].state[portno].bit.loop?"yes":"no");
			os += sprintf(buf+os,"\r\n--------------------------\r\n");
		}
	}
	send_method(buf,conn);
	return 0;
}

static int cmd_port(send_t send_method,struct netconn *conn,int argc,const char ** argv){
	const char * prompt = "usage: port <opt|eth|e1>";
	if(2 != argc){
		send_method(prompt,conn);
		return -1;
	}
	if( strcmp(argv[1],"opt")&&strcmp(argv[1],"eth")&&strcmp(argv[1],"e1") ){
		send_method(prompt,conn);
		return -1;
	}
	switch(argv[1][1]){
		case 'p':
			return cmd_port_opt(send_method,conn,shell_reply_buf);
		case 't':
			return cmd_port_eth(send_method,conn,shell_reply_buf);
		case '1':
			return cmd_port_e1(send_method,conn,shell_reply_buf);
		default:
			return 0;
	}
}

static int usart1_set_debug_level(int debug_level)
{
    cur_debug_level = debug_level;
    return 0;    
}

/*
int usart1_debug(int oper)
{
    int nret = 0;
    if (cur_debug_level & oper) {
        nret = 1;
    }
    return nret;
}
*/

static int cmd_debug(send_t send_method,struct netconn *conn,int argc, const char *argv[])
{
	char buf[32];
    uint32_t debug_level;
    if((1 == argc)){		
        snprintf(buf,sizeof buf,"current debug level %x \r\n", cur_debug_level);
		send_method(buf,conn);
		return 0;
    }    
    if ((2 == argc) && (1 == sscanf(argv[1], "%d", &debug_level) )) {
        usart1_set_debug_level(debug_level);

        snprintf(buf,sizeof buf,"cur_debug_level=%x\r\n", cur_debug_level);        
		send_method(buf,conn);
	} 
	return 0;
}


/************************************************/
void ParseArgs(char *cmdline, int *argc, const char **argv)
{
#define STATE_WHITESPACE	0
#define STATE_WORD			1

	char *c;
	char state = STATE_WHITESPACE;
	int  i;

	*argc = 0;

	if(strlen(cmdline) == 0)
		return;

	/* convert all tailed \r\n into \0 */
	for(i=strlen(cmdline)-1 ; (cmdline[i]=='\r')||(cmdline[i]=='\n') ;--i){
		cmdline[i] = 0;
	}

	/* convert all tabs into single spaces */
	c = cmdline;
	while(*c != '\0')
	{
		if(*c == '\t')
			*c = ' ';
		c++;
	}
	
	c = cmdline;
	i = 0;

	/* now find all words on the command line */
	while(*c != '\0')
	{
		if(state == STATE_WHITESPACE)
		{
			if(*c != ' ')
			{
				argv[i] = c;		//将argv[i]指向c
				i++;
				state = STATE_WORD;
			}
		}
		else
		{ /* state == STATE_WORD */
			if(*c == ' ')
			{
				*c = '\0';
				state = STATE_WHITESPACE;
			}
		}
		c++;
	}

	*argc = i;
#undef STATE_WHITESPACE
#undef STATE_WORD
}



static int GetCmdMatche(const char *cmdline)
{
	int  i;	
	
	for(i=0; CMD_INNER[i].cmd!=NULL; i++)
	{
		if(strncmp(CMD_INNER[i].cmd, cmdline, strlen(CMD_INNER[i].cmd))==0)
			return i;
	}

	return -1;
}




#define CMD_HELP_NUM1 0
#define CMD_HELP_NUM2 1
#define CMD_ADDEOS_NUM 2
#define CMD_ADDE1_NUM 3

#define CMD_REGALL_NUM 42

/* fit telnet server thread */
#define MAX_ARGS   10

//static  uchar testbuf[40];

int ParseCmd(send_t send_method,struct netconn *conn , char * cmdline, u8 cmd_len)
{
	int argc , i;
	int num_commands;
	const char * argv[MAX_ARGS];
	
	//memcpy(testbuf, cmdline, cmd_len);
	
	ParseArgs(cmdline, &argc, argv);
#if 0
	for(i=0;i<argc;++i){
		printf("argv[%d] = \"%s\"\r\n" , i,argv[i]);
	}
#endif
	/* only whitespace */
	if(argc == 0) 
		return -1;
   
	num_commands = GetCmdMatche(argv[0]);
	if((num_commands<0)||((num_commands==0xff)))
		return -1;

	if(CMD_INNER[num_commands].proc!=NULL) {
		/* execute command */
		CMD_INNER[num_commands].proc(send_method,conn,argc, argv);
	}
//	switch(num_commands){
//		
//		case CMD_HELP_NUM1:
//		case CMD_HELP_NUM2:
//			CMD_HELP();
//			break;				
////		case CMD_ADDEOS_NUM:
////			CMD_ADDEOS(argc, argv);
////			break;
////		    
////		default:
////			break;		
//	}
	return 0;			
}


void uart1_send(const char * str ,struct netconn *conn){
	PutMsg(str);
}

int MsgHdl(void)
{
    int tmp;
    if(R_msg.CurrentPos > 0){   //当有数据输入时
        //if(R_msg.CurrentPos > 1){  //去除新行键 'a'
        //	R_msg.CurrentPos -= 1;
        //}
        
        tmp = ParseCmd(uart1_send,NULL,(char*)R_msg.msg, R_msg.CurrentPos);        
		R_msg.CurrentPos=0;
	    if(tmp<0){
			PutMsg("Bad command\r\n");        
		}else if(tmp==2){
			
		}else if(tmp==0){
			//PutMsg("Correct command\r\n");       
		}
    }    
	/* print prompt characters */
	PutMsg("# ");
    return 0; 
}








int scom1_task_create_f(void_t)
{
	INT8U nret;
    int i ;
	Scom1_TxMutex = OSSemCreate(0);
	os_obj_create_check("Scom1_TxMutex(sem in fact)","scom1_task_create_f",(Scom1_TxMutex?OS_ERR_NONE:(OS_ERR_NONE+1)));
	
    for(i=0;i<MsgLen;i++)
    {
        R_msg.msg[i]=0XFF;
        //Temp_msg.msg[i]=0XFF;
    }
    R_msg.CurrentPos=0;
	ReceiveCommand = 0;
	USERSCOM1_TASK_STK = mymalloc(SRAMCCM,USERSCOM1_STK_SIZE*4);

	// buf for shell reply(port command especially)
	shell_reply_buf = mymalloc(SRAMIN , 6*1024);
	
    nret = OSTaskCreate(user_scom1_thread,(void*)0,(OS_STK*)&USERSCOM1_TASK_STK[USERSCOM1_STK_SIZE-1],USERSCOM1_TASK_PRIORITY);//创建LED任务
	/* do check */
	os_obj_create_check("user_scom1_thread","scom1_task_create_f",nret);

	return 0;
}


static void user_scom1_thread( void *pvParameters )
{
    u8 err;
    //signed char cRxedChar;
    //uint8_t ucInputIndex = 0;

	OSTimeDlyHMSM(0,0,3,0);
	printf("\r\n# ");
	for( ;; )
	{	 
		reboot_poll();
		//auth_logged_timeout_poll();
		if(ReceiveCommand)
        {
            MsgHdl(); 
            ReceiveCommand=0;
        }else{
            OSSemPend(Scom1_TxMutex,OS_TICKS_PER_SEC,&err);  //没有数据时等待信号量
		}
			
	}
}
 
void wireshark_format_printf(const u8 * p , int len){
	int i;
	for(i=0 ; i<len ; i+=16){
		int j;
		printf("%04x   " , i);
		for(j=0 ; j<16;++j){
			if((i+j)<len){
				printf("%02x ",p[i+j]);
			}
		}
		printf("\r\n");
	}
}


