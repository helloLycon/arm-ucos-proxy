#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "api.h"
#include "types.h"
//#include "cmd_tree.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.csom
//修改日期:2011/6/14
//版本：V1.4
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
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
////////////////////////////////////////////////////////////////////////////////// 	


//*---------------- Telnet ASCII Defination ----------------*/
#define In_DEL             0x7F
#define In_BKSPE           0x08     //* ASCII <DEL> */
#define In_TAB             0x09
#define In_EOL '\r'                 //* ASCII <CR> */
#define In_SKIP '\3'             //* ASCII control-C */
#define In_EOF '\x1A'               //* ASCII control-Z */
#define In_ESC 0x1b                 //* ASCII ESC */
#define In_ARROW_PREFIX    0x5b1b
#define In_UP_ARROW        0x41
#define In_DOWN_ARROW      0x42
#define In_LEFT_ARROW      0x44
#define In_RIGHT_ARROW     0x43
#define Out_DEL "\x8 \x8"           //* VT100 backspace and clear */
#define Out_SKIP "^C\r\n# "           //* ^C and new line */

#define PutMsg printf
//#define PutChar SendChar


#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收


#define MsgLen          64


typedef void (*send_t)(const char *,struct netconn *);


typedef struct tagMsg
{
    u8 CurrentPos;
    u8 msg[MsgLen];
    
}Msg_t;



typedef struct {
	const char *cmd;
	const char *hlp;
	int		(*proc)();
	//const struct cmd_tree * tree;
}CMD_STRUC;


/**
  * 调试等级定义
  */
#define	DBG_CRIT            0U  /* critical conditions  */
#define	DBG_ERR             1U  /* error conditions     */
#define	DBG_WARNING         2U  /* warning conditions   */
#define	DBG_INFO            3U  /* informational        */
#define	DBG_DEBUG           4U  /* debug-level messages */
#define DEFAULT_DBG_LEVEL   DBG_INFO


extern Msg_t R_msg;
extern u8 ReceiveCommand;
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);
void uart1_init(u32 bound);
void uart1_send(const char * str ,struct netconn *conn);
int scom1_task_create_f(void);
int ParseCmd(send_t send_method,struct netconn *conn , char * cmdline, u8 cmd_len);
//void shell_printf(enum file_desc fd , const char* fmt,...);
extern const CMD_STRUC CMD_INNER[];
status_t control_set_sysreset(void);
const char * version_ntoa(u8 ver , char * p);
void clr_line_insert(char * p ,const char * ist);


#endif


