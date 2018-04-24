#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "webserver_e1.h"
#include "httpd_cgi.h"
#include "mainconfig.h"
#include "lwip_comm.h"
#include "lwip/inet.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"

//static int board,portno;
static int e1_config_state = SSI_STATUS_EMPTY;

/*
int (* const e1_column_table[])(char*,int,int,int)={
	e1_column1_portname,
	e1_column2_port_pos,
	//e1_column3_port_comment,
	e1_column4_port_enable,
	e1_column5_loop,
	e1_column6_trap_mask,
	e1_column7_LOS,
	e1_column8_AIS,
	e1_column9_LOOP,
//	e1_column10_LOF,
//	e1_column11_error,
//	e1_column12_alarm,
//	e1_column13_CAS,
//	e1_column14_MLOF,
	e1_column15_cfg_btn,
};
*/

// 根据行号，判断是否需要显示，哪个板卡，
// 板卡中的第几个
int e1_get_msg_according_to_lineno(int lineno , int * board,int * portno){
	int i , sum=0;
	const struct web_e1 * p_e1 = web_control->e1;
	for(i=0 ; i<MAX_BOARD_NO ;++i){
		int j;
		for(j=0 ; j<(p_e1[i].amount) ;++j){
			// 第n个e1
			if( (sum+j+1)==lineno ){
				// found
				*board = i;
				*portno = j;
				return 1;
			}
		}
		sum += p_e1[i].amount;
	}
	// not found
	return 0;
}


// 端口名称
int e1_column1_portname(char * buf , int lineno,int board,int portno){
	//strcpy(buf , "E1_");
	//return 4+sprintf(buf+3 , "%d" , lineno);
	return 1+sprintf(buf,"E1_%d",lineno);
}

int e1_column2_port_pos(char * buf , int lineno,int board,int portno){
/*	//strcpy(buf,"0_OPT_0");
	int os = sprintf(buf,"%d",board+1);
	os += sprintf(buf+os,"_E1_");
	os += sprintf(buf+os , "%d",portno+1);
	return os+1;*/
	return 1+sprintf(buf,"%d_E1_%d",board+1,portno+1);
}
#if 0
int e1_column3_port_comment(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int e1_column4_port_enable(char * buf , int lineno,int board,int portno){
	union web_e1_config dat = 
		web_control->e1[board].config[portno];
	if(dat.bit.enable){
		return form_cp_ret(buf , FORM_ENABLE);
	}
	return form_cp_ret(buf , FORM_DISABLE);
}

int e1_column5_loop(char * buf , int lineno,int board,int portno){
	union web_e1_config dat = 
		web_control->e1[board].config[portno];
	if( dat.bit.circ_loop && dat.bit.dev_loop ){
		return form_cp_ret(buf , FORM_LOOP_DOUBLE);
	}
	else if( dat.bit.circ_loop ){
		return form_cp_ret(buf , FORM_LOOP_CIRCUIT);
	}
	else if( dat.bit.dev_loop ){
		return form_cp_ret(buf , FORM_LOOP_DEVICE);
	}
	return form_cp_ret(buf,FORM_LOOP_NORMAL);
}

int e1_column6_trap_mask(char * buf , int lineno,int board,int portno){
	union web_e1_config dat = 
		web_control->e1[board].config[portno];
	if( dat.bit.trap_mask ){
		return form_cp_ret(buf , FORM_TRAP_MASK);
	}
	return form_cp_ret(buf , FORM_TRAP_NOMASK);
}

int e1_column7_LOS(char * buf , int lineno,int board,int portno){
	union web_e1_state dat = 
		web_control->e1[board].state[portno];
	if( dat.bit.los ){
		return form_cp_ret(buf , FORM_STATE_WARN);
	}
	return form_cp_ret(buf,  FORM_STATE_NORMAL);
}

int e1_column8_AIS(char * buf , int lineno,int board,int portno){
	union web_e1_state dat = 
		web_control->e1[board].state[portno];
	if( dat.bit.ais ){
		return form_cp_ret(buf , FORM_STATE_WARN);
	}
	return form_cp_ret(buf,  FORM_STATE_NORMAL);
}

int e1_column9_LOOP(char * buf , int lineno,int board,int portno){
	union web_e1_state dat = 
		web_control->e1[board].state[portno];
	if( dat.bit.loop ){
		return form_cp_ret(buf , FORM_STATE_WARN);
	}
	return form_cp_ret(buf,  FORM_STATE_NORMAL);
}

/*
int e1_column10_LOF(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int e1_column11_error(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

// 对告状态
int e1_column12_alarm(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int e1_column13_CAS(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int e1_column14_MLOF(char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}
*/

int e1_column15_cfg_btn(char * buf , int lineno,int board,int portno){
	char tmp[8]="B";
	sprintf(tmp+1 , "%d" , lineno);
	return form_insert_cfg_btn(buf, tmp);
}

void e1_common_line_ssi_handler(int lineno,char * pcInsert , int len){
	const char * text_arr[E1_COLUMN_NO+1]={0};
	char buf[512];
	int board,portno ;
	int display_needed; 
	
	display_needed = e1_get_msg_according_to_lineno(lineno,&board,&portno);
	if(display_needed){
		int i , os=0;
		for( i=0 ; i< E1_COLUMN_NO ;++i){
			int len = e1_column_table[i](buf+os , lineno,board,portno);
			text_arr[i] = buf+os;
			os += len;
		}
		fill_row(pcInsert,text_arr);
	}
	else{
		strcpy(pcInsert," ");
	}
}



void e1_line1_ssi_handler(char * pcInsert , int len){
	/* obtain datas */
	web_read_e1();
	e1_common_line_ssi_handler(1,pcInsert,len);
}
void e1_line2_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(2,pcInsert,len);
}
void e1_line3_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(3,pcInsert,len);
}
void e1_line4_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(4,pcInsert,len);
}
void e1_line5_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(5,pcInsert,len);
}
void e1_line6_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(6,pcInsert,len);
}
void e1_line7_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(7,pcInsert,len);
}
void e1_line8_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(8,pcInsert,len);
}
void e1_line9_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(9,pcInsert,len);
}
void e1_line10_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(10,pcInsert,len);
}
void e1_line11_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(11,pcInsert,len);
}
void e1_line12_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(12,pcInsert,len);
}
void e1_line13_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(13,pcInsert,len);
}
void e1_line14_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(14,pcInsert,len);
}
void e1_line15_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(15,pcInsert,len);
}
void e1_line16_ssi_handler(char * pcInsert , int len){
	e1_common_line_ssi_handler(16,pcInsert,len);
}


