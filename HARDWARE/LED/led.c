#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//初始化PF9和PF10为输出口.并使能这两个口的时钟		    
//LED IO初始化

#if 0
void LED_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //使能GPIOD的时钟
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;  //推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;  //上拉
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //高速GPIO
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOF,GPIO_Pin_9|GPIO_Pin_10); //GPIOF9,10高电平
	
	
	
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
