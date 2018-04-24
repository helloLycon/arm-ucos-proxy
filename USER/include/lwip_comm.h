#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H 
#include "lan8720.h" 
#include "sockets.h"
#include "hj80_socket.h"
#include "hj_nm.h" 
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
 

#define LWIP_MAX_DHCP_TRIES		4   //DHCP服务器最大重试次数
   

//lwip控制结构体
typedef struct  
{
	u8 mac[6];      //MAC地址
	u8 remoteip[4];	//远端主机IP地址 
	u8 ip[4];       //本机IP地址
	u8 netmask[4]; 	//子网掩码
	u8 gateway[4]; 	//默认网关的IP地址
	
	u8 dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,进入DHCP获取状态
					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体
 
 

#define UDP_TRAP_SENDBUF_SIZE 486	
#define UDP_JUMP_SENDBUF_SIZE 100	//为总的心跳数据包长度*8
#define UDP_ST_SENDBUF_SIZE 256

#define HJ80_JUMP_SIZE 486

#define UNUSED_JUMP_ADDR1 0
#define UNUSED_JUMP_ADDR2 0xffffffff 


struct udp_jump_addr_s{
	
	struct sockaddr_in addr;	
	hj_socket_data_t * p_jump_listen_socket; 
	int addr_len;
	int used;	
} ;

struct udp_st_addr_s{
	
	struct sockaddr_in addr;	
	hj_socket_data_t * p_st_listen_socket; 
	int addr_len;
	int used;	
} ;


#define MAX_TCPCB 10 

struct socket_num_s{
	int socket;
	u16_t recvbuff_num;	
	u16_t inuse;
};

#define RECV_BUFF_TCPCB_SIZE 512

typedef struct tagServerS {
	hj_socket_data_t * p_5768_listen_socket;
	struct udp_jump_addr_s jump_addr;
	struct udp_st_addr_s st_recv_addr;
	struct udp_st_addr_s st_send_addr;
//	XBUFFER_S	* jumpbuf;
	u16_t			listen_port;
	struct socket_num_s socket_num[MAX_TCPCB];
		uint8_t g_recv_buff[MAX_TCPCB][RECV_BUFF_TCPCB_SIZE];
	int jump_listenfd;
	int st_recv_listenfd;
	int st_send_listenfd;
		OS_EVENT * hSocket_Mutex;
	
}LISTEN_S;




#define HJ80_TRAP_SIZE    8192

/* alert status */
#define HJ_ALERT_OK       0
#define HJ_ALERT_ERR      1

/*trap frame body*/
struct trap_frame_body {
    s32_t netunit_ipaddr;
    s16_t netunit_port;
    //s16_t device_index;
    s16_t reserve_word;
    s16_t netunit_status;

    s32_t frame_index;
    s32_t board_index;
    s32_t board_type;
    s32_t board_status;
    s32_t interface_index;
    s32_t interface_type;

    s32_t alarm_serial;
    s32_t alarm_id;
    s32_t alarm_type;
    u8_t alarm_status;
    u8_t alarm_level;
    time_t time;    
    s32_t unused;
}__attribute__((packed));

/*dyb trap frame body*/
struct dyb_trap_frame_body {
	s32_t netunit_ipaddr;
	s16_t netunit_port;
	s16_t device_index;
	s16_t reserve_word;

    s32_t netunit_status;
    s32_t frame_index;
    s32_t board_index;
    s32_t board_type;
    s32_t board_status;
    s32_t interface_index;
    s32_t interface_type;

    s32_t alarm_serial;
    s32_t alarm_id;
    s32_t alarm_type;
    u8_t alarm_status;
    u8_t alarm_level;
    time_t time;    
    s32_t unused;
} ;




struct udp_trap_addr_s{

    struct sockaddr_in addr;    
    hj_socket_data_t * p_socket_data; 
    int addr_len;
    int used;   
};

typedef struct tagtrap_S
{
    //XBUFFER_S   * sbuf;

    s32_t       wdt_fd;
    struct udp_trap_addr_s  trap_addr;

}Trap_S;










void lwip_pkt_handle(void);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
u8 lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
u8 lwip_comm_init(void);
void lwip_comm_dhcp_creat(void);
void lwip_comm_dhcp_delete(void);
void lwip_comm_destroy(void);
void lwip_comm_delete_next_timeout(void);

#endif













