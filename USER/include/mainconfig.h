/*
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_


#include "types.h"
#include "webserver_account.h"

struct tagBroadcastDataS{
    uint32_t host_ip;
    uint32_t host_port;
    uint32_t jump_ip;
    uint32_t jump_port;
    uint32_t trap_ip;
    uint32_t trap_port;

    uint32_t gateway;
//    uint32_t device_id;
    uint32_t tserver_ip;
    uint32_t netmask;
    uint32_t reserve1[4];
    uint8_t mac[6];
    uint32_t arm_version;
    //uint32_t ftp_ip;
    uint32_t reserve2[4];
    uint8_t device_name[20];
    uint32_t device_seq;
    uint8_t reserve3[100];	
}__attribute__((packed));


#define NM_TCP_SERVER_PORT             5768
#define DEFAULT_JUMP_PORT               16680
#define UDP_BOARDCAST_PORT             7000

#define UDP_ST_RECV_PORT 3760
#define UDP_ST_SEND_PORT 3761

#define BROADCAST_PORT_SEND 6000
#define BROADCAST_PORT_FROM 6000
#define INTERFACE_NAME  "eth0"
#define ADD			1
#define DEL			2




//#define STM32F4XX_BOOT 

#define STM32F4XX_APP


#define MAINCONFIG_BUFFSIZE (256)



#define MAINCONFIG_ADDR 0   
/* 20171103 */
#define WEB_ACCOUNT_PASSWD_ADDR  4096  



#define USERPROGRAM_ADDR 4*4096      

#define DEVICE_SEQ 0				//序列号
#define DEVICE_NAME  "SDH5155 2U"	//设备名称		

/*  main.c   */

#if 0
struct tagGlobalConfigS {

    uint32_t host_ip;
    uint16_t host_port;
    uint32_t netmask;
    uint32_t gateway;
	uint8_t mac[6];   
    uint32_t jump_ip;
    uint16_t jump_port;
    uint32_t trap_ip;
    uint16_t trap_port;
    uint32_t tserver_ip;
    uint8_t arm_version;
	
	uint16_t device_id;	
	uint8_t update_flag;
	//uint32_t ftp_ip;
    uint8_t unuse[24];
	
    uint16_t crc;
}__attribute__((packed));
#endif



struct tagGlobalConfigS {

    uint32_t host_ip;
    uint16_t host_port;
    uint32_t netmask;
    uint32_t gateway;
	uint8_t mac[6];   
    uint32_t jump_ip;
    uint16_t jump_port;
    uint32_t trap_ip;
    uint16_t trap_port;
    uint32_t tserver_ip;    
    uint8_t arm_version;
	
	uint8_t baud;
	uint8_t stop_bits;
	uint8_t boot_version;
	uint8_t update_flag;
	uint16_t device_id;
	
	//uint32_t ftp_ip;
    uint8_t unuse[20];
		
    uint16_t crc;
}__attribute__((packed));

struct tagUpdateConfigS {

    uint32_t update_size;  
    uint32_t update_spiflash_checksum;
    uint32_t update_start_address;
		uint32_t update_stmflash_checksum;
		uint8_t need_update_flag;
		uint8_t boot_just_update_flag;
		uint8_t boot_update_result;
		uint8_t boot_update_flash_powerdown0;
	uint8_t boot_update_flash_powerdown1;
		uint8_t unuse[13];
    uint16_t crc;
}__attribute__((packed));



#define BAUDRATE_2400 0
#define BAUDRATE_4800 1
#define BAUDRATE_9600 2
#define BAUDRATE_19200 3
#define BAUDRATE_38400 4
#define BAUDRATE_57600 5
#define BAUDRATE_115200 6


#define MODE_TCP_SERVER 1
#define MODE_UDP_SERVER 2

//baudrate:: 0:2400,1:4800,2:9600:3:19200,4:38400,5:57600,6:115200
struct tagDeviceConfigS {

    uint8_t baud;  //波特率 选择
    uint8_t stop_bits; //工作模式选择
    uint8_t unuse[6];
    uint16_t crc;
}__attribute__((packed));

struct tagMainConfigS {
    struct tagGlobalConfigS Global;
    struct tagDeviceConfigS Device;
		struct tagUpdateConfigS Update;
    OS_EVENT * hMutex;
}__attribute__((packed));
typedef struct tagMainConfigS MAINCONFIG_S;

#define MainConfig_Size (sizeof(MAINCONFIG_S))


#define MAIN_CFG_FILE_NAME              "main.cfg"
#define MAIN_CONFIG_VERSION                 "1.0.0"


