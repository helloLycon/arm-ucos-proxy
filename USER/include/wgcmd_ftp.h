#ifndef HJ_FTP_H
#define HJ_FTP_H

//驱动与应用程序的公共头文件，需要包括的头文件不同。


#include "mainconfig.h"
#include "hj_nm.h"

#include "ring_buffer.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "hj80_socket.h"

#define TX_TRAP_SIZE 512	/* Send */
#define local_ftp0_file "ftp0.bin"

#define FILE_FTP0_BIN "ftp0.bin"   //文件开头

#define PATH "/usr/local/bin"       //工作路径

#define FLASH_PORT_BYTES_CNT 88

#define MAX_ONCE_DOWNLOAD_BYTES (2+FLASH_PORT_BYTES_CNT)  // 2个字节数据长度+88字节数据内容
#define FTP_DOWNLOAD_TIMES 5
#define DOWNLOAD_THREAD_WAIT_TIMEOUT 1000
#define FTP_DOWNLOAD_END_TIMES 5

struct _tagportfilehead_S{   
    
    u16_t size;     //文件大小，仅为程序文件大小不包括文件头。    
    u32_t unuse1;
   	u32_t board_type;
    u8_t version;
    u8_t unuse[5];  //预留。
}__attribute__((packed));


typedef struct _tagportfilehead_S PortFilehead_S;



#define SIZE_320K 327680 
#define SIZE_1M 1048576     //文件大小判定
#define SIZE_64K 65536     //文件大小判定
#define SIZE_32K 32768     //文件大小判定

#define THREAD_WAIT_TIMEOUT 2 // 1s

#define HJ80_ISP_SIZE    1024

struct _tagupdatefilehead_S{
    u8_t program_type;     //程序类型，1：应用程序，2：驱动。3：MCU应用程序
    u8_t version;
	u32_t board_type;
    u8_t tar;      //是否压缩，1：程序压缩，2：程序没有压缩。
	u8_t unuse1;    
    u32_t size;     //文件大小，仅为程序文件大小不包括文件头。
    u32_t checksum; //文件和检验1048576
    u8_t unuse2[48]; //预留。
}__attribute__((packed));


typedef struct _tagupdatefilehead_S UpdateFilehead_S;

struct _tagftpcmddata_S{
	u32_t ftpserver_ip;
	u8_t filepath[64];
	UpdateFilehead_S filehead;
	u8_t unuse[28];
}__attribute__((packed));
typedef struct _tagftpcmddata_S ftpcmddata_S;



#define RING_BUFF_SIZE 256

typedef struct tagthreaddata_S
{
    //XBUFFER_S   * sbuf;
    //void_t  *hThread;
    //bool_t  b_loop;
    //bool_t  b_exit;     
		RINGBUFF_T ring;
		uint8_t ring_buff[RING_BUFF_SIZE];
    OS_EVENT * threaddatamutex; 
    OS_EVENT * ftpmutex; 
    OS_EVENT *  threadnotempty;        
    OS_EVENT *  ftpnotempty;        
    hj_socket_data_t socket_data;   //作为目的地址
    hj_nm_cmd_frame_t ackframe;
    bool_t flag_isp;
	bool_t flag_isp_arm;
    bool_t flag_import;
    bool_t flag_export;
    bool_t flag_busy;
	u32_t ftp_board_type;
    u32_t programchecksum;
    //u8_t cmd_frame_buff[256];   //保存当前的完整命令帧

   u32_t filelen;
	u8_t seq;
	u32_t Ftp_wait_flag;
	u32_t Ftp_isp_wait_cmdext;
	u32_t Thread_wait_flag;
	ftpcmddata_S ftp_cmddata;
	u32_t localorremote;
	u32_t sendaddr;
	OS_EVENT * hMutex;
	u8_t cmd_frame_buff[256];       //保存当前的完整命令帧
}Threaddata_S;

#define MAXFTPTHREADNUM  1

typedef struct tagftp_S
{    
  Threaddata_S Threaddata[MAXFTPTHREADNUM];
    
	hj_socket_data_t socket_data;   //作为目的地址
  //hj_nm_cmd_frame_t ackframe;
	
	
}Ftp_S;

extern struct tagUpdateConfigS update_cfg;



#endif//HJ80_FTP_M_H





