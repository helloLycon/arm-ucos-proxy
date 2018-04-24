



#include "wgcmd_ftp.h"
#include "ftpclient.h" 
#include "useriap.h"
#include "malloc.h"
#include "ring_buffer.h"
//#include "usart1.h"	
#include "mainconfig.h"
#include "usr_prio.h"
#include "usart.h"

static hj_nm_cmd_frame_t ack_frame;

#define STM32F407_MODULE_BOARD_TYPE 0x800d0001


//LED任务
//任务优先级
//#define USERFTP_TASK_PRIORITY		12
//任务堆栈大小
#define USERFTP_TASK_STK_SIZE		1024
//任务堆栈
//OS_STK	USERFTP_TASK_STK[USERFTP_TASK_STK_SIZE];

OS_STK	* USERFTP_TASK_STK;

#define USERFTP0_THREAD_NAME      "user_ftp0_thread"
  


//Ftp_S FtpManager;

/* Transmit and receive ring buffers */
RINGBUFF_T tx_trap_ring;
/* Transmit and receive ring buffer sizes */

/* Transmit and receive buffers */
uint8_t tx_trap_buff[TX_TRAP_SIZE];
//Ftp_S FtpManager;

#define TX_EXPORT_SIZE 256

/* Transmit and receive ring buffers */
RINGBUFF_T tx_export_ring;
/* Transmit and receive ring buffer sizes */

/* Transmit and receive buffers */
uint8_t tx_export_buff[TX_EXPORT_SIZE];
//Ftp_S FtpManager;



Ftp_S * hFtpManager = NULL;

#define FLASH_PORT_BYTES_CNT 88


//u8_t databuff[128];

//设置命令帧数据
status_t ftp0_work_set_isp_cmd_frame_buff(u8_t * buf, u32_t len, int socket)
{
    s32_t retb;
    struct hj_dump_frame_head * p_cmd_ack_hdr;
    struct hj_dump_frame_head * p_cmd_hdr = (struct hj_dump_frame_head *)buf;
    struct hj_dump_frame_tail  *p_cmd_tail;
    ftpcmddata_S * p_ftpcmddata;
    u8_t * p_cmd_ack;
    u8_t ack_frame[sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail)];
    u32_t buff_len;
    u16_t  temp_data; 
    buff_len = sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail);
    p_cmd_ack = ack_frame;
    p_cmd_ack_hdr = (struct hj_dump_frame_head *)(p_cmd_ack);
    p_cmd_ack_hdr->pre = 0x7e;
    
    p_cmd_ack_hdr->dst_deviceid = p_cmd_hdr->src_deviceid;       
    p_cmd_ack_hdr->src_deviceid = p_cmd_hdr->dst_deviceid;
    p_cmd_ack_hdr->protocolVer = p_cmd_hdr->protocolVer;
    p_cmd_ack_hdr->thread_netid = p_cmd_hdr->thread_netid;
    
    p_cmd_ack_hdr->boardIndex = p_cmd_hdr->boardIndex;

    p_cmd_ack_hdr->interfaceIndex = p_cmd_hdr->interfaceIndex;        
    p_cmd_ack_hdr->cmdExt = p_cmd_hdr->cmdExt;
    p_cmd_ack_hdr->cmdId = p_cmd_hdr->cmdId+(0x01<<4);     //命令号加1
    
    p_cmd_ack_hdr->cmdStatus = Error_UI_OK;    //成功
    
    p_cmd_ack_hdr->cmdLen = 0;
    
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_cmd_ack + sizeof(* p_cmd_ack_hdr));
    temp_data  =  hj_calc_crc (p_cmd_ack+1,buff_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;
		
		
	//memcpy(databuff, p_cmd_ack, buff_len);
    memcpy(&hFtpManager->Threaddata[0].ackframe.data_buff, p_cmd_ack, buff_len);
    send(socket, p_cmd_ack, buff_len, 0);       //先回TCP再回复UDP信息
      
    if(hFtpManager->Threaddata[0].flag_busy == 0){
        hFtpManager->Threaddata[0].flag_busy = 1;
        hFtpManager->Threaddata[0].flag_isp = 1;    
        //printf("ftp0_work_set_isp_cmd_frame_buff\n");     
        debug_puts(DBG_INFO,"ftp0_work_set_isp_cmd_frame_buff\r\n");     
        SPI_Flash_Update_Erase();   //擦除空间     
        //printf("1111ftp_get_state=%x\n", ftp_get_state(0));
        memcpy(hFtpManager->Threaddata[0].cmd_frame_buff, buf, len); 
       // p_ftpcmddata = (ftpcmddata_S *)(&(p_cmd_hdr->cmdLen)+sizeof(u16_t));
	    p_ftpcmddata = (ftpcmddata_S *)((unsigned char *)&p_cmd_hdr->cmdLen+2);
        memcpy(&hFtpManager->Threaddata[0].ftp_cmddata,p_ftpcmddata, sizeof(ftpcmddata_S));
            	    	
        if((p_cmd_hdr->dst_deviceid&0xff) == LOCAL_DEVICE_ID){	//是局端设备还是远端设备
    		hFtpManager->Threaddata[0].localorremote = 1;
    	}else{		
    		hFtpManager->Threaddata[0].localorremote = 0;
    	}     	    	
        if(hFtpManager->Threaddata[0].Thread_wait_flag)            	    	
            OSSemPost(hFtpManager->Threaddata[0].ftpnotempty);
        //printf("0::localorremote=%x\n", hFtpManager->localorremote);

        //debug_dump(p_ftpcmddata, sizeof(ftpcmddata_S), "p_ftpcmddata:");   
    }
    
    
}


//设置命令帧数据
status_t ftp0_work_set_import_cmd_frame_buff(u8_t * buf, u32_t len, int socket)
{
    s32_t retb;
    struct hj_dump_frame_head * p_cmd_ack_hdr;
    struct hj_dump_frame_head * p_cmd_hdr = (struct hj_dump_frame_head *)buf;
    struct hj_dump_frame_tail  *p_cmd_tail;
    ftpcmddata_S * p_ftpcmddata;
    u8_t * p_cmd_ack;
    u8_t ack_frame[sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail)];
    u32_t buff_len;
    u16_t  temp_data; 
    buff_len = sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail);
    p_cmd_ack = ack_frame;
    p_cmd_ack_hdr = (struct hj_dump_frame_head *)(p_cmd_ack);
    p_cmd_ack_hdr->pre = 0x7e;
    
    p_cmd_ack_hdr->dst_deviceid = p_cmd_hdr->src_deviceid;       
    p_cmd_ack_hdr->src_deviceid = p_cmd_hdr->dst_deviceid;
    p_cmd_ack_hdr->protocolVer = p_cmd_hdr->protocolVer;
    p_cmd_ack_hdr->thread_netid = p_cmd_hdr->thread_netid;
    
    p_cmd_ack_hdr->boardIndex = p_cmd_hdr->boardIndex;

    p_cmd_ack_hdr->interfaceIndex = p_cmd_hdr->interfaceIndex;        
    p_cmd_ack_hdr->cmdExt = p_cmd_hdr->cmdExt;
    //p_cmd_ack_hdr->cmdId = p_cmd_hdr->cmdId;     //命令号加1
    p_cmd_ack_hdr->cmdId = p_cmd_hdr->cmdId+(0x01<<4);     //命令号加1
    
    p_cmd_ack_hdr->cmdStatus = Error_UI_OK;    //成功
    
    p_cmd_ack_hdr->cmdLen = 0;
    
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_cmd_ack + sizeof(* p_cmd_ack_hdr));
    temp_data  =  hj_calc_crc (p_cmd_ack+1,buff_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;
    memcpy(hFtpManager->Threaddata[0].ackframe.data_buff, p_cmd_ack, buff_len);
    send(socket, p_cmd_ack, buff_len, 0);       //先回TCP再回复UDP信息
      
    if(hFtpManager->Threaddata[0].flag_busy == 0){
        hFtpManager->Threaddata[0].flag_busy = 1;
        hFtpManager->Threaddata[0].flag_import = 1;    
        //printf("ftp0_work_set_import_cmd_frame_buff\n");       
        debug_puts(DBG_INFO,"ftp0_work_set_import_cmd_frame_buff\r\n");       
        //printf("1111ftp_get_state=%x\n", ftp_get_state(0));
        SPI_Flash_Import_Erase();
        memcpy(hFtpManager->Threaddata[0].cmd_frame_buff, buf, len); 
       // p_ftpcmddata = (ftpcmddata_S *)(&(p_cmd_hdr->cmdLen)+sizeof(u16_t));
	    p_ftpcmddata = (ftpcmddata_S *)((unsigned char *)&p_cmd_hdr->cmdLen+2);
        memcpy(&hFtpManager->Threaddata[0].ftp_cmddata,p_ftpcmddata, sizeof(ftpcmddata_S));
        if(hFtpManager->Threaddata[0].Thread_wait_flag)      
            OSSemPost(hFtpManager->Threaddata[0].ftpnotempty);

        //debug_dump(p_ftpcmddata, sizeof(ftpcmddata_S), "p_ftpcmddata:");   
    }
}


