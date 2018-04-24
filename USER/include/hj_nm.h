#ifndef HJ_NM_H
#define HJ_NM_H

//驱动与应用程序的公共头文件，需要包括的头文件不同。



#include "types.h"
#include "mainconfig.h"
//#include "config.h"

#define HJ_NM_CMD_FRAME_DATA_LEN 128

#define PC_DEVICE_ID 0xffffffff
#define ARM_DEVICE_ID 0xfffffffe
#define WGDYB_DEVICE_ID 0	
#define WGDYB_DEVICE_TYPE 0x800d0000	
#define LOCAL_DEVICE_ID 0	
#define DEVICE_7200_TYPE 0x60070700

#define TRAP_CMD 7
#define JUMP_CMD 7
#define ALERTSYNC_CMD 6
#define FTP_UPDATE_CMD 7


#define LOCAL_DEVICE_ID 0

#define RESPBEAT_CMDEXT    0x100 
#define TRAPSEND_CMDEXT    0x101
#define ALERTSYNC_CMDEXT   0x102
#define ALERTSYNCEND_CMDEXT 0x102
#define FTP_UPDATE_CMDEXT 0x103


#define ST_START_QUICK_CMDEXT 0x112	//快速简单
#define ST_START_SLOW_CMDEXT 0x110	//慢速


#define ST_MIN_CMDEXT 0x110
#define ST_MAX_CMDEXT 0x120

#define BROADCAST_READ_CMDEXT 0x150
#define BROADCAST_WRITE_CMDEXT 0x151


#define UPDATE_CMDEXT 0x1f
#define IMPORT_CMDEXT 0x0f
#define EXPORT_CMDEXT 0x0e

#define CMD_FTP 0x0e
#define CMD_FTP_RESPONSE 0x0f


#define CMDEXT_UPDATE_START            0xf0
#define CMDEXT_UPDATE_PASSWORD_CONFIRM  0xf1
#define CMDEXT_UPDATE_ERASE             0xf2
#define CMDEXT_UPDATE_PROGAM            0xf3
#define CMDEXT_UPDATE_CHECK             0xf4
#define CMDEXT_UPDATE_VERSION           0xf5
#define CMDEXT_UPDATE_EXIT              0xf7

#define CMDEXT_IMPORT_FILE_HEAD         0xfa
#define CMDEXT_IMPORT_FILE_DATA			0xfb
#define CMDEXT_IMPORT_END           	0xfc

#define CMDEXT_EXPORT_FILE_HEAD         0xfd
#define CMDEXT_EXPORT_FILE_DATA			0xfe

#define THREAD_TRAP 0
#define THREAD_POLL 1
#define THREAD_TRAPSYNC 2
#define THREAD_TRAPSYNC_END 2
#define THREAD_JUMP 4
#define THREAD_DXC_REPORT 5
#define THREAD_BOOT_REPORT 6



#define THREAD_ST 0x08	//自检命令





#define THREAD_IMPORT 0x0d
#define THREAD_EXPORT 0x0e
#define THREAD_UPDATE 0x0f


#define THREAD_0_IMPORT 0x10
#define THREAD_0_EXPORT 0x11
#define THREAD_0_UPDATE 0x12


#define THREAD_1_IMPORT 0x13
#define THREAD_1_EXPORT 0x14
#define THREAD_1_UPDATE 0x15

#define THREAD_2_IMPORT 0x16
#define THREAD_2_EXPORT 0x17
#define THREAD_2_UPDATE 0x18

#define THREAD_3_IMPORT 0x19
#define THREAD_3_EXPORT 0x1a
#define THREAD_3_UPDATE 0x1b

#define PPPINITFCS16 ((u16_t) 0xFFFF)     //Initial FCS value
#define PPPGOODFCS16 ((u16_t) 0xF0B8)     // Good final FCS value


#define MAX_REMOTE_NUM 16	
#define MAX_DYB_NUM 16	


