/*
*********************************************************************************************************
*    \'-.__.-'/     | Company  :o--Shen Zhen xxxx Technology Co,.Ltd--o
*    / (o)(o) \     | Website  :o--http://www.xxxx.com.cn--o
*    \   \/   /     | Copyright: All Rights Reserved
*    /'------'\     | Product  : xxxx
*   /,   ..  , \    | File     : tftpclient.c
*  /// .::::. \\\   | Descript : basic tftp client implementation for IAP
* ///\ :::::: /\\\  | Version  : V0.10
*''   ).''''.(   `` | Author   : nicholasldf
*====(((====)))==== | EditTime : 2016-04-18-10:00
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "malloc.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/udp.h"
#include "types.h"
#include "err.h"
#include "httpd_cgi.h"
#include "httpd_ssi.h"
#include "user_iap.h"
#include "mainconfig.h"
#include "w25xxx.h" 
#include "ftpclient.h"
#include "wgcmd_ftp.h"
#include "tftp_client.h"
#include "webserver_update.h"
#include "usr_prio.h"

#if  0
OS_EVENT * tftp_start_sem;
ip_addr_t  tftp_server_ip;
/* private variables */
static struct udp_pcb * tftp_pcb;
static OS_EVENT * tftp_recv_sem;
/* the object is used to pass arguments */
static struct tftp_recv_args tftp_args;
/* business variables ---------------------------------------------------------*/
static uint16_t  tftp_remote_port;
static uint32_t  tftp_prev_block, tftp_cur_block;
static uint8_t * tftp_frame_buffer;
static u32       tftp_flash_addr;
static u32       tftp_check_sum;
static u32       tftp_file_len;
static UpdateFilehead_S tftp_update_filehead;
#endif


//线程栈地址
static OS_STK * USER_TFTP_CLIENT_TASK_STK;

/* tftp control block */
struct tftp_control * tftp_manager;



/**
  * encapsulate "udp_sendto"
  */
static err_t tftp_sendto(struct udp_pcb * pcb,const u8 * buf,u16 len,ip_addr_t dst_ip,u16_t dst_port){
	/* Chain of pbuf's to be sent */
	struct pbuf * pkt_buf; 
	err_t err;
	
	/* PBUF_TRANSPORT - specifies the transport layer */
	pkt_buf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);

	/*if the packet pbuf == NULL exit and EndTransfertransmission */
	if (!pkt_buf) {
		printf("fail to allocate pkt_buf(in tftp_sendto)!\r\n");
		return ERR_MEM;
	}
	
	/* Copy the original data buffer over to the packet buffer's payload */
	memcpy(pkt_buf->payload, buf , len);
	
	/* Sending packet by UDP protocol */
	err = udp_sendto(pcb,pkt_buf,&dst_ip,dst_port);
	
	/* free the buffer pbuf */
	pbuf_free(pkt_buf);
	return err;
}


static int tftp_handle_data(const u8 * p , int len ){
	u32 i , nret;

	//wireshark_format_printf( p, len);
	/* accumulate file length */
	tftp_manager->file_len += len;
	if( tftp_manager->file_len > Update_Flash_Memory_size ){
		printf("file size too large(%d)\r\n" , tftp_manager->file_len);
		return -1;
	}
	/* FIRST HANDLE */
	if( !(tftp_manager->check_sum) ){
		/* save "UpdateFilehead_S" */
		memcpy(&(tftp_manager->update_filehead) , p , sizeof(UpdateFilehead_S));
		/* do check sum, cut "UpdateFilehead_S" part */
		for(i=sizeof(UpdateFilehead_S) ; i<len ; ++i){
			tftp_manager->check_sum += p[i];
		}
		/* file_len为bin文件大小 */
		//tftp_manager->file_len -= sizeof(UpdateFilehead_S);
	}
	/* do check sum */
	else{
		for(i=0 ; i<len ; ++i){
			tftp_manager->check_sum += p[i];
		}
	}

	/* write into flash 1st */
	nret = SPI_Flash_Write_Page_Nret(p, tftp_manager->flash_addr, SPI_FLASH_PAGE_SIZE);      
	tftp_manager->flash_addr += nret;
	//printf("write %d bytes into flash\r\n\r\n" , nret);
	OSTimeDlyHMSM(0,0,0,10); // delay for 10 ms
	return 0;
}