//设置命令帧数据
status_t ftp0_work_set_export_cmd_frame_buff(u8_t * buf, u32_t len, int socket)
{
    s32_t retb;
    struct hj_dump_frame_head * p_cmd_ack_hdr;
    struct hj_dump_frame_head * p_cmd_hdr = (struct hj_dump_frame_head *)buf;
    struct hj_dump_frame_tail  *p_cmd_tail;
    ftpcmddata_S * p_ftpcmddata;
    u8_t * p_cmd_ack;
    u8_t ack_frame[sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail)];
    u32_t buff_len;
    u16_t  temp_data; 
    buff_len = sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail);
    p_cmd_ack = ack_frame;
    p_cmd_ack_hdr = (struct hj_dump_frame_head *)(p_cmd_ack);
    p_cmd_ack_hdr->pre = 0x7e;
    
    p_cmd_ack_hdr->dst_deviceid = p_cmd_hdr->src_deviceid;       
    p_cmd_ack_hdr->src_deviceid = p_cmd_hdr->dst_deviceid;
    p_cmd_ack_hdr->protocolVer = p_cmd_hdr->protocolVer;
    p_cmd_ack_hdr->thread_netid = p_cmd_hdr->thread_netid;
    
    p_cmd_ack_hdr->boardIndex = p_cmd_hdr->boardIndex;

    p_cmd_ack_hdr->interfaceIndex = p_cmd_hdr->interfaceIndex;        
    p_cmd_ack_hdr->cmdExt = p_cmd_hdr->cmdExt;
    //p_cmd_ack_hdr->cmdId = p_cmd_hdr->cmdId;     //命令号加1
    p_cmd_ack_hdr->cmdId = p_cmd_hdr->cmdId+(0x01<<4);     //命令号加1
    p_cmd_ack_hdr->cmdStatus = Error_UI_OK;    //成功
    
    p_cmd_ack_hdr->cmdLen = 0;
    
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_cmd_ack + sizeof(* p_cmd_ack_hdr));
    temp_data  =  hj_calc_crc (p_cmd_ack+1,buff_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;
    memcpy(hFtpManager->Threaddata[0].ackframe.data_buff, p_cmd_ack, buff_len);
    send(socket, p_cmd_ack, buff_len, 0);       //先回TCP再回复UDP信息
      
    if(hFtpManager->Threaddata[0].flag_busy == 0){
        hFtpManager->Threaddata[0].flag_busy = 1;
        hFtpManager->Threaddata[0].flag_export = 1;    
        //printf("ftp0_work_set_export_cmd_frame_buff\n");       
        debug_puts(DBG_INFO,"ftp0_work_set_export_cmd_frame_buff\r\n");       
        //printf("1111ftp_get_state=%x\n", ftp_get_state(0));
        memcpy(hFtpManager->Threaddata[0].cmd_frame_buff, buf, len); 
       // p_ftpcmddata = (ftpcmddata_S *)(&(p_cmd_hdr->cmdLen)+sizeof(u16_t));
	    p_ftpcmddata = (ftpcmddata_S *)((unsigned char *)&p_cmd_hdr->cmdLen+2);
        memcpy(&hFtpManager->Threaddata[0].ftp_cmddata,p_ftpcmddata, sizeof(ftpcmddata_S));
        if(hFtpManager->Threaddata[0].Thread_wait_flag)          	              	    	
            OSSemPost(hFtpManager->Threaddata[0].ftpnotempty);

        //debug_dump(p_ftpcmddata, sizeof(ftpcmddata_S), "p_ftpcmddata:");   
    }
}

status_t work_set_ftp_cmd_data(u32_t cmdExt, u8_t * buff, u32_t len, int socket)
{
    switch(cmdExt){
        case UPDATE_CMDEXT:
            ftp0_work_set_isp_cmd_frame_buff(buff, len, socket);
            break;
        case IMPORT_CMDEXT:
            ftp0_work_set_import_cmd_frame_buff(buff, len, socket);
            break;
        case EXPORT_CMDEXT:
            ftp0_work_set_export_cmd_frame_buff(buff, len, socket);
            break;
        default:
            break;   
    }    
}



//
//status_t ftp0_arm_update(UpdateFilehead_S * p_filehead, u32_t type)
//{
//    status_t nret = OK_T; 
//    int i;
//	u32_t j, k;   
//    u8_t program_buf[88];
//    //u8_t temp_buf[100];
//    FILE *fp1;
//	FILE *fp2;
//    fp1=fopen(local_ftp0_file,"rb");
//    
//	printf("step1\n");
//	fseek(fp1,sizeof(UpdateFilehead_S),SEEK_SET);  //设置到实际文件开头
//	if(type == 1){
//	    fp2=fopen(TEMP_APP_UPDATEFILE,"wb+");
//	}else if(type == 2){	
//	    fp2=fopen(TEMP_DRIVER_UPDATEFILE,"wb+");
//	}	
//    if(fp2)
//	{
//	    fseek(fp2,0,SEEK_SET);
//	    printf("step2\n");
//	    while(!feof(fp1))
//		{
//			i = fread(program_buf,sizeof(char),sizeof(program_buf),fp1);
//			//printf("step2 i=%x\n", i);
//			//debug_dump(program_buf, i, "1111program_buf:"); 
//			for(j=0;j<i;j++){
//			    program_buf[j] ^= arm_password[j];     //相异或后再写入
//		    }
//		    //debug_dump(program_buf, i, "2222program_buf:"); 
//			fwrite(program_buf,sizeof(char),i,fp2);
//		}
//		fclose(fp2);
//		hFtpManager->flag_isp_arm = 1;
//	}else{
//	    fclose(fp2); 
//	}	
//	if(p_filehead->tar == 2){          //没有压缩
//	    //printf("step3\n");
//        nret = OK_T;       
//		printf("update success!\n");   	
//								    
//	}
//    return nret;
//}  


int ftp0_isp_result_send(u32_t status)
{
    u32_t frame_len;
    u8_t * p_cmd_data;
    u8_t *p_ack_buff = (u8_t *)hFtpManager->Threaddata[0].ackframe.data_buff;
    struct hj_dump_frame_head *p_cmd_ack_hdr ; //去除0x7e
    struct hj_dump_frame_tail  *p_cmd_tail;   
    u16_t  temp_data;  

    if(status == 0){
        //printf("ftp0_isp_result_send::update success =%x\n", status);
        debug_printf(DBG_INFO,"ftp0_isp_result_send::update success =%x\r\n", status);
    }else{
        //printf("ftp0_isp_result_send::result=%x\n", status);
        debug_printf(DBG_ERR,"ftp0_isp_result_send::result=%x\r\n", status);
    }
//    debug_dump(p_ack_buff, 32, "111ftp0_isp_result_send:"); 
    p_cmd_ack_hdr = (struct hj_dump_frame_head * )p_ack_buff;
    
    
    //p_cmd_ack_hdr->dst_deviceid = PC_DEVICE_ID;
    //p_cmd_ack_hdr->src_deviceid = spi_control_get_deviceid();
    p_cmd_ack_hdr->pre = 0x7e;
    //p_cmd_ack_hdr->protocolVer = PROTOCOL_VER;
    //p_cmd_ack_hdr->dst_netid = 0;       
    //p_cmd_ack_hdr->thread_netid = 0;
    //temp_data = p_cmd_ack_hdr->src_deviceid;
    //temp_data = p_cmd_ack_hdr->dst_deviceid;
    //printf("p_cmd_ack_hdr->src_deviceid=%x\n", p_cmd_ack_hdr->src_deviceid);
    //p_cmd_ack_hdr->src_deviceid = p_cmd_ack_hdr->dst_deviceid;
    //printf("p_cmd_ack_hdr->dst_deviceid=%x\n", p_cmd_ack_hdr->dst_deviceid);
    //p_cmd_ack_hdr->dst_deviceid = temp_data;    
    //p_cmd_ack_hdr->boardIndex = 0;
    //p_cmd_ack_hdr->server_seq = 0;   
	p_cmd_ack_hdr->cmdStatus = status;
    p_cmd_ack_hdr->cmdExt = 0x03;       //命令编号为103
    p_cmd_ack_hdr->cmdId = 0x71;        //命令号为7
    p_cmd_ack_hdr->cmdLen = 8;
            
    frame_len = sizeof(struct hj_dump_frame_head)+3+8;
    hFtpManager->Threaddata[0].ackframe.n_data_len = frame_len;
	p_cmd_data = ((u8_t *)&p_cmd_ack_hdr->cmdLen+2);
	
	p_cmd_data[0] = 0;
	p_cmd_data[1] = 0;
	p_cmd_data[2] = 0;
	p_cmd_data[3] = 0;
	p_cmd_data[4] = 0;
	p_cmd_data[5] = 0;
	p_cmd_data[6] = 0;
	p_cmd_data[7] = 0;
	//memcpy(p_ack_buff, hFtpManager->ackframe.data_buff, sizeof(struct hj_dmp_frame_head));		
	//memcpy(p_ack_buff+sizeof(* p_cmd_ack_hdr), p_cmd_data, 8);
	            
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_ack_buff + sizeof(* p_cmd_ack_hdr)+8);
    temp_data  =  hj_calc_crc (p_ack_buff+1,frame_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;         

    //if(usart1_debug(HJ80_DEBUG_FTP)){
    //    debug_dump(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len, "ftp0_isp_result_send:"); 
    //}
    jump_send_buf(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len);              //写入trap缓冲区    

    return 0;
}


#define FTP_THREAD0_SEQ 0
#define LEN_ONCE_FRAME 80       //当分成两次时，第一次发送的字节个数

status_t ftp0_build_frame(u32_t type, u8_t cmdext, u8_t * pdata, u32_t datalen, u32_t frame_16_end)
{
	int i, framelen, board_index;
	status_t nret = OK_T;
	u8_t * p_pdata;
	static u8_t pwdata[256],pcrc[256];
    static struct hj_dyb_dmp_frame dmp_frame;
    static struct hj_dump_frame_head * p_head;
    
    p_head = (struct hj_dump_frame_head * )hFtpManager->Threaddata[0].cmd_frame_buff; //////需要读取网管帧的板位号
    
//    p_head->dst_deviceid = 0;       //////          局端第四块FX4单元板模拟
//    p_head->boardIndex = 3;         //////
//    p_head->interfaceIndex = 0;     //////
//    
//    p_head->dst_deviceid = 48;       //////          第四块FX4单元板第一个远端设备
//    p_head->boardIndex = 0;         //////              主控板
//    p_head->interfaceIndex = 0;     //////
//    
//    p_head->dst_deviceid = 48;       //////          第四块FX4单元板第一个远端设备
//    p_head->boardIndex = 4;         //////              第4块用户板
//    p_head->interfaceIndex = 0;     //////
    //printf("dst_deviceid=%x, boardIndex=%x, interfaceIndex=%x\n",p_head->dst_deviceid,p_head->boardIndex, p_head->interfaceIndex);
    hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
    
//    if(hFtpManager->Threaddata[0].localorremote){   //local 以线程号来区分线程 因为ld程序已固定
//        dmp_frame.phead->server_seq = 0;
//        if(type == UPDATE_CMDEXT){
//            dmp_frame.phead->thread_netid = THREAD_0_UPDATE;        
//            dmp_frame.phead->cmdId = CMD_FTP<<4;
//            
//        }else if(type == IMPORT_CMDEXT){
//            dmp_frame.phead->thread_netid = THREAD_0_IMPORT;
//            dmp_frame.phead->cmdId = CMD_FTP<<4;
//        }else if(type == EXPORT_CMDEXT){
//            dmp_frame.phead->thread_netid = THREAD_0_EXPORT;
//            dmp_frame.phead->cmdId = CMD_FTP<<4;
//        }
//    }else{                                          //remote 以序列号来区分线程                                                
        dmp_frame.phead->server_seq = 0;    
        if(type == UPDATE_CMDEXT){
            dmp_frame.phead->thread_netid = THREAD_UPDATE;        
            dmp_frame.phead->cmdId = CMD_FTP<<4;
            
        }else if(type == IMPORT_CMDEXT){
            dmp_frame.phead->thread_netid = THREAD_IMPORT;
            dmp_frame.phead->cmdId = CMD_FTP<<4;
        }else if(type == EXPORT_CMDEXT){
            dmp_frame.phead->thread_netid = THREAD_EXPORT;
            dmp_frame.phead->cmdId = CMD_FTP<<4;
        }
//    }

//	if(usart1_debug(HJ80_DEBUG_FTP)){
//        printf("ftp0_build_frame::dmp_frame.phead:::seq=%x\n", dmp_frame.phead->server_seq);
//    }

    
    dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
    dmp_frame.phead->src_deviceid = p_head->src_deviceid;
    dmp_frame.phead->protocolVer = 0x01;
    hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext = cmdext;
    
    
    //memcpy(&dmp_frame.phead->boardIndex,&p_head->boardIndex,p_head->cmdLen+8);
    dmp_frame.phead->boardIndex = p_head->boardIndex;
    dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
    //dmp_frame.phead->server_seq = p_head->server_seq;
    //dmp_frame.phead->cmdExt = p_head->cmdExt;
    dmp_frame.phead->cmdExt = cmdext;    
    if(frame_16_end){
        dmp_frame.phead->cmdStatus = 0x80;    
    }else{
        dmp_frame.phead->cmdStatus = 0x00;    
    }    
    dmp_frame.phead->cmdLen = datalen;
    
    p_pdata = (u8_t *)(&dmp_frame.phead->cmdLen)+sizeof(dmp_frame.phead->cmdLen); //定位到数据部分   

    memcpy(p_pdata, pdata, datalen);       //////   
      
	framelen = sizeof(struct hj_dyb_dmp_frame_head);
	framelen += datalen;
	//debug_dump(pwdata, framelen, "pwdata:");	
    framelen = hj15_crc_7e(pwdata,framelen,pcrc); 
	//printf("2221framelen=%x\n", framelen);
	//debug_dump(pcrc, framelen, "pcrc:");
    
    //if(usart1_debug(HJ80_DEBUG_FTP)){	    
    //    debug_dump(pcrc, framelen, "1111pcrc:");         
    //}

	Board_UARTX_Inset_Data_To_Link(pcrc, framelen);  		
		
		
	return OK_T;
}


