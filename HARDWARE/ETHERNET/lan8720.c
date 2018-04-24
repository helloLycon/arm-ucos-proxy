#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "usart.h" 
#include "delay.h"
#include "malloc.h" 
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

ETH_DMADESCTypeDef *DMARxDscrTab;	//��̫��DMA�������������ݽṹ��ָ��
ETH_DMADESCTypeDef *DMATxDscrTab;	//��̫��DMA�������������ݽṹ��ָ�� 
uint8_t *Rx_Buff; 					//��̫���ײ���������buffersָ�� 
uint8_t *Tx_Buff; 					//��̫���ײ���������buffersָ��
  
static void ETHERNET_NVICConfiguration(void);



/** \brief  LAN8720 PHY register offsets */
#define LAN8_BCR_REG        0x0  /**< Basic Control Register */
#define LAN8_BSR_REG        0x1  /**< Basic Status Reg */
#define LAN8_PHYID1_REG     0x2  /**< PHY ID 1 Reg  */
#define LAN8_PHYID2_REG     0x3  /**< PHY ID 2 Reg */
#define LAN8_PHYSPLCTL_REG  0x11 /**< PHY special control/status Reg */

/* LAN8720 BCR register definitions */
#define LAN8_RESET          (1 << 15)  /**< 1= S/W Reset */
#define LAN8_LOOPBACK       (1 << 14)  /**< 1=loopback Enabled */
#define LAN8_SPEED_SELECT   (1 << 13)  /**< 1=Select 100MBps */
#define LAN8_AUTONEG        (1 << 12)  /**< 1=Enable auto-negotiation */
#define LAN8_POWER_DOWN     (1 << 11)  /**< 1=Power down PHY */
#define LAN8_ISOLATE        (1 << 10)  /**< 1=Isolate PHY */
#define LAN8_RESTART_AUTONEG (1 << 9)  /**< 1=Restart auto-negoatiation */
#define LAN8_DUPLEX_MODE    (1 << 8)   /**< 1=Full duplex mode */

/* LAN8720 BSR register definitions */
#define LAN8_100BASE_T4     (1 << 15)  /**< T4 mode */
#define LAN8_100BASE_TX_FD  (1 << 14)  /**< 100MBps full duplex */
#define LAN8_100BASE_TX_HD  (1 << 13)  /**< 100MBps half duplex */
#define LAN8_10BASE_T_FD    (1 << 12)  /**< 100Bps full duplex */
#define LAN8_10BASE_T_HD    (1 << 11)  /**< 10MBps half duplex */
#define LAN8_AUTONEG_COMP   (1 << 5)   /**< Auto-negotation complete */
#define LAN8_RMT_FAULT      (1 << 4)   /**< Fault */
#define LAN8_AUTONEG_ABILITY (1 << 3)  /**< Auto-negotation supported */
#define LAN8_LINK_STATUS    (1 << 2)   /**< 1=Link active */
#define LAN8_JABBER_DETECT  (1 << 1)   /**< Jabber detect */
#define LAN8_EXTEND_CAPAB   (1 << 0)   /**< Supports extended capabilities */

/* LAN8720 PHYSPLCTL status definitions */
#define LAN8_SPEEDMASK      (0x0f << 12)   /**< Speed and duplex mask */
#define LAN8_SPEED100F      (8 << 12)   /**< 100BT full duplex */
#define LAN8_SPEED10F       (2 << 12)   /**< 10BT full duplex */
#define LAN8_SPEED100H      (4 << 12)   /**< 100BT half duplex */
#define LAN8_SPEED10H       (1 << 12)   /**< 10BT half duplex */

/* LAN8720 PHY ID 1/2 register definitions */
#define LAN8_PHYID1_OUI     0x0181     /**< Expected PHY ID1 */
#define LAN8_PHYID2_OUI     0xB0A0     /**< Expected PHY ID2, except last 4 bits */

/**
 * @brief PHY status structure used to indicate current status of PHY.
 */
