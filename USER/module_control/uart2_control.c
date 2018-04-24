
#include "uart2_control.h"
#include "ring_buffer.h"
#include "led.h"

//LED����
//�������ȼ�
#define USERSCOM2_SEND_TASK_PRIORITY		8
//�����ջ��С
#define USERSCOM2_SEND_STK_SIZE		64
//�����ջ
OS_STK	USERSCOM2_SEND_TASK_STK[USERSCOM2_SEND_STK_SIZE];
//������
void user2_scom2_send_thread(void *arg);  

static Scom2TxLink_S Scom2TxLink;
static Scom2TxLink_S * hScom2TxLink;


static Scom2RxLink_S Scom2RxLink;
static Scom2RxLink_S * hScom2RxLink;

#define SEND_BUF_SIZE 512
#define RECV_BUF_SIZE 1024
//���鶨�壬������������
u8 USART2_SEND_DATA[SEND_BUF_SIZE]; 
u8 USART2_RECEIVE_DATA[RECV_BUF_SIZE]; 
u8 USART2_TX_Finish=1; // USART2������ɱ�־��


u32 init_done_flag;     //�Ƿ��ʼ������������ʼ��ʱʹ���ź������ѣ�ֻ�еȴ���ʼ���ú����ʹ���ź�������.

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
    USART_InitStructure.USART_BaudRate = nret;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART2, &USART_InitStructure); //��ʼ������1
}




//DMA���ã�
void DMA_UART2_Configuration(void)
{
  MYDMA_Config(DMA1_Stream6,DMA_Channel_4,(u32)&USART2->DR,(u32)USART2_SEND_DATA,SEND_BUF_SIZE,DMA_DIR_MemoryToPeripheral);//DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.

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


//DMA���ã�
void DMA_UART2_RX_Configuration(void)
{
    MYDMA_Config(DMA1_Stream5,DMA_Channel_4,(u32)&USART2->DR,(u32)USART2_RECEIVE_DATA,RECV_BUF_SIZE,DMA_DIR_PeripheralToMemory);
  
}

//�ж����ȼ����ã�
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

//��ʼ��IO ����2 
//bound:������
void uart2_init(u32 bound)
{
   //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE); //ʹ��GPIODʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);//ʹ��USART2ʱ��

	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_USART2); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource6,GPIO_AF_USART2); //GPIOA10����ΪUSART1
	
	//USART2�˿�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOD,&GPIO_InitStructure); //��ʼ��PD5��PD6

   //USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART2, &USART_InitStructure); //��ʼ������1
	
    	
	//�����ж�
    USART_ITConfig(USART2, USART_IT_IDLE , ENABLE); //��������ж�
    USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���2
    USART_ClearFlag(USART2, USART_FLAG_TC);
    
	//Usart2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

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