typedef struct{
    
    u8_t verison;
    u32_t deviceid;
    u8_t unuse[7];

} __attribute__((packed)) hj_update_start_data_t;

#define UPDATE_START_TIMES 20
#define THREAD_WAIT_TIMEOUT 800
#define UPDATE_START_VALUE 0x57
status_t ftp0_create_send_update_start_frame(void)
{
    u32_t i, k;
    u8 err;
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;   
    s32_t retb; 
    
    //struct hj_dyb_dump_frame_head * p_frame = pwdata;   
    //hj_update_start_data_t * p_cmd_data = p_frame->buf;   
    //xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);
    //printf("isp_create_send_update_start_frame::deviceid=%x\n", p_cmd_data->deviceid);
    
    for(i=0;i<UPDATE_START_TIMES;i++){      //等待10s 终端应用程序收到后会重启，ld程序收到后就进入ISP。
            
//            #ifdef DEBUG
//            if(usart1_debug(HJ80_DEBUG_FTP)){	    
//                    printf("ld:i=%x\n",i);
//            }
//            #endif	
            pwdata[0] = hFtpManager->Threaddata[0].ftp_board_type;
            pwdata[1] = hFtpManager->Threaddata[0].ftp_board_type>>8;
            pwdata[2] = hFtpManager->Threaddata[0].ftp_board_type>>16;
            pwdata[3] = hFtpManager->Threaddata[0].ftp_board_type>>24;
            for(k=4;k<64;k++){        
                pwdata[k] = UPDATE_START_VALUE;        
            }
            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_START, pwdata, 64, 1);
//            retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//            printf("retb=%x\n", retb);
//            gettimeofday(&ftptv, 0);
//            ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(THREAD_WAIT_TIMEOUT);
//            ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
		    hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
		    OSSemPend(hFtpManager->Threaddata[0].threadnotempty,THREAD_WAIT_TIMEOUT,&err);  
//		    cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
             
		    hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//		    if (retb == OK_T)
//                mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex);

            if (err == OS_ERR_TIMEOUT){       //超时
                continue;
            }else if(err == OS_ERR_NONE){        //唤醒
                break;
            }      
    }        
    if(i == UPDATE_START_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
}


#define PASSWORD_CONFIRM_TIMES 5
#define PASSWORD_THREAD_WAIT_TIMEOUT 1000


status_t ftp0_create_send_update_password_confirm_frame(void)
{
    u32_t i, k, len;
    u8 err;
    u8_t pwdata[128];
    u8_t * p_pdata;  
    status_t nret = OK_T; 
    
    struct hj_dyb_dump_frame_head * p_frame = (struct hj_dyb_dump_frame_head * )pwdata;   
    s32_t retb;
    hj_update_start_data_t * p_cmd_data;   
    
    p_pdata = (u8_t *)(&p_frame->cmdLen)+sizeof(p_frame->cmdLen); //定位到数据部分   
    p_cmd_data = (hj_update_start_data_t * )p_pdata;  
   
//    nret = xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);
//    if(nret != OK_T){
//        return ERROR_T;
//    }    
    len = RingBuffer_GetCount(&hFtpManager->Threaddata[0].ring);
    if(len > 0){
        RingBuffer_PopMult(&hFtpManager->Threaddata[0].ring, pwdata, len);
    }else{
        return ERROR_T;
    }
    
    k = p_cmd_data->deviceid ;
    
//    #ifdef DEBUG
//        if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//            printf("deviceid=%x\n", p_cmd_data->deviceid);
//        }    
//    #endif	
    //printf("uniqueid=%x\n", p_cmd_data->uniqueid);
    //printf("k=%x\n", k);
    pwdata[0] = k;
    pwdata[1] = k>>8;
    pwdata[2] = k>>16;
    pwdata[3] = k>>24;    
    
    for(i=0;i<PASSWORD_CONFIRM_TIMES;i++){
        
            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_PASSWORD_CONFIRM, pwdata, 4, 1);
//            retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//            gettimeofday(&ftptv, 0);
//            ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(PASSWORD_THREAD_WAIT_TIMEOUT);
//            ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
			OSSemPend(hFtpManager->Threaddata[0].threadnotempty,PASSWORD_THREAD_WAIT_TIMEOUT,&err);  
//            cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
            hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//            if (retb == OK_T)
//                mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
            if (err == OS_ERR_TIMEOUT){       //超时
                continue;
            }else if(err == OS_ERR_NONE){        //唤醒
                break;
            }       
    }
    if(i == PASSWORD_CONFIRM_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
    return nret;
}


#define FLASH_ERASE_TIMES 5
#define ERASE_THREAD_WAIT_TIMEOUT 5000
status_t ftp0_create_send_update_erase_frame(void)
{
    u32_t i, k;
    u8 err;
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    status_t nret = OK_T, retb; 
    
    pwdata[0] = 1;
    pwdata[1] = 2;
    pwdata[2] = 3;
    pwdata[3] = 4;
    for(i=0;i<FLASH_ERASE_TIMES;i++){
            
            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_ERASE, pwdata, 4, 1);
            
       
            //retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
            //gettimeofday(&ftptv, 0);
            //ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(ERASE_THREAD_WAIT_TIMEOUT);
            //ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
            //cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
            OSSemPend(hFtpManager->Threaddata[0].threadnotempty,ERASE_THREAD_WAIT_TIMEOUT,&err);  
            hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
            //if (retb == OK_T)
            //    mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
            if (err == OS_ERR_TIMEOUT){       //超时
                continue;
            }else if(err == OS_ERR_NONE){        //唤醒
                break;
            }           
    }
    //printf("erase FLASH_ERASE_TIMES\n");
    if(i == FLASH_ERASE_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
    return nret;
}

#define FLASH_ONCE_PROGRAM_BYTES_CNT 88  //每次编程时的字节数
#define FLASH_PROGRAM_TIMES 5
#define PROGRAM_THREAD_WAIT_TIMEOUT 2000
status_t ftp0_create_send_update_program_frame(void)
{
    u32_t i, k, ret,sendcount=0;
    u32_t programtimes, addr = 0,len; 
    u32_t spiflash_startaddr, SpiFlashAddress;
    u8_t pwdata[128];
    u8 err;
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    status_t nret = OK_T; 
    s32_t retb;

//    lseek(hFtpManager->Threaddata[0].fd, sizeof(UpdateFilehead_S), SEEK_SET); //定位到文件开始位置        
    programtimes = hFtpManager->Threaddata[0].filelen/FLASH_ONCE_PROGRAM_BYTES_CNT;
    
    if((hFtpManager->Threaddata[0].filelen%FLASH_ONCE_PROGRAM_BYTES_CNT)==0){
    	
    }else{
    	programtimes += 1;		//加1
    }
    hFtpManager->Threaddata[0].sendaddr = addr;
//    #ifdef DEBUG
//    if(usart1_debug(HJ80_DEBUG_FTP)){	  
//        printf("filelen=%x,programtimes=%x\n", hFtpManager->Threaddata[0].filelen,programtimes);
//    }
//    #endif	
    spiflash_startaddr = SPI_Flash_Get_User_Code_Addr();
    hFtpManager->Threaddata[0].programchecksum = 0;
    for(k=0;k<programtimes;k++){        //发送次数
    	
	    addr = k * FLASH_ONCE_PROGRAM_BYTES_CNT; //地址即长度
//	    #ifdef DEBUG
//	    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//            printf("k=%x, addr=%x\n", k, addr);
//        }	
//        #endif	    
	    //printf("addr111=%x\n", addr);
	    pwdata[0] = addr;
	    pwdata[1] = addr >> 8;
	    pwdata[2] = addr >> 16;
	    pwdata[3] = addr >> 24;
	    //printf("addr222=%x\n", addr);
	    if(addr > (hFtpManager->Threaddata[0].filelen - FLASH_ONCE_PROGRAM_BYTES_CNT)){
	    	len = hFtpManager->Threaddata[0].filelen - addr;
	    }else{
    		len = FLASH_ONCE_PROGRAM_BYTES_CNT;
    	}
    	//ret = fread(&pwdata[4], len, 1, hFtpManager->Threaddata[0].fd);
    	//从SPI flash中读取数据
    	SpiFlashAddress = spiflash_startaddr+sizeof(UpdateFilehead_S)+addr;  //从文件头后开始读
    	SPI_Flash_Read(&pwdata[4],SpiFlashAddress,len);     //读出spi flash的内容
    	//ret = read(hFtpManager->Threaddata[0].fd, &pwdata[4], len);    	
    	//printf("addr=%x, len=%x,k=%x, SpiFlashAddress=%x\n", addr, len, k, SpiFlashAddress);
    	
    	for(i=0;i<len;i++){
    		hFtpManager->Threaddata[0].programchecksum += pwdata[4+i];
    	}
    	hFtpManager->Threaddata[0].sendaddr = addr;
	    for(i=0;i<FLASH_PROGRAM_TIMES;i++){
//	            #ifdef DEBUG
//	        	if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                    printf("program times %x\n", i);
//                }	
//                #endif		            
	            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_PROGAM, pwdata, len+4, 1);        //长度为实际字节数再加上地址两字节
	            
	            //retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
                //gettimeofday(&ftptv, 0);
                //ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(PROGRAM_THREAD_WAIT_TIMEOUT);
                //ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
    			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
    			sendcount++;
                //cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
                OSSemPend(hFtpManager->Threaddata[0].threadnotempty,PROGRAM_THREAD_WAIT_TIMEOUT,&err);  
                hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
                
                //if (retb == OK_T)
                //    mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex);  
                if (err == OS_ERR_TIMEOUT){       //超时
                    continue;
                }else if(err == OS_ERR_NONE){        //唤醒
                    break;
                } 
	    }
	    if(i == FLASH_PROGRAM_TIMES){        
	        nret = ERROR_T;
	        break;
	    }
	}
	//printf("ftp0sendcount=%x\n", sendcount);
    return nret;    
}


