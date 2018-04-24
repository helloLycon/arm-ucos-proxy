#ifndef UART3_CONTROL_H
#define UART3_CONTROL_H

//驱动与应用程序的公共头文件，需要包括的头文件不同。

#include "includes.h"
#include "led.h"
#include "types.h"
#include "hj_nm.h"


/* 2017.4.27 */
//#define WGTXLINKNODE_CNT 8
#define WGTXLINKNODE_CNT 16

#define TXWGNODESIZE 256		//最大帧长度为256字节

struct tagWgTxNodeData {	//dyb 网管信息链表节点结构体
	
	u16_t len;
	u8_t buff[TXWGNODESIZE];
};
typedef struct tagWgTxNodeData WgTxNodeData_S;

struct tagScom3TxLink{
	
	u16_t tx_rd_index;                  //rd pointer
	u16_t tx_wr_index;                  
	WgTxNodeData_S tx_frame[WGTXLINKNODE_CNT];
  OS_EVENT * hTxSem;	
	u32_t hTxWaitFlag;
	OS_EVENT * hTxMutex;
};
typedef struct tagScom3TxLink Scom3TxLink_S;


#define WGRXLINKNODE_CNT 10

#define RXWGNODESIZE 1024		//最大帧长度为256字节

struct tagWgRxNodeData {	//dyb 网管信息链表节点结构体
	
	u16_t len;
	u8_t buff[RXWGNODESIZE];
};
typedef struct tagWgRxNodeData WgRxNodeData_S;

struct tagScom3RxLink{
	
	u16_t rx_rd_index;                  //rd pointer
	u16_t rx_wr_index;                  
	WgRxNodeData_S rx_frame[WGRXLINKNODE_CNT];
//    SemaphoreHandle_t hTxMutex;
};
typedef struct tagScom3RxLink Scom3RxLink_S;





#define TXFRAMESIZE 4
#define RXFRAMESIZE 4

#define LENSIZE 128
#define MAXTHREADNUM 12      //only upto 12 thread
#define MISCTHREADNUM (MAXTHREADNUM-1)

#define TXBACKSIZE  4
#define BACKSIZE  (sizeof(hj_txback_hdr_t))    

typedef struct tagthreadmsgS
{
    u32_t thread_id;
    u8_t seq;
    //hj_nm_cmd_frame_t rxframe[RXFRAMESIZE];    
    //int rxframe_wr;
    //int rxframe_rd;
    //pthread_mutex_t rxframe_mutex;
    hj_nm_cmd_frame_t txback[TXBACKSIZE];  
    int txback_wr;
    int txback_rd;
    OS_EVENT *  hmutex;

    //hj_nm_cmd_frame_t txframe[TXFRAMESIZE];
    //int txframe_wr;
    //int txframe_rd;
    //pthread_mutex_t txframe_mutex;
    //hj_socket_data_t * p_socket_data;

}THREADMSG_S;

#define BUFFERSIZE 1024 
#define UART_TX_BUFF_LEN 1024
#define UART_RX_BUFF_LEN 1024
#define UART_BUFF_LEN 1024
#define UART_RX_FUFF_SIZE (BUFFERSIZE-1)

#define UART_GOOD_BUFF_LEN 512

typedef struct taguartS
{

    //u8_t tx_buff[UART_TX_BUFF_LEN];

    //u32_t tx_wr_index;
    //u32_t tx_rd_index;
    u8_t rx_buff[UART_RX_BUFF_LEN];
    u32_t rx_wr_index;
    u32_t rx_rd_index;

    u32_t UARTGoodFrameFlag;    
    u32_t UARTEscapeFlag;
    u32_t UARTFrameHeadFlag;
    u32_t UARTfcsReceive;

    u8_t ComputerGoodFrame[UART_GOOD_BUFF_LEN];  
    u8_t ComputerGoodFrameCounter;

}UART_S;

typedef struct tagcontrolS
{
    UART_S uart;
	  THREADMSG_S threaddata[MAXTHREADNUM];
	//struct timeval jump_poll_time;
}CONTROL_S;


void uart3_control_task(void *pdata);
void uart3_init(u32 bound);


#endif /*UART2_CONTROL_H*/
