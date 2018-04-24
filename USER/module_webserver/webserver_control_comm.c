#include <stdio.h>
#include <string.h>
#include "hj_nm.h"
#include "malloc.h"
#include "webserver_control_comm.h"
#include "gan111_type.h"


struct web_ctrl_comm * web_control;

//need boards management板卡管理
const u32 boards_contain_boards_mng[]={
	GAN111_MAIN_CTRL,
	GAN111_8E1,
	NULL,
};

// 包含光口的板卡类型
const u32 boards_contain_opt[]={
	GAN111_MAIN_CTRL,
	NULL,
};

// 包含GigaEth的板卡
const u32 boards_contain_eth_ge[]={
	GAN111_MAIN_CTRL,
	NULL,
};
// 包含FastEth的板卡
const u32 boards_contain_eth_fe[]={
	GAN111_MAIN_CTRL,
	NULL,
};
// 包含e1的板卡
const u32 boards_contain_e1[]={
	GAN111_8E1,
	NULL,
};

int ctrl_web_control_comm_init(void){
	web_control = (struct web_ctrl_comm *)mymalloc( SRAMCCM , sizeof(struct web_ctrl_comm));
	if( !web_control ){
		printf("ERROR: failed to alloc \"web_control\" in \"ctrl_web_control_comm_init\"\r\n");
		return -1;
	}
	// all clear
	memset(web_control , 0 , sizeof(struct web_ctrl_comm));
	web_control->wait_sem = OSSemCreate(0);
	web_control->ll_access_mtx = OSSemCreate(1);
	return 0;
}

static int ctrl_send_dyb_frame(const struct web_dyb_arg * arg){
	void hj_nm_build_dump_frame(void *, unsigned char *, int);
	unsigned int hj15_crc_7e(unsigned char *, int,unsigned char *);
	void Board_UARTX_Inset_Data_To_Link(uint8_t *, uint32_t);
	int framelen;
    unsigned char pwdata[256],pcrc[256];
    struct hj_dyb_dmp_frame dmp_frame;
	hj_nm_build_dump_frame(&dmp_frame, pwdata,0);

	//printf("dmp_frame.phead:::seq=%x\n", seq);
	dmp_frame.phead->dst_deviceid = arg->dev_id;
	dmp_frame.phead->src_deviceid = 0;
	dmp_frame.phead->protocolVer = 1;
	dmp_frame.phead->thread_netid = WEB_THREADID_FIELD;
	//printf("cmdLen=%x\n",p_head->cmdLen);
	//memcpy(&dmp_frame.phead->boardIndex,&p_head->boardIndex,p_head->cmdLen+8);
	dmp_frame.phead->boardIndex = arg->board;
	dmp_frame.phead->interfaceIndex = arg->intf;
	dmp_frame.phead->server_seq = WEB_SEQ_FIELD;
	//dmp_frame.phead->server_seq = p_head->server_seq;
	dmp_frame.phead->cmdExt = (u8_t)(arg->cmdExt);
	dmp_frame.phead->cmdId = (arg->cmdId<<4)|((arg->cmdExt&0xfff)>>8);
	dmp_frame.phead->cmdStatus = 0;
	dmp_frame.phead->cmdLen = arg->len;
/*
	// wtf?
	p_dst_char_buff = (u8_t *)&(dmp_frame.phead->cmdLen);
	p_dst_char_buff++;
	p_dst_char_buff++;
	p_src_char_buff = (u8_t *)&(p_head->cmdLen);
	p_src_char_buff++;
	p_src_char_buff++;
	memcpy(p_dst_char_buff,p_src_char_buff,p_head->cmdLen);
*/	           
	if(arg->len && arg->buf){
		memcpy(dmp_frame.phead+1 ,arg->buf,arg->len );
	}

	framelen = arg->len + sizeof(struct hj_dyb_dmp_frame_head);
	framelen = hj15_crc_7e(pwdata,framelen,pcrc);
	//debug_dump(pcrc, sizeof(struct hj_txback_hdr), "pcrc:");
	Board_UARTX_Inset_Data_To_Link(pcrc, framelen);
	return framelen;
}

static int ctrl_wait_dyb_reply(const char * prompt){
	INT8U err;
	OSSemSet(web_control->wait_sem,0, &err);
	OSSemPend(web_control->wait_sem,OS_TICKS_PER_SEC,&err);
	if( OS_ERR_TIMEOUT==err ){
		printf("ERR: timeout(%s)\r\n",prompt);
		return -1;
	}
	return 0;
}