#define CHECK_THREAD_WAIT_TIMEOUT 1000
#define CHECK_TIMES 5
status_t ftp0_create_send_update_check_frame(void)
{
    u32_t i, k;
    u8_t pwdata[128];
    u8 err;
    //struct timeval ftptv;
    //struct timespec ftpabstime;    
    status_t nret = OK_T, retb; 
    
    pwdata[0] = hFtpManager->Threaddata[0].programchecksum;
    pwdata[1] = hFtpManager->Threaddata[0].programchecksum>>8;
    pwdata[2] = hFtpManager->Threaddata[0].programchecksum>>16;
    pwdata[3] = hFtpManager->Threaddata[0].programchecksum>>24;    
    
    for(i=0;i<CHECK_TIMES;i++){
            
//            #ifdef DEBUG
//    	    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                printf("isp_create_send_update_check_frame ::%x\n", i);
//            }	
//            #endif
            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_CHECK, pwdata, 4, 1);
            
            //retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
            //gettimeofday(&ftptv, 0);
            //ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(CHECK_THREAD_WAIT_TIMEOUT);
            //ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
            //cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
            OSSemPend(hFtpManager->Threaddata[0].threadnotempty,CHECK_THREAD_WAIT_TIMEOUT,&err);  
           
            //printf("1111retb=%d,cond_nret=%x\n", retb,cond_nret);
            hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
            //if (retb == OK_T)
            //    mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
            //printf("2222cond_nret=%x\n", cond_nret);    
            if (err == OS_ERR_TIMEOUT){       //超时
                continue;
            }else if(err == OS_ERR_NONE){        //唤醒
                break;
            }      
    }
    if(i == CHECK_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
    return nret;
}


#define EXIT_TIMES 5
#define EXIT_THREAD_WAIT_TIMEOUT 1000


status_t ftp0_create_send_update_exit_frame(void)
{
    u32_t i, k, len;
    u8 err;
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    status_t nret = OK_T; 
    s32_t retb;
    struct hj_dyb_dump_frame_head * p_frame = (struct hj_dyb_dump_frame_head *)pwdata;    
    //nret = xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);    
    len = RingBuffer_GetCount(&hFtpManager->Threaddata[0].ring);
    if(len > 0){
        RingBuffer_PopMult(&hFtpManager->Threaddata[0].ring, pwdata, len);
    }else{
        return ERROR_T;
    }
//    if(nret == OK_T){
        if(p_frame->cmdStatus == 0){ //成功
//            #ifdef DEBUG
//            if(usart1_debug(HJ80_DEBUG_FTP)){	  
                debug_printf(DBG_INFO,"p_frame->cmdStatus=%x, success!!!\r\n", p_frame->cmdStatus);
//            }	
//            #endif        
            //return OK_T;    
        }else{                          //失败        
//            #ifdef DEBUG
//    	    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                printf("p_frame->cmdStatus=%x, failure!!!\n", p_frame->cmdStatus);
//            }	
//            #endif 
            return ERROR_T;
        }
//    }else{
//        return ERROR_T;
//    }
    for(i=0;i<EXIT_TIMES;i++){
            
//            #ifdef DEBUG
//    	    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                printf("isp_create_send_update_exit_frame exit ::%x\n", i);
//            }	
//            #endif 
            ftp0_build_frame(UPDATE_CMDEXT, CMDEXT_UPDATE_EXIT, pwdata, 0, 1);
//            retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//            gettimeofday(&ftptv, 0);
//            ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(EXIT_THREAD_WAIT_TIMEOUT);
//            ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
			OSSemPend(hFtpManager->Threaddata[0].threadnotempty,EXIT_THREAD_WAIT_TIMEOUT,&err);  
//            cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
            hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//            if (retb == OK_T)
//                mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
            if (err == OS_ERR_TIMEOUT){       //超时
                continue;
            }else if(err == OS_ERR_NONE){        //唤醒
                break;
            }  
                
    }
    if(i == EXIT_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
    return nret;
}

//单元板或者远端设备升级程序    
status_t ftp0_cpu_update(void)
{
    status_t nret = OK_T;      

    debug_puts(DBG_INFO,"isp_create_send_update_start_frame...\r\n");
    nret = ftp0_create_send_update_start_frame();
    //printf("isp_create_send_update_start_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"isp_create_send_update_start_frame failed!!!\r\n");
        return Error_FTP_COMM_FAILURE;
    }
	
    debug_puts(DBG_INFO,"ftp0_create_send_update_password_confirm_frame...\r\n");
    nret = ftp0_create_send_update_password_confirm_frame();
    //printf("isp_create_send_update_password_confirm_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"isp_create_send_update_password_confirm_frame failed!!!\r\n");
        return Error_FTP_PASSWORD_FAILURE;
    }
	
    debug_puts(DBG_INFO,"ftp0_create_send_update_erase_frame...\r\n");
    nret = ftp0_create_send_update_erase_frame();
    //printf("isp_create_send_update_erase_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"isp_create_send_update_erase_frame failed!!!\r\n");
        return Error_FTP_FLASH_ERASE_FAILURE;
    }
	
    debug_puts(DBG_INFO,"ftp0_create_send_update_program_frame...\r\n");
    nret = ftp0_create_send_update_program_frame();
    //printf("isp_create_send_update_program_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"ftp0_create_send_update_program_frame failed!!!\r\n");
        return Error_FTP_FLASH_PROGRAM_FAILURE;
    }
	
    debug_puts(DBG_INFO,"ftp0_create_send_update_check_frame...\r\n");
    nret = ftp0_create_send_update_check_frame();
    //printf("isp_create_send_update_check_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"isp_create_send_update_check_frame failed!!!\r\n");
        return Error_FTP_FLASH_CHECK_FAILURE;
    }
	
    debug_puts(DBG_INFO,"ftp0_create_send_update_exit_frame...\r\n");
    nret = ftp0_create_send_update_exit_frame();
    //printf("isp_create_send_update_exit_frame\n");
    if(nret == ERROR_T){
	    debug_puts(DBG_ERR,"isp_create_send_update_exit_frame failed!!!\r\n");
        return Error_FTP_UPDATE_EXIT_FAILURE;
    }    
    return nret;
}

struct tagUpdateConfigS update_cfg;

status_t ftp0_update_f(void)
{
    int down_filelen;
    status_t nresult = OK_T;
    UpdateFilehead_S filehead;
    
    int start_address;
//    u8_t * pfile;
    u32_t checksum = 0;
    //static char cmdBuf[256] = {0};    
    char ipBuf[64] = {0};    
    //uint8_t tempbuff[1024];
    //hFtpManager->ftp_cmddata.ftpserver_ip = 0xc0a80437;         //////
    sprintf(ipBuf, "%d.%d.%d.%d ",(hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>24)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>16)&0xff, 
        (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>8)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip)&0xff); 
    //printf("ipBuf :: %s\n", ipBuf);   
    debug_printf(DBG_INFO,"ftpserver-IP: %s\r\n", ipBuf);   
    //hFtpManager->ftp_cmddata.filehead.program_type = 1;         //////
    //if(hFtpManager->ftp_cmddata.filehead.program_type == 1){      //更新ARM应用程序

	/**
	  * 2017.5.6: 屏蔽自动更新心跳地址
	  * NOTES: 这里更新mainconfig.c->hManager和spiflash内mainconfig,
	  *        没有更新lwip_comm.c->hManager, 导致三者不一致
	  */
	//listen_set_ftpserver_ip(hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip);     

    //down_filelen = ftp_get(ipBuf, "root", "123456", hFtpManager->ftp_cmddata.filepath, tempbuff, 512, DEST_FLASH, &start_address, &checksum);   
    down_filelen = ftp_get(ipBuf, "root", "123456", hFtpManager->Threaddata[0].ftp_cmddata.filepath, DEST_RAM_UPDATE, &start_address, &checksum, (void *)&filehead);   
    
    if(down_filelen > 0){   //ftp ok
        update_cfg.update_spiflash_checksum = checksum;     
        update_cfg.update_start_address = start_address;
        update_cfg.update_size = down_filelen;
//        debug_dump(&filehead, sizeof(UpdateFilehead_S), "filehead:");        
        memcpy((u8_t *)(&hFtpManager->Threaddata[0].ftp_cmddata.filehead), (u8_t *)(&filehead), 64);

//        if(hFtpManager->Threaddata[0].ftp_cmddata.filehead.board_type != STM32F407_MODULE_BOARD_TYPE){  //如果板卡类型相同
//            return Error_FTP_FILE_CHECK_FAILURE;
//        }
        hFtpManager->Threaddata[0].ftp_board_type = filehead.board_type;
        
        if(hFtpManager->Threaddata[0].ftp_cmddata.filehead.checksum != checksum){
            return Error_FTP_FILE_CHECK_FAILURE;
        }        
        if(hFtpManager->Threaddata[0].ftp_cmddata.filehead.program_type == 1){                 //更新ARM应用程序
            if(hFtpManager->Threaddata[0].ftp_board_type != STM32F407_MODULE_BOARD_TYPE){   //板卡类型是否相同
                
                return Error_FTP_BOARD_TYPE_FAILURE;
            }
            update_cfg.need_update_flag = 1;
    		update_cfg.boot_just_update_flag = 0;
            main_config_write_update_f(&update_cfg, sizeof(struct tagUpdateConfigS));       //置升级标志
            hFtpManager->Threaddata[0].flag_isp_arm = 1;
        }else if(hFtpManager->Threaddata[0].ftp_cmddata.filehead.program_type == 3){      //更新单元板程序
            hFtpManager->Threaddata[0].filelen = down_filelen;            
            nresult = ftp0_cpu_update();               
        }   
    }else{      //ftp error
        update_cfg.need_update_flag = 0;
        hFtpManager->Threaddata[0].flag_isp_arm = 0;
		debug_puts(DBG_ERR,"ftp error!!!\r\n" );
        return Error_FTP_FILE_DOWNLOAD_FAILURE;
    }
    //}else if(hFtpManager->ftp_cmddata.filehead.program_type == 2){   //更新驱动
    //}else if(hFtpManager->ftp_cmddata.filehead.program_type == 3){   //更新MCU应用程序
    //}
    return nresult;
}