typedef struct {
	uint32_t     phy_speed_100mbs:2; /**< 10/100 MBS connection speed flag. */
	uint32_t     phy_full_duplex:2;  /**< Half/full duplex connection speed flag. */
	uint32_t     phy_link_active:2;  /**< Phy link active flag. */
	uint32_t     phy_link_changed:2;
} PHY_STATUS_TYPE;

/** \brief  PHY update flags */
static PHY_STATUS_TYPE physts;

/** \brief  Last PHY update flags, used for determing if something has changed */
static PHY_STATUS_TYPE olddphysts;

static uint32_t phyustate;


//��̫��IO,�ж�����
void ETH_GPIO_Configuration(void)
{
	uint8_t  time=50;
	uint32_t i;
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��GPIOʱ�� RMII�ӿ�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //ʹ��SYSCFGʱ��
	
	/*������������ RMII�ӿ�
	ETH_MDIO -------------------------> PA2
	ETH_MDC --------------------------> PC1
	ETH_RMII_REF_CLK------------------> PA1
	ETH_RMII_CRS_DV ------------------> PA7
	ETH_RMII_RXD0 --------------------> PC4
	ETH_RMII_RXD1 --------------------> PC5
	ETH_RMII_TX_EN -------------------> PB11
	ETH_RMII_TXD0 --------------------> PG13
	ETH_RMII_TXD1 --------------------> PB13
	ETH_RESET-------------------------> PA3*/
					
	//����PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); //���Ÿ��õ�����ӿ���
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	//����PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); //���Ÿ��õ�����ӿ���
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
	//����PB13 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);
		
	//����PG13, PG11
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	
	
	//����PA3Ϊ�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //����PA3
	
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	while(time--)
	{
		for(i=10000;i>0;i--);        //��ʱһ��
	}
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//����PA3  ��λ
	while(time--)
	{
		for(i=100000;i>0;i--);        //��ʱһ��
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //����PA3
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	while(time--)
	{
		for(i=10000;i>0;i--);        //��ʱһ��
	}
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//����PA3  ��λ
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
    while(time--)
	{
		for(i=100000;i>0;i--);        //��ʱһ��
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //����PA3
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
}



/** \brief  Update PHY status from passed value
 *
 *  This function updates the current PHY status based on the
 *  passed PHY status word. The PHY status indicate if the link
 *  is active, the connection speed, and duplex.
 *
 *  \param[in]    netif   NETIF structure
 *  \param[in]    linksts Status word with link state
 *  \param[in]    sdsts   Status word with speed and duplex states
 *  \return        1 if the status has changed, otherwise 0
 */
//static int32_t lpc_update_phy_sts(struct netif *netif, uint32_t linksts, uint32_t sdsts)
static int32_t lpc_update_phy_sts(uint32_t linksts, uint32_t sdsts)
{
	int32_t status = 0;

	/* Update link active status */
	if (linksts & LAN8_LINK_STATUS)
		physts.phy_link_active = 1;
	else
		physts.phy_link_active = 0;

	switch (sdsts & LAN8_SPEEDMASK) {
		case LAN8_SPEED100F:
		default:
			physts.phy_speed_100mbs = 1;
			physts.phy_full_duplex = 1;
			break;

		case LAN8_SPEED10F:
			physts.phy_speed_100mbs = 0;
			physts.phy_full_duplex = 1;
			break;

		case LAN8_SPEED100H:
			physts.phy_speed_100mbs = 1;
			physts.phy_full_duplex = 0;
			break;

		case LAN8_SPEED10H:
			physts.phy_speed_100mbs = 0;
			physts.phy_full_duplex = 0;
			break;
	}

	if (physts.phy_speed_100mbs != olddphysts.phy_speed_100mbs) {
		//changed = 1;
		if (physts.phy_speed_100mbs) {
			/* 100MBit mode. */
			ETH_emac_set_speed(1);

			//NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);
		}
		else {
			/* 10MBit mode. */
			ETH_emac_set_speed(0);

			//NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);
		}

		olddphysts.phy_speed_100mbs = physts.phy_speed_100mbs;
	}

	if (physts.phy_full_duplex != olddphysts.phy_full_duplex) {
		//changed = 1;
		if (physts.phy_full_duplex)
			ETH_emac_set_duplex(1);
		else
			ETH_emac_set_duplex(0);

		olddphysts.phy_full_duplex = physts.phy_full_duplex;
	}

	if (physts.phy_link_active != olddphysts.phy_link_active) {
		physts.phy_link_changed = 1;
//		if (physts.phy_link_active)
//			netif_set_link_up(netif);
//		else
//			netif_set_link_down(netif);
//
		olddphysts.phy_link_active = physts.phy_link_active;
	}else{
	    physts.phy_link_changed = 0;
    }    
    status = physts.phy_speed_100mbs;
    status <<= 2;
    status |= physts.phy_full_duplex;
    status <<= 2;
    status |= physts.phy_link_active;
    status <<= 2;
    status |= physts.phy_link_changed;
	return status;
}

/* Phy status update state machine */
//int32_t lpc_phy_sts_sm(struct netif *netif)
int32_t lpc_phy_sts_sm(void)
{
	static uint32_t sts;
	int32_t changed = 0;

	switch (phyustate) {
		default:
		case 0:
			/* Read BMSR to clear faults */
			ETH_ReadPHYRegister_NonBlock(LAN8720_PHY_ADDRESS, LAN8_BSR_REG);
			phyustate = 1;
			break;

		case 1:
			/* Wait for read status state */
			if (!ETH_PHY_is_busy()) {
				/* Get PHY status with link state */
				sts = ETH_PHY_read_data();
				ETH_ReadPHYRegister_NonBlock(LAN8720_PHY_ADDRESS, LAN8_PHYSPLCTL_REG);
				phyustate = 2;
			}
			break;

		case 2:
			/* Wait for read status state */
			if (!ETH_PHY_is_busy()) {
				/* Update PHY status */
				//changed = lpc_update_phy_sts(netif, sts, lpc_mii_read_data());
				changed = lpc_update_phy_sts(sts, ETH_PHY_read_data());
				
				phyustate = 0;
			}
			break;
	}

	return changed;
}






//LAN8720��ʼ��
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 LAN8720_Init(void)
{
	u8 rval=0;
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG, ENABLE);//ʹ��GPIOʱ�� RMII�ӿ�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //ʹ��SYSCFGʱ��
  
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII); //MAC��PHY֮��ʹ��RMII�ӿ�
#if 0
	/*������������ RMII�ӿ�
	  ETH_MDIO -------------------------> PA2
	  ETH_MDC --------------------------> PC1
	  ETH_RMII_REF_CLK------------------> PA1
	  ETH_RMII_CRS_DV ------------------> PA7
	  ETH_RMII_RXD0 --------------------> PC4
	  ETH_RMII_RXD1 --------------------> PC5
	  ETH_RMII_TX_EN -------------------> PG11
	  ETH_RMII_TXD0 --------------------> PG13
	  ETH_RMII_TXD1 --------------------> PG14
	  ETH_RESET-------------------------> PD3*/
					
	  //����PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); //���Ÿ��õ�����ӿ���
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	//����PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); //���Ÿ��õ�����ӿ���
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
	//����PG11, PG14 and PG13 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
	
	//����PD3Ϊ�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	LAN8720_RST=0;					//Ӳ����λLAN8720
	delay_ms(50);	
	LAN8720_RST=1;				 	//��λ���� 
#endif	
	ETH_GPIO_Configuration();
	ETHERNET_NVICConfiguration();	//�����ж����ȼ�
	rval=ETH_MACDMA_Config();		//����MAC��DMA
	return !rval;					//ETH�Ĺ���Ϊ:0,ʧ��;1,�ɹ�;����Ҫȡ��һ�� 
}