#define ALERT_LEVEL_BIT  5
#define ALERT_LEVEL_CRITICAL    (4)
#define ALERT_LEVEL_MAJOR       (3)
#define ALERT_LEVEL_MINOR       (2)
#define ALERT_LEVEL_WARNING     (1)
#define ALERT_LEVEL_MASK        (0)


#define ALERT_CRITICAL    (4)
#define ALERT_MAJOR       (3)
#define ALERT_MINOR       (2)
#define ALERT_WARNING     (1)
#define ALERT_MASK        (0)


struct _hj_nm_cmd_frame
{
    //unsigned int n_board_index;/*Board Index*/
    unsigned short int inuse;		/* in use*/
    unsigned short int n_data_len;/*data in buff*/
    unsigned char data_buff[HJ_NM_CMD_FRAME_DATA_LEN];
};

typedef struct _hj_nm_cmd_frame hj_nm_cmd_frame_t;

struct hj_nm_cmd_frame_list
{
    hj_nm_cmd_frame_t cmd_frame;
    struct hj_nm_cmd_frame_list * p_next;
};


struct hj_dmp_frame_head
{
    //u8_t pre;/*0x7e*/
    u8_t protocolVer;
	u32_t dst_netid;
    u32_t thread_netid;     /*线程网元ID        */  
	u16_t dst_deviceid;
    u16_t src_deviceid;
    u8_t boardIndex;
    u8_t interfaceIndex;
    u8_t server_seq;
    u8_t cmdExt;
    u8_t cmdId;	/*cmd id*/
    u8_t cmdStatus;
    u16_t cmdLen;/*data len*/
	//u8_t buf[0];
    // unsigned char data;
}__attribute__((packed));

struct hj_dmp_frame_tail
{
    u8_t crcl;/*crc low*/
    u8_t crch;/*crc high*/
    u8_t suf;/*0x7e*/
};

struct hj_dmp_frame
{
    struct hj_dmp_frame_head * phead;
    u8_t * pdata;
    struct hj_dmp_frame_tail * ptail;
};

struct hj_dump_frame_head
{
    u8_t pre;/*0x7e*/
    u8_t protocolVer;
	u32_t dst_netid;
    u32_t thread_netid;     /*线程网元ID        */  
	u16_t dst_deviceid;
    u16_t src_deviceid;
    u8_t boardIndex;
    u8_t interfaceIndex;
    u8_t server_seq;
    u8_t cmdExt;/*cmd id*/
	u8_t cmdId;
    u8_t cmdStatus;
    u16_t cmdLen;/*data len*/
    //u8_t buf[0];
}__attribute__((packed));



struct hj_dyb_dmp_frame_head
{
    //u8_t pre;/*0x7e*/
    u16_t dst_deviceid;
    u16_t src_deviceid;
    u8_t protocolVer;
	//u32_t dst_netid;
    u32_t thread_netid;     /*线程网元ID        */  	
    u8_t boardIndex;
    u8_t interfaceIndex;
    u8_t server_seq;
    u8_t cmdExt;
    u8_t cmdId;	/*cmd id*/
    u8_t cmdStatus;
    u16_t cmdLen;/*data len*/
	//u8_t buf;
    // unsigned char data;
}__attribute__((packed));


struct hj_dyb_dmp_frame_tail
{
    u8_t crcl;/*crc low*/
    u8_t crch;/*crc high*/
    u8_t suf;/*0x7e*/
};

struct hj_dyb_dmp_frame
{
    struct hj_dyb_dmp_frame_head * phead;
    u8_t * pdata;
    struct hj_dyb_dmp_frame_tail * ptail;
};


