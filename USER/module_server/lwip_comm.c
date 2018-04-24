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
#include "usart1.h"	
#include "uart3_control.h"
#include "tcp_server.h"
#include "usr_prio.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//lwip通用驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
   
  
__lwip_dev lwipdev;						//lwip控制结构体 

extern struct netif lwip_netif;         //在main.c里面定义
extern u32 memp_get_memorysize(void);	//在memp.c里面定义
extern u8_t *memp_memory;				//在memp.c里面定义.
extern u8_t *ram_heap;					//在mem.c里面定义.




/* Transmit and receive ring buffers */
static RINGBUFF_T tx_jump_ring;
/* Transmit and receive ring buffer sizes */
#define TX_JUMP_SIZE 256	/* Send */
/* Transmit and receive buffers */
static uint8_t tx_jump_buff[TX_JUMP_SIZE];

/* Transmit and receive ring buffers */
static RINGBUFF_T tx_trap_ring;
/* Transmit and receive ring buffer sizes */
#define TX_TRAP_SIZE 512	/* Send */
/* Transmit and receive buffers */
static uint8_t tx_trap_buff[TX_TRAP_SIZE];

/* Transmit and receive ring buffers */
static RINGBUFF_T tx_st_ring;
/* Transmit and receive ring buffer sizes */
#define TX_ST_SIZE 256	/* Send */
/* Transmit and receive buffers */
static uint8_t tx_st_buff[TX_ST_SIZE];

/////////////////////////////////////////////////////////////////////////////////
//lwip两个任务定义(内核任务和DHCP任务)

//lwip内核任务堆栈(优先级和堆栈大小在lwipopts.h定义了) 
OS_STK * TCPIP_THREAD_TASK_STK;	 

//lwip DHCP任务
//设置任务优先级
//#define LWIP_DHCP_TASK_PRIO       		7
//设置任务堆栈大小
#define LWIP_DHCP_STK_SIZE  		    512
//任务堆栈，采用内存管理的方式控制申请	
OS_STK * LWIP_DHCP_TASK_STK;	
//任务函数
void lwip_dhcp_task(void *pdata); 





void cmd_set_ipaddr(uint32_t ipaddr)
{
    ip_addr_t temp_ipaddr;
    IP4_ADDR(&temp_ipaddr, (ipaddr>>24)&0xff, (ipaddr>>16)&0xff, (ipaddr>>8)&0xff, ipaddr&0xff);
    netif_set_ipaddr(&lwip_netif, &temp_ipaddr);    
}

void cmd_set_gw(uint32_t gw)
{
    ip_addr_t temp_gw;
    IP4_ADDR(&temp_gw, (gw>>24)&0xff, (gw>>16)&0xff, (gw>>8)&0xff, gw&0xff);
    netif_set_gw(&lwip_netif, &temp_gw);    
}

void cmd_set_netmask(uint32_t netmask)
{
    ip_addr_t temp_netmask;
    IP4_ADDR(&temp_netmask, (netmask>>24)&0xff, (netmask>>16)&0xff, (netmask>>8)&0xff, netmask&0xff);
    netif_set_netmask(&lwip_netif, &temp_netmask);    
}






//用于以太网中断调用
void lwip_pkt_handle(void)
{
	ethernetif_input(&lwip_netif);
}
//lwip内核部分,内存申请
//返回值:0,成功;
//    其他,失败
u8 lwip_comm_mem_malloc(void)
{
	u32 mempsize;
	u32 ramheapsize; 
	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
	printf("mempsize=%x\r\n", mempsize);
	memp_memory=mymalloc(SRAMIN,mempsize);	//为memp_memory申请内存
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//得到ram heap大小
	ram_heap=mymalloc(SRAMIN,ramheapsize);	//为ram_heap申请内存 
	printf("ram_heap=%x\r\n", ramheapsize);
	TCPIP_THREAD_TASK_STK=mymalloc(SRAMCCM,TCPIP_THREAD_STACKSIZE*4);//给内核任务申请堆栈 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMCCM,LWIP_DHCP_STK_SIZE*4);		//给dhcp任务堆栈申请内存空间
	if(!memp_memory||!ram_heap||!TCPIP_THREAD_TASK_STK||!LWIP_DHCP_TASK_STK)//有申请失败的
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip内核部分,内存释放
void lwip_comm_mem_free(void) 
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
	myfree(SRAMCCM,TCPIP_THREAD_TASK_STK);
	myfree(SRAMCCM,LWIP_DHCP_TASK_STK);
}
//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
    uint8_t * pmac;	
	u32 sn0;
	uint32_t temp;
//	//sn0=*(vu32*)(0x1FFF7A10);//??STM32???ID??24???MAC??????
//	//????IP?:192.168.1.100
//	lwipx->remoteip[0]=192;	
//	lwipx->remoteip[1]=168;
//	lwipx->remoteip[2]=4;
//	lwipx->remoteip[3]=11;
//	//MAC????(???????:2.0.0,?????STM32??ID)
//	lwipx->mac[0]=0x2c;//????(IEEE???????ID,OUI)?????:2.0.0
//	lwipx->mac[1]=0x06;
//	lwipx->mac[2]=0x23;
//	lwipx->mac[3]=(sn0>>16)&0XFF;//?????STM32???ID
//	lwipx->mac[4]=(sn0>>8)&0XFFF;;
//	lwipx->mac[5]=sn0&0XFF; 
//	//????IP?:192.168.1.30
//	lwipx->ip[0]=192;	
//	lwipx->ip[1]=168;
//	lwipx->ip[2]=4;
//	lwipx->ip[3]=30;
//	//??????:255.255.255.0
//	lwipx->netmask[0]=255;	
//	lwipx->netmask[1]=255;
//	lwipx->netmask[2]=255;
//	lwipx->netmask[3]=0;

	temp = main_config_get_main_gateway();
    printf("netif_add gw=%x\r\n",temp);
    lwipx->gateway[0]=(temp>>24)&0xff;	
	lwipx->gateway[1]=(temp>>16)&0xff;
	lwipx->gateway[2]=(temp>>8)&0xff;
	lwipx->gateway[3]=temp&0xff;
