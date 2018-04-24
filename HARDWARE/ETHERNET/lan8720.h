#ifndef __LAN8720_H
#define __LAN8720_H
#include "sys.h"
#include "stm32f4x7_eth.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LAN8720 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 


#define LAN8720_PHY_ADDRESS  	0x00				//LAN8720 PHYоƬ��ַ.
#define LAN8720_RST 		   	PAout(3) 			//LAN8720��λ����	 

extern ETH_DMADESCTypeDef *DMARxDscrTab;			//��̫��DMA�������������ݽṹ��ָ��
extern ETH_DMADESCTypeDef *DMATxDscrTab;			//��̫��DMA�������������ݽṹ��ָ�� 
extern uint8_t *Rx_Buff; 							//��̫���ײ���������buffersָ�� 
extern uint8_t *Tx_Buff; 							//��̫���ײ���������buffersָ��
extern ETH_DMADESCTypeDef  *DMATxDescToSet;			//DMA����������׷��ָ��
extern ETH_DMADESCTypeDef  *DMARxDescToGet; 		//DMA����������׷��ָ�� 
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;	//DMA�����յ���֡��Ϣָ��
 

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