status_t ftp0_recv_export_frame(u8_t * p_frame, u8_t seq, u8_t cmdext, u32_t len)
{
    s32_t retb;
    u8_t pwdata[128];
    //#ifdef DEBUG
//    if(usart1_debug(HJ80_DEBUG_FTP)){
//        printf("ftp0_recv_export_frame::Ftp_isp_wait_cmdext=%x, cmdext=%x\n", hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext, cmdext);      
//    }
    debug_printf(DBG_INFO,"ftp0_recv_export_frame::Ftp_isp_wait_cmdext=%x, cmdext=%x\r\n", hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext, cmdext);      
    //#endif	
    if(hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext == cmdext){        //只有当命令编号相同时才唤醒
        if((cmdext == CMDEXT_EXPORT_FILE_HEAD)||(cmdext == CMDEXT_EXPORT_FILE_DATA)){
            //xbuffer_reset_f(hFtpManager->Threaddata[0].sbuf);        //只保存一帧数据，每次处理完后就清空
            //RingBuffer_Init(&hFtpManager->Threaddata[0].ring, hFtpManager->Threaddata[0].ring_buff, 1, RING_BUFF_SIZE);  
            
            //RingBuffer_Init(&tx_export_ring, tx_export_buff, 1, TX_EXPORT_SIZE);  
            
            //ack_frame.n_data_len = 0;
            
//            #ifdef DEBUG
//            if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                debug_dump(p_frame, 64, "ftp0_recv_export_frame:");      
//            }
//            #endif	
//            if(xbuffer_write_f(hFtpManager->Threaddata[0].sbuf, p_frame, 128) == ERROR_T){       //这里直接写入128个字节以便于读取整数个字节
//                printf("isp buffer have no space\n");
//            }    
            //RingBuffer_InsertMult(&hFtpManager->Threaddata[0].ring, p_frame, 128);
            //RingBuffer_InsertMult(&tx_export_ring, p_frame, len);
            ack_frame.n_data_len = len;
            memcpy(ack_frame.data_buff, p_frame, ack_frame.n_data_len);
//            len = RingBuffer_GetCount(&tx_export_ring);
//            RingBuffer_PopMult(&tx_export_ring, pwdata, len);
//            RingBuffer_InsertMult(&tx_export_ring, p_frame, len);
            
        }
//        retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
    	if(hFtpManager->Threaddata[0].Ftp_wait_flag)     
        	//pthread_cond_signal(&hFtpManager->Threaddata[0].threadnotempty);   //需要判断是否有线程等待，当有才去唤醒，没有则忽略此帧
            OSSemPost(hFtpManager->Threaddata[0].threadnotempty);
//        if (retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex);
    }
	
}


//接收终端数据，如果序列号与先前命令相同，则唤醒，否则一直等待。
status_t ftp0_recv_import_frame(u8_t * p_frame, u8_t seq, u8_t cmdext, u32_t len)
{
    s32_t retb;
    
    //#ifdef DEBUG
    //if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
    //printf("ftp0_recv_import_frame::Ftp_isp_wait_cmdext=%x, cmdext=%x\n", hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext, cmdext);      
    debug_printf(DBG_INFO,"ftp0_recv_import_frame::Ftp_isp_wait_cmdext=%x, cmdext=%x\r\n", hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext, cmdext);      
    //}
    //#endif	
    if(hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext == cmdext){        //只有当命令编号相同时才唤醒
        if((cmdext == CMDEXT_IMPORT_FILE_HEAD)||(cmdext == CMDEXT_IMPORT_FILE_DATA)){
            //xbuffer_reset_f(hFtpManager->Threaddata[0].sbuf);        //只保存一帧数据，每次处理完后就清空
            RingBuffer_Init(&hFtpManager->Threaddata[0].ring, hFtpManager->Threaddata[0].ring_buff, 1, RING_BUFF_SIZE);  
//            #ifdef DEBUG
//            if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                debug_dump(p_frame, 64, "ftp0_recv_import_frame:");      
//            }
//            #endif	
//            if(xbuffer_write_f(hFtpManager->Threaddata[0].sbuf, p_frame, 128) == ERROR_T){       //这里直接写入128个字节以便于读取整数个字节
//                printf("isp buffer have no space\n");
//            }    
            RingBuffer_InsertMult(&hFtpManager->Threaddata[0].ring, p_frame, 128);
        }
//        retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
    	if(hFtpManager->Threaddata[0].Ftp_wait_flag)     
        	//pthread_cond_signal(&hFtpManager->Threaddata[0].threadnotempty);   //需要判断是否有线程等待，当有才去唤醒，没有则忽略此帧
            OSSemPost(hFtpManager->Threaddata[0].threadnotempty);
//        if (retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex);
    }
	
}




//接收终端数据，如果序列号与先前命令相同，则唤醒，否则一直等待。
status_t ftp0_recv_isp_frame(u8_t * p_frame, u8_t seq, u8_t cmdext, u32_t len)
{
    s32_t retb;
    struct hj_dyb_dump_frame_head *p_head;
    uint32_t ackaddr;
//    #ifdef DEBUG
//    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//        printf("ftp0_recv_isp_frame::Ftp_isp_wait_cmdext=%x, cmdext=%x\n", hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext, cmdext);      
//    }
//    #endif	
    if(hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext == cmdext){        //只有当命令编号相同时才唤醒
        if((cmdext == CMDEXT_UPDATE_START)||(cmdext == CMDEXT_UPDATE_CHECK)){
            //xbuffer_reset_f(hFtpManager->Threaddata[0].sbuf);        //只保存一帧数据，每次处理完后就清空
            RingBuffer_Init(&hFtpManager->Threaddata[0].ring, hFtpManager->Threaddata[0].ring_buff, 1, RING_BUFF_SIZE);  
            //debug_dump(p_frame, 64, "ftp0_recv_isp_frame:"); 
//            #ifdef DEBUG
//            if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                debug_dump(p_frame, 64, "ftp0_recv_isp_frame:");      
//            }
//            #endif	
            RingBuffer_InsertMult(&hFtpManager->Threaddata[0].ring, p_frame, 128);
//            if(xbuffer_write_f(hFtpManager->Threaddata[0].sbuf, p_frame, 128) == ERROR_T){       //这里直接写入128个字节以便于读取整数个字节
//                printf("isp buffer have no space\n");
//            }    
        }else if(cmdext == CMDEXT_UPDATE_PROGAM){
            p_head = (struct hj_dyb_dump_frame_head *)p_frame; 	      
            if(p_head->cmdLen > 0){
                //printf("2222ftp0::p_head->cmdLen=%x\n", p_head->cmdLen);
                //p_ackaddr = &p_head->buf[0];
                memcpy(&ackaddr, ((u8_t *)&p_head->cmdLen+2), 4);
                //printf("ftp0::p_ackaddr=%x,addr=%x\n",*p_ackaddr, hFtpManager->Threaddata[0].sendaddr);
                
                //printf("ftp0::ackaddr=%x\n", ackaddr);
                if(ackaddr != hFtpManager->Threaddata[0].sendaddr){  //只有回复的地址相同时，才发下一个地址，才去唤醒。
                    printf("err::addr!=ackaddr\n");
                    return 0;
                } 
            }    
        }
//        retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
    	if(hFtpManager->Threaddata[0].Ftp_wait_flag)     
        	//pthread_cond_signal(&hFtpManager->Threaddata[0].threadnotempty);   //需要判断是否有线程等待，当有才去唤醒，没有则忽略此帧
            OSSemPost(hFtpManager->Threaddata[0].threadnotempty);
//        if(retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex);

    }

}



status_t ftp0_export_result_send(u32_t result)
{
    struct hj_dump_frame_head * p_cmd_hdr;
    u32_t frame_len;
    u8_t * p_ack_buff;
    struct hj_dump_frame_tail  *p_cmd_tail;   
    u16_t  temp_data;  
    u8_t ack_frame[sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail)];
    if(result == 0){
        //printf("export success =%x\n", result);
        debug_printf(DBG_INFO,"export success =%x\r\n", result);
    }else{
        //printf("ftp0_export_result_send::result=%x\n", result);
        debug_printf(DBG_ERR,"ftp0_export_result_send::result=%x\r\n", result);
    }
        
    frame_len = sizeof(* p_cmd_hdr)+3;
    hFtpManager->Threaddata[0].ackframe.n_data_len = frame_len;
	p_ack_buff = ack_frame;
	memcpy(p_ack_buff, hFtpManager->Threaddata[0].ackframe.data_buff, sizeof(struct hj_dump_frame_head));
		
	p_cmd_hdr = (struct hj_dump_frame_head * )p_ack_buff;
	p_cmd_hdr->cmdStatus = result;
    p_cmd_hdr->cmdExt = 0x05;       //命令编号为105
    p_cmd_hdr->cmdId = 0x71;        //命令号为7
    p_cmd_hdr->cmdLen = 0;
	   	            
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_ack_buff + sizeof(* p_cmd_hdr));
    temp_data  =  hj_calc_crc (p_ack_buff+1,frame_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;     
    
//    #ifdef DEBUG
//        if(usart1_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//            debug_dump(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len, "ftp0_export_result_send:"); 
//        }
//    #endif	
    jump_send_buf(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len);              //写入trap缓冲区   
//    if(p_ack_buff){
//        free(p_ack_buff);
//    } 
    return 0;
}

static PortFilehead_S filehead;


