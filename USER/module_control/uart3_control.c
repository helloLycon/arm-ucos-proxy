
#include "uart3_control.h"
#include "ring_buffer.h"
#include "led.h"
#include "hj_nm.h"
#include "malloc.h"
#include "lwip_comm.h" 
#include "usart1.h"	
#include "usr_prio.h"
#include "webserver_control_comm.h"
//LED����
//�������ȼ�
//#define USERSCOM3_SEND_TASK_PRIORITY		8
//�����ջ��С
#define USERSCOM3_SEND_STK_SIZE		256
//�����ջ
//OS_STK	* USERSCOM3_SEND_TASK_STK[USERSCOM3_SEND_STK_SIZE];
OS_STK	* USERSCOM3_SEND_TASK_STK;
//������
void user_scom3_send_thread(void *arg);  

//static Scom3TxLink_S Scom3TxLink;
Scom3TxLink_S * hScom3TxLink;


//static Scom3RxLink_S Scom3RxLink;
Scom3RxLink_S * hScom3RxLink;


//static CONTROL_S uartcontrol;
CONTROL_S * h_uartcontrol;

#define SEND_BUF_SIZE 512
#define RECV_BUF_SIZE 1024
//���鶨�壬������������
u8 USART3_SEND_DATA[SEND_BUF_SIZE]; 
u8 USART3_RECEIVE_DATA[RECV_BUF_SIZE]; 
u8 USART3_TX_Finish=1; // USART3������ɱ�־��

//static u32 stm32f407_update_start_flag;   //�Ƿ�����������־������������ʱ��Ҫ��ֹ���ڽ�������
//static u32 init_start_flag;     //�Ƿ����ڳ�ʼ��������ʼ��ʱʹ���ź������ѣ�ֻ�еȴ���ʼ���ú����ʹ���ź�������.
//static u32 scom_run_ok_flag;    //��������������־ 1������0������
//static u32 scom_run_ok_count;


//�������м���
//void inc_scom_run_ok_count(void)
//{
//    scom_run_ok_count++;
//    if(scom_run_ok_count > 1500){ //����Խ1����ʱ��1500��û���յ�ʱ����Ϣ����Ҫ�ݴ�
//        
//    }
//}

//�����̳�ʼ��������ʱδ��
status_t control_set_default(void)
{
    s32_t nret = 0; 

    return nret;
}
#if 1

status_t control_set_alterid_ext40(u8_t * p_buff)
{
    s32_t nret = 0;
    
	PARAM_UP_EXT40_S * p_ext40 = (PARAM_UP_EXT40_S *)p_buff;	
//	int i;

//	h_control->alterid_ramdom[0] = p_ramdom[0];		
//	h_control->alterid_ramdom[1] = p_ramdom[1];	
//	h_control->alterid_ramdom[2] = p_ramdom[2];	
//	h_control->alterid_ramdom[3] = p_ramdom[3];	
	
	p_ext40->id = main_config_get_main_device_id();
	p_ext40->type = 0x00;   
    nret = sizeof(PARAM_UP_EXT40_S);

    return nret;
}
#endif
void control_set_scom3_baud(u32 bound)
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
    USART_Init(USART3, &USART_InitStructure); //��ʼ������1
}




//DMA���ã�
void DMA_UART3_Configuration(void)
{
  MYDMA_Config(DMA1_Stream3,DMA_Channel_4,(u32)&USART3->DR,(u32)USART3_SEND_DATA,SEND_BUF_SIZE,DMA_DIR_MemoryToPeripheral);//DMA2,STEAM7,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.

  DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
  DMA_ITConfig(DMA1_Stream3, DMA_IT_TE, ENABLE);
  
  /* Enable USART3 DMA TX request */
  USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
  DMA_Cmd(DMA1_Stream3, DISABLE);
  
    MYDMA_Config(DMA1_Stream1,DMA_Channel_4,(u32)&USART3->DR,(u32)USART3_RECEIVE_DATA,RECV_BUF_SIZE,DMA_DIR_PeripheralToMemory);
  
  /* Enable USART3 DMA RX request */
  USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
  DMA_Cmd(DMA1_Stream1, ENABLE);
}


//DMA���ã�
void DMA_UART3_RX_Configuration(void)
{
    MYDMA_Config(DMA1_Stream1,DMA_Channel_4,(u32)&USART3->DR,(u32)USART3_RECEIVE_DATA,RECV_BUF_SIZE,DMA_DIR_PeripheralToMemory);
  
}

