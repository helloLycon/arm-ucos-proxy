#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "usart.h" 
#include "delay.h"
#include "malloc.h" 
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

ETH_DMADESCTypeDef *DMARxDscrTab;	//以太网DMA接收描述符数据结构体指针
ETH_DMADESCTypeDef *DMATxDscrTab;	//以太网DMA发送描述符数据结构体指针 
uint8_t *Rx_Buff; 					//以太网底层驱动接收buffers指针 
uint8_t *Tx_Buff; 					//以太网底层驱动发送buffers指针
  
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


//以太网IO,中断配置
void ETH_GPIO_Configuration(void)
{
	uint8_t  time=50;
	uint32_t i;
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIO时钟 RMII接口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //使能SYSCFG时钟
	
	/*网络引脚设置 RMII接口
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
					
	//配置PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	//配置PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
	//配置PB13 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);
		
	//配置PG13, PG11
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	
	
	//配置PA3为推完输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推完输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //拉高PA3
	
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	while(time--)
	{
		for(i=10000;i>0;i--);        //延时一会
	}
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//拉低PA3  复位
	while(time--)
	{
		for(i=100000;i>0;i--);        //延时一会
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //拉高PA3
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
	while(time--)
	{
		for(i=10000;i>0;i--);        //延时一会
	}
	GPIO_ResetBits(GPIOA,GPIO_Pin_3);//拉低PA3  复位
	led0_port_toggle();
	Main_Port_Wdi_Toggle();
    while(time--)
	{
		for(i=100000;i>0;i--);        //延时一会
	}
	GPIO_SetBits(GPIOA,GPIO_Pin_3);  //拉高PA3
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






//LAN8720初始化
//返回值:0,成功;
//    其他,失败
u8 LAN8720_Init(void)
{
	u8 rval=0;
	GPIO_InitTypeDef GPIO_InitStructure;
  
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOG, ENABLE);//使能GPIO时钟 RMII接口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);   //使能SYSCFG时钟
  
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII); //MAC和PHY之间使用RMII接口
#if 0
	/*网络引脚设置 RMII接口
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
					
	  //配置PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	//配置PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); //引脚复用到网络接口上
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                                
	//配置PG11, PG14 and PG13 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
	
	//配置PD3为推完输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推完输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;  
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	LAN8720_RST=0;					//硬件复位LAN8720
	delay_ms(50);	
	LAN8720_RST=1;				 	//复位结束 
#endif	
	ETH_GPIO_Configuration();
	ETHERNET_NVICConfiguration();	//设置中断优先级
	rval=ETH_MACDMA_Config();		//配置MAC及DMA
	return !rval;					//ETH的规则为:0,失败;1,成功;所以要取反一下 
}

//以太网中断分组配置
void ETHERNET_NVICConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;  //以太网中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0X00;  //中断寄存器组2最高优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0X00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


//得到8720的速度模式
//返回值:
//001:10M半双工
//101:10M全双工
//010:100M半双工
//110:100M全双工
//其他:错误.
u8 LAN8720_Get_Speed(void)
{
	u8 speed;
	speed=((ETH_ReadPHYRegister(0x00,31)&0x1C)>>2); //从LAN8720的31号寄存器中读取网络速度和双工模式
	return speed;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//以下部分为STM32F407网卡配置/接口函数.

//初始化ETH MAC层及DMA配置
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_MACDMA_Config(void)
{
	u8 rval;
	ETH_InitTypeDef ETH_InitStructure; 
	
	//使能以太网MAC以及MAC接收和发送时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
                        
	ETH_DeInit();  								//AHB总线重启以太网
	ETH_SoftwareReset();  						//软件重启网络
	while (ETH_GetSoftwareResetStatus() == SET){
		
		Main_Port_Wdi_Toggle();    
				
	};//等待软件重启网络完成  //////
//	delay_ms(100);
//    Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
//	Main_Port_Wdi_Toggle();
//	delay_ms(100);
	
	
	ETH_StructInit(&ETH_InitStructure); 	 	//初始化网络为默认值  

	///网络MAC参数设置 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			//开启网络自适应功能
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					//关闭反馈
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		//关闭重传功能
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	//关闭自动去除PDA/CRC功能 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						//关闭接收所有的帧
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//允许接收所有广播帧
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			//关闭混合模式的地址过滤  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//对于组播地址使用完美地址过滤   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	//对单播地址使用完美地址过滤 
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			//开启ipv4和TCP/UDP/ICMP的帧校验和卸载   
#endif
	//当我们使用帧校验和卸载功能的时候，一定要使能存储转发模式,存储转发模式中要保证整个帧存储在FIFO中,
	//这样MAC能插入/识别出帧校验值,当真校验正确的时候DMA就可以处理帧,否则就丢弃掉该帧
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //开启丢弃TCP/IP错误帧
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;     //开启接收数据的存储转发模式    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;   //开启发送数据的存储转发模式  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;     	//禁止转发错误帧  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//不转发过小的好帧 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		//打开处理第二帧功能
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	//开启DMA传输的地址对齐功能
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			//开启固定突发功能    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		//DMA发送的最大突发长度为32个节拍   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			//DMA接收的最大突发长度为32个节拍
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	rval=ETH_Init(&ETH_InitStructure,LAN8720_PHY_ADDRESS);		//配置ETH
	if(rval==ETH_SUCCESS)//配置成功
	{
		ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);  	//使能以太网接收中断	
	}
	return rval;
}

extern void lwip_pkt_handle(void);		//在lwip_comm.c里面定义
//以太网DMA接收中断服务函数
void ETH_IRQHandler(void)
{
	while(ETH_GetRxPktSize(DMARxDescToGet)!=0) 	//检测是否收到数据包
	{ 
		lwip_pkt_handle();		
	}
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R); 	//清除DMA中断标志位
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);	//清除DMA接收中断标志位
}  
//接收一个网卡数据包
//返回值:网络数据包帧结构体
FrameTypeDef ETH_Rx_Packet(void)
{ 
	u32 framelength=0;
	FrameTypeDef frame={0,0};   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMARxDescToGet->Status&ETH_DMARxDesc_OWN)!=(u32)RESET)
	{	
		frame.length=ETH_ERROR; 
		if ((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)  
		{ 
			ETH->DMASR = ETH_DMASR_RBUS;//清除ETH DMA的RBUS位 
			ETH->DMARPDR=0;//恢复DMA接收
		}
		return frame;//错误,OWN位被设置了
	}  
	if(((DMARxDescToGet->Status&ETH_DMARxDesc_ES)==(u32)RESET)&& 
	((DMARxDescToGet->Status & ETH_DMARxDesc_LS)!=(u32)RESET)&&  
	((DMARxDescToGet->Status & ETH_DMARxDesc_FS)!=(u32)RESET))  
	{       
		framelength=((DMARxDescToGet->Status&ETH_DMARxDesc_FL)>>ETH_DMARxDesc_FrameLengthShift)-4;//得到接收包帧长度(不包含4字节CRC)
 		frame.buffer = DMARxDescToGet->Buffer1Addr;//得到包数据所在的位置
	}else framelength=ETH_ERROR;//错误  
	frame.length=framelength; 
	frame.descriptor=DMARxDescToGet;  
	//更新ETH DMA全局Rx描述符为下一个Rx描述符
	//为下一次buffer读取设置下一个DMA Rx描述符
	DMARxDescToGet=(ETH_DMADESCTypeDef*)(DMARxDescToGet->Buffer2NextDescAddr);   
	return frame;  
}
//发送一个网卡数据包
//FrameLength:数据包长度
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{   
	//检查当前描述符,是否属于ETHERNET DMA(设置的时候)/CPU(复位的时候)
	if((DMATxDescToSet->Status&ETH_DMATxDesc_OWN)!=(u32)RESET)return ETH_ERROR;//错误,OWN位被设置了 
 	DMATxDescToSet->ControlBufferSize=(FrameLength&ETH_DMATxDesc_TBS1);//设置帧长度,bits[12:0]
	DMATxDescToSet->Status|=ETH_DMATxDesc_LS|ETH_DMATxDesc_FS;//设置最后一个和第一个位段置位(1个描述符传输一帧)
  	DMATxDescToSet->Status|=ETH_DMATxDesc_OWN;//设置Tx描述符的OWN位,buffer重归ETH DMA
	if((ETH->DMASR&ETH_DMASR_TBUS)!=(u32)RESET)//当Tx Buffer不可用位(TBUS)被设置的时候,重置它.恢复传输
	{ 
		ETH->DMASR=ETH_DMASR_TBUS;//重置ETH DMA TBUS位 
		ETH->DMATPDR=0;//恢复DMA发送
	} 
	//更新ETH DMA全局Tx描述符为下一个Tx描述符
	//为下一次buffer发送设置下一个DMA Tx描述符 
	DMATxDescToSet=(ETH_DMADESCTypeDef*)(DMATxDescToSet->Buffer2NextDescAddr);    
	return ETH_SUCCESS;   
}
//得到当前描述符的Tx buffer地址
//返回值:Tx buffer地址
u32 ETH_GetCurrentTxBuffer(void)
{  
  return DMATxDescToSet->Buffer1Addr;//返回Tx buffer地址  
}

//为ETH底层驱动申请内存
//返回值:0,正常
//    其他,失败
u8 ETH_Mem_Malloc(void)
{ 
	DMARxDscrTab=mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存
	DMATxDscrTab=mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//申请内存  
	Rx_Buff=mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//申请内存
	Tx_Buff=mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//申请内存
	printf("DMARxDscrTab=%x,DMATxDscrTab=%x\r\n",ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef),ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));
	printf("Tx_Buff=%x,Rx_Buff=%x\r\n",ETH_TX_BUF_SIZE*ETH_TXBUFNB,ETH_RX_BUF_SIZE*ETH_RXBUFNB);
	
	
	if(!DMARxDscrTab||!DMATxDscrTab||!Rx_Buff||!Tx_Buff)
	{
		ETH_Mem_Free();
		return 1;	//申请失败
	}	
	return 0;		//申请成功
}

//释放ETH 底层驱动申请的内存
void ETH_Mem_Free(void)
{ 
	myfree(SRAMIN,DMARxDscrTab);//释放内存
	myfree(SRAMIN,DMATxDscrTab);//释放内存
	myfree(SRAMIN,Rx_Buff);		//释放内存
	myfree(SRAMIN,Tx_Buff);		//释放内存  
}

















