
#include "uart2_control.h"
#include "ring_buffer.h"
#include "led.h"

//LED任务
//任务优先级
#define USERSCOM2_SEND_TASK_PRIORITY		8
//任务堆栈大小
#define USERSCOM2_SEND_STK_SIZE		64
//任务堆栈
OS_STK	USERSCOM2_SEND_TASK_STK[USERSCOM2_SEND_STK_SIZE];
//任务函数
void user2_scom2_send_thread(void *arg);  

static Scom2TxLink_S Scom2TxLink;
static Scom2TxLink_S * hScom2TxLink;


static Scom2RxLink_S Scom2RxLink;
static Scom2RxLink_S * hScom2RxLink;

#define SEND_BUF_SIZE 512
#define RECV_BUF_SIZE 1024
//数组定义，含义如题名：
u8 USART2_SEND_DATA[SEND_BUF_SIZE]; 
u8 USART2_RECEIVE_DATA[RECV_BUF_SIZE]; 
u8 USART2_TX_Finish=1; // USART2发送完成标志量


u32 init_done_flag;     //是否初始化结束，当初始化时使用信号来唤醒，只有等待初始化好后才能使用信号来唤醒.

void control_set_scom2_baud(u32 bound)
{
    u32 nret = 19200;    
    USART_InitTypeDef USART_InitStructure;
    //printf("baudrate44=%x\r\n",bound);
    if(bound >= 2400){
        nret = bound;
          
    }else{
        //test_byte = baudrate;
        switch(bound){
            case 0:
                nret = 2400; 
                break;   
            case 1:
                nret = 4800;  
                break; 
            case 2:
                nret = 9600;
                break;
            case 3:
                nret = 19200; 
                break;   
            case 4:
                nret = 38400;  
                break; 
            case 5:
                nret = 57600;  
                break;   
            case 6:
                nret = 115200;   
                break;  
            default:
                break;     
        }  
    }            
    USART_InitStructure.USART_BaudRate = nret;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口1
}




//DMA配置：
void DMA_UART2_Configuration(void)
{
  MYDMA_Config(DMA1_Stream6,DMA_Channel_4,(u32)&USART2->DR,(u32)USART2_SEND_DATA,SEND_BUF_SIZE,DMA_DIR_MemoryToPeripheral);//DMA2,STEAM7,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.

  DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA1_Stream6, DMA_IT_TE, ENABLE);
  
  /* Enable USART2 DMA TX request */
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  DMA_Cmd(DMA1_Stream6, DISABLE);
  
    MYDMA_Config(DMA1_Stream5,DMA_Channel_4,(u32)&USART2->DR,(u32)USART2_RECEIVE_DATA,RECV_BUF_SIZE,DMA_DIR_PeripheralToMemory);
  
  /* Enable USART2 DMA RX request */
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  DMA_Cmd(DMA1_Stream5, ENABLE);
}


//DMA配置：
void DMA_UART2_RX_Configuration(void)
{
    MYDMA_Config(DMA1_Stream5,DMA_Channel_4,(u32)&USART2->DR,(u32)USART2_RECEIVE_DATA,RECV_BUF_SIZE,DMA_DIR_PeripheralToMemory);
  
}