/**
  * @brief Sets the TFTP opcode 
  * @param  buffer: pointer on the TFTP packet
  * @param  opcode: TFTP opcode
  * @retval none
  */
static void tftp_set_opcode(uint8_t * buffer, tftp_opcode opcode)
{
	*((u16*)buffer) = htons((u16)opcode);
}

/**
  * @brief Sets the TFTP block number 
  * @param packet: pointer on the TFTP packet 
  * @param  block: block number
  * @retval none
  */
static void tftp_set_block(uint8_t * buffer, uint16_t block)
{
//	uint16_t *p = (uint16_t *)buffer;
//	p[1] = ntohs(block);
	((u16*)buffer)[1] = htons(block);
}

/**
  * @brief Sends TFTP ACK packet  
  * @param to: pointer on the receive IP address structure
  * @param to_port: receive port number
  * @param block: block number
  * @retval: err_t: error code 
  */
static err_t tftp_send_ack_packet(int block)
{
	/* define the first two bytes of the packet */
	tftp_set_opcode(tftp_manager->frame_buffer, TFTP_ACK);
	
	/* Specify the block number being ACK'd.
	* If we are ACK'ing a DATA pkt then the block number echoes that of the DATA pkt being ACK'd (duh)
	* If we are ACK'ing a WRQ pkt then the block number is always 0
	* RRQ packets are never sent ACK pkts by the server, instead the server sends DATA pkts to the
	* host which are, obviously, used as the "acknowledgement".  This saves from having to sEndTransferboth
	* an ACK packet and a DATA packet for RRQs - see RFC1350 for more info.  */
	tftp_set_block(tftp_manager->frame_buffer, block);
	
	/* Sending packet by UDP protocol */
	//sendto(TFTP_SOCKET_NUM, tftp_manager->frame_buffer, 4, tftp_manager->server_ip, tftp_manager->server_port);
	return tftp_sendto(tftp_manager->pcb, tftp_manager->frame_buffer, 4, tftp_manager->server_ip, tftp_manager->server_port);
}

/**
  * @brief Sends TFTP error packet  
  * @param errorcode: error code indicating the nature of the error
  * @param errorStr: human consumption infomation
  */
static err_t tftp_send_error_packet(tftp_errorcode err_code, const char *err_str)
{
	int len;
	
	/* define the first two bytes of the packet */
	tftp_set_opcode(tftp_manager->frame_buffer, TFTP_ERROR);
	
	//set errorcode
	(tftp_manager->frame_buffer)[2] = 0;
	(tftp_manager->frame_buffer)[3] = err_code;
	//set error string
	strcpy((char*)(tftp_manager->frame_buffer) + 4, err_str);
	
	len = strlen(err_str) + 5;
	/* Sending packet by UDP protocol */
	//sendto(TFTP_SOCKET_NUM, tftp_manager->frame_buffer, len, tftp_manager->server_ip, tftp_manager->server_port);
	return tftp_sendto(tftp_manager->pcb,tftp_manager->frame_buffer,len,tftp_manager->server_ip,tftp_manager->server_port);
}

/**
  * @brief Sends TFTP read file request packet  
  * @param tftp_filename: file name of the file which tftpclient want to read from tftpserver
  * @retval: err_t: error code 
  */
