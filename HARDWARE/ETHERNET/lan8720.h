#ifndef __LAN8720_H
#define __LAN8720_H
#include "sys.h"
#include "stm32f4x7_eth.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LAN8720 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHY芯片地址.
#define LAN8720_RST 		   	PAout(3) 			//LAN8720复位引脚	 

extern ETH_DMADESCTypeDef *DMARxDscrTab;			//以太网DMA接收描述符数据结构体指针
extern ETH_DMADESCTypeDef *DMATxDscrTab;			//以太网DMA发送描述符数据结构体指针 
extern uint8_t *Rx_Buff; 							//以太网底层驱动接收buffers指针 
extern uint8_t *Tx_Buff; 							//以太网底层驱动发送buffers指针
extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA发送描述符追踪指针
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA接收描述符追踪指针 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA最后接收到的帧信息指针
 

#if 0	
#define PHY_LINK_SPEED100  (1 << 6)	/*!< PHY status bit for 100Mbps mode */
#define PHY_LINK_FULLDUPLX (1 << 4)	/*!< PHY status bit for full duplex mode */
#define PHY_LINK_CONNECTED (1 << 2)	/*!< PHY status bit for connected state */
#define PHY_LINK_CHANGED   (1 << 0)	/*!< PHY status bit for changed state (not persistent) */
#define PHY_LINK_BUSY      (1 << 8)	/*!< PHY status bit for MII link busy */
#define PHY_LINK_ERROR     (1 << 10)	/*!< PHY status bit for link error */

#endif
/**
 * @brief	Phy status update state machine
 * @return	An Or'ed value of PHY_LINK_* statuses
 * @note	This function can be called at any rate and will poll the the PHY status. Multiple
 * calls may be needed to determine PHY status.
 */
uint32_t lpcPHYStsPoll(void);

int32_t lpc_phy_sts_sm(void); 

u8 LAN8720_Init(void);
u8 LAN8720_Get_Speed(void);
u8 ETH_MACDMA_Config(void);
FrameTypeDef ETH_Rx_Packet(void);
u8 ETH_Tx_Packet(u16 FrameLength);
u32 ETH_GetCurrentTxBuffer(void);
u8 ETH_Mem_Malloc(void);
void ETH_Mem_Free(void);
#endif 

