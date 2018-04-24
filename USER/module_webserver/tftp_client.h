#ifndef __TFTP_CLIENT_H
#define __TFTP_CLIENT_H



/* thread relative */
//任务优先级
//#define USER_TFTP_CLIENT_TASK_PRIORITY  16
//任务栈大小
#define USER_TFTP_CLIENT_TASK_STK_SIZE  512
//tftp服务端口
#define TFTP_WELLKNOWN_PORT		        69
#define TFTP_BLOCK_SIZE                 256








#define SPI_FLASH_PAGE_SIZE   256
#define STM32F407_MODULE_BOARD_TYPE 0x800d0001



/* TFTP opcodes as specified in RFC1350   */
typedef enum {
  TFTP_RRQ = 1,
  TFTP_WRQ = 2,
  TFTP_DATA = 3,
  TFTP_ACK = 4,
  TFTP_ERROR = 5,
  TFTP_OACK = 6,
} tftp_opcode;


/* TFTP error codes as specified in RFC1350  */
typedef enum {
  TFTP_ERR_NOTDEFINED = 0,
  TFTP_ERR_FILE_NOT_FOUND,
  TFTP_ERR_ACCESS_VIOLATION,
  TFTP_ERR_DISKFULL,
  TFTP_ERR_ILLEGALOP,
  TFTP_ERR_UKNOWN_TRANSFER_ID,
  TFTP_ERR_FILE_ALREADY_EXISTS,
  TFTP_ERR_NO_SUCH_USER,
} tftp_errorcode;


/* pass args from udp receive callback(can not take too long time) to tftp thread */
struct tftp_recv_args {
	void *arg;
	struct udp_pcb * pcb;
	struct pbuf * pkt_buf;
	int len;
	ip_addr_t * addr;
	u16_t port;
};

struct tftp_control {
	/* frame buffer for tftp transmission */
	uint8_t * frame_buffer;
	/* control block for udp transmission */
	struct udp_pcb * pcb;
	/* tftp server ip */
	ip_addr_t  server_ip;
	/* be 69 mostly */
	uint16_t   server_port;
	/* semaphore for starting a tftp download */
	OS_EVENT * start_sem;
	/* semaphore for udp receiving(post it when receiving a frame) */
	OS_EVENT * recv_sem;
	/* previous block number received from tftp server(refer to rfc1350 for more details) */
	uint32_t   prev_block;
	/* current block number received from tftp server(refer to rfc1350 for more details) */
	uint32_t   cur_block;
	/* flash addr to write into */
	u32        flash_addr;
	/* data check sum */
	u32        check_sum;
	/* file length of ".bin" file */
	u32        total_file_len;
	u32        file_len;
	/* update file head for checking(board type,checksum and so on...) */
	UpdateFilehead_S update_filehead;
	/* pass args from udp receive callback(can not take too long time) to tftp thread */
	struct tftp_recv_args recv_args;
};


//int tftpclient_version_check(void);
//int tftpclient_fireware_exist_check(void);
//int tftpclient_fireware_update(void);
status_t tftp_client_create_f(void_t);
int tftp_get_progress_percent(void);
extern struct tftp_control * tftp_manager;


#endif