static err_t tftp_send_readfile_request(const char *tftp_filename)
{
	int len ;
	unsigned char * ptr = tftp_manager->frame_buffer;
	
	//set opcode
	tftp_set_opcode(ptr , TFTP_RRQ );
	ptr += 2;
	
	//set filename
	strcpy((char*)ptr , tftp_filename);
	ptr += strlen(tftp_filename)+1;
	
	//set mode
	strcpy((char*)ptr, "octet");
	ptr += strlen("octet")+1;

	//set blcok size
	ptr += sprintf((char*)ptr , "blksize%c%d%c", 0, TFTP_BLOCK_SIZE , 0);

	/* 详见rfc1350 */
	len = ptr - tftp_manager->frame_buffer;

/*
	printf("tftp_send_readfile_request : \r\n");
	for(i=0 ; i<len ;++i){
		if((tftp_manager->frame_buffer)[i]){
			int PutChar(int);
			PutChar(tftp_manager->frame_buffer[i]);
		}
		else
			printf("\\0");
	}
	printf("\r\n");
*/
	//send packet
	//net_send_udp_packet(net_server_ethaddr, tftp_remote_ip, tftp_manager->server_port, tftp_our_port, len);
	//sendto(TFTP_SOCKET_NUM, tftp_manager->frame_buffer, len, tftp_manager->server_ip, tftp_manager->server_port);
	return tftp_sendto(tftp_manager->pcb,
	                   tftp_manager->frame_buffer,
	                   len,
	                   tftp_manager->server_ip,
	                   tftp_manager->server_port);
}


/**
  * The callback is RESPONSIBLE FOR FREEING THE PBUF
  * if it's not used any more.
  */
static void tftp_recv_callback(void *arg, struct udp_pcb * pcb, struct pbuf *pkt_buf,ip_addr_t *addr, u16_t port)
{
	//printf("tftp_recv_callback called(len=%d)\r\n",pkt_buf->tot_len);
	tftp_manager->recv_args.arg = arg;
	tftp_manager->recv_args.pcb = pcb;
	tftp_manager->recv_args.pkt_buf = pkt_buf;
	/* significant item */
	/* cpy data from pbuf */
	memcpy( tftp_manager->frame_buffer, pkt_buf->payload, pkt_buf->tot_len);
	/* pass length argument */
	tftp_manager->recv_args.len  = pkt_buf->tot_len;
	tftp_manager->recv_args.addr = addr;
	tftp_manager->recv_args.port = port;

	/* free the pbuf */
	pbuf_free(pkt_buf);
	OSSemPost(tftp_manager->recv_sem);
}

static void tftp_firmware_update_init(void){
	char tmp_ip_buf[32];
	/* 初始化ip和port */
	printf("INIT: tftp_manager->server_ip: %s\r\n" , inet_ntoa_r(tftp_manager->server_ip,tmp_ip_buf,sizeof(tmp_ip_buf)));
	tftp_manager->server_port = TFTP_WELLKNOWN_PORT;
	
	tftp_manager->prev_block = 0;
	tftp_manager->cur_block = 0;
	/* initializations */
	tftp_manager->flash_addr = SPI_Flash_Get_User_Code_Addr();
	printf("tftp_manager->flash_addr: %#x\r\n" , tftp_manager->flash_addr);
	tftp_manager->check_sum = 0;
	tftp_manager->file_len = 0;
	// erase flash blocks for update datas 
	printf("erase flash for updating...\r\n");
	SPI_Flash_Update_Erase();

	OSTimeDlyHMSM(0,0,2,0);  // delay for some secs
}