int32_t main_config_init_f(void_t);
int32_t main_config_release_f(void_t);
int32_t main_config_read_gloabl_f(uint8_t *pData, uint32_t len);
int32_t main_config_write_gloabl_f(uint8_t *pData, uint32_t len);
int32_t main_config_save_f(MAINCONFIG_S *pConfig);
int32_t main_config_get_trap(uint32_t * p_trap_ip, uint32_t * p_trap_port);
//int32_t main_config_get_control_com1_baud(s32_t * baud, s32_t * sel);
//int32_t main_config_get_ms_ip(s32_t * p_slave_ip, s32_t * p_master_ip);
uint32_t main_config_get_main_host_ip(void_t);
int32_t main_config_set_main_host_ip(uint32_t ip);

uint32_t main_config_get_main_host_mask(void_t);
int32_t main_config_set_main_host_mask(uint32_t netmask);
uint32_t main_config_get_main_gateway(void_t);
int32_t main_config_set_main_gateway(uint32_t gateway);
uint8_t * main_config_get_main_mac(void_t);
int32_t main_config_set_main_mac(uint8_t * p_mac);
//int32_t main_config_set_main_mac(uint8_t * p_mac);
void * main_config_get_h_main(void);
int32_t main_config_refresh_h_main(void);

/* 20171103 */
u16_t crc16_calc_f(void *buf, u32_t size);
void web_account_write(struct web_account * acct);
int web_account_read(struct web_account * acct);
uint32_t main_config_get_bootversion(void);
uint32_t main_config_get_appversion(void);
uint32_t main_config_get_main_host_ip(void_t);
uint32_t main_config_get_jump_ip(void);
uint32_t main_config_get_jump_port(void);
uint32_t main_config_get_main_host_port(void_t);
void main_config_read_dev_ip(char * dst);
void main_config_read_dev_port(char * dst);
void main_config_read_dev_mask(char * dst);
void main_config_read_dev_gateway(char * dst);
void main_config_read_dev_mac(char * dst);
void main_config_read_mng_ip(char * dst);
void main_config_read_mng_port(char * dst);
void main_config_read_v1(char * dst);
void main_config_read_v2(char * dst);


//int32_t ConfigSave(uint8_t *filename, MAINCONFIG_S *pConfig);
//
///*  id.c   */
//
//#define NEWID 0x01234567   //导入导出数据时需要进行类型区分，以区分不同数据格式
//
//#define NEWID_SIZE 32   //
//
//struct taglpc3250idS{
//    uint32_t id;
//}__attribute__((packed));
//
//
//struct tagNewIdS{
//
//    uint8_t buff[NEWID_SIZE];
//
//}__attribute__((packed));
//
//typedef struct tagNewIdS NEWID_S;
//
//#define NewId_Size (sizeof(NEWID_S))
//
//
//#define ID_CFG_FILE_NAME        "id.cfg"
//#define ID_CONFIG_VERSION       "1.0.0"
//
//int32_t id_config_init_f(void_t);
//int32_t id_config_release_f(void_t);
//int32_t id_config_Save_f(void_t);
//int32_t IdConfig_read_f(NEWID_S *pData, s32_t len);
//void *  IdConfig_read_all_f(void_t);
//void * id_config_get_h_id(void);
//
//#define ARM_ID_FG 0xfeef0110

/*  all    */

//struct tagFULLCONFIGS {
//    uint32_t id_fg;
//    NEWID_S id;
//    MAINCONFIG_S main;  
//    uint8_t unuse[333];		//sizeof(hj_nm_cmd_frame_t) - sizeof(NEWID_S) - sizeof(MAINCONFIG_S)
//}__attribute__((packed));
//
//typedef struct tagFULLCONFIGS FULLCONFIG_S;
//
//#define FULLCONFIG_SIZE sizeof(FULLCONFIG_S)//全部配置数据大小


/*  config.c     */

//int32_t ConfigReadFile(s8_t *filename, s8_t *buf, s32_t *size);
//int32_t ConfigWriteFile(s8_t *filename, s8_t *buf, s32_t size);
//int32_t ConfigCRCCheck(s8_t *buf, s32_t size);
//int32_t ConfigCRCSet(s8_t *buf, s32_t size);
//
//s32_t ConfigMemeryGet(s8_t *mem, s8_t *buf, s32_t size);
//s32_t ConfigMemerySet(s8_t *mem, s32_t size, s8_t *buf);
//s32_t ConfigStringGet(s8_t* str, s8_t *buf, s32_t size);
//s32_t ConfigStringSet(s8_t *str, s8_t *buf);




#endif
