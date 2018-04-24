#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
#if 0
//LED�˿ڶ���
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


void LED_Init(void); //��ʼ��
void Set_Led(u8 LED,u8 state);
#endif