//��̫���жϷ�������
void ETHERNET_NVICConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;  //��̫���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0X00;  //�жϼĴ�����2������ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0X00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//�õ�8720���ٶ�ģʽ
//����ֵ:
//001:10M��˫��
//101:10Mȫ˫��
//010:100M��˫��
//110:100Mȫ˫��
//����:����.
u8 LAN8720_Get_Speed(void)
{
	u8 speed;
	speed=((ETH_ReadPHYRegister(0x00,31)&0x1C)>>2); //��LAN8720��31�żĴ����ж�ȡ�����ٶȺ�˫��ģʽ
	return speed;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//���²���ΪSTM32F407��������/�ӿں���.

//��ʼ��ETH MAC�㼰DMA����
//����ֵ:ETH_ERROR,����ʧ��(0)
//		ETH_SUCCESS,���ͳɹ�(1)
u8 ETH_MACDMA_Config(void)
{
	u8 rval;
	ETH_InitTypeDef ETH_InitStructure; 
	
	//ʹ����̫��MAC�Լ�MAC���պͷ���ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
                        
	ETH_DeInit();  								//AHB����������̫��
	ETH_SoftwareReset();  						//�����������
	while (ETH_GetSoftwareResetStatus() == SET){
		
		Main_Port_Wdi_Toggle();    
				
	};//�ȴ���������������  //////
//	delay_ms(100);
//    Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
	
	
	ETH_StructInit(&ETH_InitStructure); 	 	//��ʼ������ΪĬ��ֵ  

	///����MAC�������� 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			//������������Ӧ����
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					//�رշ���
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		//�ر��ش�����
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	//�ر��Զ�ȥ��PDA/CRC���� 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						//�رս������е�֡
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//����������й㲥֡
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			//�رջ��ģʽ�ĵ�ַ����  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//�����鲥��ַʹ��������ַ����   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	//�Ե�����ַʹ��������ַ���� 
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			//����ipv4��TCP/UDP/ICMP��֡У���ж��   
#endif
	//������ʹ��֡У���ж�ع��ܵ�ʱ��һ��Ҫʹ�ܴ洢ת��ģʽ,�洢ת��ģʽ��Ҫ��֤����֡�洢��FIFO��,
	//����MAC�ܲ���/ʶ���֡У��ֵ,����У����ȷ��ʱ��DMA�Ϳ��Դ���֡,����Ͷ�������֡
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //��������TCP/IP����֡
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;     //�����������ݵĴ洢ת��ģʽ    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;   //�����������ݵĴ洢ת��ģʽ  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;     	//��ֹת������֡  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//��ת����С�ĺ�֡ 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		//�򿪴���ڶ�֡����
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	//����DMA����ĵ�ַ���빦��
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			//�����̶�ͻ������    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		//DMA���͵����ͻ������Ϊ32������   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			//DMA���յ����ͻ������Ϊ32������
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	rval=ETH_Init(&ETH_InitStructure,LAN8720_PHY_ADDRESS);		//����ETH
	if(rval==ETH_SUCCESS)//���óɹ�
	{
		ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);  	//ʹ����̫�������ж�	
	}
	return rval;
}