static int ctrl_require(
	const struct web_dyb_arg * arg ,
	void * cp_dst,
	int cp_size,
	const char * prompt)
{
	INT8U err;
	int ret;
	OSSemPend(web_control->ll_access_mtx,0,&err);
	ctrl_send_dyb_frame(arg);
	ret = ctrl_wait_dyb_reply(prompt);
	if( (!(ret<0)) && cp_dst ){
		memcpy(cp_dst , web_control->frame.body , cp_size);
	}
	OSSemPost(web_control->ll_access_mtx);
	return ret;
}

int ctrl_dyb_frame_capture(const struct hj_dyb_dump_frame_head * p_head){
	if( WEB_SEQ_FIELD == p_head->server_seq && WEB_THREADID_FIELD==p_head->thread_netid){
		//static int cnter = 0;
		//printf("cnter=%d\r\n" , ++cnter);
		memcpy(&web_control->frame , p_head, sizeof(*p_head)+p_head->cmdLen);
		OSSemPost(web_control->wait_sem);
		return 1;
	}
	return 0;
}

static int ctrl_check_contain_iftype(u32 type,const u32 * tab){
	int i;
	for(i=0 ; tab[i];++i){
		if(type==tab[i])
			return 1;
	}
	return 0;
}


int web_read_device_id(void){
	u8_t buf[8] = {0,1,2,3};
	struct web_dyb_arg arg;
	
	arg.dev_id = 0xffff;
	arg.board = 0;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_SETCONFIG;
	arg.cmdExt = cr_remote_chosen?PARAM_REMOTE_ALTERID_EXT4A:PARAM_ALTERID_EXT40;
	arg.buf = buf;
	arg.len = sizeof(buf);
/*
	// send...
	if(ctrl_require(&arg,&web_control->dev_id,sizeof(web_control->dev_id),"read device ID")<0)
		return -1;

	web_control->dev_id = *((u16_t*)(web_control->frame.body));
	//printf("devid=%d\r\n" , *p_devid);
	return 0;*/
	return ctrl_require(&arg,
	                    &web_control->dev_id,
	                    sizeof(web_control->dev_id),
	                    "read device ID");
}

int web_read_board_exist(void){
	void_t debug_dump(void *, s32_t, u8_t *);
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = 0xff;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_BOARD_EXIST;
	arg.buf = NULL;
	arg.len = 0;
/*	if(ctrl_require(&arg,"read boards")<0)
		return -1;
	
	// succeed
	memcpy(&web_control->boards_exist ,
	       web_control->frame.body ,
	       sizeof(struct web_boards_exist));
	//debug_dump(&web_control->boards_exist,sizeof(struct web_boards_exist),"boards:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    &web_control->boards_exist,
	                    sizeof(struct web_boards_exist),
	                    "read boards");
}


int web_read_bulb_ver_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_BULB_VER;
	arg.buf = NULL;
	arg.len = 0;
/*	if(ctrl_require(&arg,"read bulb_ver")<0)
		return -1;
	// succeed
	memcpy(&(web_control->bulb_ver[board].frame),
	       web_control->frame.body ,
	       sizeof(struct web_bulb_ver_frm));
	return 0;*/
	return ctrl_require(&arg,
	                    &(web_control->bulb_ver[board].frame),
	                    sizeof(struct web_bulb_ver_frm),
	                    "read bulb_ver");
}

int web_read_panel(void){
	if(web_read_device_id()<0){
		memset(web_control->bulb_ver,0,sizeof(web_control->bulb_ver));
		return -1;
	}
	if( web_read_bulb_ver_one_board(0)<0 ){
		memset(web_control->bulb_ver,0,sizeof(web_control->bulb_ver));
		return -1;
	}
	return 0;
}

int web_read_boards_version(void){
	int i;
	if(web_read_device_id()<0){
		memset(web_control->bulb_ver,0,sizeof(web_control->bulb_ver));
		return -1;
	}
	if(web_read_board_exist()<0){
		memset(web_control->bulb_ver,0,sizeof(web_control->bulb_ver));
		return -1;
	}
	for(i=0;i<MAX_BOARD_NO;++i){
		if( ( web_control->boards_exist.exist&(1<<i))&&
			ctrl_check_contain_iftype(web_control->boards_exist.type[i],boards_contain_boards_mng)){
			if(web_read_bulb_ver_one_board(i)<0){
				web_control->bulb_ver[i].set = 0;
				continue;
			}
			else
				web_control->bulb_ver[i].set = 1;
		}
		else
			web_control->bulb_ver[i].set = 0;
	}
	return 0;
}

