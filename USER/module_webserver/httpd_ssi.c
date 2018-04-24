#include <string.h>
#include <stdlib.h>
#include "malloc.h"
#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "httpd_ssi.h"
#include "webserver_login.h"
#include "webserver_net_config.h"
#include "webserver_reboot.h"
#include "webserver_update.h"
#include "webserver_opt.h"
#include "webserver_eth.h"
#include "webserver_e1.h"
#include "webserver_account.h"
#include "webserver_panel.h"
#include "webserver_cent_rmt.h"
#include "webserver_boards_mng.h"

/*
 * initializations of ssi objects in webpage...
 * use SSI_OBJ(tag,hdl) to initialize a ssi object
 * tag    : corresponding ssi tag (<!--#tag--> in the html file)
 * handler: method of inserting the context
 */
static const struct httpd_ssi_class httpd_ssi_objs[]={
	/* login/username+passwd */
	SSI_OBJ("login_usrpswd"   ,login_usrpasswd_ssi_handler),
	/* panel+cent/rmt(main page) */
	SSI_OBJ("panel_bulb"   ,  panel_ssi_handler),
	SSI_OBJ("cr_read"   ,cent_rmt_read_ssi_handler),
	/* account */
	SSI_OBJ("account_status"     ,account_status_ssi_handler),
	/* net/device info */
	SSI_OBJ("net_read_data"      , net_read_data_ssi_handler),
	SSI_OBJ("net_config_status"  , net_config_status_ssi_handler),
	/* update */
	SSI_OBJ("update_status"    ,update_status_ssi_handler),
	/* 板卡管理 */
	SSI_OBJ("board_line1",  board_line1_ssi_handler),
	SSI_OBJ("board_line2",  board_line2_ssi_handler),
	SSI_OBJ("board_line3",  board_line3_ssi_handler),
	SSI_OBJ("board_line4",  board_line4_ssi_handler),
	SSI_OBJ("board_line5",  board_line5_ssi_handler),
	SSI_OBJ("board_line6",  board_line6_ssi_handler),
	SSI_OBJ("board_line7",  board_line7_ssi_handler),
	SSI_OBJ("board_line8",  board_line8_ssi_handler),
	SSI_OBJ("board_line9",  board_line8_ssi_handler),
	SSI_OBJ("board_line10", board_line10_ssi_handler),
	/* port_manager--opt */
	SSI_OBJ("opt_read_data"    ,opt_read_data_ssi_handler),
	SSI_OBJ("opt_config_status",opt_config_status_ssi_handler),
	/* port_manager--eth */
	SSI_OBJ("eth_read_data"    ,eth_read_data_ssi_handler),
	SSI_OBJ("eth_config_status",eth_config_status_ssi_handler),
	/* port_manager--e1 */
	SSI_OBJ("e1_read_data"    ,e1_read_data_ssi_handler),
	SSI_OBJ("e1_config_status",e1_config_status_ssi_handler),
};


static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
	if( (httpd_ssi_objs + iIndex)->handler ){
		(httpd_ssi_objs + iIndex)->handler(pcInsert , iInsertLen);
	}
	else{
		printf("WARNING: null pointer in httpd_ssi_objs!\r\n");
	}
	return strlen(pcInsert);
}

/* 初始化ssi句柄 */
void httpd_ssi_init(void){  
	int i ;
	int ssi_num = 
		sizeof(httpd_ssi_objs)/sizeof(struct httpd_ssi_class);
#if  0
	static const char * ppcTAGs[HTTPD_SSI_MAX];
	/* 判断ssi对象容器是否太小 */
	if( ssi_num > HTTPD_SSI_MAX){
		printf("ERROR: \"HTTPD_SSI_MAX\" is too small!!!\r\n");
		real_ssi_num = HTTPD_SSI_MAX;
	}
	else{
		real_ssi_num = ssi_num;
		//printf("sizeof(struct http_state)=%d\r\n" , sizeof(struct http_state));
	}
#endif
	const char ** ppcTAGs = (const char**)mymalloc(SRAMIN, ssi_num*sizeof(void*));
	if(!ppcTAGs){
		printf("ERROR: failed to allocate ppcTAGs!\r\n");
		return;
	}
	// initialization
	for(i = 0 ; i < ssi_num ; ++i){
		ppcTAGs[i] = (httpd_ssi_objs+i)->tag;
	}
	http_set_ssi_handler(SSIHandler,ppcTAGs,ssi_num);
}


int ajax_get_fill(char * p  , const char * name,void(*call)(char*),const char * val){
	if(call){
		char tmp[64];
		call(tmp);
		return sprintf(p , "%s=%s;\n" , name,tmp);
	}
	return sprintf(p , "%s=%s;\n" , name,val);
}

char ntoalpha(int n){
	if(n<10){
		return '0'+n;
	}
	if(n<36){
		return 'a'+n-10;
	}
	if(n<62){
		return 'A'+n-36;
	}
	return '0';
}