extern void lwip_pkt_handle(void);		//��lwip_comm.c���涨��
//��̫��DMA�����жϷ�����
void ETH_IRQHandler(void)
{
	while(ETH_GetRxPktSize(DMARxDescToGet)!=0) 	//����Ƿ��յ����ݰ�
	{ 
		lwip_pkt_handle();		
	}
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R); 	//���DMA�жϱ�־λ
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);	//���DMA�����жϱ�־λ
}  
//����һ���������ݰ�
//����ֵ:�������ݰ�֡�ṹ��
FrameTypeDef ETH_Rx_Packet(void)
{ 
	u32 framelength=0;
	FrameTypeDef frame={0,0};   
	//��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��)
	if((DMARxDescToGet->Status&ETH_DMARxDesc_OWN)!=(u32)RESET)
	{	
		frame.length=ETH_ERROR; 
		if ((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)  
		{ 
			ETH->DMASR = ETH_DMASR_RBUS;//���ETH DMA��RBUSλ 
			ETH->DMARPDR=0;//�ָ�DMA����
		}
		return frame;//����,OWNλ��������
	}  
	if(((DMARxDescToGet->Status&ETH_DMARxDesc_ES)==(u32)RESET)&& 
	((DMARxDescToGet->Status & ETH_DMARxDesc_LS)!=(u32)RESET)&&  
	((DMARxDescToGet->Status & ETH_DMARxDesc_FS)!=(u32)RESET))  
	{       
		framelength=((DMARxDescToGet->Status&ETH_DMARxDesc_FL)>>ETH_DMARxDesc_FrameLengthShift)-4;//�õ����հ�֡����(������4�ֽ�CRC)
 		frame.buffer = DMARxDescToGet->Buffer1Addr;//�õ����������ڵ�λ��
	}else framelength=ETH_ERROR;//����  
	frame.length=framelength; 
	frame.descriptor=DMARxDescToGet;  
	//����ETH DMAȫ��Rx������Ϊ��һ��Rx������
	//Ϊ��һ��buffer��ȡ������һ��DMA Rx������
	DMARxDescToGet=(ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);   
	return frame;  
}
//����һ���������ݰ�
//FrameLength:���ݰ�����
//����ֵ:ETH_ERROR,����ʧ��(0)
//		ETH_SUCCESS,���ͳɹ�(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{   
	//��鵱ǰ������,�Ƿ�����ETHERNET DMA(���õ�ʱ��)/CPU(��λ��ʱ��)
	if((DMATxDescToSet->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)return ETH_ERROR;//����,OWNλ�������� 
 	DMATxDescToSet->ControlBufferSize=(FrameLength&ETH_DMATxDesc_TBS1);//����֡����,bits[12:0]
	DMATxDescToSet->Status|=ETH_DMATxDesc_LS|ETH_DMATxDesc_FS;//�������һ���͵�һ��λ����λ(1������������һ֡)
  	DMATxDescToSet->Status|=ETH_DMATxDesc_OWN;//����Tx��������OWNλ,buffer�ع�ETH DMA
	if((ETH->DMASR&ETH_DMASR_TBUS)!=(u32)RESET)//��Tx Buffer������λ(TBUS)�����õ�ʱ��,������.�ָ�����
	{ 
		ETH->DMASR=ETH_DMASR_TBUS;//����ETH DMA TBUSλ 
		ETH->DMATPDR=0;//�ָ�DMA����
	} 
	//����ETH DMAȫ��Tx������Ϊ��һ��Tx������
	//Ϊ��һ��buffer����������һ��DMA Tx������ 
	DMATxDescToSet=(ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);    
	return ETH_SUCCESS;   
}
//�õ���ǰ��������Tx buffer��ַ
//����ֵ:Tx buffer��ַ
u32 ETH_GetCurrentTxBuffer(void)
{  
  return DMATxDescToSet->Buffer1Addr;//����Tx buffer��ַ  
}

//ΪETH�ײ����������ڴ�
//����ֵ:0,����
//    ����,ʧ��
u8 ETH_Mem_Malloc(void)
{ 
	DMARxDscrTab=mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//�����ڴ�
	DMATxDscrTab=mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//�����ڴ�  
	Rx_Buff=mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//�����ڴ�
	Tx_Buff=mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//�����ڴ�
	printf("DMARxDscrTab=%x,DMATxDscrTab=%x\r\n",ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef),ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));
	printf("Tx_Buff=%x,Rx_Buff=%x\r\n",ETH_TX_BUF_SIZE*ETH_TXBUFNB,ETH_RX_BUF_SIZE*ETH_RXBUFNB);
	
	
	if(!DMARxDscrTab||!DMATxDscrTab||!Rx_Buff||!Tx_Buff)
	{
		ETH_Mem_Free();
		return 1;	//����ʧ��
	}	
	return 0;		//����ɹ�
}

//�ͷ�ETH �ײ�����������ڴ�
void ETH_Mem_Free(void)
{ 
	myfree(SRAMIN,DMARxDscrTab);//�ͷ��ڴ�
	myfree(SRAMIN,DMATxDscrTab);//�ͷ��ڴ�
	myfree(SRAMIN,Rx_Buff);		//�ͷ��ڴ�
	myfree(SRAMIN,Tx_Buff);		//�ͷ��ڴ�  
}

















