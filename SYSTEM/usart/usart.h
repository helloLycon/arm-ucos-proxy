#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
#include "api.h"
#include "types.h"
//#include "cmd_tree.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.csom
//�޸�����:2011/6/14
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
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


#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����


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
  * ���Եȼ�����
  */
#define	DBG_CRIT            0U  /* critical conditions  */
#define	DBG_ERR             1U  /* error conditions     */
#define	DBG_WARNING         2U  /* warning conditions   */
#define	DBG_INFO            3U  /* informational        */
#define	DBG_DEBUG           4U  /* debug-level messages */
#define DEFAULT_DBG_LEVEL   DBG_INFO


extern Msg_t R_msg;
extern u8 ReceiveCommand;
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
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


