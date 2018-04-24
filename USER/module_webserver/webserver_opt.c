#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "webserver_opt.h"
#include "httpd_cgi.h"
#include "mainconfig.h"
#include "lwip_comm.h"
#include "lwip/inet.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"


static int opt_config_state;


// 根据行号，判断是否需要显示，哪个板卡，
// 板卡中的第几个
int opt_get_msg_according_to_lineno(int lineno , int * board,int * portno){
	int i , sum=0;
	const struct web_opt * p_opt = web_control->opt;
	for(i=0 ; i<MAX_BOARD_NO ;++i){
		int j;
		for(j=0 ; j<(p_opt[i].amount) ;++j){
			// 第n个端口
			if( (sum+j+1)==lineno ){
				// found
				*board = i;
				*portno = j;
				return 1;
			}
		}
		sum += p_opt[i].amount;
	}
	// not found
	return 0;
}


// 端口名称
int opt_column1_portname(char * buf , int lineno,int board,int portno){
	return 1+sprintf(buf, "OPT_%d",lineno);
}

int opt_column2_port_pos(char * buf , int lineno,int board,int portno){
	return 1+sprintf(buf,"%d_OPT_%d" , board+1,portno+1);
}


#if  0
int (* const opt_column_table[])(char*,int,int,int)={
	opt_column1_portname,
	opt_column2_port_pos,
	opt_column3_port_comment,
	opt_column4_port_enable,
	opt_column5_loop,
	opt_column6_trap_mask,
	opt_column7_LOF,
	opt_column8_NOP,
	opt_column9_LOOP,
	opt_column10_belonging_grp,
	opt_column11_1p1_protect,
	opt_column12_cfg_btn,
};