//USART2�жϷ�����
void USART2_IRQHandler(void)
{
    uint16_t DATA_LEN;
    uint16_t i;
    WgRxNodeData_S * p_rx_frame;

	//LED2_OFF;
	//LED2_ON;
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //���Ϊ���������ж�
    {
        USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
		DMA_Cmd(DMA1_Stream5, DISABLE);//�ر�DMA,��ֹ�������������
		
        DATA_LEN=RECV_BUF_SIZE-DMA_GetCurrDataCounter(DMA1_Stream5); 
		
		if((DATA_LEN == 200) ||(DATA_LEN == 400) ||(DATA_LEN == 600) ){      //��ʱ���ܻ��յ�����������жϣ����Ի�������Ҫ���ó� ���֡*2.
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
        {                                 //��������DMA�洢��ַ
//            for(i=0;i<DATA_LEN;i++)
//            {
//                USART2_SEND_DATA[i]=USART2_RECEIVE_DATA[i];
//            }
//            //USART��DMA���������ѯ��ʽ���ͣ��˷��������ȼ��ж϶�������֡����
//            DMA_Cmd(DMA1_Stream6, DISABLE); //�ı�datasizeǰ��Ҫ��ֹͨ������
//            //DMA1_Stream6->CNDTR=DATA_LEN; //DMA1,����������
//            DMA_SetCurrDataCounter(DMA1_Stream6, DATA_LEN);
//            DMA_Cmd(DMA1_Stream6, ENABLE);                        
//        }
            p_rx_frame = &hScom2RxLink->rx_frame[hScom2RxLink->rx_rd_index];        
            p_rx_frame->len = DATA_LEN;
            //��������DMA�洢��ַ
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
        
        DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_FEIF5 | DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5);//���־
		USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
        DMA_Cmd(DMA1_Stream5, ENABLE);//������,�ؿ�DMA
                //��SR���DR���Idle
        i = USART2->SR;
        i = USART2->DR;
    }else{
		testbyte70++;
	}
    if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//����
    {
        USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE);
    }
    USART_ClearITPendingBit(USART2, USART_IT_TC);
    USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    //LED2_OFF;
}
uint32_t testbyte22=0;
uint32_t testbyte23=0;
//DMA1_Stream6�жϷ�����
//USART2ʹ��DMA�������жϷ������
void DMA1_Stream6_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6)){
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        DMA_Cmd(DMA1_Stream6, DISABLE);//�ر�DMA,��ֹ�������������
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

//���շ����������ٷ��͸�����
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



//��ȡ���ڻ������ĳ���
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

//���մ��������ٷ��͸�������
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
        if(hScom2TxLink->tx_wr_index != hScom2TxLink->tx_rd_index){ //������֡λ�ò���ͬʱ,��������
            //printf("tx_wr_index=%x,tx_rd_index=%x\n", hScom2TxLink->tx_wr_index, hScom2TxLink->tx_rd_index);
            usart2_tx_count = 0;             
            while(USART2_TX_Finish == 0){   //���仹δ����
                
                led2_state = !led2_state;
                if(led2_state){
                    LED2_ON;    
                }else{
                    LED2_OFF;    
                }
                
                OSTimeDly(1);               //2ms ��ʱ
                
                usart2_tx_count++;
				testbyte24++;
                if(usart2_tx_count >= MAX_TX_DELAY_TIMES){  //�����û�в���ʱ����Ҫ��ʼ��
                    uart2_init(115200);
                    testbyte25++;
                }
            }            
            p_tx_frame = &hScom2TxLink->tx_frame[hScom2TxLink->tx_rd_index];        
                        
            //��������DMA�洢��ַ
            for(i=0;i<p_tx_frame->len;i++)
            {
                USART2_SEND_DATA[i]=p_tx_frame->buff[i];
            }
            //USART��DMA���������ѯ��ʽ���ͣ��˷��������ȼ��ж϶�������֡����
            DMA_Cmd(DMA1_Stream6, DISABLE); //�ı�datasizeǰ��Ҫ��ֹͨ������
            DMA_SetCurrDataCounter(DMA1_Stream6, p_tx_frame->len);
            DMA_Cmd(DMA1_Stream6, ENABLE);    
            USART2_TX_Finish = 0;       //���ڷ����Ϳ�ʼ
        	hScom2TxLink->tx_rd_index++;
            if(hScom2TxLink->tx_rd_index >= WGTXLINKNODE_CNT){
        	    hScom2TxLink->tx_rd_index = 0;
        	}			    

        	//��Ϊ���Ĳ�����ʱ����Ҫ�ȷ�������֮���ٸ��Ĳ�����
        	if((p_tx_frame->buff[0]==0x7e)&&(p_tx_frame->buff[1]==0x34)&&(p_tx_frame->buff[2]==0x2A)){
        	    printf("pc set scom2 baudrate = %x\n", p_tx_frame->buff[5]);        //�ӳ�һ��ʱ��
        	    if((p_tx_frame->buff[5]<=0x36)&&((p_tx_frame->buff[5]>=0x30))){        	        
        	        control_set_scom2_baud(p_tx_frame->buff[5]-0x30);
        	    }
        	}
        	if(init_done_flag == 0){
        	    testbyte20++;    
        	}
        }else{
            if(init_done_flag){     //����ʼ��ʱ������ʹ���ź�������ᵼ��TCP�޷����ӡ�
                OSTimeDly(2);
            }else{                  //����ʼ������ʱ������ʹ���ź���������                            
                OSSemPend(hScom2TxLink->hTxMutex,0,&err);  //�ȴ��ź���    
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
//����ʼ������ʱ���ó�ʼ����ɱ�־λ
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

    nret = OSTaskCreate(user2_scom2_send_thread,(void*)0,(OS_STK*)&USERSCOM2_SEND_TASK_STK[USERSCOM2_SEND_STK_SIZE-1],USERSCOM2_SEND_TASK_PRIORITY);//����LED����

    return 0;
}


status_t scom2_task_destroy_f(void_t)
{
    hScom2TxLink = NULL;
    hScom2RxLink = NULL;
    return 0;
}