//	//IP4_ADDR(&lwipx->gateway, (temp>>24)&0xff, (temp>>16)&0xff, (temp>>8)&0xff, temp&0xff);
	
	temp = main_config_get_main_host_ip();
	printf("netif_add host_ip=%x\r\n",temp);
	lwipx->ip[0]=(temp>>24)&0xff;	
	lwipx->ip[1]=(temp>>16)&0xff;
	lwipx->ip[2]=(temp>>8)&0xff;
	lwipx->ip[3]=temp&0xff;
//	//IP4_ADDR(&lwipx->ip, (temp>>24)&0xff, (temp>>16)&0xff, (temp>>8)&0xff, temp&0xff);
	temp = main_config_get_main_host_mask();
	printf("netif_add host_mask=%x\r\n",temp);
	lwipx->netmask[0]=(temp>>24)&0xff;	
	lwipx->netmask[1]=(temp>>16)&0xff;
	lwipx->netmask[2]=(temp>>8)&0xff;
	lwipx->netmask[3]=temp&0xff;
//	//IP4_ADDR(&lwipx->netmask, (temp>>24)&0xff, (temp>>16)&0xff, (temp>>8)&0xff, temp&0xff);
//	
	pmac = (uint8_t * )main_config_get_main_mac();
    printf("netif_add pmac[0]=%x\r\n", pmac[0]);
    printf("netif_add pmac[1]=%x\r\n", pmac[1]);
    printf("netif_add pmac[2]=%x\r\n", pmac[2]);
    printf("netif_add pmac[3]=%x\r\n", pmac[3]);
    printf("netif_add pmac[4]=%x\r\n", pmac[4]);
    printf("netif_add pmac[5]=%x\r\n", pmac[5]);   
    lwipx->mac[0]=pmac[0];//????(IEEE???????ID,OUI)?????:2.0.0
	lwipx->mac[1]=pmac[1];
	lwipx->mac[2]=pmac[2];
	lwipx->mac[3]=pmac[3];//?????STM32???ID
	lwipx->mac[4]=pmac[4];
	lwipx->mac[5]=pmac[5]; 
    
	
}
//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,LAN8720初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init(void)
{
	OS_CPU_SR cpu_sr;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			//ip地址
	struct ip_addr netmask; 			//子网掩码
	struct ip_addr gw;      			//默认网关 
	if(ETH_Mem_Malloc())return 1;		//内存申请失败
	if(lwip_comm_mem_malloc())return 1;	//内存申请失败
	if(LAN8720_Init())return 2;			//初始化LAN8720失败 
	Main_Port_Wdi_Toggle();
	tcpip_init(NULL,NULL);				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	Main_Port_Wdi_Toggle();
	lwip_comm_default_ip_set(&lwipdev);	//设置默认IP等信息
#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else				//使用静态IP
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("网卡en的MAC地址为:................%x:%x:%x:%x:%x:%x\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
	OS_ENTER_CRITICAL();  //进入临界区
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	OS_EXIT_CRITICAL();  //退出临界区
	Main_Port_Wdi_Toggle();
	if(Netif_Init_Flag==NULL)return 3;//网卡添加失败 
	else//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	return 0;//操作OK.
}   


#define MAX_SERV                 5         /* Maximum number of USERTCP services. Don't need too many */
//static LISTEN_S Manager;                          
                                              
static LISTEN_S * hManager = NULL_T;     


                                              
#define WG_TCP_SERVER_PORT 5768
#define PBX_TCP_SERVER_PORT 15768      
#define UDP_ST_RECV_PORT 3760
#define UDP_ST_SEND_PORT 3761
#define DEFAULT_JUMP_PORT               16680
                                              
struct pbx_tcpcb 
{
    struct pbx_tcpcb *next;
    int socket;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    //char nextchar;
};

static struct pbx_tcpcb *pbx_cb_list = 0;

static int do_send_data_to_pbx(struct pbx_tcpcb *p_pbx_tcpcb);
static void close_pbx_tcp(struct pbx_tcpcb *p_pbx_tcpcb);


struct wg_tcpcb 
{
    struct wg_tcpcb *next;
    int socket;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    //char nextchar;
};

static struct wg_tcpcb *wg_cb_list = 0;
static int do_wg_read(struct wg_tcpcb *p_wg_tcpcb);
static void close_wg_tcp(struct wg_tcpcb *p_wg_tcpcb);

//static uint8_t g_wg_buff[512];



static u32_t g_recv_cur_pos[MAX_TCPCB];
//static uint8_t g_recv_buff[MAX_TCPCB][RECV_BUFF_TCPCB_SIZE];


/**************************************************************
 * void close_pbx_tcp(struct pbx_tcpcb *p_pbx_tcpcb)
 *
 * Close the socket and remove this pbx_tcpcb from the list.
 **************************************************************/
static void close_pbx_tcp(struct pbx_tcpcb *p_pbx_tcpcb)
{
    struct pbx_tcpcb *p_search_tcbcb;

        /* Either an error or tcp connection closed on other
         * end. Close here */
        close(p_pbx_tcpcb->socket);
        
        /* Free pbx_tcpcb */
        if (pbx_cb_list == p_pbx_tcpcb)
            pbx_cb_list = p_pbx_tcpcb->next;
        else
            for (p_search_tcbcb = pbx_cb_list; p_search_tcbcb; p_search_tcbcb = p_search_tcbcb->next)
            {
                if (p_search_tcbcb->next == p_pbx_tcpcb)
                {
                    p_search_tcbcb->next = p_pbx_tcpcb->next;
                    break;
                }
            }
        //mem_free(p_pbx_tcpcb);
        myfree(SRAMIN,p_pbx_tcpcb);
}


int Board_UARTX_Rxring_GetCount(void)
{
    return 256;
}


//创建socket与接收缓冲区的对应关系
int listen_create_socket_num(int socket)
{
    int i;
    int nret = ERROR_T;
    uint8_t err = 0;  
    if(socket >= 0){
        OSMutexPend(hManager->hSocket_Mutex,0,&err); 
        for(i=0;i<MAX_TCPCB;i++){
            if(hManager->socket_num[i].inuse == 0){
                hManager->socket_num[i].inuse = 1;
                hManager->socket_num[i].socket = socket;
                nret = OK_T;
                break;
            }            
        }
        OSMutexPost(hManager->hSocket_Mutex); 
    }
    return nret;
}
//去除socket与接收缓冲区的对应关系，当关闭链接时使用
int listen_destroy_socket_num(int socket)
{
    int i;
    int nret = ERROR_T;
    uint8_t err = 0;  
    if(socket >= 0){
        OSMutexPend(hManager->hSocket_Mutex,0,&err); 
        for(i=0;i<MAX_TCPCB;i++){
            if(hManager->socket_num[i].socket == socket){
                hManager->socket_num[i].inuse = 0;
                hManager->socket_num[i].socket = -1;
                nret = OK_T;
                break;
            }            
        }
        OSMutexPost(hManager->hSocket_Mutex); 
    }
    return nret;
}
//根据socket来取得接收缓冲区的指针
int listen_return_socket_num(int socket)
{
    int i;    
    int nret = ERROR_T;
    uint8_t err = 0;  
    if(socket >= 0){
        OSMutexPend(hManager->hSocket_Mutex,0,&err); 
        for(i=0;i<MAX_TCPCB;i++){
            if(hManager->socket_num[i].socket == socket){
                nret = i;
                break;
            }             
        }
        OSMutexPost(hManager->hSocket_Mutex); 
    }
    return nret;
}


/**************************************************************
 * void close_wg_tcp(struct wg_tcpcb *p_wg_tcpcb)
 *
 * Close the socket and remove this wg_tcpcb from the list.
 **************************************************************/
static void close_wg_tcp(struct wg_tcpcb *p_wg_tcpcb)
{
    struct wg_tcpcb *p_search_tcbcb;
    listen_destroy_socket_num(p_wg_tcpcb->socket);    
    work_clear_control_thread_id(p_wg_tcpcb->socket);
    /* Either an error or tcp connection closed on other
     * end. Close here */
    close(p_wg_tcpcb->socket);
    
    /* Free wg_tcpcb */
    if (wg_cb_list == p_wg_tcpcb){
        wg_cb_list = p_wg_tcpcb->next;
    }else
        for (p_search_tcbcb = wg_cb_list; p_search_tcbcb; p_search_tcbcb = p_search_tcbcb->next)
        {
            if (p_search_tcbcb->next == p_wg_tcpcb)
            {
                p_search_tcbcb->next = p_wg_tcpcb->next;
                break;
            }
        }
        //mem_free(p_wg_tcpcb);
    myfree(SRAMIN,p_wg_tcpcb);
    
}





#define WORK_SOCKET_BUFF_SIZE  1024




//需要建立socket与接收缓冲区的对应关系，以处理TCP粘包。UDP不存在粘包情况。
/**************************************************************
 * void do_send_data_to_pbx(struct pbx_tcpcb *p_pbx_tcpcb)
 *
 * Socket definitely is ready for reading. Read a buffer from the socket and
 * discard the data.  If no data is read, then the socket is closed and the
 * pbx_tcpcb is removed from the list and freed.
 **************************************************************/
static int do_wg_read(struct wg_tcpcb *p_wg_tcpcb)
{
    static char buffer[WORK_SOCKET_BUFF_SIZE];
    s32_t datalen, nleft, ncpy;
    s32_t ack_cmd_len,thread_num;
    s32_t cur_pos = 0;
    /* Read some data */
    u32_t recv_tcp_num;
    u8_t * p_g_recv_buff_num;
    
    datalen = read(p_wg_tcpcb->socket, &buffer, 512);
    if (datalen <= 0)
    {
        close_wg_tcp(p_wg_tcpcb);
        return -1;
    }else{

        recv_tcp_num = listen_return_socket_num(p_wg_tcpcb->socket);
        p_g_recv_buff_num = (u8_t *)hManager->g_recv_buff;
        p_g_recv_buff_num += recv_tcp_num*RECV_BUFF_TCPCB_SIZE;
        nleft = WORK_SOCKET_BUFF_SIZE - g_recv_cur_pos[recv_tcp_num];
        if(nleft > 0){
            ncpy = (nleft > datalen)?datalen : nleft;
            memcpy(p_g_recv_buff_num + g_recv_cur_pos[recv_tcp_num], buffer, ncpy);
            g_recv_cur_pos[recv_tcp_num] += ncpy;
        }
        //printf("nleft=%x, g_recv_cur_pos=%x\n", nleft, g_recv_cur_pos);
        ack_cmd_len = work_do_hj_cmd(p_wg_tcpcb->socket, p_g_recv_buff_num, g_recv_cur_pos[recv_tcp_num]);
        
        g_recv_cur_pos[recv_tcp_num] = g_recv_cur_pos[recv_tcp_num] - ack_cmd_len;
        //printf("g_recv_cur_pos=%x,ack_cmd_len=%x\n", g_recv_cur_pos, ack_cmd_len);
        if(g_recv_cur_pos[recv_tcp_num] >= RECV_BUFF_TCPCB_SIZE){     //容错，当超出范围时
            g_recv_cur_pos[recv_tcp_num] = 0;
        }
        //Board_UARTX_Recv_From_Server(buffer, readcount);
    }
    return 0;
}

//升级时ftp服务器地址作为UDP升级结束通知的IP地址
void listen_set_ftpserver_ip(u32_t ip)
{    
    uint32_t jump_ip;
    //ftpserver_ip = ip;
    
    jump_ip = main_config_get_jump_ip();    
    if(jump_ip != ip){
        main_config_set_main_jump_ip(ip);
    }
    return;
}


static int hj80_udpif_stframe(struct hj_dump_frame_head *phead)
{
    int nret = 0;
    control_build_st_frame(phead);
    return nret;
}

static int hj80_udpif_syncalert(struct hj_dump_frame_head *phead)
{
    int nret = 0;
    control_build_trapsync_frame(phead);
    return nret;
}

int clientSocket(u8_t * p_data, u32_t datalen)
{
	int n = 1, nlen;
	int client_socket;
	struct sockaddr_in client;
    int nret;
	client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(client_socket == -1){
	    printf("create client_socket error \n");
		return -1;
	}
//	if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1){
//	    printf("create SO_REUSEADDR error \n");
//		return -1;
//	}
	if(setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n)) == -1){
	    printf("create SO_BROADCAST error \n");
		return -1;
	}
   // bzero(&client, sizeof(client));