int ftp0_import_result_send(u32_t result)
{
    struct hj_dump_frame_head * p_cmd_hdr;
    u32_t frame_len;
    u8_t * p_ack_buff;
    struct hj_dump_frame_tail  *p_cmd_tail;   
    u16_t  temp_data;  
    u8_t ack_frame[sizeof(struct hj_dump_frame_head)+sizeof(struct hj_dump_frame_tail)];
    if(result == 0){
        //printf("ftp0_import_result_send::import success =%x\n", result);
        debug_printf(DBG_INFO,"ftp0_import_result_send::import success =%x\r\n", result);
    }else{
        //printf("ftp0_import_result_send::result=%x\n", result);
        debug_printf(DBG_ERR,"ftp0_import_result_send::result=%x\r\n", result);
    }
        
    frame_len = sizeof(* p_cmd_hdr)+3;
    hFtpManager->Threaddata[0].ackframe.n_data_len = frame_len;
	p_ack_buff = ack_frame;
	memcpy(p_ack_buff, hFtpManager->Threaddata[0].ackframe.data_buff, sizeof(struct hj_dump_frame_head));
		
	p_cmd_hdr = (struct hj_dump_frame_head * )p_ack_buff;
	p_cmd_hdr->cmdStatus = result;
    p_cmd_hdr->cmdExt = 0x04;       //命令编号为104
    p_cmd_hdr->cmdId = 0x71;        //命令号为7
    p_cmd_hdr->cmdLen = 0;
	   	            
    p_cmd_tail = (struct hj_dump_frame_tail *)(p_ack_buff + sizeof(* p_cmd_hdr));
    temp_data  =  hj_calc_crc (p_ack_buff+1,frame_len -4);
    p_cmd_tail -> crcl = temp_data & 0xff;
    p_cmd_tail -> crch = (temp_data>>8) & 0xff;
    p_cmd_tail -> suf = 0x7e;     
    
//    #ifdef DEBUG
//        if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//            debug_dump(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len, "import_result_send:"); 
//        }
//    #endif	
    jump_send_buf(p_ack_buff, hFtpManager->Threaddata[0].ackframe.n_data_len);              //写入trap缓冲区   
//    if(p_ack_buff){
//        free(p_ack_buff);
//    } 
    return 0;
}



#define IMPORT_FILE_HEAD_TIMES 5
status_t ftp0_create_send_import_file_head_frame(void)
{
    u32_t i, k,SpiFlashAddress;
    u8_t pwdata[128];
    u8 err;
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    s32_t retb;
    
    SpiFlashAddress = SPI_Flash_Get_Import_Data_Addr();  //从文件头后开始读
    SPI_Flash_Read(pwdata,SpiFlashAddress,sizeof(PortFilehead_S));     //读出spi flash的内容
            
    for(i=0;i<IMPORT_FILE_HEAD_TIMES;i++){      
                       
        ftp0_build_frame(IMPORT_CMDEXT, CMDEXT_IMPORT_FILE_HEAD, pwdata, sizeof(PortFilehead_S), 1);
  
//        retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//        gettimeofday(&ftptv, 0);
//        ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(THREAD_WAIT_TIMEOUT);
//        ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
		hFtpManager->Threaddata[0].Ftp_wait_flag = 1;

        OSSemPend(hFtpManager->Threaddata[0].threadnotempty,THREAD_WAIT_TIMEOUT,&err);  
        hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//        if (retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
        if (err == OS_ERR_TIMEOUT){       //超时
            continue;
        }else if(err == OS_ERR_NONE){        //唤醒
            break;
        }          
    }        
    if(i == IMPORT_FILE_HEAD_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
}

status_t ftp0_create_send_import_file_data_frame(void)
{
    u32_t i, k, SpiFlashAddress;
    u8 err;
    u32_t downloadtimes = 0, localaddr = 0,len; 
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;      
    
    status_t nret = OK_T; 
    s32_t retb;
    struct hj_dyb_dump_frame_head * p_frame = (struct hj_dyb_dump_frame_head * )pwdata;   
//    hj_update_start_data_t * p_cmd_data = p_frame->buf;   
//    nret == xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);
    len = RingBuffer_GetCount(&hFtpManager->Threaddata[0].ring);
    if(len > 0){
        RingBuffer_PopMult(&hFtpManager->Threaddata[0].ring, pwdata, len);
    }else{
        return ERROR_T;
    }
    
    //if(nret == OK_T){
        if(p_frame->cmdStatus != OK_T){     //是否类型相一致
            return Error_FTP_BOARD_TYPE_FAILURE;
        }
        SpiFlashAddress = SPI_Flash_Get_Import_Data_Addr();  //从文件头后开始读
        SpiFlashAddress += sizeof(PortFilehead_S);
        
        downloadtimes = hFtpManager->Threaddata[0].filelen/FLASH_PORT_BYTES_CNT;
        if((hFtpManager->Threaddata[0].filelen%FLASH_PORT_BYTES_CNT)==0){  //刚好是倍数关系
        	
        }else{
        	downloadtimes += 1;		//加1
        }
    //}
    
    for(k=0;k<downloadtimes;k++){        //发送次数
    	
	    localaddr = k * FLASH_PORT_BYTES_CNT; //地址即长度
//	    #ifdef DEBUG
//	    if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//            printf("k=%x, localaddr=%x\n", k, localaddr);
//        }	
//        #endif	    
	    pwdata[0] = localaddr;
	    pwdata[1] = localaddr >> 8;	    
	    
	    if(localaddr > (hFtpManager->Threaddata[0].filelen - FLASH_PORT_BYTES_CNT)){
	    	len = hFtpManager->Threaddata[0].filelen - localaddr;
	    }else{
    		len = FLASH_PORT_BYTES_CNT;
    	}    	
    	//nret = read(hFtpManager->Threaddata[0].fd, &pwdata[2], len);
    	SPI_Flash_Read(&pwdata[2],SpiFlashAddress,len);     //读出spi flash的内容
    	SpiFlashAddress += len;
	    for(i=0;i<FTP_DOWNLOAD_TIMES;i++){
//	            #ifdef DEBUG
//	        	if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_FTP)){
//                    printf("download times %x\n", i);
//                }	
//                #endif		            
	            ftp0_build_frame(IMPORT_CMDEXT, CMDEXT_IMPORT_FILE_DATA, pwdata, len+2, 1);        //长度为实际字节数再加上地址两字节
	                
//	            retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//                gettimeofday(&ftptv, 0);
//                ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(DOWNLOAD_THREAD_WAIT_TIMEOUT);
//                ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
        		hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
                //cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
                OSSemPend(hFtpManager->Threaddata[0].threadnotempty,DOWNLOAD_THREAD_WAIT_TIMEOUT,&err);  
                hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//                if (retb == OK_T)
//                    mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
                if (err == OS_ERR_TIMEOUT){       //超时
                    continue;
                }else if(err == OS_ERR_NONE){        //唤醒
                    break;
                }    
	    }
	    if(i == FTP_DOWNLOAD_TIMES){        
	        return Error_FTP_COMM_FAILURE;
	    }
	}
    return OK_T;    
}


status_t ftp0_create_send_import_file_end_frame(void)
{
    u32_t i, k;
    u8 err;
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    status_t nret = OK_T;     
    s32_t retb;
    for(i=0;i<FTP_DOWNLOAD_END_TIMES;i++){      //发下载结束命令
	    
	    ftp0_build_frame(IMPORT_CMDEXT, CMDEXT_IMPORT_END, pwdata, 0, 1);	  
	        
//	    retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//        gettimeofday(&ftptv, 0);
//        ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(DOWNLOAD_THREAD_WAIT_TIMEOUT);
//        ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
		hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
//        cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
        OSSemPend(hFtpManager->Threaddata[0].threadnotempty,DOWNLOAD_THREAD_WAIT_TIMEOUT,&err);  
        hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//        if (retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
        if (err == OS_ERR_TIMEOUT){       //超时
            continue;
        }else if(err == OS_ERR_NONE){        //唤醒
            break;
        }        
	        
	}    
    if(i == FTP_DOWNLOAD_END_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
    return nret;
}

status_t ftp0_download_fulldata2device(void)
{
    status_t nresult = OK_T; 
    
    debug_printf(DBG_INFO,"ftp0_create_send_import_file_head_frame...\r\n");
    nresult = ftp0_create_send_import_file_head_frame();
    //printf("ftp0_create_send_import_file_head_frame\n");
    if(nresult == ERROR_T){
    	debug_printf(DBG_ERR,"ftp0_create_send_import_file_head_frame failed!!!\r\n");
        return Error_FTP_COMM_FAILURE;
    }
	
    debug_printf(DBG_INFO,"ftp0_create_send_import_file_data_frame...\r\n");
    nresult = ftp0_create_send_import_file_data_frame();
    //printf("ftp0_create_send_import_file_data_frame\n");
    if((nresult == Error_FTP_COMM_FAILURE)||(nresult == Error_FTP_BOARD_TYPE_FAILURE)){
    	debug_printf(DBG_ERR,"ftp0_create_send_import_file_data_frame failed!!!\r\n");
        return nresult;
    }
	
    debug_printf(DBG_INFO,"ftp0_create_send_import_file_end_frame...\r\n");
    nresult = ftp0_create_send_import_file_end_frame();
    //printf("ftp0_create_send_import_file_end_frame\n");
    if(nresult == ERROR_T){
    	debug_printf(DBG_ERR,"ftp0_create_send_import_file_end_frame failed!!!\r\n");
        return Error_FTP_COMM_FAILURE;
    }
    return nresult;
}

status_t ftp0_download_fulldata2nmboard(void)
{
    status_t nresult = OK_T;
    u8_t pwdata[512];        
    s32_t ret,SpiFlashAddress, len;
    PortFilehead_S portfilehead;
    u8_t * p_data = pwdata;
    //lseek(hFtpManager->Threaddata[0].fd, 0, SEEK_SET); //定位到开始位置
    //ret = read(hFtpManager->Threaddata[0].fd, &portfilehead, sizeof(PortFilehead_S));
    len = MainConfig_Size + sizeof(PortFilehead_S);
    SpiFlashAddress = SPI_Flash_Get_Import_Data_Addr();  //从文件头后开始读
    
    printf("SpiFlashAddress=%x\n", SpiFlashAddress);
    SPI_Flash_Read(pwdata,SpiFlashAddress,len);     //读出spi flash的内容
//    debug_dump(p_data, MainConfig_Size, "222buff:");
    memcpy(&portfilehead, p_data, sizeof(PortFilehead_S));    
//    debug_dump(&portfilehead,sizeof(PortFilehead_S),  "portfilehead:");
    printf("portfilehead.board_type=%x\n", portfilehead.board_type);
    if(portfilehead.board_type != WGDYB_DEVICE_TYPE){
        return Error_FTP_BOARD_TYPE_FAILURE;
    }
    p_data += sizeof(PortFilehead_S);	

    main_config_save_f((MAINCONFIG_S *)p_data);
    main_config_refresh_h_main();
//	debug_dump(p_data, MainConfig_Size, "buff:");
	     
    return nresult;
}

status_t ftp0_download_fulldata(void)
{
    status_t nresult = OK_T;
    u32_t device_id;
    struct hj_dump_frame_head * p_cmd_hdr = (struct hj_dump_frame_head *)hFtpManager->Threaddata[0].cmd_frame_buff;
    device_id = p_cmd_hdr->dst_deviceid;
    device_id &= 0xff;
    //if((device_id == LOCAL_DEVICE_ID)&&(p_cmd_hdr->boardIndex == WG_BOARD)){	//如果是网管板直接备份到本板而不用下发命令
        
    //    nresult = ftp0_download_fulldata2nmboard();
    //}else{	        //如果是其它板卡或者远端设备               
        nresult = ftp0_download_fulldata2device();
    //}	
    return nresult;
}

status_t ftp0_import_f(void)
{
    int down_filelen;
    status_t nresult = OK_T;
    PortFilehead_S filehead;
    
    int start_address;
    u32_t checksum = 0;
    char ipBuf[64] = {0};    
    //uint8_t tempbuff[1024];
    //hFtpManager->ftp_cmddata.ftpserver_ip = 0xc0a80437;         //////
    sprintf(ipBuf, "%d.%d.%d.%d ",(hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>24)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>16)&0xff, 
        (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>8)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip)&0xff); 
    //printf("ipBuf :: %s\n", ipBuf);   
    debug_printf(DBG_INFO,"ftpserver-IP: %s\r\n", ipBuf);   

    //listen_set_ftpserver_ip(hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip);     
    //down_filelen = ftp_get(ipBuf, "root", "123456", hFtpManager->ftp_cmddata.filepath, tempbuff, 512, DEST_FLASH, &start_address, &checksum);   
    down_filelen = ftp_get(ipBuf, "root", "123456", hFtpManager->Threaddata[0].ftp_cmddata.filepath, DEST_RAM_IMPORT, &start_address, &checksum, &filehead);   
    if(down_filelen > 0){   //ftp ok
 
//        debug_dump(&filehead, sizeof(PortFilehead_S), "filehead:");        
        //memcpy((u8_t *)(&hFtpManager->Threaddata[0].ftp_cmddata.filehead), (u8_t *)(&filehead), 64);

            hFtpManager->Threaddata[0].filelen = filehead.size;        
            if(filehead.size > SIZE_32K){
                return Error_FTP_FILE_CHECK_FAILURE;
            }
            if((filehead.board_type == 0)||(filehead.board_type == 0xffffffff)){      
                return Error_FTP_FILE_CHECK_FAILURE;
            }
                
            hFtpManager->Threaddata[0].ftp_board_type = filehead.board_type;
            hFtpManager->Threaddata[0].Ftp_isp_wait_cmdext = 0;       //清除当前等待的命令编号
   
            nresult = ftp0_download_fulldata();  
        
    }else{      //ftp error

        return Error_FTP_FILE_DOWNLOAD_FAILURE;
    }

    return nresult;
}