static int tftp_firmware_download_finish(void){
	INT8U err;
	// 检查板型
	if(tftp_manager->update_filehead.board_type != STM32F407_MODULE_BOARD_TYPE){  
		printf("ERROR: inconsistent board type(%#x, should be %#x)!\r\n",
		       tftp_manager->update_filehead.board_type,
		       STM32F407_MODULE_BOARD_TYPE);
		return -1;
	}
	if(tftp_manager->update_filehead.checksum != tftp_manager->check_sum){
		printf("ERROR: inconsistent check sum(%lu, should be %lu)!\r\n",
		       tftp_manager->check_sum,
		       tftp_manager->update_filehead.checksum);
		return -1;
	}       
	// "ARM应用程序"
	if(tftp_manager->update_filehead.program_type != 1){                 
		printf("ERROR: inconsistent program type(%d, should be 1)!\r\n",
		       tftp_manager->update_filehead.program_type );
		return -1;
	}
	/* 通过检查 */
	update_cfg.update_spiflash_checksum = tftp_manager->check_sum;   
	printf("update_cfg.update_spiflash_checksum = %lu\r\n",update_cfg.update_spiflash_checksum);
	update_cfg.update_start_address = SPI_Flash_Get_User_Code_Addr();
	printf("update_cfg.update_start_address = %#x\r\n",update_cfg.update_start_address);
	update_cfg.update_size = tftp_manager->file_len;
	printf("update_cfg.update_size = %d\r\n",update_cfg.update_size);
	update_cfg.need_update_flag = 1;
	update_cfg.boot_just_update_flag = 0;
	//置升级标志
	main_config_write_update_f((uint8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));   
	/* reboot */
	printf("rebooting...\r\n\r\n");

	//update_refresh_time = 0;
	update_ssi_status = SSI_STATUS_UPDATE_SUCCEED;
	/* 等待web服务器返回页面再重启，否则浏览器看不到"成功" */
//	OSSemSet(update_showpage_sem, 0, &err);
//	OSSemPend(update_showpage_sem, 0 , &err);
	OSTimeDlyHMSM(0,0,5,0);
	control_set_sysreset();
	return 0;
}

/**
  * @brief  try to read firmware file form TFTP server and write to stm32 flash
  * @param  none
  * @retval error code
  */