struct hj_dyb_dump_frame_head
{
    u8_t pre;/*0x7e*/
	u16_t dst_deviceid;
    u16_t src_deviceid;
    u8_t protocolVer;
	//u32_t dst_netid;
    u32_t thread_netid;     /*线程网元ID        */ 
    u8_t boardIndex;
    u8_t interfaceIndex;
    u8_t server_seq;
    u8_t cmdExt;/*cmd id*/
	u8_t cmdId;
    u8_t cmdStatus;
    u16_t cmdLen;/*data len*/
//    u8_t buf[0];
}__attribute__((packed));


struct hj_dump_frame_tail
{
    u8_t crcl;/*crc low*/
    u8_t crch;/*crc high*/
    u8_t suf;/*0x7e*/
};

struct hj_dump_frame
{
    struct hj_dump_frame_head * phead;
    u8_t * pdata;
    struct hj_dump_frame_tail * ptail;
};




struct hj_txback_hdr{

    u8_t pre;/*0x7e*/
    u8_t protocolVer;
	u32_t dst_netid;
    u32_t thread_netid;     /*   */  
	u16_t dst_deviceid;
    u16_t src_deviceid;
    u8_t boardIndex;
    u8_t interfaceIndex;
    u8_t server_seq;
    u8_t cmdExt;/*cmd id*/
	u8_t cmdId;
    u8_t cmdStatus;
    u16_t cmdLen;/*data len*/
    u32_t thread_id;
    u32_t seq;
    u32_t times; //

}__attribute__((packed));
//typedef struct hj_txback_hdr hj_txback_hdr_t;





union idc{
    unsigned int i_data;
    unsigned char c_data[4];
}__attribute__((packed));



#define WG_BOARD 0

#define WG_DEVICE 0xff







#define HJ80_CMD_SYNCDEVICE                                  0x01
#define HJ80_CMD_SYNCDEVICEADD                               0x02
#define HJ80_CMD_SYNCDEVICEDEL                               0x03
#define HJ80_CMD_GETSTATUSPARAM                          0x04
#define HJ80_CMD_GETSTATUSPARAMRESPONSE          0x05
#define HJ80_CMD_GETCONFIGPARAM                          0x06
#define HJ80_CMD_GETCONFIGPARAMRESPONSE          0x07
#define HJ80_CMD_SETCONFIG                                       0x08
#define HJ80_CMD_SETCONFIGRESPONSE                        0x09
#define HJ80_CMD_GETFULLDATA						0x0a
#define HJ80_CMD_GETFULLDATARESPONSE				0x0b
#define HJ80_CMD_GETFULLDATARESPONSEEND                       0x0c
#define HJ80_CMD_SETFULLDATA                       0x0d
#define HJ80_CMD_SETFULLDATARESPONSEEND                       0x0e



#define ARMPOLLDYBCMD                   0xf2        //arm轮询单元板命令



#define RESPBEAT_CMDEXT    0x100
#define TRAPSEND_CMDEXT    0x101
#define ALERTSYNC_CMDEXT   0x102

#define CMDEXT_BOARD_EXIST  0x00
#define CMDEXT_BULB_VER    0x01
#define CMDEXT_OPT_CONFIG   0x03
#define CMDEXT_OPT_STATE    0x10
#define CMDEXT_ETH_GE_CONFIG   0x11
#define CMDEXT_ETH_FE_CONFIG   0x04
#define CMDEXT_E1_CONFIG_STATE 0x02
#define CMDEXT_GLOBALDATA 	0x05
#define CMDEXT_TIME 	0x06
#define CMDEXT_NIC_MAC    0x80
#define CMDEXT_NIC_MAC_STEP1  (CMDEXT_NIC_MAC|0x0)
#define CMDEXT_NIC_MAC_STEP2  (CMDEXT_NIC_MAC|0x1)
#define CMDEXT_NIC_MAC_STEP3  (CMDEXT_NIC_MAC|0x2)

#define CMDEXT_GATEWAY 0x08
#define CMDEXT_DEFAULT 0x45
#define CMDEXT_SYSRESET 0x4f
#define CMDEXT_DEVICEDATA 	0x24