#define EXPORT_FILE_HEAD_TIMES 3
status_t ftp0_create_send_export_file_head_frame(void)
{
    u32_t i, k;
    u8 err;
    u8_t pwdata[128];
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    s32_t retb;
    for(i=0;i<EXPORT_FILE_HEAD_TIMES;i++){      
                       
        ftp0_build_frame(EXPORT_CMDEXT, CMDEXT_EXPORT_FILE_HEAD, pwdata, 0, 1);
              
//        retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//        gettimeofday(&ftptv, 0);
//        ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(THREAD_WAIT_TIMEOUT);
//        ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
		hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
//        cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
        OSSemPend(hFtpManager->Threaddata[0].threadnotempty,THREAD_WAIT_TIMEOUT,&err);  
        hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//        if (retb == OK_T)
//            mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
        if (err == OS_ERR_TIMEOUT){       //超时
            continue;
        }else if(err == OS_ERR_NONE){        //唤醒
            break;
        }
    }        
    if(i == EXPORT_FILE_HEAD_TIMES){        
        return ERROR_T;
    }else{
        return OK_T;    
    }
}
#define FTP_UPLOAD_TIMES 5

status_t ftp0_create_send_export_file_data_frame(void)
{
    u8_t pwdata[128];
    status_t nret;
    u8 err;
    status_t nresult = OK_T;
    u32_t localaddr = 0, i,k,remoteaddr;
    u32_t count = 0, len, SpiFlashAddress;
    s32_t retb;
//    struct timeval ftptv;
//    struct timespec ftpabstime;    
    PortFilehead_S * p_filehead;
    struct hj_dyb_dump_frame_head * p_frame = ( struct hj_dyb_dump_frame_head * )ack_frame.data_buff;   
    //hj_update_start_data_t * p_cmd_data = p_frame->buf;   
//    nret = xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);
    //len = RingBuffer_GetCount(&hFtpManager->Threaddata[0].ring);
    len = ack_frame.n_data_len;
    
    if(len > 0){
        //RingBuffer_PopMult(&hFtpManager->Threaddata[0].ring, pwdata, len);

        //memcpy(pwdata, ack_frame.data_buff, ack_frame.n_data_len);
        ack_frame.n_data_len = 0;
    }else{
        return ERROR_T;
    }
    //if(nret == OK_T){
        p_filehead = (PortFilehead_S *)((u8_t *)(&p_frame->cmdLen)+2);
        
        
        //lseek(hFtpManager->Threaddata[0].fd, localaddr, SEEK_SET); //定位到文件相应位置        
        //write(hFtpManager->Threaddata[0].fd, p_filehead, sizeof(PortFilehead_S));
        
        SpiFlashAddress = SPI_Flash_Get_Export_Data_Addr();    
        SPI_Flash_Write(p_filehead, SpiFlashAddress, sizeof(PortFilehead_S));
        SpiFlashAddress += sizeof(PortFilehead_S);
        //printf("head len=%x\n", p_frame->cmdLen);
//        debug_dump(p_filehead, sizeof(PortFilehead_S), "head:");               
        count = p_filehead->size/FLASH_PORT_BYTES_CNT;
        if((p_filehead->size%FLASH_PORT_BYTES_CNT)==0){  //刚好是倍数关系
        	
        }else{
        	count += 1;		//加1
        }
    //}else{        
    //    return ERROR_T;
    //}
    debug_printf(DBG_DEBUG,"0000nresult=%x, count=%x\r\n", nresult, count);  
    if(count > 0){
        
        for(k=0;k<count;k++){        //发读取参数命令          	      
    	    for(i=0;i<FTP_UPLOAD_TIMES;i++){
    	            debug_printf(DBG_DEBUG,"upload times %x\r\n", i);
    	            remoteaddr = k*FLASH_PORT_BYTES_CNT;
    	            pwdata[0] = remoteaddr;          //前两字节为地址
    	            pwdata[1] = remoteaddr>>8;
    	            pwdata[2] = FLASH_PORT_BYTES_CNT;   //最大字节长度
    	            ftp0_build_frame(EXPORT_CMDEXT, CMDEXT_EXPORT_FILE_DATA, pwdata, 3, 1);    
    	              		
//    	            retb = mutex_lock_f(hFtpManager->Threaddata[0].threaddatamutex, -1);
//                    gettimeofday(&ftptv, 0);
//                    ftpabstime.tv_sec = ftptv.tv_sec + (time_t)(DOWNLOAD_THREAD_WAIT_TIMEOUT);
//                    ftpabstime.tv_nsec = ftptv.tv_usec * 1000; 
        			hFtpManager->Threaddata[0].Ftp_wait_flag = 1;
//                    cond_nret = pthread_cond_timedwait(&(hFtpManager->Threaddata[0].threadnotempty), hFtpManager->Threaddata[0].threaddatamutex, &ftpabstime);
                    OSSemPend(hFtpManager->Threaddata[0].threadnotempty,THREAD_WAIT_TIMEOUT,&err);  
                    hFtpManager->Threaddata[0].Ftp_wait_flag = 0;
//                    if (retb == OK_T)
//                        mutex_unlock_f(hFtpManager->Threaddata[0].threaddatamutex); 
                    if (err == OS_ERR_TIMEOUT){       //超时
                        continue;
                    }else if(err == OS_ERR_NONE){        //唤醒                        
                        //nret = xbuffer_read_f(hFtpManager->Threaddata[0].sbuf, pwdata, 128);
                        
                        //len = RingBuffer_GetCount(&hFtpManager->Threaddata[0].ring);
                        len = ack_frame.n_data_len;
                        
                        if(len > 0){
                            //RingBuffer_PopMult(&tx_export_ring, pwdata, len);
                            
                            //memcpy(pwdata, ack_frame.data_buff, ack_frame.n_data_len);
                            ack_frame.n_data_len = 0;
                            //RingBuffer_PopMult(&hFtpManager->Threaddata[0].ring, pwdata, len);
        	                //localaddr += p_frame->cmdLen; //地址即长度        	                
        	                //printf("bbb localaddr =%x, len=%x\n", localaddr, p_frame->cmdLen);
        	                //write(hFtpManager->Threaddata[0].fd, &p_frame->buf[0], p_frame->cmdLen);
        	                
//        	                debug_dump(((u8_t *)(&p_frame->cmdLen)+2), p_frame->cmdLen, "data:");       
        	                debug_printf(DBG_DEBUG,"data len=%x\r\n", p_frame->cmdLen);
        	                
        	                SPI_Flash_Write(((u8_t *)(&p_frame->cmdLen)+2), SpiFlashAddress, p_frame->cmdLen);
        	                SpiFlashAddress += p_frame->cmdLen;
        	                break;
        	            }                 
                        
                    }            
    	    }
    	    if(i == FTP_UPLOAD_TIMES){        
    	        return ERROR_T;
    	    }
    	}
    } 
    return nresult;
}

status_t ftp0_upload_fulldata2device(void)
{
    status_t nresult = OK_T;
    //printf("ftp0_upload_fulldata2device\n");
    debug_printf(DBG_INFO,"ftp0_create_send_export_file_head_frame...\r\n");
    nresult = ftp0_create_send_export_file_head_frame();
    //printf("ftp0_create_send_export_file_head_frame\n");
    if(nresult == ERROR_T){
	    debug_printf(DBG_ERR,"ftp0_create_send_export_file_head_frame failed!!!\r\n");
        return Error_FTP_COMM_FAILURE;
    }

    debug_printf(DBG_INFO,"ftp0_create_send_export_file_data_frame...\r\n");
    nresult = ftp0_create_send_export_file_data_frame();
    //printf("ftp0_create_send_export_file_data_frame\n");
    if(nresult == ERROR_T){
	    debug_printf(DBG_ERR,"ftp0_create_send_export_file_data_frame failed!!!\r\n");
        return Error_FTP_COMM_FAILURE;
    }
    return nresult;
}
status_t ftp0_upload_fulldata2nmboard(void)
{
    status_t nresult = OK_T;
    u8_t pwdata[512];        
    s32_t ret, SpiFlashAddress;
    PortFilehead_S portfilehead;
    printf("ftp0_upload_fulldata2nmboard\n");
//    lseek(hFtpManager->Threaddata[0].fd, 0, SEEK_SET); //定位到文件起始位置        
    portfilehead.board_type = WGDYB_DEVICE_TYPE;
    portfilehead.version = ARM_VERSION;
    portfilehead.size = sizeof(struct tagGlobalConfigS);    

    main_config_read_gloabl_f(pwdata, sizeof(struct tagGlobalConfigS));
    
    SpiFlashAddress = SPI_Flash_Get_Export_Data_Addr();    
    SPI_Flash_Write(&portfilehead, SpiFlashAddress, sizeof(PortFilehead_S));
    SpiFlashAddress += sizeof(PortFilehead_S);
    SPI_Flash_Write(pwdata, SpiFlashAddress, sizeof(struct tagGlobalConfigS));

	//debug_dump(p_config, MainConfig_Size, "buff:");
	     
    return nresult;
}