static int tftp_firmware_update(void)
{
	INT8U sem_err;
	unsigned short change_flag = 0, proto;
	unsigned int len;

	/*** init ***/
	tftp_firmware_update_init();
	printf("tftp_firmware_update : start\r\n");
	
	//requet read firmware file from server
	tftp_send_readfile_request(update_filename);
	for(;;){
		/* 清空信号量 */
		OSSemSet(tftp_manager->recv_sem , 0 ,&sem_err);
		/* wait response for 5 seconds */
		OSSemPend(tftp_manager->recv_sem, 5*OS_TICKS_PER_SEC , &sem_err);
		/* 超时，升级失败 */
		if( OS_ERR_TIMEOUT == sem_err ){
			printf("timeout, no response from server\r\n");
			return -1;
		}
		len = tftp_manager->recv_args.len;
		
#if  0
		/* ip网段会发生变化，暂不作检查 */
		if( (tftp_manager->recv_args.addr->addr) != tftp_manager->server_ip.addr){
			char tmp_str[16];
			printf("tftp_manager->server_ip error(%s), continue\r\n",
			       inet_ntoa_r(*(tftp_manager->recv_args.addr),tmp_str,sizeof(tmp_str)));
			continue;
		}
#endif

		//the port may change, due to server may reserve wellkonw UDP port for other client
		/* A requesting host chooses its source TID as described above, and sends
		its initial request to the known TID 69 decimal (105 octal) on the
		serving host.  The response to the request, under normal operation,
		uses a TID chosen by the server as its source TID and the TID chosen
		for the previous message by the requestor as its destination TID.
		The two chosen TID's are then used for the remainder of the transfer.
		*/
		if((0 == change_flag) && (tftp_manager->recv_args.port != tftp_manager->server_port)) {
			printf("tftp_firmware_update : remote server port change from %d to %d\r\n", tftp_manager->server_port, tftp_manager->recv_args.port);
			tftp_manager->server_port = tftp_manager->recv_args.port;
			change_flag = 1;
		}
		else if((1 == change_flag) && (tftp_manager->recv_args.port != tftp_manager->server_port)){
			//error
			printf("tftp_firmware_update : Error port (data from invalid tid)\r\n");
			tftp_send_error_packet(TFTP_ERR_UKNOWN_TRANSFER_ID, "Unknown TID");
			return -1;
		}
		
		//check packet len
		if (len < 2)
			return -1;
		len -= 2;
		
		/* get tftp operate code */
		proto = *( (unsigned short *)tftp_manager->frame_buffer );
		proto = ntohs(proto);
		switch (proto) {
			//-----------------------------------data packet-----------------------------------
			case TFTP_DATA:{
				//printf("opcode: TFTP_DATA\r\n");
				//set len equal to effective data counts
				if (len < 2) 
					return -1;
				len -= 2;
				
				/*
				 * RFC1350 specifies that the first data packet will
				 * have sequence number 1. If we receive a sequence
				 * number of 0 this means that there was a wrap
				 * around of the (16 bit) counter.
				 */
				tftp_manager->cur_block = ntohs( *( (unsigned short *)(tftp_manager->frame_buffer+2) ) );
				if ( (tftp_manager->cur_block == 0) || (tftp_manager->cur_block > (tftp_manager->prev_block + 1)) ) {
					printf("tftp_firmware_update : block error, tftp_manager->prev_block(%d), tftp_manager->cur_block(%d)\r\n", \
						tftp_manager->prev_block, tftp_manager->cur_block);
					return -1;
				}
				
				//printf("block(%d) size(%d)  \r\n", tftp_manager->cur_block, len);
				printf("[B%d,%d]%s",tftp_manager->cur_block,len,len==TFTP_BLOCK_SIZE?"-":"\r\n");
				/* previous old block or Same block again; ignore it. */
				if (tftp_manager->cur_block <= tftp_manager->prev_block) {
					//Acknowledge the block just received, which will prompt the remote for the next one.
					tftp_send_ack_packet(tftp_manager->cur_block);
					break;
				}
				
				tftp_manager->prev_block = tftp_manager->cur_block;
				/* Does this packet have any valid data to write? */
				if ( (0<len) && (len<=TFTP_BLOCK_SIZE) ) {
					/* Write received data in Flash */
					if(tftp_handle_data(tftp_manager->frame_buffer+4,len) < 0){
						printf("tftp_handle_data failed(returned a negative value)!\r\n");
						return -1;
					}
				}
				else{
					printf("ERROR: data packet length exceeds %d!\r\n",TFTP_BLOCK_SIZE);
					return -1;
				}
				
				/*
				 *	Acknowledge the block just received, which will prompt
				 *	the remote for the next one.
				 */
				tftp_send_ack_packet(tftp_manager->cur_block);
				
				/* If the last write returned less than the maximum TFTP data pkt length,
				 * then we've received the whole file and so we can quit (this is how TFTP
				 * signals the EndTransferof a transfer!)
				*/
				if (len < TFTP_BLOCK_SIZE ) {
					printf("transmission completed, size of file received: %d\r\n",(tftp_manager->cur_block-1)*TFTP_BLOCK_SIZE+len);
					return tftp_firmware_download_finish();
				}
				break;
			}
			//-----------------------------------error packet-----------------------------------
			case TFTP_ERROR: {
				printf("opcode: TFTP_ERROR\r\n");
				printf("ERROR: \"%s\" error_code: %d\r\n", 
					(tftp_manager->frame_buffer+4), ntohs(*(unsigned short *)(tftp_manager->frame_buffer+2)));
				/*switch (ntohs(*(unsigned short *)pkt)) {
				case TFTP_ERR_FILE_NOT_FOUND:
				case TFTP_ERR_ACCESS_DENIED:
					puts("Not retrying...\n");
					eth_halt();
					net_set_state(NETLOOP_FAIL);
					break;
				case TFTP_ERR_UNDEFINED:
				case TFTP_ERR_DISK_FULL:
				case TFTP_ERR_UNEXPECTED_OPCODE:
				case TFTP_ERR_UNKNOWN_TRANSFER_ID:
				case TFTP_ERR_FILE_ALREADY_EXISTS:
				default:
					puts("Starting again\n\n");
					net_start_again();
					break;
				}*/
				return -1;
			}
			/* option acknowledgement */
			case TFTP_OACK: {
				printf("proto: TFTP_OACK\r\n");
				tftp_send_ack_packet(0);
				break;
			}
			default: {
				printf("tftp_firmware_update : Illegal operation code(%d)\r\n", proto);
				tftp_send_error_packet(TFTP_ERR_ILLEGALOP, "Illegal operation");
				return -1;
			}
		}
	}
}

