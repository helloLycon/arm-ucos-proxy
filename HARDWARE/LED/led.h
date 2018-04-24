#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
#if 0
//LED端口定义
#define LED0 PFout(9)
#define LED1 PFout(10)
#endif



#define LED0_ON    GPIO_ResetBits(GPIOF,GPIO_Pin_11);
#define LED1_ON    GPIO_ResetBits(GPIOF,GPIO_Pin_10);
#define LED2_ON    GPIO_ResetBits(GPIOF,GPIO_Pin_9);

#define LED0_OFF   GPIO_SetBits(GPIOF,GPIO_Pin_11);
#define LED1_OFF   GPIO_SetBits(GPIOF,GPIO_Pin_10);
#define LED2_OFF   GPIO_SetBits(GPIOF,GPIO_Pin_9);

#define WDI_ON    GPIO_SetBits(GPIOC,GPIO_Pin_0);
#define WDI_OFF    GPIO_ResetBits(GPIOC,GPIO_Pin_0);


void LED_Init(void); //初始化
void Set_Led(u8 LED,u8 state);
#endif