status_t ftp0_upload_fulldata(void)
{
    status_t nresult = OK_T;
    u32_t device_id;
    struct hj_dump_frame_head * p_cmd_hdr = (struct hj_dump_frame_head *)hFtpManager->Threaddata[0].cmd_frame_buff;
    device_id = p_cmd_hdr->dst_deviceid;
    device_id &= 0xff;
    //printf("device_id=%x, boardIndex=%x\n", device_id,p_cmd_hdr->boardIndex );
    debug_printf(DBG_DEBUG,"device_id=%x, boardIndex=%x\r\n", device_id,p_cmd_hdr->boardIndex );
    //if((device_id == LOCAL_DEVICE_ID)&&(p_cmd_hdr->boardIndex == WG_BOARD)){	//如果是网管板直接备份到本板而不用下发命令
        
    //    nresult = ftp0_upload_fulldata2nmboard();
    //}else{	        //如果是其它板卡或者远端设备               
        nresult = ftp0_upload_fulldata2device();
    //}	
    return nresult;
}

u8_t export_buff[8192];
//导出所有参数
status_t ftp0_export_f(void)
{
    status_t nresult = OK_T;
    int nret, SpiFlashAddress;
    static char cmdBuf[256] = {0};    
    static char ipBuf[32] = {0};    
    u8_t * p_export_buff;
    PortFilehead_S PortFilehead;
    //hFtpManager->Threaddata[0].fd = fopen(local_ftp_file, "wb");  //创建文件
    //hFtpManager->Threaddata[0].fd = open(local_ftp0_file, O_WRONLY|O_CREAT);   //|O_APPEND
    //printf("ftp_export_f fd=%x\n", hFtpManager->Threaddata[0].fd);
    //if (hFtpManager->Threaddata[0].fd <= 0)
    //    return ERROR_T;
    
    //lseek(hFtpManager->Threaddata[0].fd, 0, SEEK_SET); //定位到文件开始位置
    nresult = ftp0_upload_fulldata();   
    
    if(nresult == OK_T){
        
        //////////////////Usage: ftpput [options] remote-host remote-file local-file
        
        sprintf(ipBuf, "%d.%d.%d.%d ",(hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>24)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>16)&0xff, 
            (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip>>8)&0xff, (hFtpManager->Threaddata[0].ftp_cmddata.ftpserver_ip)&0xff); 
          
        sprintf(cmdBuf, "ftpput -u %s -p %s  %s  %s ftp0.bin ","root", "123456", ipBuf,
                       hFtpManager->Threaddata[0].ftp_cmddata.filepath);   
        //printf("ipBuf :: %s\n", ipBuf); 
                
        SpiFlashAddress = SPI_Flash_Get_Export_Data_Addr();
        SPI_Flash_Read(&PortFilehead,SpiFlashAddress,sizeof(PortFilehead_S));     //读出spi flash的内容
        p_export_buff = export_buff;
        SPI_Flash_Read(p_export_buff,SpiFlashAddress,sizeof(PortFilehead_S));  
        debug_printf(DBG_DEBUG,"PortFilehead.size=%x\r\n", PortFilehead.size);
        
//        debug_dump(export_buff, sizeof(PortFilehead_S), "ftp0_export_f head:"); 
        
        SpiFlashAddress += sizeof(PortFilehead_S);
        p_export_buff += sizeof(PortFilehead_S);
        SPI_Flash_Read(p_export_buff,SpiFlashAddress,PortFilehead.size);
//        debug_dump(export_buff, PortFilehead.size+sizeof(PortFilehead_S), "ftp0_export_f data:");         
        ftp_put(ipBuf, "root", "123456", 
            hFtpManager->Threaddata[0].ftp_cmddata.filepath, export_buff, PortFilehead.size+sizeof(PortFilehead_S));
        
        hFtpManager->Threaddata[0].flag_export = 0;
        
//        nret = system(cmdBuf);
//        if(WIFEXITED(nret)){
//            if (WEXITSTATUS(nret) == 0){    //文件提取成功
//                
//                hFtpManager->Threaddata[0].flag_export = 0;
//                //printf("flag_export over\n");
//            }else{
//                printf("execve illegal!!\n");
//                nresult = Error_FTP_FILE_UPLOAD_FAILURE;		    
//            }
//        }else{
//            printf("execve illegal!!\n");
//            nresult = Error_FTP_FILE_UPLOAD_FAILURE;		
//    	}
    }

    return nresult;
}



static void user_ftp0_thread(void *arg)
{
	u8 err;
    int len;
    s32_t nret;
    while(1){    
        //printf("ispProc\n");
        if(hFtpManager->Threaddata[0].flag_isp){      //当有升级
            
            //printf("0::flag_isp=%x\n", hFtpManager->Threaddata[0].flag_isp);
            debug_printf(DBG_INFO,"0::flag_isp=%x\r\n", hFtpManager->Threaddata[0].flag_isp);
			
            nret = ftp0_update_f();
            
			ftp0_isp_result_send(nret);
			
            hFtpManager->Threaddata[0].flag_isp = 0;            
            hFtpManager->Threaddata[0].flag_busy = 0;
            if(hFtpManager->Threaddata[0].flag_isp_arm == 1){        //如果是ARM升级成功，则需要延时一段时间后再重启

                //#ifdef STM32F4XX_APP                      //如果是APP中则需要复位后进入BOOT再升级
                //delay_ms(500);                           //等待升级结果数据帧发送完毕   
                //printf("xxxxxxxxxxx\r\n");               
                control_set_sysreset();
                //#endif
            }
        }
		else if(hFtpManager->Threaddata[0].flag_export){      //当需要上传参数
            //printf("0::flag_export=%x\n", hFtpManager->Threaddata[0].flag_export);
            debug_printf(DBG_INFO,"0::flag_export=%x\r\n", hFtpManager->Threaddata[0].flag_export);
            nret = ftp0_export_f();
            ftp0_export_result_send(nret);
                            
            hFtpManager->Threaddata[0].flag_export = 0;
            hFtpManager->Threaddata[0].flag_busy = 0;
        }
		else if(hFtpManager->Threaddata[0].flag_import){      //当需要下载参数
            //printf("0::flag_import=%x\n", hFtpManager->Threaddata[0].flag_import);
            debug_printf(DBG_INFO,"0::flag_import=%x\r\n", hFtpManager->Threaddata[0].flag_import);
            nret = ftp0_import_f();
            ftp0_import_result_send(nret);
            
            hFtpManager->Threaddata[0].flag_import = 0;
            hFtpManager->Threaddata[0].flag_busy = 0;
        }else{      //当没有升级时阻塞            
            hFtpManager->Threaddata[0].Thread_wait_flag = 1;    
            OSSemPend(hFtpManager->Threaddata[0].ftpnotempty,0,&err);  
            hFtpManager->Threaddata[0].Thread_wait_flag = 0;    
            //printf("hFtpManager OSSemPend\r\n");
        }
    }
    //return OK_T;
}




status_t ftp_create_f(void_t)
{    
	int nret,num;
	INT8U err;
	OS_CPU_SR cpu_sr;
    //hFtpManager = &FtpManager;
    hFtpManager = mymalloc(SRAMCCM,sizeof(Ftp_S));
    for(num=0;num<MAXFTPTHREADNUM;num++){
        
        hFtpManager->Threaddata[num].ftpmutex = OSSemCreate(0);  
        hFtpManager->Threaddata[num].threaddatamutex = OSSemCreate(0);  
        
        hFtpManager->Threaddata[num].seq = 0;
    	hFtpManager->Threaddata[num].Ftp_wait_flag = 0;
    	hFtpManager->Threaddata[num].Thread_wait_flag = 0;
        hFtpManager->Threaddata[num].flag_isp = 0;            
        hFtpManager->Threaddata[num].flag_busy = 0;
        hFtpManager->Threaddata[num].flag_import = 0;
        hFtpManager->Threaddata[num].flag_export = 0;
        hFtpManager->Threaddata[num].flag_isp_arm = 0;
        hFtpManager->Threaddata[num].programchecksum = 0;
        hFtpManager->Threaddata[num].ftp_board_type = 0;
        
        hFtpManager->Threaddata[num].ftpnotempty = OSSemCreate(0);
		os_obj_create_check("hFtpManager->Threaddata[num].ftpnotempty","ftp_create_f",(hFtpManager->Threaddata[num].ftpnotempty)?OS_ERR_NONE:(OS_ERR_NONE+1));
		hFtpManager->Threaddata[num].threadnotempty = OSSemCreate(0);
		os_obj_create_check("hFtpManager->Threaddata[num].threadnotempty","ftp_create_f",(hFtpManager->Threaddata[num].threadnotempty)?OS_ERR_NONE:(OS_ERR_NONE+1));
		RingBuffer_Init(&hFtpManager->Threaddata[num].ring, hFtpManager->Threaddata[num].ring_buff, 1, RING_BUFF_SIZE);           
    }
    RingBuffer_Init(&tx_trap_ring, tx_trap_buff, 1, TX_TRAP_SIZE);
    RingBuffer_Init(&tx_export_ring, tx_export_buff, 1, TX_EXPORT_SIZE);
        
    USERFTP_TASK_STK = mymalloc(SRAMCCM,USERFTP_TASK_STK_SIZE*4);
       		
	OS_ENTER_CRITICAL();	//关中断
	err = OSTaskCreate(user_ftp0_thread,(void*)0,(OS_STK*)&USERFTP_TASK_STK[USERFTP_TASK_STK_SIZE-1],USERFTP_TASK_PRIORITY); //创建TCP服务器线程
	/* do check */
	os_obj_create_check("user_ftp0_thread","ftp_create_f", err);
	
	OS_EXIT_CRITICAL();		//开中断			
    return OK_T;
}


status_t ftp_destroy_f(void_t)
{    
    hFtpManager = NULL;
    return OK_T;    
}