int web_read_opt_config_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_OPT_CONFIG;
	arg.buf = NULL;
	arg.len = 0;

/*	if(ctrl_require(&arg,"read opt_config")<0)
		return -1;
	// succeed
	memcpy(web_control->opt[board].config , web_control->frame.body,sizeof(web_control->opt[0].config));
	//debug_dump(web_control->opt[board].config,sizeof(web_control->opt[0].config),"opt_config:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    web_control->opt[board].config,
	                    sizeof(web_control->opt[0].config),
	                    "read opt_config");
}



int web_read_opt_state_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_OPT_STATE;
	arg.buf = NULL;
	arg.len = 0;

/*	if(ctrl_require(&arg,"read opt_state")<0)
		return -1;
	// succeed
	memcpy(web_control->opt[board].state, web_control->frame.body,sizeof(web_control->opt[0].state));
	//debug_dump(web_control->opt[board].state,sizeof(web_control->opt[0].state),"opt_state:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    web_control->opt[board].state,
	                    sizeof(web_control->opt[0].state),
	                    "read opt_state");
}

// 读光口参数
int web_read_opt(void){
#if   1
	int i;
	if(web_read_device_id()<0){
		memset(web_control->opt , 0 , sizeof(web_control->opt));
		return -1;
	}
	if(web_read_board_exist()<0){
		memset(web_control->opt , 0 , sizeof(web_control->opt));
		return -1;
	}
	for(i=0; i<MAX_BOARD_NO;++i){
		if(( web_control->boards_exist.exist & (1<<i))&&
		( ctrl_check_contain_iftype(web_control->boards_exist.type[i],boards_contain_opt))){
			// the board has opt interface
			if(web_read_opt_config_one_board(i)<0){
				web_control->opt[i].amount = 0;
				continue;
			}
			if(web_read_opt_state_one_board(i)<0){
				web_control->opt[i].amount = 0;
				continue;
			}
			web_control->opt[i].amount = MAX_OPT_PORT_AMOUNT;
		}
		else
			web_control->opt[i].amount = 0;
	}
	return 0;
#else
	web_control->opt[0].amount = 2;
	web_control->opt[4].amount = 2;
	return 0;
#endif
}

int web_write_opt(int board , const union web_opt_config * buf){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_SETCONFIG;
	arg.cmdExt = CMDEXT_OPT_CONFIG;
	arg.buf = (const void*)buf;
	arg.len = sizeof(web_control->opt[0].config);
	
	if(ctrl_require(&arg,NULL,0,"write opt_config")<0)
		return -1;
	// succeed
	if( web_control->frame.hdr.cmdStatus){
		printf("ERR: cmdStatus = %d\r\n",web_control->frame.hdr.cmdStatus);
		return -1;
	}
	return 0;
}

int web_read_eth_ge_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_ETH_GE_CONFIG;
	arg.buf = NULL;
	arg.len = 0;

/*	if(ctrl_require(&arg,"read GE")<0)
		return -1;
	// succeed
	memcpy(web_control->eth[board].ge, web_control->frame.body,sizeof(web_control->eth[0].ge));
	//debug_dump(web_control->opt[board].config,sizeof(web_control->opt[0].config),"opt_config:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    web_control->eth[board].ge,
	                    sizeof(web_control->eth[0].ge),
	                    "read GE");
}


int web_read_eth_fe_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_ETH_FE_CONFIG;
	arg.buf = NULL;
	arg.len = 0;

/*	if(ctrl_require(&arg,"read FE")<0)
		return -1;
	// succeed
	memcpy(web_control->eth[board].fe, web_control->frame.body,sizeof(web_control->eth[0].fe));
	//debug_dump(web_control->opt[board].config,sizeof(web_control->opt[0].config),"opt_config:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    web_control->eth[board].fe,
	                    sizeof(web_control->eth[0].fe),
	                    "read FE");
}

// 读eth参数
int web_read_eth(void){
#if 1
	int i;
	if(web_read_device_id()<0){
		memset(web_control->eth, 0 , sizeof(web_control->eth));
		return -1;
	}
	if(web_read_board_exist()<0){
		memset(web_control->eth, 0 , sizeof(web_control->eth));
		return -1;
	}
	// 获取GE数据
	for(i=0; i<MAX_BOARD_NO;++i){
		if(( web_control->boards_exist.exist & (1<<i))&&
		( ctrl_check_contain_iftype(web_control->boards_exist.type[i],boards_contain_eth_ge))){
			// the board has e1 interface
			if(web_read_eth_ge_one_board(i)<0){
				web_control->eth[i].ge_amount = 0;
				continue;
			}
			web_control->eth[i].ge_amount = MAX_ETH_GE_PORT_AMOUNT;
		}
		else
			web_control->eth[i].ge_amount = 0;
	}
	// 获取FE数据
	for(i=0; i<MAX_BOARD_NO;++i){
		if(( web_control->boards_exist.exist & (1<<i))&&
		( ctrl_check_contain_iftype(web_control->boards_exist.type[i],boards_contain_eth_fe))){
			// the board has e1 interface
			if(web_read_eth_fe_one_board(i)<0){
				web_control->eth[i].fe_amount = 0;
				continue;
			}
			web_control->eth[i].fe_amount = MAX_ETH_FE_PORT_AMOUNT;
		}
		else
			web_control->eth[i].fe_amount = 0;
	}
	return 0;
#else
	web_control->eth[0].ge_amount = 4;
	web_control->eth[4].fe_amount = 4;
	return 0;
#endif
}

int web_write_eth(int ge,int board , const union web_eth_config * buf){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_SETCONFIG;
	arg.cmdExt = ge?CMDEXT_ETH_GE_CONFIG:CMDEXT_ETH_FE_CONFIG;
	arg.buf = (const void*)buf;
	arg.len = ge?sizeof(web_control->eth[0].ge):sizeof(web_control->eth[0].fe);
	
	if(ctrl_require(&arg,NULL,0,"write eth_config")<0)
		return -1;
	// succeed
	if( web_control->frame.hdr.cmdStatus){
		printf("ERR: cmdStatus = %d\r\n",web_control->frame.hdr.cmdStatus);
		return -1;
	}
	return 0;
}

int web_read_e1_one_board(int board){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_GETCONFIGPARAM;
	arg.cmdExt = CMDEXT_E1_CONFIG_STATE;
	arg.buf = NULL;
	arg.len = 0;

/*	if(ctrl_require(&arg,"read e1")<0)
		return -1;
	// succeed
	memcpy(web_control->e1+board, web_control->frame.body, (int)(&((struct web_e1*)NULL)->amount));
	//debug_dump(web_control->opt[board].config,sizeof(web_control->opt[0].config),"opt_config:\r\n");
	return 0;*/
	return ctrl_require(&arg,
	                    web_control->e1+board,
	                    (int)(&((struct web_e1*)NULL)->amount),
	                    "read e1");
}