//	client.sin_family = AF_INET;
//	client.sin_port = htons(3760);
//	client.sin_addr.s_addr = INADDR_ANY;
//	if(bind(client_socket,(struct sockaddr *)&client, sizeof(struct sockaddr))==-1){
//	    printf("bind error\n");
//		return -1;
//	}
    //bzero(&client, sizeof(struct sockaddr_in));  
    client.sin_family=AF_INET;  
    client.sin_addr.s_addr=INADDR_BROADCAST;  
    client.sin_port=htons(3762);  
    //int nlen=sizeof(addrto);  
    if(connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
        printf("connect error\n");
		return -1;
	}
    nret = send(client_socket, p_data, datalen, 0);
    if(nret < 0){
        printf("client sendto error\n");
    }else{
        //printf("client sendto ok\n");
        debug_puts(DBG_DEBUG,"client sendto ok\r\n");
    }    
    close(client_socket);
	return nret;
}

static int hj80_udpif_broadcast_read_frame(struct hj_dump_frame_head *p_head)
{
    int nret = 0;    
    u8_t pwdata[256], pcrc[256];
    u32_t framelen;
    //u16_t src_deviceid;
    struct tagGlobalConfigS  global_cfg;      
    u8_t  * p_dstdata;
    struct hj_dmp_frame dmp_frame;
    memset(pwdata, 0, 256);     //清0
    //struct dyb_trap_frame_body * p_dyb_trap_frame_body;
    //memcpy(pwdata, p_head, sizeof(struct hj_dyb_dump_frame_head)); //frame head
    //p_head = (struct hj_dyb_dump_frame_head *)pwdata;              //恢复网管与ARM间通信协议
	//hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
	dmp_frame.phead = (struct hj_dmp_frame_head *)pwdata;
	dmp_frame.phead->protocolVer = p_head->protocolVer;
	dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
	dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
	dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
	//src_deviceid = control_get_local_device_id();
	
	//printf("src_deviceid1111=%x\n", src_deviceid);
	//src_deviceid <<= 8;	
	//src_deviceid |= p_head->src_deviceid&0xff;
	//printf("src_deviceid2222=%x\n", src_deviceid);
	dmp_frame.phead->src_deviceid = 0;  
	dmp_frame.phead->boardIndex = p_head->boardIndex;
	dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
	
	dmp_frame.phead->server_seq = 0x01;
	dmp_frame.phead->cmdId = p_head->cmdId;
	dmp_frame.phead->cmdExt = p_head->cmdExt;
	dmp_frame.phead->cmdStatus = 0;
	framelen = sizeof(struct tagGlobalConfigS) + 81;        //总长度
	dmp_frame.phead->cmdLen = framelen;	
    main_config_read_gloabl_f((u8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
        
    p_dstdata = pwdata;
    p_dstdata += sizeof(struct hj_dmp_frame_head);
    memcpy(p_dstdata, &global_cfg, sizeof(struct tagGlobalConfigS));
    //debug_dump(pwdata, sizeof(struct tagGlobalConfigS), "selftest:");
    //p_dstdata += sizeof(struct tagGlobalConfigS);
    //memcpy(p_dstdata, &global_cfg, sizeof(struct tagGlobalConfigS));
    
    framelen = sizeof(struct hj_dmp_frame_head) + framelen;
    //printf("framelen1111=%x\n", framelen);
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
    //printf("framelen2222=%x\n", framelen);
    //debug_dump(pcrc, sizeof(struct tagGlobalConfigS), "selftest:");
    
    //route_add_host(ADD);       
    clientSocket(pcrc, framelen);	
	//route_add_host(DEL);	
    return nret;
}


static status_t udp_if_execute_one_cmd(u8_t *buff, s32_t len)
{
	u8_t tempcmd;
	u16_t tempcmdext;
	u32_t deviceid;
    struct hj_dump_frame_head *phead = (struct hj_dump_frame_head *)buff;
    s32_t nret, cmdlen;    
    //printf("len=%x, cmdlen=%x\n", len, cmdlen);

    tempcmd = phead->cmdId;
    tempcmd >>= 4;
    tempcmdext = phead->cmdId;
    tempcmdext &= 0x0f;
    tempcmdext <<= 8;
    tempcmdext |= phead->cmdExt;
    
    deviceid = phead->dst_deviceid;
    deviceid &= 0xff;    
    if((tempcmd == ALERTSYNC_CMD)&&(tempcmdext == ALERTSYNC_CMDEXT)){  
        if((phead->boardIndex < 8)||(phead->boardIndex == 0xff)){   //远端设备只做8个板位
            hj80_udpif_syncalert(phead);  
        }
    }else if((tempcmd == JUMP_CMD)&&((tempcmdext >= ST_MIN_CMDEXT)&&(tempcmdext <= ST_MAX_CMDEXT))){	//st命令
        hj80_udpif_stframe(phead);         
    }else if((tempcmd == JUMP_CMD)&&((tempcmdext == BROADCAST_READ_CMDEXT))){	//read ip命令
        
        hj80_udpif_broadcast_read_frame(phead);    
    }else if((tempcmd == JUMP_CMD)&&((tempcmdext == BROADCAST_WRITE_CMDEXT))){	//write ip命令
        
        //hj80_udpif_broadcast_write_frame(p_socket_data, phead);       //暂时未实现
    }
    return nret;
}



#if 1

/*************************************************                                                     
  Function:        setsockopt_reuse                                                                        
  Description:     设置端口重用选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t setsockopt_reuse(s32_t sockfd)
{
    s32_t ret;
    s32_t ReuseOn = 1;      
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void_t *) &ReuseOn,sizeof(ReuseOn))) == ERROR_T){        
        return ERROR_T;
    }
    return OK_T;
}

/*************************************************                                                     
  Function:        setsockopt_keepalive                                                                        
  Description:     设置socket keepalive
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄      
                   s32_t min      : 分钟                                                                                                                              
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t setsockopt_keepalive(s32_t sockfd, s32_t min)
{   
    s32_t keepAlive = 1; // 开启keepalive属性
    //s32_t keepIdle = 60*min; // 如该连接在min分钟内没有任何数据往来,则进行探测 

    s32_t keepIdle = 60 *min; 
    s32_t keepInterval = 30; // 探测时发包的时间间隔为30 秒
    s32_t keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
    s32_t nret;
    nret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    //printf("SO_KEEPALIVE nret=%x\n",nret);
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    //printf("TCP_KEEPIDLE nret=%x\n",nret);
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    //printf("TCP_KEEPINTVL nret=%x\n",nret);
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
    //printf("TCP_KEEPCNT nret=%x\n",nret);
    return OK_T;
}

#endif
#define UDPIF_SOCKET_TMP_SIZE  512
static status_t listen_jump_read_Proc(void)
{
    static s8_t buff[UDPIF_SOCKET_TMP_SIZE];
    u32_t fromlen = sizeof(struct sockaddr_in);
    s32_t  datalen; 
	struct sockaddr tmp_sockaddr;
    //datalen = recvfrom(hManager->jump_listenfd, (s8_t *)&buff, UDPIF_SOCKET_TMP_SIZE, 0, (struct sockaddr *)&hManager->jump_addr.addr , (socklen_t *)&fromlen);
    datalen = recvfrom(hManager->jump_listenfd, (s8_t *)&buff, UDPIF_SOCKET_TMP_SIZE, 0, &tmp_sockaddr , (socklen_t *)&fromlen);
    if(datalen > 0){
        udp_if_execute_one_cmd(buff, datalen);
    }       
    return OK_T;    
}

static status_t listen_st_read_Proc(void)
{
    static s8_t buff[UDPIF_SOCKET_TMP_SIZE];
    u32_t fromlen = sizeof(struct sockaddr_in);
    s32_t  datalen; 
	struct sockaddr tmp_sockaddr;
    //datalen = recvfrom(hManager->st_recv_listenfd, (s8_t *)&buff, UDPIF_SOCKET_TMP_SIZE, 0, (struct sockaddr *)&hManager->st_recv_addr.addr , (socklen_t *)&fromlen);
    datalen = recvfrom(hManager->st_recv_listenfd, (s8_t *)&buff, UDPIF_SOCKET_TMP_SIZE, 0, &tmp_sockaddr , (socklen_t *)&fromlen);
    if(datalen > 0){
        udp_if_execute_one_cmd(buff, datalen);
    }       
    return OK_T;    
}

//  产生JUMP数据        
status_t jump_send_buf(u8_t * buf, int len)
{
    s32_t retb;

    if(RingBuffer_InsertMult(&tx_jump_ring, buf, len)==0){
	    printf("trap buffer have no space\n");
	}
    return OK_T;
}

//  产生JUMP数据        
status_t trap_send_buf(u8_t * buf, int len)
{
    s32_t retb;

    if(RingBuffer_InsertMult(&tx_trap_ring, buf, len)==0){
	    printf("trap buffer have no space\n");
	}
    return OK_T;
}

static status_t jump_udp_send(u8_t *p_data, int len)
{
    //if(hManager->jump_addr.used){  
        //debug_dump(p_data, len, "jump_udp_send:");
        if (sendto(hManager->jump_listenfd, p_data, len, 0, (struct sockaddr *)(&hManager->jump_addr.addr), sizeof(struct sockaddr)) < 0) {
            //perror("sendto:");
        } 

    return OK_T;
}


status_t jump_update_udp_addr(void_t)
{
    s32_t jump_ip, jump_port;

    if(main_config_get_jump(&jump_ip, &jump_port)==OK_T){

        if((jump_ip!=UNUSED_JUMP_ADDR1)&&(jump_ip!=UNUSED_JUMP_ADDR2)){
            memset(&hManager->jump_addr.addr, 0, sizeof(hManager->jump_addr.addr));
            hManager->jump_addr.addr.sin_family      = AF_INET;
            hManager->jump_addr.addr.sin_addr.s_addr = htonl(jump_ip);  
            hManager->jump_addr.addr.sin_port = htons(jump_port);       
            hManager->jump_addr.used = 1;
        } else {
            hManager->jump_addr.used = 0;
        }
        //printf("jump::ip=%x, port=%x, used=%x\n", jump_ip, jump_port, hManager->jump_addr.used);
        //debug_printf(DBG_INFO,"jump::ip=%x, port=%d, used=%x\r\n", jump_ip, jump_port, hManager->jump_addr.used);
        debug_printf(DBG_DEBUG,"jump_update_udp_addr: hManager->jump_addr.addr.sin_addr.s_addr=%x, hManager->jump_addr.addr.sin_port=%d\r\n", hManager->jump_addr.addr.sin_addr.s_addr, hManager->jump_addr.addr.sin_port);
    }
    return OK_T;
}

//更新UDP发送IP地址
status_t st_update_udp_send_addr(void_t)
{
    static s32_t st_ip, st_port;

    if(main_config_get_send_st(&st_ip, &st_port)==OK_T){

        if((st_ip!=UNUSED_JUMP_ADDR1)&&(st_ip!=UNUSED_JUMP_ADDR2)){
            hManager->st_send_addr.addr.sin_addr.s_addr = htonl(st_ip);  
            hManager->st_send_addr.addr.sin_port = htons(st_port);       
            hManager->st_send_addr.used = 1;
        } else {
            hManager->st_send_addr.used = 0;
        }
        //printf("st send::ip=%x, port=%x, used=%x\n", st_ip, st_port, hManager->st_send_addr.used);
        debug_printf(DBG_INFO,"st send::ip=%x, port=%d, used=%x\r\n", st_ip, st_port, hManager->st_send_addr.used);
    }
    return OK_T;
}





/*
将心跳包从控制模块的缓冲区读出再发送到相应端口
心跳与告警共用一个端口，所以就共用发送程序，只是每次发送的长度不同。
*/
status_t listen_jump_write_Proc(void)
{
    int len = 0;
    static u8_t send_buf[HJ80_JUMP_SIZE];
    
    len = RingBuffer_GetCount(&tx_jump_ring);
    while(len > 0){
        //printf("listen_jump len=%x\n",len);
        if(len>UDP_JUMP_SENDBUF_SIZE){  
            
            RingBuffer_PopMult(&tx_jump_ring, send_buf, UDP_JUMP_SENDBUF_SIZE);
            
            len  = UDP_JUMP_SENDBUF_SIZE ;
	        debug_printf(DBG_DEBUG,"send jump: hManager->jump_addr.addr.sin_addr.s_addr=%x, hManager->jump_addr.addr.sin_port=%d\r\n", hManager->jump_addr.addr.sin_addr.s_addr, hManager->jump_addr.addr.sin_port);
            jump_udp_send (send_buf, len);
            OSTimeDly(2);
        }else{
            RingBuffer_PopMult(&tx_jump_ring, send_buf, len);
	        debug_printf(DBG_DEBUG,"send jump: hManager->jump_addr.addr.sin_addr.s_addr=%x, hManager->jump_addr.addr.sin_port=%d\r\n", hManager->jump_addr.addr.sin_addr.s_addr, hManager->jump_addr.addr.sin_port);
            jump_udp_send (send_buf, len);          
        }
        len = RingBuffer_GetCount(&tx_jump_ring);
    }
    
    len = RingBuffer_GetCount(&tx_trap_ring);
    while(len > 0){
        //printf("listen_jump len=%x\n",len);
        if(len>UDP_TRAP_SENDBUF_SIZE){  
            
            RingBuffer_PopMult(&tx_trap_ring, send_buf, UDP_TRAP_SENDBUF_SIZE);
            
            len  = UDP_TRAP_SENDBUF_SIZE ;
	        debug_printf(DBG_DEBUG,"send trap: hManager->jump_addr.addr.sin_addr.s_addr=%x, hManager->jump_addr.addr.sin_port=%d\r\n", hManager->jump_addr.addr.sin_addr.s_addr, hManager->jump_addr.addr.sin_port);
            jump_udp_send (send_buf, len);
            OSTimeDly(2);
        }else{
            RingBuffer_PopMult(&tx_trap_ring, send_buf, len);
	        debug_printf(DBG_DEBUG,"send trap: hManager->jump_addr.addr.sin_addr.s_addr=%x, hManager->jump_addr.addr.sin_port=%d\r\n", hManager->jump_addr.addr.sin_addr.s_addr, hManager->jump_addr.addr.sin_port);
            jump_udp_send (send_buf, len);          
        }
        len = RingBuffer_GetCount(&tx_trap_ring);
    }    
    return OK_T;
}    

//  产生JUMP数据        
status_t st_send_buf(u8_t * buf, int len)
{
    s32_t retb;

    if(RingBuffer_InsertMult(&tx_st_ring, buf, len)==0){
	    printf("trap buffer have no space\n");
	}
    return OK_T;
}
static status_t st_udp_send(u8_t *p_data, int len)
{
    if(hManager->st_send_addr.used){   
        //printf("jump::%x used\n");
        if (sendto(hManager->st_send_listenfd, p_data, len, 0, (struct sockaddr *)(&hManager->st_send_addr.addr), sizeof(struct sockaddr)) < 0) {
            //perror("sendto:");
        } 
    }
    
    return OK_T;
}

status_t listen_st_write_Proc(void)
{
    int len = 0;
    static u8_t send_buf[HJ80_JUMP_SIZE];
    
    len = RingBuffer_GetCount(&tx_st_ring);
    while(len > 0){
        //printf("listen_st write len=%x\n",len);
        if(len>UDP_ST_SENDBUF_SIZE){  
            
            RingBuffer_PopMult(&tx_st_ring, send_buf, UDP_ST_SENDBUF_SIZE);
            
            len  = UDP_ST_SENDBUF_SIZE ;
            st_udp_send (send_buf, len);
            OSTimeDly(2);
        }else{
            RingBuffer_PopMult(&tx_st_ring, send_buf, len);
            st_udp_send (send_buf, len);          
        }
        len = RingBuffer_GetCount(&tx_st_ring);
    }
    return OK_T;
}    






//u8 led1_state=0;



///** RTP stream multicast address as IPv4 address in "u32_t" format */
//#ifndef RTP_STREAM_ADDRESS
//#define RTP_STREAM_ADDRESS          inet_addr("192.168.4.55")
//#endif
//
///** RTP packet/payload size */
//#define RTP_PACKET_SIZE             1500
//#define RTP_PAYLOAD_SIZE            1024

/** RTP packets */
//static u8_t rtp_send_packet[RTP_PACKET_SIZE];



#define SERVER_IP "192.168.253.253"
//#define PORT 16680
int tcp_count;
/**************************************************************
 * void USERTCP_thread(void *arg)
 *
 * USERTCP task. This server will wait for connections on well
 * known TCP port number: 19. For every connection, the server will
 * write as much data as possible to the tcp port.
 **************************************************************/
static void user_tcp_thread(void *arg)
{
//    int pbx_listenfd;

    struct sockaddr_in pbx_tcp_saddr;
    fd_set readset;
    fd_set writeset;
    int i, maxfdp1,datalen, num;
    struct pbx_tcpcb *p_pbx_tcpcb;
	//static uint8_t sendbuff[256];
	static WgRxNodeData_S scom2_rx_frame;
	WgRxNodeData_S * p_rx_frame = &scom2_rx_frame;
	struct timeval  tv = {0, 5000};    //5ms

    int wg_listenfd;
    struct sockaddr_in wg_tcp_saddr;
    struct wg_tcpcb *p_wg_tcpcb;
    int sock;
    struct sockaddr cliaddr;
    socklen_t clilen;    
    int nret;
    //int                hManager->jump_listenfd;
    struct sockaddr_in local;
//    struct sockaddr_in to;    
    //u32_t              rtp_stream_address;
        
    /* initialize RTP stream address */
    //rtp_stream_address = RTP_STREAM_ADDRESS;
        
    /* First acquire our socket for listening for connections */
//    pbx_listenfd = socket(AF_INET, SOCK_STREAM, 0);
//
//    LWIP_ASSERT("USERTCP_thread(): Socket create failed.", pbx_listenfd >= 0);
//    memset(&pbx_tcp_saddr, 0, sizeof(pbx_tcp_saddr));
//    pbx_tcp_saddr.sin_family = AF_INET;
//    pbx_tcp_saddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
//    pbx_tcp_saddr.sin_port = htons(PBX_TCP_SERVER_PORT);     /* USERTCP server port */
//    setsockopt_reuse(pbx_listenfd);
//    if (bind(pbx_listenfd, (struct sockaddr *) &pbx_tcp_saddr, sizeof(pbx_tcp_saddr)) == -1)
//        LWIP_ASSERT("USERTCP_thread(): Socket bind failed.", 0);
//
//    /* Put socket into listening mode */
//    if (listen(pbx_listenfd, MAX_SERV) == -1)
//        LWIP_ASSERT("USERTCP_thread(): Listen failed.", 0);
//    
        
    /* First acquire our socket for listening for connections */
    wg_listenfd = socket(AF_INET, SOCK_STREAM, 0);

    LWIP_ASSERT("USERTCP_thread(): Socket create failed.", wg_listenfd >= 0);
    memset(&wg_tcp_saddr, 0, sizeof(wg_tcp_saddr));
    wg_tcp_saddr.sin_family = AF_INET;
    wg_tcp_saddr.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
    wg_tcp_saddr.sin_port = htons(WG_TCP_SERVER_PORT);     /* USERTCP server port */
    setsockopt_reuse(wg_listenfd);
    if (bind(wg_listenfd, (struct sockaddr *) &wg_tcp_saddr, sizeof(wg_tcp_saddr)) == -1)
        LWIP_ASSERT("USERTCP_thread(): Socket bind failed.", 0);

    /* Put socket into listening mode */
    if (listen(wg_listenfd, MAX_SERV) == -1)
        LWIP_ASSERT("USERTCP_thread(): Listen failed.", 0);     
                
    hManager->jump_listenfd = socket(AF_INET, SOCK_DGRAM, 0);    
    if (hManager->jump_listenfd >= 0) {
        printf("jump_listenfd is %d\r\n", hManager->jump_listenfd);
      /* prepare local address */
        memset(&local, 0, sizeof(local));
        local.sin_family      = AF_INET;
        local.sin_port        = PP_HTONS(INADDR_ANY);
        local.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
        setsockopt_reuse(hManager->jump_listenfd);
        bind(hManager->jump_listenfd, (struct sockaddr *)&local, sizeof(local));
                
        hManager->jump_addr.addr.sin_family = PF_INET;
        hManager->jump_addr.addr.sin_port = htons(PORT);  
        hManager->jump_addr.addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
        hManager->jump_addr.addr_len = sizeof(hManager->jump_addr.addr);        
        jump_update_udp_addr();
    }else{
        printf("jump_listenfd is error\n");
    }    
    hManager->st_recv_listenfd = socket(AF_INET, SOCK_DGRAM, 0);    
    if (hManager->st_recv_listenfd >= 0) {
        printf("st_recv_listenfd is %d\r\n", hManager->st_recv_listenfd);
      /* prepare local address */
        memset(&local, 0, sizeof(local));
        local.sin_family      = AF_INET;
        local.sin_port        = htons(UDP_ST_RECV_PORT);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        setsockopt_reuse(hManager->st_recv_listenfd);
        
        nret = bind(hManager->st_recv_listenfd, (struct sockaddr *)&local, sizeof(struct sockaddr_in));
        if(nret >= 0){
            printf("st_recv_listenfd bind sucess\r\n");
        }else{
            printf("st_recv_listenfd bind error\r\n");
        }
    }else{
        printf("st_recv_listenfd is error\r\n");
    }  
    
    hManager->st_send_listenfd = socket(AF_INET, SOCK_DGRAM, 0);    
    if (hManager->st_send_listenfd >= 0) {
        printf("st_send_listenfd is %d\r\n", hManager->st_send_listenfd);
      /* prepare local address */
        memset(&local, 0, sizeof(local));
        local.sin_family      = AF_INET;
        local.sin_port        = PP_HTONS(INADDR_ANY);
        local.sin_addr.s_addr = PP_HTONL(INADDR_ANY);
        //setsockopt_reuse(hManager->st_send_listenfd);
        //bind(hManager->st_recv_listenfd, (struct sockaddr *)&local, sizeof(local));
        
        hManager->st_send_addr.addr.sin_family = PF_INET;
        hManager->st_send_addr.addr.sin_port = htons(UDP_ST_SEND_PORT);  
        hManager->st_send_addr.addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
        hManager->st_send_addr.addr_len = sizeof(hManager->st_send_addr.addr);        
        st_update_udp_send_addr();
    }else{
        printf("st_send_listenfd is error\r\n");
    }  
    
    /* Wait forever for network input: This could be connections or data */
    for (;;)
    {
        maxfdp1 = wg_listenfd+1;
        //if (maxfdp1 < wg_listenfd + 1)
        //    maxfdp1 = wg_listenfd + 1;
        /* Determine what sockets need to be in readset */
        FD_ZERO(&readset);
        FD_ZERO(&writeset);

        if((RingBuffer_GetCount(&tx_jump_ring) > 0)||(RingBuffer_GetCount(&tx_trap_ring) > 0)){
            FD_SET(hManager->jump_listenfd, &writeset);
        }
        if((RingBuffer_GetCount(&tx_st_ring) > 0)){
            FD_SET(hManager->st_send_listenfd, &writeset);
        }
        FD_SET(wg_listenfd, &readset);
        FD_SET(hManager->jump_listenfd, &readset);
        FD_SET(hManager->st_recv_listenfd, &readset);
				tcp_count = 0;
        for (p_wg_tcpcb = wg_cb_list; p_wg_tcpcb; p_wg_tcpcb = p_wg_tcpcb->next)
        {
                if (maxfdp1 < p_wg_tcpcb->socket + 1)
                    maxfdp1 = p_wg_tcpcb->socket + 1;
                FD_SET(p_wg_tcpcb->socket, &readset);
								tcp_count++;
        }
        if (maxfdp1 < hManager->jump_listenfd + 1){
            maxfdp1 = hManager->jump_listenfd + 1;
        }
        if (maxfdp1 < hManager->st_recv_listenfd + 1){
            maxfdp1 = hManager->st_recv_listenfd + 1;
        }
        
        if (maxfdp1 < hManager->st_send_listenfd + 1){
            maxfdp1 = hManager->st_send_listenfd + 1;
        }
        
//        led1_state = !led1_state;
//		if(led1_state){
//			LED1_ON;
//		}else{
//			LED1_OFF;
//		}
		
		uartrecvProc();	
		
        /* Wait for data or a new connection */
        i = select(maxfdp1, &readset, &writeset, 0, &tv);
        
        if (i == 0)
            continue;

        if (FD_ISSET(wg_listenfd, &readset))
        {
            /* We have a new connection request!!! */
            /* Lets create a new control block */
            
            p_wg_tcpcb = (struct wg_tcpcb *)mymalloc(SRAMIN,sizeof(struct wg_tcpcb));	
            if (p_wg_tcpcb)
            {
                p_wg_tcpcb->socket = accept(wg_listenfd,
                                        (struct sockaddr *) &p_wg_tcpcb->cliaddr,
                                        &p_wg_tcpcb->clilen);
                if (p_wg_tcpcb->socket < 0){
                    //mem_free(p_wg_tcpcb);
                    myfree(SRAMIN,p_wg_tcpcb);
                }else{
                    
                    i = listen_create_socket_num(p_wg_tcpcb->socket); //判断缓冲区是否满，如果满不处理，否则就处理               
                    if(i >= 0){
                    
                        /* Keep this tecb in our list */
                        p_wg_tcpcb->next = wg_cb_list;
                        wg_cb_list = p_wg_tcpcb;
                        setsockopt_keepalive(p_wg_tcpcb->socket, 10);  //设置Keep Alive时间
                        num = control_get_free_thread_num();   //建立线程号与socket的对应关系                    
                        work_set_control_thread_num(p_wg_tcpcb->socket,num); 
                    }else{
                        myfree(SRAMIN,p_wg_tcpcb);
        
                        sock = accept(wg_listenfd, &cliaddr, &clilen);
                        if (sock >= 0)
                            close(sock);
                    }
                }
            } else {
                /* No memory to accept connection. Just accept and then close */
                //int sock;
                //struct sockaddr cliaddr;
                //socklen_t clilen;

                sock = accept(wg_listenfd, &cliaddr, &clilen);
                if (sock >= 0)
                    close(sock);
            }
        }
        
        /* Go through list of connected clients and process data */
        for (p_wg_tcpcb = wg_cb_list; p_wg_tcpcb; p_wg_tcpcb = p_wg_tcpcb->next)
        {
            if (FD_ISSET(p_wg_tcpcb->socket, &readset))
            {
                /* This socket is ready for reading. This could be because someone typed
                 * some characters or it could be because the socket is now closed. Try reading
                 * some data to see. */
                if (do_wg_read(p_wg_tcpcb) < 0){
                    continue;
                }
            }
        }
       
        if (FD_ISSET(hManager->jump_listenfd, &writeset))
        { 
            listen_jump_write_Proc();
        }
        if (FD_ISSET(hManager->jump_listenfd, &readset))
        { 
            listen_jump_read_Proc();
        }
        if (FD_ISSET(hManager->st_recv_listenfd, &readset))
        { 
            listen_st_read_Proc();
        }
        
        if (FD_ISSET(hManager->st_send_listenfd, &writeset))
        {
            listen_st_write_Proc();     
        }
        
    }    
}


//创建TCP服务器线程
//返回值:0 TCP服务器创建成功
//		其他 TCP服务器创建失败
void user_tcp_create_f(void)
{
	INT8U res;
	INT8U i;
	OS_CPU_SR cpu_sr;
	uint8_t err = 0;  
	
	//hManager = &Manager;
	hManager = mymalloc(SRAMCCM,sizeof(LISTEN_S));
	
	hManager->hSocket_Mutex = OSMutexCreate(TCP_THREAD_MUTEX_PRIO, &err);
	/* do check */
	os_obj_create_check("hManager->hSocket_Mutex","user_tcp_create_f",err);


    for(i=0;i<MAX_TCPCB;i++){
        hManager->socket_num[i].inuse = 0;
        hManager->socket_num[i].socket = -1;
        hManager->socket_num[i].recvbuff_num = 0;
        g_recv_cur_pos[i] = 0;
    }
    /* Before using the ring buffers, initialize them using the ring
        buffer init function */
	RingBuffer_Init(&tx_jump_ring, tx_jump_buff, 1, TX_JUMP_SIZE);    
    RingBuffer_Init(&tx_trap_ring, tx_trap_buff, 1, TX_TRAP_SIZE);
    RingBuffer_Init(&tx_st_ring, tx_st_buff, 1, TX_ST_SIZE);        
            
        
//    create_socket_udp_data(&(hManager->jump_addr.p_jump_listen_socket));
//    
//
//    sprintf(hManager->jump_addr.p_jump_listen_socket->local_addr, "%s", "*");
//    hManager->jump_addr.p_jump_listen_socket->local_port = DEFAULT_JUMP_PORT;
//    strcpy(hManager->jump_addr.p_jump_listen_socket->remote_addr, SERVER_IP);
//    hManager->jump_addr.p_jump_listen_socket->remote_port = PORT;
//    init_socket_data(hManager->jump_addr.p_jump_listen_socket);
////    setsockopt_reuse(hManager->jump_addr.p_jump_listen_socket->socket_fd);
//
//    hManager->jump_addr.addr.sin_family = PF_INET;
//    hManager->jump_addr.addr.sin_port = htons(PORT);  
//    hManager->jump_addr.addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
//    hManager->jump_addr.addr_len = sizeof(hManager->jump_addr.addr);
//    jump_update_udp_addr(); 
	
	OS_ENTER_CRITICAL();	//关中断
	res = OSTaskCreate(user_tcp_thread,(void*)0,(OS_STK*)&LWIP_DHCP_TASK_STK[LWIP_DHCP_STK_SIZE-1],LWIP_DHCP_TASK_PRIO); //创建TCP服务器线程

	/* do check */
	os_obj_create_check("user_tcp_thread","user_tcp_create_f",res);
	   
	OS_EXIT_CRITICAL();		//开中断	
}




void user_tcp_destroy_f(void)
{
    
}