int opt_column3_port_comment(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int opt_column4_port_enable(char * buf , int lineno,int board,int portno){
	union web_opt_config dat = 
		web_control->opt[board].config[portno];
	if(dat.bit.enable){
		return form_cp_ret( buf,FORM_ENABLE);
	}
	return form_cp_ret(buf,FORM_DISABLE);
}

int opt_column5_loop(char * buf , int lineno,int board,int portno){
	union web_opt_config dat = 
		web_control->opt[board].config[portno];
	if( dat.bit.circ_loop && dat.bit.dev_loop ){
		return form_cp_ret(buf,FORM_LOOP_DOUBLE);
	}
	else if( dat.bit.circ_loop ){
		return form_cp_ret(buf,FORM_LOOP_CIRCUIT);
	}
	else if( dat.bit.dev_loop ){
		return form_cp_ret(buf,FORM_LOOP_DEVICE);
	}
	return form_cp_ret(buf,FORM_LOOP_NORMAL);
}

int opt_column6_trap_mask(char * buf , int lineno,int board,int portno){
	union web_opt_config dat = 
		web_control->opt[board].config[portno];
	if( dat.bit.trap_mask ){
		return form_cp_ret(buf , FORM_TRAP_MASK);
	}
	return form_cp_ret(buf , FORM_TRAP_NOMASK);
}

int opt_column7_LOF(char * buf , int lineno,int board,int portno){
	union web_opt_state dat = 
		web_control->opt[board].state[portno];
	if( dat.bit.lof ){
		return form_cp_ret(buf , FORM_STATE_WARN);
	}
	return form_cp_ret( buf,FORM_STATE_NORMAL);
}

int opt_column8_NOP(char * buf , int lineno,int board,int portno){
	union web_opt_state dat = 
		web_control->opt[board].state[portno];
	if( dat.bit.nop ){
		return form_cp_ret(buf , FORM_STATE_WARN);
	}
	return form_cp_ret(buf , FORM_STATE_NORMAL);
}

int opt_column9_LOOP(char * buf , int lineno,int board,int portno){
	union web_opt_state dat = 
		web_control->opt[board].state[portno];
	if(dat.bit.loop){
		return form_cp_ret(buf ,FORM_STATE_WARN);
	}
	return form_cp_ret(buf , FORM_STATE_NORMAL);
}

int opt_column10_belonging_grp(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int opt_column11_1p1_protect(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}
int opt_column12_cfg_btn(char * buf , int lineno,int board,int portno){
	char tmp[8]="B";
	sprintf(tmp+1 , "%d" , lineno);
	return form_insert_cfg_btn(buf, tmp);
}


void opt_common_line_ssi_handler(int lineno,char * pcInsert , int len){
	const char * text_arr[OPT_COLUMN_NO+1]={0};
	char buf[512];
	int board,portno ;
	int display_needed; 

	display_needed = opt_get_msg_according_to_lineno(lineno,&board,&portno);
	if(display_needed){
		int i , os=0;
		for( i=0 ; i< OPT_COLUMN_NO ;++i){
			int len = opt_column_table[i](buf+os , lineno,board,portno);
			text_arr[i] = buf+os;
			os += len;
		}
		fill_row(pcInsert,text_arr);
	}
	else{
		strcpy(pcInsert," ");
	}
}

void opt_line1_ssi_handler(char * pcInsert , int len){
	/* obtain datas */
	web_read_opt();
	opt_common_line_ssi_handler(1,pcInsert,len);
}
void opt_line2_ssi_handler(char * pcInsert , int len){
	opt_common_line_ssi_handler(2,pcInsert,len);
}
void opt_line3_ssi_handler(char * pcInsert , int len){
	opt_common_line_ssi_handler(3,pcInsert,len);
}
void opt_line4_ssi_handler(char * pcInsert , int len){
	opt_common_line_ssi_handler(4,pcInsert,len);
}

void opt_config_name_ssi_handler(char * pcInsert , int len){
	sprintf(pcInsert , "OPT_%d" , opt_config_lineno);
}
void opt_switch_enable_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_switch_disable_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void opt_trap_nomask_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void opt_trap_mask_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_loop_normal_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if( (!dat.bit.circ_loop)&&(!dat.bit.dev_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_loop_circuit_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if( (!dat.bit.dev_loop)&&(dat.bit.circ_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_loop_device_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if( (!dat.bit.circ_loop)&&(dat.bit.dev_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_loop_double_ssi_handler(char * pcInsert , int len){
	union web_opt_config dat;
	dat = web_control->opt[board].config[portno];
	if( (dat.bit.circ_loop)&&(dat.bit.dev_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void opt_config_state_ssi_handler(char * pcInsert , int len){
	if(SSI_STATUS_WRITE_FAILURE == opt_config_state){
		strcpy(pcInsert, "\nalert(\"ERROR: Configuration Failed!\");\n");
	}
	else if(SSI_STATUS_WRITE_SUCCESS == opt_config_state){
		strcpy(pcInsert, "\nalert(\"Configuration Succeed!\");\n");
	}
	else{
		*pcInsert = 0;
	}
	opt_config_state = SSI_STATUS_EMPTY;
}
#endif

int 
opt_write_frame(const char * cginame,
                     int iNumParams,
                     char *pcParam[],
                     char *pcValue[],
                     union web_opt_config* buf,
                     int board,
                     int portno,
                     int (*pfunc)(union web_opt_config *,int,int,const char *)){
	int iIndex;
	if((iIndex = FindCGIParameter(cginame,pcParam , iNumParams))<0){
		printf("ERROR: no parameter called \"%s\"!\r\n",cginame);
		return -1;
	}
	else{
		if(pfunc(buf,board,portno,pcValue[iIndex])<0){
			printf("pcValue of \"%s\" does not match!\r\n",cginame);
			return -1;
		}
		else
			return 0;
	}
}

int opt_write_frame_switch(union web_opt_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_ENABLE)){
		buf[portno].bit.enable = 1;
		debug_printf(DBG_DEBUG, "<enable>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_DISABLE)){
		buf[portno].bit.enable = 0;
		debug_printf(DBG_DEBUG, "<disable>\r\n");
		return 0;
	}
	return -1;
}

int opt_write_frame_trap_mask(union web_opt_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_TRAP_MASK)){
		buf[portno].bit.trap_mask = 1;
		debug_printf(DBG_DEBUG, "<trap_mask>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_TRAP_NOMASK)){
		buf[portno].bit.trap_mask = 0;
		debug_printf(DBG_DEBUG, "<trap_nomask>\r\n");
		return 0;
	}
	return -1;
}

int opt_write_frame_loop(union web_opt_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_LOOP_NORMAL)){
		buf[portno].bit.circ_loop = 0;
		buf[portno].bit.dev_loop = 0;
		debug_printf(DBG_DEBUG, "<loop_normal>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_LOOP_CIRCUIT)){
		buf[portno].bit.circ_loop = 1;
		buf[portno].bit.dev_loop = 0;
		debug_printf(DBG_DEBUG, "<loop_circuit>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_LOOP_DEVICE)){
		buf[portno].bit.circ_loop = 0;
		buf[portno].bit.dev_loop = 1;
		debug_printf(DBG_DEBUG, "<loop_device>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_LOOP_DOUBLE)){
		buf[portno].bit.circ_loop = 1;
		buf[portno].bit.dev_loop = 1;
		debug_printf(DBG_DEBUG, "<loop_double>\r\n");
		return 0;
	}
	return -1;
}



const char* opt_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	union web_opt_config buf[16]={0};
	int board,portno;
	//cgi_print_submission( iNumParams, pcParam,pcValue);
	if(iNumParams<2){
		return "/assets/ajax/opt_read.ajax";
	}
	board = atoi(pcValue[FindCGIParameter("board",pcParam,iNumParams)]);
	portno= atoi(pcValue[FindCGIParameter("portno",pcParam,iNumParams)]);
	memcpy(buf , web_control->opt[board].config,sizeof(web_control->opt[0].config));

	// 开关
	opt_write_frame("opt_switch",iNumParams,pcParam,pcValue,buf,board,portno,opt_write_frame_switch);
	// 告警抑制
	opt_write_frame("opt_trap_mask",iNumParams,pcParam,pcValue,buf,board,portno,opt_write_frame_trap_mask);
	// 环回状态
	opt_write_frame("opt_loop",iNumParams,pcParam,pcValue,buf,board,portno,opt_write_frame_loop);

	// configure it
	if(web_write_opt(board, buf)<0)
		opt_config_state = -1;
	else
		opt_config_state = 0;
	return "/assets/ajax/opt_write.ajax";
}

int opt_make_data(char * p , int board,int portno,u8 cfg,u8 state){
	p[0] = ntoalpha(board);
	p[1] = ntoalpha(portno);
	sprintf(p+2 , "%02x" , cfg);
	sprintf(p+4 , "%02x" , state);
	return 6;
}

void opt_read_data_ssi_handler(char * p , int len){
	int board,portno,os = 0;
	if(web_read_opt()<0){
		strcpy(p , FAIL_STR);
		return;
	}
	for(board = 0 ; board<MAX_BOARD_NO;++board){
		for(portno = 0;portno<web_control->opt[board].amount;++portno){
			os += opt_make_data(p+os,
			                    board,
			                    portno,
			                    web_control->opt[board].config[portno].byte,
			                    web_control->opt[board].state[portno].byte);
		}
	}
}

void opt_config_status_ssi_handler(char * p , int len){
	if(opt_config_state<0)
		strcpy(p , FAIL_STR);
	else
		strcpy(p,SUCCESS_STR);
}