//�ж����ȼ����ã�
void DMA_UART3_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

  /*Enable DMA Channel6 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//��ʼ��IO ����2 
//bound:������
void uart3_init(u32 bound)
{
   //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE); //ʹ��GPIOBʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOCʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);//ʹ��USART3ʱ��

	//����3��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11����ΪUSART3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_USART3); //GPIOC10����ΪUSART3
	
	//USART3�˿�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; //GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����  
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOB,&GPIO_InitStructure); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //GPIOB10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOC,&GPIO_InitStructure); 

   //USART3 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART3, &USART_InitStructure); //��ʼ������1
	
    	
	//�����ж�
    USART_ITConfig(USART3, USART_IT_IDLE , ENABLE); //��������ж�
    USART_Cmd(USART3, ENABLE);  //ʹ�ܴ���3
    USART_ClearFlag(USART3, USART_FLAG_TC);
    
	//Usart3 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

    DMA_UART3_Configuration();
	DMA_UART3_NVIC_Configuration();

}

//int testbyte70=0;
//uint32_t testbyte10=0;
//uint32_t testbyte11=0;
//uint32_t testbyte12=0;
//uint32_t testbyte13=0;
//uint32_t testbyte14=0;
//uint32_t testbyte15=0;
//uint32_t testbyte16=0;


//USART3�жϷ�����
void USART3_IRQHandler(void)
{
    uint16_t DATA_LEN;
    uint16_t i;
    WgRxNodeData_S * p_rx_frame;

	//LED2_OFF;
	//LED2_ON;
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET) //���Ϊ���������ж�
    {
        USART_DMACmd(USART3, USART_DMAReq_Rx, DISABLE);
		DMA_Cmd(DMA1_Stream1, DISABLE);//�ر�DMA,��ֹ�������������
		
        DATA_LEN=RECV_BUF_SIZE-DMA_GetCurrDataCounter(DMA1_Stream1); 
		
//		if((DATA_LEN == 200) ||(DATA_LEN == 400) ||(DATA_LEN == 600) ){      //��ʱ���ܻ��յ�����������жϣ����Ի�������Ҫ���ó� ���֡*2.
//		    testbyte12++;
//            if(DATA_LEN == 400){
//               testbyte14++; 
//            }else if(DATA_LEN == 600){
//               testbyte15++;  
//            }
//		}else{
//		    testbyte13++;
//		}
        //testbyte11++;
		//testbyte16 += DATA_LEN;
        if(DATA_LEN > 0)
        {                                 //��������DMA�洢��ַ

            //led0_port_toggle();
//            for(i=0;i<DATA_LEN;i++)
//            {
//                USART3_SEND_DATA[i]=USART3_RECEIVE_DATA[i];
//            }
//            //USART��DMA���������ѯ��ʽ���ͣ��˷��������ȼ��ж϶�������֡����
//            DMA_Cmd(DMA1_Stream3, DISABLE); //�ı�datasizeǰ��Ҫ��ֹͨ������
//            //DMA1_Stream3->CNDTR=DATA_LEN; //DMA1,����������
//            DMA_SetCurrDataCounter(DMA1_Stream3, DATA_LEN);
//            DMA_Cmd(DMA1_Stream3, ENABLE);                        
//        }
            p_rx_frame = &hScom3RxLink->rx_frame[hScom3RxLink->rx_wr_index];        
            p_rx_frame->len = DATA_LEN;
            //��������DMA�洢��ַ
            for(i=0;i<p_rx_frame->len;i++)
            {
                p_rx_frame->buff[i] = USART3_RECEIVE_DATA[i];
            }
    		hScom3RxLink->rx_wr_index++;
            if(hScom3RxLink->rx_wr_index >= WGRXLINKNODE_CNT){
            	hScom3RxLink->rx_wr_index = 0;
            }	
        }
		
		
        DMA_SetCurrDataCounter(DMA1_Stream1, RECV_BUF_SIZE);
        DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
        DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TEIF1);
        
        DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_FEIF1 | DMA_FLAG_TCIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_DMEIF1);//���־
		USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
        DMA_Cmd(DMA1_Stream1, ENABLE);//������,�ؿ�DMA
                //��SR���DR���Idle
        i = USART3->SR;
        i = USART3->DR;
    //}else{
		//testbyte70++;
	}
    if(USART_GetITStatus(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//����
    {
        USART_ClearITPendingBit(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE);
    }
    USART_ClearITPendingBit(USART3, USART_IT_TC);
    USART_ClearITPendingBit(USART3, USART_IT_IDLE);
    //LED2_OFF;
}
//uint32_t testbyte22=0;
//uint32_t testbyte23=0;
//DMA1_Stream3�жϷ�����
//USART3ʹ��DMA�������жϷ������
void DMA1_Stream3_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3)){
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
        DMA_Cmd(DMA1_Stream3, DISABLE);//�ر�DMA,��ֹ�������������
        DMA_SetCurrDataCounter(DMA1_Stream3, SEND_BUF_SIZE);
//			if(init_start_flag == 0){
//					testbyte23++;
//			}
			USART3_TX_Finish = 1;
    }
    if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TEIF3)){
        printf("dma1 irq error\n");
        USART3_TX_Finish = 1;
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TEIF3);	
    }
//    if(init_start_flag == 0){
//        testbyte22++;
//    }
}
#define BASE_SOCKET_THREAD_ID 0x100



//�̰߳��ţ���work�߳�Ϊ��������udp��jump�̣߳�Ϊ���һ���߳�����
s32_t control_get_free_thread_num(void_t)
{
    int i, num = 0, id;
    uint8_t err = 0; 
    for(i=0;i<(MAXTHREADNUM-1);i++){
//        printf("i=%x, 111thread_id=%x\n",i, h_uartcontrol->threaddata[i].thread_id);
        OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err);  
        id = h_uartcontrol->threaddata[i].thread_id;
        OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
        if(id==0){
            num = i;
            break;
        }
    }
    return num;
}

s32_t control_get_thread_num_from_fd(s32_t fd)
{
    int i, num = 0, id;
    uint8_t err = 0; 
    for(i=0;i<(MAXTHREADNUM-1);i++){
//        printf("i=%x, 111thread_id=%x\n",i, h_uartcontrol->threaddata[i].thread_id);
        OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err);  
        id = h_uartcontrol->threaddata[i].thread_id;
        OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
        if(id == (fd+BASE_SOCKET_THREAD_ID)){
            num = i;
            break;
        }
    }
    return num;
}
s32_t control_get_fd_from_thread_num(s32_t thread_num)
{
    int i,socket,thread_id;
    uint8_t err = 0; 
    OSMutexPend(h_uartcontrol->threaddata[thread_num].hmutex,0,&err);  
    thread_id = h_uartcontrol->threaddata[thread_num].thread_id;
    OSMutexPost(h_uartcontrol->threaddata[thread_num].hmutex); 
    if(thread_id != 0){
        socket = thread_id - BASE_SOCKET_THREAD_ID;    
    }else{
        return -1;
    }    
    return socket;
}
s32_t control_get_fd_from_thread_id(s32_t thread_id)
{
    int i,socket;
    socket = thread_id - BASE_SOCKET_THREAD_ID;    
    return socket;
}
s32_t work_set_control_thread_num(s32_t socket, u32_t num)
{      
    uint8_t err = 0;        
    OSMutexPend(h_uartcontrol->threaddata[num].hmutex,0,&err); 
    h_uartcontrol->threaddata[num].thread_id = socket + BASE_SOCKET_THREAD_ID;   
    OSMutexPost(h_uartcontrol->threaddata[num].hmutex); 
//    h_uartcontrol->threaddata[num].p_socket_data = p_socket_data;
    return OK_T;
}

s32_t work_clear_control_thread_id(s32_t socket)
{
    int i, id, k;
    uint8_t err = 0;  
    hj_nm_cmd_frame_t *ptxback;
    struct hj_txback_hdr * ptxbackhdr;
    id = socket + BASE_SOCKET_THREAD_ID;
    
    for(i=0;i<(MAXTHREADNUM-1);i++){
//        printf("i=%x, 111thread_id=%x\n",i, h_uartcontrol->threaddata[i].thread_id);
        
            
        OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
        if(h_uartcontrol->threaddata[i].thread_id==id){
            h_uartcontrol->threaddata[i].thread_id = 0;
            
            ptxback = h_uartcontrol->threaddata[i].txback;	
      	    for(k=0;k<TXBACKSIZE;k++){
      		    ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);	
    	  		memset(ptxbackhdr,0,sizeof(struct hj_txback_hdr));
    	  		ptxback++;
      		}        
			// breakǰ���ͷŸû��������������
			OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
            break;
        }
        OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
    }
    return 0;
}

uint32_t testbyte21=0;

//���շ����������ٷ��͸�����
void Board_UARTX_Inset_Data_To_Link(uint8_t * pdata, uint32_t len)
{    
  	WgTxNodeData_S * p_tx_frame;
    uint8_t err = 0;  
    OSMutexPend(hScom3TxLink->hTxMutex,0,&err); 
    p_tx_frame = &hScom3TxLink->tx_frame[hScom3TxLink->tx_wr_index];
    OSMutexPost(hScom3TxLink->hTxMutex); 
    p_tx_frame->len = len;
    memcpy(p_tx_frame->buff, pdata, len);
    
//  	testbyte21++;  	
//  	if(init_start_flag){ //�����ڳ�ʼ��ʱ����ʹ���ź�������������Ⲣ��ֻ�г�ʼ���ɹ����ת�����ݷ���ת����
//  	    
//    }else{
        OSMutexPend(hScom3TxLink->hTxMutex,0,&err); 
        hScom3TxLink->tx_wr_index++;
        if(hScom3TxLink->tx_wr_index >= WGTXLINKNODE_CNT){
    	    hScom3TxLink->tx_wr_index = 0;
    	}
    	OSMutexPost(hScom3TxLink->hTxMutex); 
    	if(hScom3TxLink->hTxWaitFlag)   //����н��̵ȴ�����ȥ����
            OSSemPost(hScom3TxLink->hTxSem);
//    }
}
//void Init_Board_UARTX_Inset_Data_To_Link(void)
//{    
//    WgTxNodeData_S * p_tx_frame;
//    int len;
//    //int nret;
//    uint8_t frame_poll_baud[8]={0x7e,0x31,0x32,0x23};
//    len = 4;
//    p_tx_frame = &hScom3TxLink->tx_frame[hScom3TxLink->tx_wr_index];
//    p_tx_frame->len = len;
//    memcpy(p_tx_frame->buff, frame_poll_baud, len);
//    hScom3TxLink->tx_wr_index++;
//    if(hScom3TxLink->tx_wr_index >= WGTXLINKNODE_CNT){
//	    hScom3TxLink->tx_wr_index = 0;
//	}
//}

//u8 led1_state=0;
//��ȡ���ڻ������ĳ���
uint32_t Board_UARTX_RxFrame_GetCount(void)
{
    int nret = 0, i;
//    if((init_start_flag)){ //�����ڳ�ʼ��ʱ,û�����ݿ��Զ�
//        
//    }else{
        
//        led1_state = !led1_state;
//		if(led1_state){
//			LED1_ON;
//		}else{
//			LED1_OFF;
//		}
        
        NVIC_DisableIRQ(USART3_IRQn);
        if(hScom3RxLink->rx_wr_index != hScom3RxLink->rx_rd_index){
            nret = 1;
        }
        //i = hScom3RxLink->rx_wr_index;
        NVIC_EnableIRQ(USART3_IRQn);
        //printf("1wr=%x,rd=%x\r\n",i,hScom3RxLink->rx_rd_index);
//    }
    return nret;
}

//���մ��������ٷ��͸�������
void Board_UARTX_Send_To_Server(WgRxNodeData_S * p_tx_frame)
{    
    WgRxNodeData_S * p_rx_frame;
    struct hj_dmp_frame_tail * p_cmd_ack_tail;    
    struct hj_dmp_frame_head * p_cmd_ack_hdr;
    u8_t * p_tmp_buff;
	int i;
	p_cmd_ack_hdr = (struct hj_dmp_frame_head *)p_tx_frame->buff;
	
	p_tmp_buff = p_tx_frame->buff + sizeof(* p_cmd_ack_hdr);
	
    //xSemaphoreTake(txing_mutex, portMAX_DELAY);
    
    NVIC_DisableIRQ(USART3_IRQn);
    p_rx_frame = &hScom3RxLink->rx_frame[hScom3RxLink->rx_rd_index]; 
    
    hScom3RxLink->rx_rd_index++;
    if(hScom3RxLink->rx_rd_index >= WGRXLINKNODE_CNT){
	    hScom3RxLink->rx_rd_index = 0;
	}           
	//i = hScom3RxLink->rx_wr_index;
    NVIC_EnableIRQ(USART3_IRQn);
    
   
}







uint32_t testbyte20 = 0;
uint32_t testbyte24 = 0;
uint32_t testbyte25 = 0;
#define MAX_TX_DELAY_TIMES 100

static void user_scom3_send_thread(void *arg)
{
    int sendflag,i;
	u8 err;
    WgTxNodeData_S * p_tx_frame;
    
    uint32_t usart3_tx_count = 0;
    while(1){  
        sendflag = 0;
        OSMutexPend(hScom3TxLink->hTxMutex,0,&err); 
        if(hScom3TxLink->tx_wr_index != hScom3TxLink->tx_rd_index){ //������֡λ�ò���ͬʱ,��������
            sendflag = 1;
        }
        OSMutexPost(hScom3TxLink->hTxMutex); 
        if(sendflag){ //������֡λ�ò���ͬʱ,��������
            //printf("tx_wr_index=%x,tx_rd_index=%x\n", hScom3TxLink->tx_wr_index, hScom3TxLink->tx_rd_index);
            usart3_tx_count = 0;             
            while(USART3_TX_Finish == 0){   //���仹δ����
                

                //led2_port_toggle();
                OSTimeDly(1);               //2ms ��ʱ
                
                usart3_tx_count++;
				//testbyte24++;
                if(usart3_tx_count >= MAX_TX_DELAY_TIMES){  //�����û�в���ʱ����Ҫ��ʼ��
                    //uart3_init(115200, USART_StopBits_1);   //�����Ƿ���Ҫ����ȥȡpbx baud
                    //init_start_flag = 1;
            //      //testbyte25++;
                    break;
                }
            }          
            OSMutexPend(hScom3TxLink->hTxMutex,0,&err); 
            p_tx_frame = &hScom3TxLink->tx_frame[hScom3TxLink->tx_rd_index];   
            hScom3TxLink->tx_rd_index++;
            if(hScom3TxLink->tx_rd_index >= WGTXLINKNODE_CNT){
        	    hScom3TxLink->tx_rd_index = 0;
        	}   
            OSMutexPost(hScom3TxLink->hTxMutex); 
                    
            //��������DMA�洢��ַ
            for(i=0;i<p_tx_frame->len;i++)
            {
                USART3_SEND_DATA[i]=p_tx_frame->buff[i];
            }
            //USART��DMA���������ѯ��ʽ���ͣ��˷��������ȼ��ж϶�������֡����
            DMA_Cmd(DMA1_Stream3, DISABLE); //�ı�datasizeǰ��Ҫ��ֹͨ������
            DMA_SetCurrDataCounter(DMA1_Stream3, p_tx_frame->len);
            DMA_Cmd(DMA1_Stream3, ENABLE);    
            USART3_TX_Finish = 0;       //���ڷ����Ϳ�ʼ
        				    

        	//��Ϊ���Ĳ�����ʱ����Ҫ�ȷ�������֮���ٸ��Ĳ�����
//        	if((p_tx_frame->buff[0]==0x7e)&&(p_tx_frame->buff[1]==0x34)&&(p_tx_frame->buff[2]==0x2A)){
//        	    printf("pc set scom3 baudrate = %x\n", p_tx_frame->buff[5]);        //�ӳ�һ��ʱ��
//        	    if((p_tx_frame->buff[5]<=0x36)&&((p_tx_frame->buff[5]>=0x30))){        	        
//        	        //control_set_scom3_baud(p_tx_frame->buff[5]-0x30);
//        	    }
//        	}
//        	if(init_start_flag == 0){
//        	    testbyte20++;    
//        	}
        }else{
            //if(init_start_flag){     //�����ڳ�ʼ��ʱ������ʹ���ź�������ᵼ��TCP�޷����ӡ�
            //    OSTimeDly(2);
            //}else{                  //����ʼ������ʱ������ʹ���ź���������      
                hScom3TxLink->hTxWaitFlag = 1;               
                OSSemPend(hScom3TxLink->hTxSem,0,&err);  //�ȴ��ź���    
                hScom3TxLink->hTxWaitFlag = 0;      
            }
        }
//        led2_state = !led2_state;
//		if(led2_state){
//			LED2_ON;
//		}else{
//			LED2_OFF;
//		}
        //OSTimeDly(5);
//    }
}


void hj_nm_attach_dump_frame(struct hj_dmp_frame * p_dump_frame, hj_nm_cmd_frame_t * p_cmd_frame)
{
    //printf("hj_nm_attach_dump_frameaaa\n");
    p_dump_frame->phead = (struct hj_dmp_frame_head *)p_cmd_frame->data_buff;
    //printf("phead=%x\n",p_dump_frame->phead );
    p_dump_frame->pdata = (u8_t *)(&(p_dump_frame->phead->cmdLen)) + 1;
    //printf("pdata=%x\n",p_dump_frame->pdata );
    p_dump_frame->ptail = (struct hj_dmp_frame_tail *)(p_dump_frame->pdata + p_dump_frame->phead->cmdLen);
    //printf("ptail=%x\n",p_dump_frame->ptail );
}


void hj_nm_build_dump_frame(struct hj_dmp_frame * p_dump_frame, unsigned char * p_buff, int ndatalen)
{
    p_dump_frame->phead = (struct hj_dmp_frame_head *)p_buff;
	//printf("phead=%x\n", p_dump_frame->phead);
    p_dump_frame->pdata = (u8_t *)(&(p_dump_frame->phead->cmdLen)) + 2;
    //p_dump_frame->ptail = (struct hj_dmp_frame_tail *)(p_dump_frame->pdata + ndatalen);

}


void hj_nm_build_dyb_dump_frame(struct hj_dyb_dmp_frame * p_dump_frame, unsigned char * p_buff, int ndatalen)
{
    p_dump_frame->phead = (struct hj_dyb_dmp_frame_head *)p_buff;
	//printf("phead=%x\n", p_dump_frame->phead);
    p_dump_frame->pdata = (u8_t *)(&(p_dump_frame->phead->cmdLen)) + 2;
    //p_dump_frame->ptail = (struct hj_dmp_frame_tail *)(p_dump_frame->pdata + ndatalen);

}

//�����澯ͬ������֡��������֡�������һ�������߳�
status_t control_build_trapsync_frame(struct hj_dump_frame_head * ptrapsynchead)
{
    s32_t i,framelen;  
    u8_t pwdata[128], pcrc[128]; 
    struct hj_dyb_dmp_frame dmp_frame;
        
    hj_nm_build_dyb_dump_frame(&dmp_frame, pwdata,0);

    //dmp_frame.phead->pre = 0x7e;
    dmp_frame.phead->protocolVer = ptrapsynchead->protocolVer;
    dmp_frame.phead->thread_netid = THREAD_TRAPSYNC;
    dmp_frame.phead->src_deviceid = ptrapsynchead->src_deviceid;
	dmp_frame.phead->dst_deviceid = ptrapsynchead->dst_deviceid;
	//printf("dst_deviceid=%x\n",dmp_frame.phead->dst_deviceid);
    dmp_frame.phead->boardIndex = ptrapsynchead->boardIndex;
    dmp_frame.phead->interfaceIndex = ptrapsynchead->interfaceIndex;
    dmp_frame.phead->server_seq = 0x01;
    dmp_frame.phead->cmdId = ptrapsynchead->cmdId;
    dmp_frame.phead->cmdExt = ptrapsynchead->cmdExt;
    dmp_frame.phead->cmdStatus = 0;
    dmp_frame.phead->cmdLen = 0;
    framelen = sizeof(struct hj_dump_frame_head);
    framelen = hj15_crc_7e(pwdata,framelen,pcrc);
    
    Board_UARTX_Inset_Data_To_Link(pcrc, framelen);    
    return OK_T;
}

//�����澯ͬ������֡��������֡�������һ�������߳�
status_t control_build_st_frame(struct hj_dump_frame_head * phead)
{
    s32_t i,framelen;  
    u8_t pwdata[128], pcrc[128];
    u16_t src_deviceid;

    struct hj_dyb_dmp_frame dmp_frame;
    
    hj_nm_build_dyb_dump_frame(&dmp_frame, pwdata,0);

    dmp_frame.phead->protocolVer = 0x01;
    dmp_frame.phead->thread_netid = THREAD_ST;

	dmp_frame.phead->src_deviceid = phead->src_deviceid;  
	dmp_frame.phead->dst_deviceid = phead->dst_deviceid;
    dmp_frame.phead->boardIndex = phead->boardIndex;
    dmp_frame.phead->interfaceIndex = phead->interfaceIndex;
    dmp_frame.phead->server_seq = 0;
    dmp_frame.phead->cmdId = phead->cmdId;
    dmp_frame.phead->cmdExt = phead->cmdExt;
    dmp_frame.phead->cmdStatus = phead->cmdStatus;
    dmp_frame.phead->cmdLen = phead->cmdLen;
    
    memcpy((u8_t *)(&dmp_frame.phead->cmdLen)+2, (u8_t *)(&phead->cmdLen)+2, phead->cmdLen);
    
    framelen = sizeof(struct hj_dyb_dmp_frame_head)+phead->cmdLen;
    //printf("xxxxframelen=%x, cmdLen=%x\n", framelen, phead->cmdLen);    
    framelen = hj15_crc_7e(pwdata,framelen,pcrc);
    Board_UARTX_Inset_Data_To_Link(pcrc, framelen);   
    return OK_T;
}


#define BOOT_ALARM_ID 36
static status_t control_build_boot_report_frame(u8_t * p_recv_good_frame)
{
    u8_t pwdata[128], pcrc[128];
    u32_t framelen;
    u8_t * p_pdata;
    struct hj_dmp_frame dmp_frame;
    struct trap_frame_body framebody;    
    struct hj_dyb_dump_frame_head *p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame; //ȥ��0x7e

    hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
    dmp_frame.phead->protocolVer = p_head->protocolVer;
    dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
    dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
    dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
    dmp_frame.phead->src_deviceid = p_head->src_deviceid;  
    
    dmp_frame.phead->boardIndex = p_head->boardIndex;
    dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
    dmp_frame.phead->server_seq = 0x01;
    dmp_frame.phead->cmdId = 0x71;
    dmp_frame.phead->cmdExt = 0x01;
    dmp_frame.phead->cmdStatus = 0;
    dmp_frame.phead->cmdLen = sizeof(struct trap_frame_body);
         
	framebody.netunit_ipaddr = main_config_get_main_host_ip(); 
    framebody.netunit_port = main_config_get_main_host_port();
    framebody.reserve_word = 0;
    framebody.board_status = 4;
	framebody.netunit_status = 4;
	        
	framebody.frame_index = 0;
	framebody.board_index = p_head->boardIndex;
	framebody.board_type = 0;
	framebody.interface_index = p_head->interfaceIndex;
	framebody.interface_type = 0;
	framebody.alarm_serial = 0;
	framebody.alarm_id = BOOT_ALARM_ID; 
	framebody.alarm_type = 0;
	framebody.alarm_status = 1;
	framebody.alarm_level = 4;
	p_pdata = pwdata;
    p_pdata += sizeof(struct hj_dmp_frame_head);
    memcpy(p_pdata, &framebody, sizeof(struct trap_frame_body));
    
    framelen = sizeof(struct hj_dmp_frame_head) + sizeof(struct trap_frame_body);
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
	//debug_dump(pcrc, framelen, "trap:"); 
    trap_send_buf(pcrc, framelen);              //д��trap������
}
static status_t control_build_jump_frame(u8_t * p_recv_good_frame)
{
    u8_t pwdata[128], pcrc[128];
    u32_t framelen;
    u8_t * p_pdata;
    struct hj_dmp_frame dmp_frame;
    //u32_t trap_ip,trap_port;
    struct hj_dyb_dump_frame_head *p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame; //ȥ��0x7e

    hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
    dmp_frame.phead->protocolVer = p_head->protocolVer;
    dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
    dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
    dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
    dmp_frame.phead->src_deviceid = p_head->src_deviceid;    
    dmp_frame.phead->boardIndex = p_head->boardIndex;
    dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
    dmp_frame.phead->server_seq = 0x01;
    dmp_frame.phead->cmdId = p_head->cmdId;
    dmp_frame.phead->cmdExt = p_head->cmdExt;
    dmp_frame.phead->cmdStatus = p_head->cmdStatus;
    dmp_frame.phead->cmdLen = p_head->cmdLen;
    
	//debug_dump(pwdata, sizeof(struct hj_dmp_frame_head), "jump:"); 
    framelen = sizeof(struct hj_dmp_frame_head);
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
    //debug_dump(pcrc, framelen, "jump:"); 
    jump_send_buf(pcrc, framelen);              //д��trap������
    return OK_T;
}
//�����澯ͬ����������֡
static status_t control_build_trapsync_end_frame(u8_t * p_recv_good_frame)
{
    u8_t pwdata[128], pcrc[128];
    u8_t * p_pdata; 
    u32_t framelen;
    struct hj_dmp_frame dmp_frame;
    struct hj_dyb_dump_frame_head *p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame;
    
    hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
    dmp_frame.phead->protocolVer = p_head->protocolVer;
    dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
    dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
    dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
    dmp_frame.phead->src_deviceid = p_head->src_deviceid;    
    dmp_frame.phead->boardIndex = p_head->boardIndex;
    dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
    dmp_frame.phead->server_seq = 0x01;
    dmp_frame.phead->cmdId = p_head->cmdId;
    dmp_frame.phead->cmdExt = p_head->cmdExt;
    dmp_frame.phead->cmdStatus = p_head->cmdStatus;
    dmp_frame.phead->cmdLen = p_head->cmdLen;

    framelen = sizeof(struct hj_dmp_frame_head) ;
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);

    jump_send_buf(pcrc, framelen);              //д��trap������

    return OK_T;
}

//�����澯����֡
static status_t control_build_trap_frame(u8_t * p_recv_good_frame)
{
    u8_t pwdata[128], pcrc[128];
    u32_t framelen;
    u8_t * p_pdata;
    struct hj_dmp_frame dmp_frame;
	struct trap_frame_body framebody;
    struct hj_dyb_dump_frame_head * p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame; //ȥ��0x7e
    //memcpy(pwdata, p_head, sizeof(struct hj_dyb_dump_frame_head)); //frame head
    //p_head = (struct hj_dyb_dump_frame_head *)pwdata;              //�ָ�������ARM��ͨ��Э��
	hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
	dmp_frame.phead->protocolVer = p_head->protocolVer;
	dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
	dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
	dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
	dmp_frame.phead->src_deviceid = p_head->src_deviceid;  
	dmp_frame.phead->boardIndex = p_head->boardIndex;
	dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
	dmp_frame.phead->server_seq = 0x01;
	dmp_frame.phead->cmdId = p_head->cmdId;
	dmp_frame.phead->cmdExt = p_head->cmdExt;
	dmp_frame.phead->cmdStatus = p_head->cmdStatus;
	dmp_frame.phead->cmdLen = p_head->cmdLen;
	
    p_pdata = (u8_t *)p_recv_good_frame;
    p_pdata += sizeof(struct hj_dyb_dump_frame_head);
    

    //main_config_get_trap(&trap_ip, &trap_port);
    
    memcpy((u8_t *)&framebody, p_pdata, sizeof(struct trap_frame_body)); //�ݲ�����0X7Eβ
    
    //debug_dump(&framebody, sizeof(struct trap_frame_body), "framebody:"); 
    
	framebody.netunit_ipaddr = main_config_get_main_host_ip(); 
    framebody.netunit_port = main_config_get_main_host_port();
    //framebody.device_index = 0;
    framebody.reserve_word = 0;
    //framebody.time = time(NULL);
    
    //printf("alarm_id=%x\n", framebody.alarm_id);
        
    p_pdata = pwdata;
    p_pdata += sizeof(struct hj_dmp_frame_head);
    memcpy(p_pdata, (u8_t *)&framebody, sizeof(struct trap_frame_body));
    
    framelen = sizeof(struct hj_dmp_frame_head) + sizeof(struct trap_frame_body);
    //debug_dump(pwdata, framelen, "trap:");
    //printf("framelen=%x\n", framelen);
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
	//debug_dump(pcrc, framelen, "trap:"); 
    trap_send_buf(pcrc, framelen);              //д��trap������

    return OK_T;
}





//�����澯����֡
static status_t control_build_selftest_frame(u8_t * p_recv_good_frame)
{
    u8_t pwdata[128], pcrc[128];
    u32_t framelen;
    //u16_t src_deviceid;
    u8_t * p_srcdata, * p_dstdata;
    struct hj_dmp_frame dmp_frame;
    struct hj_dyb_dump_frame_head * p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame; //ȥ��0x7e
    //struct dyb_trap_frame_body * p_dyb_trap_frame_body;
    //memcpy(pwdata, p_head, sizeof(struct hj_dyb_dump_frame_head)); //frame head
    //p_head = (struct hj_dyb_dump_frame_head *)pwdata;              //�ָ�������ARM��ͨ��Э��
	hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
	dmp_frame.phead->protocolVer = p_head->protocolVer;
	dmp_frame.phead->dst_netid = PC_DEVICE_ID;    
	dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
	dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;

	dmp_frame.phead->src_deviceid = p_head->src_deviceid;  
	dmp_frame.phead->boardIndex = p_head->boardIndex;
	dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
	
	dmp_frame.phead->server_seq = 0x01;
	dmp_frame.phead->cmdId = p_head->cmdId;
	dmp_frame.phead->cmdExt = p_head->cmdExt;
	dmp_frame.phead->cmdStatus = p_head->cmdStatus;
	dmp_frame.phead->cmdLen = p_head->cmdLen;
	
    p_srcdata = (u8_t *)p_recv_good_frame;
    p_srcdata += sizeof(struct hj_dyb_dump_frame_head);
    //printf("p_head->cmdLen=%x\n", p_head->cmdLen);
    p_dstdata = pwdata;
    p_dstdata += sizeof(struct hj_dmp_frame_head);
    memcpy(p_dstdata, p_srcdata, p_head->cmdLen);
    
    framelen = sizeof(struct hj_dmp_frame_head) + p_head->cmdLen;

    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
    
    st_send_buf(pcrc, framelen);              //д��selftest������

    return OK_T;
}


//p_recv_good_frame �������Ѿ�����7e��CRC 
static int control_recv_cmd(u8_t * p_recv_good_frame, u16_t len)
{
    int i,j,n,k,socket_fd;
    uint8_t err = 0;  
    //hj_nm_cmd_frame_t *prxframe;
    char *prxbyte;
    struct hj_dyb_dump_frame_head *p_head;
    hj_nm_cmd_frame_t *ptxback;
	hj_nm_cmd_frame_t *prxframe;
	static u8_t buff[512], pcrc[512];
	struct hj_txback_hdr * ptxbackhdr;
	int txlen, txback_count;
	char *pdata;
	//txback_count = 0;
    p_head = (struct hj_dyb_dump_frame_head *)p_recv_good_frame;

    i = p_head->cmdId;
    
    i >>= 4;    
    //printf("cmd=%x\n", i);
    if((i < 6)){
        printf("cmd err\n");
        return 0;
    }
    //printf("p_head->cmdLen=%x\n", p_head->cmdLen);
    if(p_head->cmdLen > 0x100){
		printf("cmd len err\n");
        return 0;
	}
	//printf("thread_netid=%#x\r\n", p_head->thread_netid);
	if(p_head->thread_netid < 0x100){   //���ذ���ARM����ģ���ͨ��
	
    	switch(p_head->thread_netid)            //trap
    	{	    
    	    case THREAD_TRAP:
                control_build_trap_frame(p_recv_good_frame);             
                break;
    //	    case THREAD_POLL:
    //            i = p_head->cmdId;
    //    		i >>= 4;
    //            if(i == DYBACKARMPOLLCMD){
    //                control_get_poll_data(p_recv_good_frame);
    //            }          
    //            break;
            case THREAD_TRAPSYNC_END:
                control_build_trapsync_end_frame(p_recv_good_frame);     
                break;
            case THREAD_JUMP:
                control_build_jump_frame(p_recv_good_frame);       
                break;
    //        case THREAD_IMPORT:
    //            ftp_recv_import_frame(p_recv_good_frame, p_head->server_seq, p_head->cmdExt, len);     
    //            break;
    //        case THREAD_UPDATE:
    //            ftp_recv_isp_frame(p_recv_good_frame, p_head->server_seq, p_head->cmdExt, len);
    //            
    //            break;
    //        case THREAD_EXPORT:
    //            ftp_recv_export_frame(p_recv_good_frame, p_head->server_seq, p_head->cmdExt, len);    
    //            break;
            case THREAD_ST:
                control_build_selftest_frame(p_recv_good_frame);     
                break;            
            case THREAD_BOOT_REPORT:
                control_build_boot_report_frame(p_recv_good_frame);		
                break;            
            case THREAD_IMPORT:
                ftp0_recv_import_frame(p_recv_good_frame, 0, p_head->cmdExt, len);       
                break;
                
            case THREAD_EXPORT:              
                ftp0_recv_export_frame(p_recv_good_frame, 0, p_head->cmdExt, len);     
                break;
            case THREAD_UPDATE:
                ftp0_recv_isp_frame(p_recv_good_frame, 0, p_head->cmdExt, len);    
                break;
    //        case THREAD_1_IMPORT:
    //            ftp_recv_import_frame(p_recv_good_frame, 1, p_head->cmdExt, len);       
    //            break;
    //        case THREAD_1_EXPORT:              
    //            ftp_recv_export_frame(p_recv_good_frame, 1, p_head->cmdExt, len);     
    //            break;
    //        case THREAD_1_UPDATE:
    //            ftp_recv_isp_frame(p_recv_good_frame, 1, p_head->cmdExt, len);    
    //            break;    
    //        case THREAD_2_IMPORT:
    //            ftp_recv_import_frame(p_recv_good_frame, 2, p_head->cmdExt, len);       
    //            break;
    //        case THREAD_2_EXPORT:              
    //            ftp_recv_export_frame(p_recv_good_frame, 2, p_head->cmdExt, len);     
    //            break;
    //        case THREAD_2_UPDATE:
    //            ftp_recv_isp_frame(p_recv_good_frame, 2, p_head->cmdExt, len);    
    //            break;    
    //        case THREAD_3_IMPORT:
    //            ftp_recv_import_frame(p_recv_good_frame, 3, p_head->cmdExt, len);       
    //            break;
    //        case THREAD_3_EXPORT:              
    //            ftp_recv_export_frame(p_recv_good_frame, 3, p_head->cmdExt, len);     
    //            break;
    //        case THREAD_3_UPDATE:
    //            ftp_recv_isp_frame(p_recv_good_frame, 3, p_head->cmdExt, len);    
    //            break;  
    	    default:
                break;
        }
    }
	else{          //��Ҫת��������
		//20171008
		//web������ץȡ��Ԫ������֡
		if(ctrl_dyb_frame_capture(p_head))
			return 0;
		for(i=0;i<MAXTHREADNUM;i++){
            OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
            ptxback = h_uartcontrol->threaddata[i].txback;	
            OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
            for(j=0;j<TXBACKSIZE;j++){
                OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
                ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);	
                OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 		
          		if(ptxbackhdr->pre!=0){
          		    //txback_count ++;
                }else{
                    continue;
                }
                //printf("p_head->thread_netid=%x,ptxbackhdr->thread_id=%x\n", p_head->thread_netid, ptxbackhdr->thread_id);
                //printf("p_head->server_seq=%x,ptxbackhdr->seq=%x\n", p_head->server_seq, ptxbackhdr->seq);
                if((p_head->thread_netid==ptxbackhdr->thread_id)&&(p_head->server_seq==ptxbackhdr->seq)){	//when the same,send the data to pc			
                    
                    txlen = sizeof(struct hj_txback_hdr)-17;		//ȥ��ĩβ12�ֽ�+0x7e�ֽ�  ��ȥ������ţ�����ȵ�4�ֽ�  -13-4
    				//printf("txlen111=%x\n",txlen);
    				//printf("111dst_deviceid=%x,src_deviceid=%x\n", ptxbackhdr->dst_deviceid, ptxbackhdr->src_deviceid);
//    				k = ptxbackhdr->src_deviceid;
//    				ptxbackhdr->src_deviceid = ptxbackhdr->dst_deviceid;
//    				ptxbackhdr->dst_deviceid = k;
    				//printf("222dst_deviceid=%x,src_deviceid=%x\n", ptxbackhdr->dst_deviceid, ptxbackhdr->src_deviceid);
    				
//    				k = ptxbackhdr->dst_netid;
//    				ptxbackhdr->dst_netid = ptxbackhdr->thread_netid;
//    				ptxbackhdr->thread_netid = k;							
    				
                	memcpy(buff,&ptxbackhdr->protocolVer,txlen);//ȥ��0x7e,������֡
                	//debug_dump(buff, txlen, "66send:");
                	OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
    				memset(ptxbackhdr,0,sizeof(struct hj_txback_hdr));
    				OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 	
    				pdata = &buff[txlen];
                    memcpy(pdata,(&p_head->cmdId),p_head->cmdLen+4);	//ȡ����ţ�����ȵ�4�ֽ�                       
                    txlen += p_head->cmdLen+4;				//���������ܳ���	��Ҫ���¼���CRC 
                    //printf("txlen222=%x,phead->cmdId=%x\n",txlen,phead->cmdId);
    				//debug_dump(buff, txlen, "111send:");					
    			    txlen = hj15_crc_7e_notxor_20(buff,txlen,pcrc);
    				//printf("txlen333=%x\n",txlen);            
    				socket_fd = control_get_fd_from_thread_id(p_head->thread_netid);        	                        
    				send(socket_fd, pcrc, txlen, 0);  			
    //							#ifdef DEBUG
                            	//if(usart1_debug(HJ80_DEBUG_PC_SEND)){
                                     //debug_dump(pcrc, txlen, "send:");                                      
                            	//}
    //                        	#endif	   			  						
                    
                    
                }
                ptxback++;
            }
            
//            if(TXBACKSIZE==txback_count){   //�ݴ�,�����ͻ�������ʱ����շ��ͻ�����
//                OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
//          	    ptxback = h_uartcontrol->threaddata[i].txback;	
//          	    
//          	    for(k=0;k<TXBACKSIZE;k++){
//          		    ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);	
//        	  		memset(ptxbackhdr,0,sizeof(struct hj_txback_hdr));
//        	  		ptxback++;
//          		}
//          		OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
//          	} 
        }
    }
}

//�ݴ���鷢�������Ƿ�ʱ������ʱ��ɾ��������ֻ������
void control_check_send_cmd(void)
{
    int i,j,n,k;
    hj_nm_cmd_frame_t *ptxback;
	struct hj_txback_hdr * ptxbackhdr;
	int txback_count[MAXTHREADNUM];
	uint8_t err = 0;  
	
    
    for(i=0;i<MAXTHREADNUM;i++){
        txback_count[i] = 0;    
        //OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
        ptxback = h_uartcontrol->threaddata[i].txback;	
        //OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
        for(j=0;j<TXBACKSIZE;j++){
            OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
            ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);	
            OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 		
      		if(ptxbackhdr->pre!=0){
      		    txback_count[i] ++;
  		    		
  		    	OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
  		    	ptxbackhdr->times++;
  		    	OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
  				if(ptxbackhdr->times>60){		//timeout then send the data to pc
  					printf("timeout\n");
  					//ptxbackhdr->cmdStatus = Error_UI_TIMEOUT;
  					//ptxbackhdr->cmdLen = 0;  		
  					OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 	                                  	                        		
  					//memset(ptxbackhdr,0,sizeof(struct hj_txback_hdr));
  					ptxbackhdr->pre = 0;
  					OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
  				}
            }                
            ptxback++;
        }
        if(TXBACKSIZE==txback_count[i]){   //�ݴ�,�����ͻ�������ʱ����շ��ͻ�����
            printf("txback_count full\n");
            OSMutexPend(h_uartcontrol->threaddata[i].hmutex,0,&err); 
      	    ptxback = h_uartcontrol->threaddata[i].txback;	
      	    
      	    for(k=0;k<TXBACKSIZE;k++){
      		    ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);	
    	  		memset(ptxbackhdr,0,sizeof(struct hj_txback_hdr));
    	  		ptxback++;
      		}
      		OSMutexPost(h_uartcontrol->threaddata[i].hmutex); 
      	} 
    }
}

static status_t uartcrcprocess(UART_S *uart_rec_data)
{
    u8_t  x;
    //printf("tttrx_rd_index=%x,rx_wr_index=%x\n", uart_rec_data->rx_rd_index, uart_rec_data->rx_wr_index);
    while(uart_rec_data->rx_rd_index!=uart_rec_data->rx_wr_index)
    {
        //printf("ppprx_rd_index=%x,rx_wr_index=%x\n", uart_rec_data->rx_rd_index, uart_rec_data->rx_wr_index);
        x=uart_rec_data->rx_buff[uart_rec_data->rx_rd_index];
        uart_rec_data->rx_rd_index=(++uart_rec_data->rx_rd_index)&UART_RX_FUFF_SIZE;
        if(x==0x7e){
            //          uart_rec_data->UARTFrameHeadFlag=~uart_rec_data->UARTFrameHeadFlag;
            //printf("head=%x\n", uart_rec_data->UARTFrameHeadFlag);
            if(uart_rec_data->UARTFrameHeadFlag){
                uart_rec_data->UARTFrameHeadFlag=0;
            }else{
                uart_rec_data->UARTFrameHeadFlag=1;
            }
            if(uart_rec_data->UARTFrameHeadFlag){//frame head
                uart_rec_data->ComputerGoodFrameCounter=0;
                uart_rec_data->UARTfcsReceive=PPPINITFCS16;
                uart_rec_data->ComputerGoodFrame[(uart_rec_data->ComputerGoodFrameCounter++)] = x;   
            }else{//frame tail
                //printf("999rx_rd_index=%x,rx_wr_index=%x\n", uart_rec_data->rx_rd_index, uart_rec_data->rx_wr_index);
                if(uart_rec_data->UARTfcsReceive==PPPGOODFCS16){
                    uart_rec_data->UARTGoodFrameFlag=1;
                    uart_rec_data->ComputerGoodFrame[(uart_rec_data->ComputerGoodFrameCounter++)] = x;   
                    
                    //uart_process_rec_data();
                    //printf("counter=%x\n", uart_rec_data->ComputerGoodFrameCounter);
                    //debug_dump(uart_rec_data->ComputerGoodFrame, uart_rec_data->ComputerGoodFrameCounter, "goodframe:");      
                    control_recv_cmd(uart_rec_data->ComputerGoodFrame, uart_rec_data->ComputerGoodFrameCounter);
                    uart_rec_data->UARTGoodFrameFlag=0;
                    //uart_rec_data->ComputerGoodFrameCounter-=2;//discard crc  ���Լ�
                    //return OK_T;
                }else{
                    uart_rec_data->ComputerGoodFrameCounter=0;
                    uart_rec_data->UARTGoodFrameFlag=0;
                    //return ERROR_T;
                }
                //break;
            }
        }
        else
        {
            if(uart_rec_data->UARTFrameHeadFlag)
            {//data
                if(x!=0x7d)
                {
                    if(uart_rec_data->UARTEscapeFlag)
                    {
                        x=x^0x20;
                        uart_rec_data->UARTEscapeFlag=0;
                    }
                    uart_rec_data->UARTfcsReceive=FCS(uart_rec_data->UARTfcsReceive,x);
                    uart_rec_data->ComputerGoodFrame[(uart_rec_data->ComputerGoodFrameCounter++)]=x;
                }
                else
                {
                    uart_rec_data->UARTEscapeFlag=1;
                }
            }
            else
            {//error
                //printf("crc error\n");
                uart_rec_data->UARTFrameHeadFlag=1;
                uart_rec_data->ComputerGoodFrameCounter=0;
                uart_rec_data->UARTfcsReceive=PPPINITFCS16;
                if(x!=0x7d)
                {
                    if(uart_rec_data->UARTEscapeFlag)
                    {
                        x=x^0x20;
                        uart_rec_data->UARTEscapeFlag=0;
                    }
                    uart_rec_data->UARTfcsReceive=FCS(uart_rec_data->UARTfcsReceive,x);
                    uart_rec_data->ComputerGoodFrame[(uart_rec_data->ComputerGoodFrameCounter++)]=x;
                }
                else
                {
                    uart_rec_data->UARTEscapeFlag=1;
                }
            }
        }
    }
    //printf("yyyrx_rd_index=%x,rx_wr_index=%x,size=%x\n", uart_rec_data->rx_rd_index, uart_rec_data->rx_wr_index,UART_RX_FUFF_SIZE);
}
#if 1
int uartrecvProc(void)
{
    int ncount, i;  
    WgRxNodeData_S * p_rx_frame;  
    static u8_t uart_recv[BUFFERSIZE];
    u32_t recv_len;
    u32_t index;

    ncount = Board_UARTX_RxFrame_GetCount();
    if(ncount > 0){   //��������ʱ
        NVIC_DisableIRQ(USART3_IRQn);
        p_rx_frame = &hScom3RxLink->rx_frame[hScom3RxLink->rx_rd_index];    
        hScom3RxLink->rx_rd_index++;
        if(hScom3RxLink->rx_rd_index >= WGRXLINKNODE_CNT){
    	    hScom3RxLink->rx_rd_index = 0;
    	}
    	NVIC_EnableIRQ(USART3_IRQn);
        for(i=0;i<p_rx_frame->len;i++)
        {
            uart_recv[i] = p_rx_frame->buff[i] ;
        }
        recv_len = p_rx_frame->len;
	            
        if((h_uartcontrol->uart.rx_wr_index+recv_len)>=BUFFERSIZE)
        {
        	index=BUFFERSIZE-h_uartcontrol->uart.rx_wr_index;
            memcpy(&h_uartcontrol->uart.rx_buff[h_uartcontrol->uart.rx_wr_index],uart_recv,index);
            memcpy(h_uartcontrol->uart.rx_buff,&uart_recv[index],(recv_len-index));
            h_uartcontrol->uart.rx_wr_index = recv_len-index;
        	//printf("yyyrx_wr_index=%x\n", h_control->uart.rx_wr_index);
        }else{
            //printf("nnnrx_wr_index=%x\n", h_control->uart.rx_wr_index);
        	memcpy(&h_uartcontrol->uart.rx_buff[h_uartcontrol->uart.rx_wr_index],uart_recv,recv_len);             
            //printf("iiirx_wr_index=%x, recv_len=%x\n", h_control->uart.rx_wr_index, recv_len);
            h_uartcontrol->uart.rx_wr_index = h_uartcontrol->uart.rx_wr_index+recv_len;
        }
        uartcrcprocess(&h_uartcontrol->uart);
        
        
    }


    return OK_T;
}
#endif



static status_t control_init_threadmsg(void_t)
{
	INT8U err;
	u32_t i, j, k;
	for(i=0;i<MAXTHREADNUM;i++){		

		for(j=0;j<TXBACKSIZE;j++){
			memset(h_uartcontrol->threaddata[i].txback[j].data_buff, 0, HJ_NM_CMD_FRAME_DATA_LEN);
		}
//		for(j=0;j<TXFRAMESIZE;j++){
//			memset(h_uartcontrol->threaddata[i].txframe[j].data_buff, 0, HJ_NM_CMD_FRAME_DATA_LEN);
//		}
		h_uartcontrol->threaddata[i].thread_id = 0;
		h_uartcontrol->threaddata[i].txback_wr = 0;
		h_uartcontrol->threaddata[i].txback_rd = 0;
//		h_uartcontrol->threaddata[i].txframe_wr = 0;
//		h_uartcontrol->threaddata[i].txframe_rd = 0;	
//		h_uartcontrol->threaddata[i].hmutex = OSSemCreate(0);
		h_uartcontrol->threaddata[i].hmutex = OSMutexCreate(THREAD_MUTEX_PRIO_START+i,&err);

		os_obj_create_check("h_uartcontrol->threaddata[i].hmutex", "control_init_threadmsg", err);
	
	}
	h_uartcontrol->threaddata[MISCTHREADNUM].thread_id = MISCTHREADNUM;
	return OK_T;
}
	

static status_t control_destroy_threadmsg(void_t)
{		
}

static status_t control_init_uart(void_t)
{
    u32_t i;
//    for(i = 0; i < UART_BUFF_LEN; i++){
//        h_uartcontrol->uart.tx_buff[i] = 0;

//    }
    for(i = 0; i < UART_GOOD_BUFF_LEN; i++){
			  h_uartcontrol->uart.rx_buff[i] = 0;
        h_uartcontrol->uart.ComputerGoodFrame[i] = 0;
    }	
//    h_uartcontrol->uart.tx_wr_index = 0;
//    h_uartcontrol->uart.tx_rd_index = 0;
    h_uartcontrol->uart.rx_wr_index = 0;
    h_uartcontrol->uart.rx_rd_index = 0;
    h_uartcontrol->uart.UARTGoodFrameFlag = 0;
    h_uartcontrol->uart.UARTEscapeFlag = 0;
    h_uartcontrol->uart.UARTFrameHeadFlag = 0;
    h_uartcontrol->uart.UARTfcsReceive = 0;
    h_uartcontrol->uart.ComputerGoodFrameCounter = 0;
    return OK_T;
}

//����ʼ������ʱ���ó�ʼ����ɱ�־λ
void control_set_init_start_end_flags(void)
{
	uint8_t err = 0;  
//    init_start_flag = 0;
    
    OSMutexPend(hScom3TxLink->hTxMutex,0,&err); 
    hScom3RxLink->rx_rd_index = hScom3RxLink->rx_wr_index;    
    hScom3TxLink->tx_wr_index = hScom3TxLink->tx_rd_index;
    OSMutexPost(hScom3TxLink->hTxMutex); 
}

//uint32_t control_get_init_start_flag(void)
//{
//    return init_start_flag;
//}




status_t scom3_task_create_f(void_t)
{        
    INT8U err;
    
    //h_uartcontrol = &uartcontrol;
    h_uartcontrol = mymalloc(SRAMCCM,sizeof(CONTROL_S));
    
    control_init_uart();
    control_init_threadmsg();
    //hScom3TxLink = &Scom3TxLink;
    hScom3TxLink = mymalloc(SRAMCCM,sizeof(Scom3TxLink_S));
    hScom3TxLink->tx_rd_index = 0;
    hScom3TxLink->tx_wr_index = 0;    
    hScom3TxLink->hTxSem = OSSemCreate(0);
	/* check semaphore */
	os_obj_create_check("hScom3TxLink->hTxSem","scom3_task_create_f",((hScom3TxLink->hTxSem)?OS_ERR_NONE:(OS_ERR_NONE+1)));
	
    hScom3TxLink->hTxWaitFlag = 0;
    //hScom3TxLink->hTxMutex = OSSemCreate(0);
    hScom3TxLink->hTxMutex = OSMutexCreate(SCOM3_THREAD_MUTEX_PRIO, &err);
	/* do check */
	os_obj_create_check("hScom3TxLink->hTxMutex","scom3_task_create_f",err);
	
//    init_start_flag = 0;
//    scom_run_ok_count = 0;
//    scom_run_ok_flag = 0;
    //hScom3RxLink = &Scom3RxLink;
    hScom3RxLink = mymalloc(SRAMCCM,sizeof(Scom3RxLink_S));
    hScom3RxLink->rx_rd_index = 0;
    hScom3RxLink->rx_wr_index = 0;
    
    USERSCOM3_SEND_TASK_STK = mymalloc(SRAMCCM,USERSCOM3_SEND_STK_SIZE*4);
//    hScom3RxLink->hRxMutex = xSemaphoreCreateMutex();

    err = OSTaskCreate(user_scom3_send_thread,(void*)0,(OS_STK*)&USERSCOM3_SEND_TASK_STK[USERSCOM3_SEND_STK_SIZE-1],USERSCOM3_SEND_TASK_PRIORITY);//����LED����
	/* do check */
	os_obj_create_check("user_scom3_send_thread","scom3_task_create_f",err);
	
    return 0;
}


status_t scom3_task_destroy_f(void_t)
{
    hScom3TxLink = NULL;
    hScom3RxLink = NULL;
    return 0;
}