//中断优先级配置：
void DMA_UART2_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

  /*Enable DMA Channel6 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//初始化IO 串口2 
//bound:波特率
void uart2_init(u32 bound)
{
   //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //使能GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);//使能USART2时钟

	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); //GPIOA10复用为USART1
	
	//USART2端口配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure); //初始化PD5，PD6

   //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口1
	
    	
	//空闲中断
    USART_ITConfig(USART2, USART_IT_IDLE , ENABLE); //开启相关中断
    USART_Cmd(USART2, ENABLE);  //使能串口2
    USART_ClearFlag(USART2, USART_FLAG_TC);
    
	//Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

    DMA_UART2_Configuration();
	DMA_UART2_NVIC_Configuration();
}

int testbyte70=0;
uint32_t testbyte10=0;
uint32_t testbyte11=0;
uint32_t testbyte12=0;
uint32_t testbyte13=0;
uint32_t testbyte14=0;
uint32_t testbyte15=0;
uint32_t testbyte16=0;

//USART2中断服务函数
void USART2_IRQHandler(void)
{
    uint16_t DATA_LEN;
    uint16_t i;
    WgRxNodeData_S * p_rx_frame;

	//LED2_OFF;
	//LED2_ON;
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //如果为空闲总线中断
    {
        USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
		DMA_Cmd(DMA1_Stream5, DISABLE);//关闭DMA,防止处理其间有数据
		
        DATA_LEN=RECV_BUF_SIZE-DMA_GetCurrDataCounter(DMA1_Stream5); 
		
		if((DATA_LEN == 200) ||(DATA_LEN == 400) ||(DATA_LEN == 600) ){      //有时可能会收到两个包后才中断，所以缓冲区需要设置成 最大帧*2.
		    testbyte12++;
            if(DATA_LEN == 400){
               testbyte14++; 
            }else if(DATA_LEN == 600){
               testbyte15++;  
            }
		}else{
		    testbyte13++;
		}
		testbyte16 += DATA_LEN;
        if(DATA_LEN > 0)
        {                                 //将数据送DMA存储地址
//            for(i=0;i<DATA_LEN;i++)
//            {
//                USART2_SEND_DATA[i]=USART2_RECEIVE_DATA[i];
//            }
//            //USART用DMA传输替代查询方式发送，克服被高优先级中断而产生丢帧现象。
//            DMA_Cmd(DMA1_Stream6, DISABLE); //改变datasize前先要禁止通道工作
//            //DMA1_Stream6->CNDTR=DATA_LEN; //DMA1,传输数据量
//            DMA_SetCurrDataCounter(DMA1_Stream6, DATA_LEN);
//            DMA_Cmd(DMA1_Stream6, ENABLE);                        
//        }
            p_rx_frame = &hScom2RxLink->rx_frame[hScom2RxLink->rx_rd_index];        
            p_rx_frame->len = DATA_LEN;
            //将数据送DMA存储地址
            for(i=0;i<p_rx_frame->len;i++)
            {
                p_rx_frame->buff[i] = USART2_RECEIVE_DATA[i];
            }
    		hScom2RxLink->rx_wr_index++;
            if(hScom2RxLink->rx_wr_index >= WGRXLINKNODE_CNT){
            	hScom2RxLink->rx_wr_index = 0;
            }	
        }
		
		
        DMA_SetCurrDataCounter(DMA1_Stream5, RECV_BUF_SIZE);
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TEIF5);
        
        DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_FEIF5 | DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5);//清标志
		USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
        DMA_Cmd(DMA1_Stream5, ENABLE);//处理完,重开DMA
                //读SR后读DR清除Idle
        i = USART2->SR;
        i = USART2->DR;
    }else{
		testbyte70++;
	}
    if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//出错
    {
        USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE);
    }
    USART_ClearITPendingBit(USART2, USART_IT_TC);
    USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    //LED2_OFF;
}
uint32_t testbyte22=0;
uint32_t testbyte23=0;
//DMA1_Stream6中断服务函数
//USART2使用DMA发数据中断服务程序
void DMA1_Stream6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6)){
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        DMA_Cmd(DMA1_Stream6, DISABLE);//关闭DMA,防止处理其间有数据
        DMA_SetCurrDataCounter(DMA1_Stream6, SEND_BUF_SIZE);
				if(init_done_flag == 0){
					testbyte23++;
			}
			USART2_TX_Finish = 1;
    }
    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TEIF6)){
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TEIF6);	
    }
    if(init_done_flag == 0){
        testbyte22++;
    }
}

uint32_t testbyte21=0;

//接收服务器数据再发送给串口
void Board_UARTX_Inset_Data_To_Link(uint8_t * pdata, uint32_t len)
{    
    WgTxNodeData_S * p_tx_frame;
    p_tx_frame = &hScom2TxLink->tx_frame[hScom2TxLink->tx_wr_index];
    p_tx_frame->len = len;
    memcpy(p_tx_frame->buff, pdata, len);
    hScom2TxLink->tx_wr_index++;
    if(hScom2TxLink->tx_wr_index >= WGTXLINKNODE_CNT){
	    hScom2TxLink->tx_wr_index = 0;
	}
  	testbyte21++;  	
  	OSSemPost(hScom2TxLink->hTxMutex);
  	
}

void Init_Board_UARTX_Inset_Data_To_Link(void)
{    
    WgTxNodeData_S * p_tx_frame;
    int len;
    //int nret;
    uint8_t frame_poll_baud[12]={0x7e,0x31,0x32,0x23};
    len = 4;
    p_tx_frame = &hScom2TxLink->tx_frame[hScom2TxLink->tx_wr_index];
    p_tx_frame->len = len;
    memcpy(p_tx_frame->buff, frame_poll_baud, len);
    hScom2TxLink->tx_wr_index++;
    if(hScom2TxLink->tx_wr_index >= WGTXLINKNODE_CNT){
	    hScom2TxLink->tx_wr_index = 0;
	}
  
  	//nret = OSSemPost(hScom2TxLink->hTxMutex);
}



//获取串口缓冲区的长度
uint32_t Board_UARTX_RxFrame_GetCount(void)
{
    int nret = 0;
    //xSemaphoreTake(txing_mutex, portMAX_DELAY);
    
    if(hScom2RxLink->rx_wr_index != hScom2RxLink->rx_rd_index){
        nret = 1;
    }
    //xSemaphoreGive(txing_mutex);
    return nret;
	//	return 0;
}

//接收串口数据再发送给服务器
void Board_UARTX_Send_To_Server(WgRxNodeData_S * p_tx_frame)
{    
    WgRxNodeData_S * p_rx_frame;
	int i;
    //xSemaphoreTake(txing_mutex, portMAX_DELAY);
    
    p_rx_frame = &hScom2RxLink->rx_frame[hScom2RxLink->rx_rd_index];        
                
    for(i=0;i<p_rx_frame->len;i++)
    {
        p_tx_frame->buff[i] = p_rx_frame->buff[i] ;
    }
    p_tx_frame->len = p_rx_frame->len;
    hScom2RxLink->rx_rd_index++;
    if(hScom2RxLink->rx_rd_index >= WGRXLINKNODE_CNT){
	    hScom2RxLink->rx_rd_index = 0;
	}	
    
    //xSemaphoreGive(txing_mutex);
    //debug_dump(pdata, len, "rxring:");
}





u8 led2_state=0;
uint32_t testbyte20 = 0;
uint32_t testbyte24 = 0;
uint32_t testbyte25 = 0;
#define MAX_TX_DELAY_TIMES 100
static void user2_scom2_send_thread(void *arg)
{
	int i;
	u8 err;
    WgTxNodeData_S * p_tx_frame;
    uint32_t usart2_tx_count = 0;
    while(1){  
        if(hScom2TxLink->tx_wr_index != hScom2TxLink->tx_rd_index){ //当数据帧位置不相同时,发送数据
            //printf("tx_wr_index=%x,tx_rd_index=%x\n", hScom2TxLink->tx_wr_index, hScom2TxLink->tx_rd_index);
            usart2_tx_count = 0;             
            while(USART2_TX_Finish == 0){   //传输还未结束
                
                led2_state = !led2_state;
                if(led2_state){
                    LED2_ON;    
                }else{
                    LED2_OFF;    
                }
                
                OSTimeDly(1);               //2ms 延时
                
                usart2_tx_count++;
				testbyte24++;
                if(usart2_tx_count >= MAX_TX_DELAY_TIMES){  //当许久没有操作时则需要初始化
                    uart2_init(115200);
                    testbyte25++;
                }
            }            
            p_tx_frame = &hScom2TxLink->tx_frame[hScom2TxLink->tx_rd_index];        
                        
            //将数据送DMA存储地址
            for(i=0;i<p_tx_frame->len;i++)
            {
                USART2_SEND_DATA[i]=p_tx_frame->buff[i];
            }
            //USART用DMA传输替代查询方式发送，克服被高优先级中断而产生丢帧现象。
            DMA_Cmd(DMA1_Stream6, DISABLE); //改变datasize前先要禁止通道工作
            DMA_SetCurrDataCounter(DMA1_Stream6, p_tx_frame->len);
            DMA_Cmd(DMA1_Stream6, ENABLE);    
            USART2_TX_Finish = 0;       //串口发传送开始
        	hScom2TxLink->tx_rd_index++;
            if(hScom2TxLink->tx_rd_index >= WGTXLINKNODE_CNT){
        	    hScom2TxLink->tx_rd_index = 0;
        	}			    

        	//当为更改波特率时，需要先发送命令之后再更改波特率
        	if((p_tx_frame->buff[0]==0x7e)&&(p_tx_frame->buff[1]==0x34)&&(p_tx_frame->buff[2]==0x2A)){
        	    printf("pc set scom2 baudrate = %x\n", p_tx_frame->buff[5]);        //延迟一段时间
        	    if((p_tx_frame->buff[5]<=0x36)&&((p_tx_frame->buff[5]>=0x30))){        	        
        	        control_set_scom2_baud(p_tx_frame->buff[5]-0x30);
        	    }
        	}
        	if(init_done_flag == 0){
        	    testbyte20++;    
        	}
        }else{
            if(init_done_flag){     //当初始化时，不可使用信号量否则会导致TCP无法连接。
                OSTimeDly(2);
            }else{                  //当初始化结束时，可以使用信号量来唤醒                            
                OSSemPend(hScom2TxLink->hTxMutex,0,&err);  //等待信号量    
            }
        }
//        led2_state = !led2_state;
//		if(led2_state){
//			LED2_ON;
//		}else{
//			LED2_OFF;
//		}
        //OSTimeDly(5);
    }
}
//当初始化结束时，置初始化完成标志位
void init_done_func(void)
{
    init_done_flag = 0;
    hScom2RxLink->rx_rd_index = hScom2RxLink->rx_wr_index;    
}

status_t scom2_task_create_f(void_t)
{        
    int nret;
    hScom2TxLink = &Scom2TxLink;
    hScom2TxLink->tx_rd_index = 0;
    hScom2TxLink->tx_wr_index = 0;    
    hScom2TxLink->hTxMutex = OSSemCreate(0);
    
    init_done_flag = 1;
    hScom2RxLink = &Scom2RxLink;
    hScom2RxLink->rx_rd_index = 0;
    hScom2RxLink->rx_wr_index = 0;
//    hScom2RxLink->hRxMutex = xSemaphoreCreateMutex();

    nret = OSTaskCreate(user2_scom2_send_thread,(void*)0,(OS_STK*)&USERSCOM2_SEND_TASK_STK[USERSCOM2_SEND_STK_SIZE-1],USERSCOM2_SEND_TASK_PRIORITY);//创建LED任务

    return 0;
}


status_t scom2_task_destroy_f(void_t)
{
    hScom2TxLink = NULL;
    hScom2RxLink = NULL;
    return 0;
}