#define CMDEXT_DYB_DXC_EXT30 0x30


#define CMDEXT_GETFULLDATA  0x0c
#define CMDEXT_SETFULLDATA  0x0d


#define SDHPARAM_GATEWAYID_EXT08 	0x08

#define PROTOCOL_VER                                0x01
#define SERVER_ID                                       0x01


//CMD STATUS

//1 命令成功，没有错误。
#define  Error_UI_OK                                  (0x00)
//2 未定错误。
#define  Error_UI_Error                               (0x01)
//3 命令长度错。
#define  Error_UI_INVLDLEN                            (0x02)
//4 CommandID错。
#define  Error_UI_INVLDCMDID                          (0x03)
//5 超时错。
#define  Error_UI_TIMEOUT                             (0x04)
#define  Error_UI_MAC_TIMEOUT                         (0x05)

#define Error_FTP_COMM_FAILURE							(10)
#define Error_FTP_PASSWORD_FAILURE						(11)
#define Error_FTP_FLASH_ERASE_FAILURE					(12)
#define Error_FTP_FLASH_PROGRAM_FAILURE					(13)
#define Error_FTP_FLASH_CHECK_FAILURE					(14)
#define Error_FTP_UPDATE_EXIT_FAILURE					(15)
#define Error_FTP_BUSY_FAILURE							(16)
#define Error_FTP_FILE_UPLOAD_FAILURE					(19)
#define Error_FTP_FILE_DOWNLOAD_FAILURE					(20)
#define Error_FTP_FILE_CHECK_FAILURE					(21)
#define Error_FTP_BOARD_TYPE_FAILURE					(23)





struct hj_nm_version{
    u32_t arm_version;
    u32_t fpga_version;
    u8_t unused[32];
};




//#define ARM_VERSION 0x24		//001,00,000   V1.04
/* 2017.4.15 */
//#define ARM_VERSION 0x25		//001,00,101   V1.05
/* 2017.5.6 */
//#define ARM_VERSION 0x26		//001,00,110   V1.06
/* 2017.5.9 */
//#define ARM_VERSION 0x27		//001,00,111   V1.07
/* 2017.5.25 */
//#define ARM_VERSION 0x28		//001,10,000   V1.10
/* 20171025 */
//#define ARM_VERSION 0x29		//001,01,001   V1.11
#define ARM_VERSION 0x2B		//001,01,011   V1.13


#define  A_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) ((x)>>5)
#define  B_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) (0x3&((x)>>3))
#define  C_OF_FORMAT_A_DOT_B_C_OF_U8_VERSION(x) (0x7&(x))


struct hj80_time {
  int year;
  int mon;
  int mday;
  int hour;
  int min;
  int sec;
  //int usec;
} __attribute__((packed)) ;

#define SERVER_IP "192.168.253.253"
#define PORT 3758


#define PPPINITFCS16 ((u16_t) 0xFFFF)     //Initial FCS value
#define PPPGOODFCS16 ((u16_t) 0xF0B8)     // Good final FCS value



struct tagdybdevice_ext07S
{
	u16_t deviceid;
	u32_t devicetype; 
	u8_t unuse[10];
}__attribute__((packed));
typedef struct tagdybdevice_ext07S DYBDEVICE_EXT07_S;

struct taggateway_ext08S
{
	u8_t type;
	u16_t gateway_id; 
	u8_t unuse[5];
}__attribute__((packed));
typedef struct taggateway_ext08S GATEWAY_EXT08_S;

struct tagparam_up_ext40S {
    u16_t id;
	u32_t type;
	unsigned char unuse[2];

}__attribute__((packed));
typedef struct tagparam_up_ext40S PARAM_UP_EXT40_S;

#define PARAM_ALTERID_EXT40 0x40
#define PARAM_REMOTE_ALTERID_EXT4A 0x4a



#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))


#endif /*HJ_NM_H*/