// 读e1参数
int web_read_e1(void){
#if  1
	int i;
	if(web_read_device_id()<0){
		memset(web_control->e1, 0 , sizeof(web_control->e1));
		return -1;
	}
	if(web_read_board_exist()<0){
		memset(web_control->e1 , 0 , sizeof(web_control->e1));
		return -1;
	}
	for(i=0; i<MAX_BOARD_NO;++i){
		if(( web_control->boards_exist.exist & (1<<i))&&
		( ctrl_check_contain_iftype(web_control->boards_exist.type[i],boards_contain_e1))){
			// the board has e1 interface
			if(web_read_e1_one_board(i)<0){
				web_control->e1[i].amount = 0;
				continue;
			}
			web_control->e1[i].amount = MAX_E1_PORT_AMOUNT;
		}
		else
			web_control->e1[i].amount = 0;
	}
	return 0;
#else
	web_control->e1[0].amount = 8;
	web_control->e1[8].amount = 8;
	return 0;
#endif
}

int web_write_e1(int board , const struct web_e1 * buf){
	struct web_dyb_arg arg;
	arg.dev_id = web_control->dev_id;
	arg.board = board;
	arg.intf = 0xff;
	arg.cmdId = HJ80_CMD_SETCONFIG;
	arg.cmdExt = CMDEXT_E1_CONFIG_STATE;
	arg.buf = (const void*)buf;
	arg.len = (int)&((struct web_e1*)NULL)->amount;
	
	if(ctrl_require(&arg,NULL,0,"write e1_config")<0)
		return -1;
	// succeed
	if( web_control->frame.hdr.cmdStatus){
		printf("ERR: cmdStatus = %d\r\n",web_control->frame.hdr.cmdStatus);
		return -1;
	}
	return 0;
}

