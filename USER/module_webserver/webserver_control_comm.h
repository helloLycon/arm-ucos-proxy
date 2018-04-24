#ifndef __WEBSERVER_CONTROL_COMM_H
#define __WEBSERVER_CONTROL_COMM_H

#include "hj_nm.h"
#include "webserver_bulb_ver_frame.h"
#include "webserver_opt_frame.h"
#include "webserver_eth_frame.h"
#include "webserver_e1_frame.h"


#define  MAX_BOARD_NO              9u

#define  WEB_SEQ_FIELD        0x33
#define  WEB_THREADID_FIELD   0x333


// 下发给单元板的参数
struct web_dyb_arg {
	u16_t dev_id;
	u8_t board;
	u8_t intf;
	u8_t cmdId;
	u16_t cmdExt;
	const void * buf;
	u16_t len;
};



// 数据帧保存
struct web_dyb_frame{
	struct hj_dyb_dump_frame_head hdr;
	u8_t body[256];
} __attribute__((packed));



// 板存在信息
struct web_boards_exist {
	u16 exist;
	u32 type[MAX_BOARD_NO];
	u8  __rsv[2];
} __attribute__((packed));


struct web_ctrl_comm {
	OS_EVENT * wait_sem;
	OS_EVENT * ll_access_mtx;
	struct web_dyb_frame frame;
	u16_t dev_id;
	struct web_boards_exist boards_exist;      //板存在
	struct web_bulb_ver bulb_ver[MAX_BOARD_NO];//指示灯参数
	struct web_opt opt[MAX_BOARD_NO];          //光路参数
	struct web_eth eth[MAX_BOARD_NO];          //以太网参数
	struct web_e1  e1[MAX_BOARD_NO];           //e1参数
};


extern struct web_ctrl_comm * web_control;
extern int cr_remote_chosen;
int debug_printf(int dbg_level , const char * fmt,...);
int ctrl_web_control_comm_init(void);
int ctrl_dyb_frame_capture(const struct hj_dyb_dump_frame_head * p_head);
int web_read_device_id(void);
int web_read_panel(void);
int web_read_boards_version(void);
//int web_read_bulb_ver_main(void);
int web_read_opt(void);
int web_write_opt(int board , const union web_opt_config * buf);
int web_read_eth(void);
int web_write_eth(int ge,int board , const union web_eth_config * buf);
int web_read_e1(void);
int web_write_e1(int board , const struct web_e1 * buf);



#endif

