#ifndef UART2_CONTROL_H
#define UART2_CONTROL_H

//驱动与应用程序的公共头文件，需要包括的头文件不同。

#include "includes.h"
#include "led.h"
#include "types.h"



#define WGTXLINKNODE_CNT 8

#define TXWGNODESIZE 256		//最大帧长度为256字节

struct tagWgTxNodeData {	//dyb 网管信息链表节点结构体
	
	u32_t len;
	u8_t buff[TXWGNODESIZE];
};
typedef struct tagWgTxNodeData WgTxNodeData_S;

struct tagScom2TxLink{
	
	u32_t tx_rd_index;                  //rd pointer
	u32_t tx_wr_index;                  
	WgTxNodeData_S tx_frame[WGTXLINKNODE_CNT];
  OS_EVENT * hTxMutex;
};
typedef struct tagScom2TxLink Scom2TxLink_S;


#define WGRXLINKNODE_CNT 10

#define RXWGNODESIZE 1024		//最大帧长度为256字节

struct tagWgRxNodeData {	//dyb 网管信息链表节点结构体
	
	u32_t len;
	u8_t buff[RXWGNODESIZE];
};
typedef struct tagWgRxNodeData WgRxNodeData_S;

struct tagScom2RxLink{
	
	u32_t rx_rd_index;                  //rd pointer
	u32_t rx_wr_index;                  
	WgRxNodeData_S rx_frame[WGRXLINKNODE_CNT];
//    SemaphoreHandle_t hTxMutex;
};
typedef struct tagScom2RxLink Scom2RxLink_S;



void uar2_control_task(void *pdata);
void uart2_init(u32 bound);


#endif /*UART2_CONTROL_H*/