static void user_tftp_client_thread(void *p_arg){
	/* create a semphore for tftp thread */
	tftp_manager->start_sem = OSSemCreate(0);
	tftp_manager->recv_sem  = OSSemCreate(0);
	//update_showpage_sem = OSSemCreate(0);
	
	tftp_manager->pcb = udp_new();

	/* allocation succeed or not */
	if( !tftp_manager->pcb ){
		printf("failed to allocate udp_pcb(in user_tftp_client_thread)!\r\n");
		for(;;){
			OSTimeDlyHMSM(0,1,0,0);
		}
	}

	/* set callback of udp recv */
	udp_recv(tftp_manager->pcb, tftp_recv_callback, NULL);

	for(;;){
		int ret;
		INT8U sem_err;
		OSSemSet(tftp_manager->start_sem, 0, &sem_err);
		OSSemPend(tftp_manager->start_sem , 0 , &sem_err);
		ret = tftp_firmware_update();
		if(ret < 0){
			// 失败标志(用于显示)
			*update_filename = 0;
			update_ssi_status = SSI_STATUS_UPDATE_FAILED;
			//update_refresh_time = 0;
			printf("tftp_firmware_update failed!\r\n");
		}
		else{
			printf("tftp_firmware_update succeed!\r\n");
		}
	}
}

status_t tftp_client_create_f(void_t) {
	INT8U err;
	OS_CPU_SR cpu_sr;

	tftp_manager = mymalloc(SRAMCCM,sizeof(*tftp_manager));
//	tftp_manager = mymalloc(SRAMEX,sizeof(*tftp_manager));
	if( !tftp_manager ){
		printf("failed to allocate tftp_manager(in tftp_client_create_f)!\r\n");
		return ERROR_T;
	}

	tftp_manager->frame_buffer = mymalloc(SRAMCCM,TFTP_BLOCK_SIZE*2);
//	tftp_manager->frame_buffer = mymalloc(SRAMEX,TFTP_BLOCK_SIZE*2);
	if( !(tftp_manager->frame_buffer) ){
		printf("failed to allocate tftp_manager->frame_buffer(in tftp_client_create_f)!\r\n");
		return ERROR_T;
	}
	USER_TFTP_CLIENT_TASK_STK = mymalloc(SRAMCCM,USER_TFTP_CLIENT_TASK_STK_SIZE*4);
//	USER_TFTP_CLIENT_TASK_STK = mymalloc(SRAMEX,USER_TFTP_CLIENT_TASK_STK_SIZE*4);
	if( !USER_TFTP_CLIENT_TASK_STK ){
		printf("failed to allocate USER_TFTP_CLIENT_TASK_STK(in tftp_client_create_f)!\r\n");
		return ERROR_T;
	}
	OS_ENTER_CRITICAL();	//关中断

	err = OSTaskCreate(
		user_tftp_client_thread,
		(void*)0,
		(OS_STK*)&USER_TFTP_CLIENT_TASK_STK[USER_TFTP_CLIENT_TASK_STK_SIZE-1],
		USER_TFTP_CLIENT_TASK_PRIORITY
	); 
	os_obj_create_check("user_tftp_client_thread","tftp_client_create_f",err);
	
	OS_EXIT_CRITICAL();		//开中断			
    return OK_T;
}

int tftp_get_progress_percent(void){
	return tftp_manager->file_len*100u/tftp_manager->total_file_len;
}
