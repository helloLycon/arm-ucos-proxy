#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��PF9��PF10Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��

#if 0
void LED_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //ʹ��GPIOD��ʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //����
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //����GPIO
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOF,GPIO_Pin_9|GPIO_Pin_10); //GPIOF9,10�ߵ�ƽ
	
	
	
}
#endif


void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);//??GPIOD??

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10 | GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //??????
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //????
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //??
	GPIO_Init(GPIOF, &GPIO_InitStructure);               //???PD12.13.14.15

	GPIO_SetBits(GPIOF,GPIO_Pin_11 | GPIO_Pin_10 | GPIO_Pin_9);//PD12.13.14.15???,??
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//GPIOC PIN0 WDI

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //??????
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //????
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //??
	GPIO_Init(GPIOC, &GPIO_InitStructure);               //???PD12.13.14.15

	GPIO_SetBits(GPIOC,GPIO_Pin_0);//PC0	
	
}

void Port_Wdi_Toggle(uint32_t state)
{
    switch(state)
	{
		
		case 0: WDI_ON;
		    break;
		case 1: WDI_OFF;
		    break;
		default:
		    break;    
	}
}


#if 0
void Set_Led(u8 LED,u8 state)
{
	if(LED==0)
	{
		switch(state)
		{
			//case 0: LED0=1;
			case 0: LED0_ON;
							break;
			//case 1: LED0=0;
			case 1: LED0_OFF;
							break;
		}
	}else if(LED==1)
	{
		switch(state)
		{
			//case 0: LED1=1;
			case 0: LED1_ON;
							break;
			//case 1: LED1=0;
			case 1: LED1_OFF;
							break;
		}
	}
}
#endif