void e1_config_name_ssi_handler(char * pcInsert , int len){
	sprintf(pcInsert , "E1_%d" , e1_config_lineno);
}
void e1_switch_enable_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void e1_switch_disable_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void e1_trap_nomask_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void e1_trap_mask_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void e1_loop_normal_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if( (!dat.bit.circ_loop)&&(!dat.bit.dev_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void e1_loop_circuit_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if( dat.bit.circ_loop )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void e1_loop_device_ssi_handler(char * pcInsert , int len){
	union web_e1_config dat;
	dat = web_control->e1[board].config[portno];
	if( (!dat.bit.circ_loop)&&(dat.bit.dev_loop) )
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
#endif

void e1_config_status_ssi_handler(char * p , int len){
	if(e1_config_state<0)
		strcpy(p , FAIL_STR);
	else
		strcpy(p , SUCCESS_STR);
}


int e1_make_data(char * p , int board,int portno,u8 cfg,u8 state){
	p[0] = ntoalpha(board);
	p[1] = ntoalpha(portno);
	sprintf(p+2 , "%02x" , cfg);
	sprintf(p+4 , "%02x" , state);
	return 6;
}

void e1_read_data_ssi_handler(char * p , int len){
	int board,portno,os = 0;
	if(web_read_e1()<0){
		strcpy(p , FAIL_STR);
		return;
	}
	for(board = 0 ; board<MAX_BOARD_NO;++board){
		for(portno = 0;portno<web_control->e1[board].amount;++portno){
			os += e1_make_data(p+os,
			                    board,
			                    portno,
			                    web_control->e1[board].config[portno].byte,
			                    web_control->e1[board].state[portno].byte);
		}
	}
}


int 
e1_write_frame(const char * cginame,
                     int iNumParams,
                     char *pcParam[],
                     char *pcValue[],
                     struct web_e1 * buf,
                     int board,
                     int portno,
                     int (*pfunc)(struct web_e1 *,int,int,const char *)){
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

int e1_write_frame_switch(struct web_e1 * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_ENABLE)){
		buf->config[portno].bit.enable = 1;
		debug_printf(DBG_DEBUG, "<enable>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_DISABLE)){
		buf->config[portno].bit.enable = 0;
		debug_printf(DBG_DEBUG, "<disable>\r\n");
		return 0;
	}
	return -1;
}

int e1_write_frame_trap_mask(struct web_e1 * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_TRAP_MASK)){
		buf->config[portno].bit.trap_mask = 1;
		debug_printf(DBG_DEBUG, "<trap_mask>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_TRAP_NOMASK)){
		buf->config[portno].bit.trap_mask = 0;
		debug_printf(DBG_DEBUG, "<trap_nomask>\r\n");
		return 0;
	}
	return -1;
}

int e1_write_frame_loop(struct web_e1 * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_LOOP_NORMAL)){
		buf->config[portno].bit.circ_loop = 0;
		buf->config[portno].bit.dev_loop = 0;
		debug_printf(DBG_DEBUG, "<loop_normal>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_LOOP_CIRCUIT)){
		buf->config[portno].bit.circ_loop = 1;
		buf->config[portno].bit.dev_loop = 0;
		debug_printf(DBG_DEBUG, "<loop_circuit>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_LOOP_DEVICE)){
		buf->config[portno].bit.circ_loop = 0;
		buf->config[portno].bit.dev_loop = 1;
		debug_printf(DBG_DEBUG, "<loop_device>\r\n");
		return 0;
	}
	return -1;
}

const char* e1_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	int board,portno;
	struct web_e1 buf={0};
	if(iNumParams<2){
		return "/assets/ajax/e1_read.ajax";
	}
	board = atoi(pcValue[FindCGIParameter("board",pcParam,iNumParams)]);
	portno= atoi(pcValue[FindCGIParameter("portno",pcParam,iNumParams)]);
	memcpy(&buf , web_control->e1[board].config,sizeof(web_control->e1[0].config));
	// 开关
	e1_write_frame("e1_switch",iNumParams,pcParam,pcValue,&buf,board,portno,e1_write_frame_switch);
	// 告警抑制
	e1_write_frame("e1_trap_mask",iNumParams,pcParam,pcValue,&buf,board,portno,e1_write_frame_trap_mask);
	// 环回状态
	e1_write_frame("e1_loop",iNumParams,pcParam,pcValue,&buf,board,portno,e1_write_frame_loop);

	// configure it
	if(web_write_e1(board, &buf)<0)
		e1_config_state = -1;
	else
		e1_config_state = 0;
	return "/assets/ajax/e1_write.ajax";
}

