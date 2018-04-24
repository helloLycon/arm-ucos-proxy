#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "webserver_eth.h"
#include "httpd_cgi.h"
#include "mainconfig.h"
#include "lwip_comm.h"
#include "lwip/inet.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"

//static int eth_config_lineno,eth_config_ge,board,portno;
static int eth_config_state;

/*
int (* const eth_column_table[])(int,char*,int,int,int)={
	eth_column1_board_name,
	eth_column2_portname,
	eth_column3_port_pos,
	eth_column4_port_type,
	//eth_column5_port_comment,
	eth_column6_port_enable,
	eth_column7_link_state,
	eth_column8_auto_nego,
	eth_column9_bandwidth,
	eth_column10_full_duplex,
	eth_column11_flow_ctrl,
	eth_column12_trap_mask,
	eth_column13_cfg_btn,
};
*/

// 根据行号，判断是否需要显示，哪个板卡，
// 板卡中的第几个
int eth_get_msg_according_to_lineno(int ge,int lineno , int * board,int * portno){
	int i , sum=0;
	const struct web_eth * p_eth = web_control->eth;
	for(i=0 ; i<MAX_BOARD_NO ;++i){
		int j;
		for(j=0 ; j<(ge?p_eth[i].ge_amount:p_eth[i].fe_amount) ;++j){
			// 第n个端口
			if( (sum+j+1)==lineno ){
				// found
				*board = i;
				*portno = j;
				return 1;
			}
		}
		sum += ge?p_eth[i].ge_amount:p_eth[i].fe_amount;
	}
	// not found
	return 0;
}


// 端口名称
int eth_column2_portname(int ge,char * buf , int lineno,int board,int portno){
	//strcpy(buf , ge?"GTH_":"ETH_");
	//return 5+sprintf(buf+4 , "%d" , lineno);
	return 1+sprintf(buf,ge?"GTH_%d":"ETH_%d",lineno);
}

int eth_column3_port_pos(int ge,char * buf , int lineno,int board,int portno){
/*	int os = sprintf(buf,"%d",board+1);
	os += sprintf(buf+os,ge?"_GTH_":"_ETH_");
	os += sprintf(buf+os , "%d",portno+1);
	return os+1;*/
	return 1+sprintf(buf,ge?"%d_GTH_%d":"%d_ETH_%d",board+1,portno+1);
}
#if  0
int eth_column4_port_type(int ge,char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}
int eth_column5_port_comment(int ge,char * buf , int lineno,int board,int portno){
	return form_cp_ret(buf, FORM_EMTPY);
}

int eth_column6_port_enable(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	if(dat.bit.enable){
		return form_cp_ret(buf , FORM_ENABLE);
	}
	return form_cp_ret(buf , FORM_DISABLE);
}

int eth_column7_link_state(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	if(dat.bit.link){
		return form_cp_ret(buf , FORM_LINK);
	}
	return form_cp_ret(buf , FORM_UNLINK);
}

int eth_column8_auto_nego(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	if(dat.bit.auto_nego){
		return form_cp_ret(buf , FORM_AUTO_NEGO);
	}
	return form_cp_ret(buf , FORM_FORCE);
}

int eth_column9_bandwidth(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	//sprintf(buf , "%d" , dat.bit.bandwidth);
	if(0==dat.bit.bandwidth)
		return form_cp_ret(buf, FORM_10M);
	else if(1==dat.bit.bandwidth)
		return form_cp_ret(buf, FORM_100M);
	return form_cp_ret( buf, FORM_1000M);
}

int eth_column10_full_duplex(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	//sprintf(buf , "%d" , dat.bit.full_duplex);
	if(dat.bit.full_duplex)
		return form_cp_ret(buf, FORM_DUPLEX_FULL);
	return form_cp_ret(buf, FORM_DUPLEX_HALF);
}

int eth_column11_flow_ctrl(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	if(dat.bit.flow_ctrl){
		return form_cp_ret(buf,FORM_OPEN);
	}
	return form_cp_ret(buf,FORM_CLOSE);
}

int eth_column12_trap_mask(int ge,char * buf , int lineno,int board,int portno){
	union web_eth_config dat ;
	if(ge)
		dat = web_control->eth[board].ge[portno];
	else
		dat = web_control->eth[board].fe[portno];
	if(dat.bit.trap_mask){
		return form_cp_ret(buf,FORM_TRAP_MASK);
	}
	return form_cp_ret(buf,FORM_TRAP_NOMASK);
}

int eth_column13_cfg_btn(int ge,char * buf , int lineno,int board,int portno){
	char tmp[8];
	*tmp = ge?'G':'F';
	sprintf(tmp+1 , "%d" , lineno);
	return form_insert_cfg_btn(buf, tmp);
}

void eth_common_line_ssi_handler(int ge,int lineno,char * pcInsert , int len){
	const char * text_arr[ETH_COLUMN_NO+1]={0};
	char buf[512];
	int board,portno ;
	int display_needed; 
	
	display_needed = eth_get_msg_according_to_lineno(ge,lineno,&board,&portno);
	if(display_needed){
		int i , os=0;
		for( i=0 ; i< ETH_COLUMN_NO ;++i){
			int len = eth_column_table[i](ge,buf+os , lineno,board,portno);
			text_arr[i] = buf+os;
			os += len;
		}
		fill_row(pcInsert,text_arr);
	}
	else{
		strcpy(pcInsert," ");
	}
}

void eth_ge_line1_ssi_handler(char * pcInsert , int len){
	/* obtain datas */
//	web_control->eth[0].ge_amount = 1;
//	web_control->eth[1].ge_amount = 1;
//	web_control->eth[8].ge_amount = 2;
//	web_control->eth[1].fe_amount = 2;
//	web_control->eth[8].fe_amount = 1;
//	web_control->eth[8].fe[0].half_word = 0xffff;
	web_read_eth();
	eth_common_line_ssi_handler(1,1,pcInsert,len);
}
void eth_ge_line2_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(1,2,pcInsert,len);
}
void eth_ge_line3_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(1,3,pcInsert,len);
}
void eth_ge_line4_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(1,4,pcInsert,len);
}

void eth_fe_line1_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,1,pcInsert,len);
}
void eth_fe_line2_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,2,pcInsert,len);
}
void eth_fe_line3_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,3,pcInsert,len);
}
void eth_fe_line4_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,4,pcInsert,len);
}
void eth_fe_line5_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,5,pcInsert,len);
}
void eth_fe_line6_ssi_handler(char * pcInsert , int len){
	eth_common_line_ssi_handler(0,6,pcInsert,len);
}


void eth_config_name_ssi_handler(char * pcInsert , int len){
	sprintf(pcInsert,
	        eth_config_ge?"GTH_%d":"ETH_%d" ,
	        eth_config_lineno);
}

void eth_switch_enable_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_switch_disable_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.enable)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void eth_force_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.auto_nego)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void eth_auto_nego_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.auto_nego)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_10M_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(0 == dat.bit.bandwidth)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_100M_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(1 == dat.bit.bandwidth)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_1000M_full_ssi_handler(char * pcInsert , int len){
	if(eth_config_ge){
		union web_eth_config dat;
		dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
		if(2 == dat.bit.bandwidth)
			strcpy(pcInsert,"<option selected>1000M</option>");
		else
			strcpy(pcInsert,"<option>1000M</option>");
	}
	else
		*pcInsert = 0;
}
void eth_duplex_half_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.full_duplex)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void eth_duplex_full_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.full_duplex)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_flow_ctrl_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.flow_ctrl)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
void eth_flow_noctrl_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.flow_ctrl)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void eth_trap_nomask_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_UNSELECTED);
	else
		strcpy(pcInsert,OPTION_SELECTED);
}
void eth_trap_mask_ssi_handler(char * pcInsert , int len){
	union web_eth_config dat;
	dat = eth_config_ge?(web_control->eth[board].ge[portno]):(web_control->eth[board].fe[portno]);
	if(dat.bit.trap_mask)
		strcpy(pcInsert,OPTION_SELECTED);
	else
		strcpy(pcInsert,OPTION_UNSELECTED);
}
#endif


int eth_make_data(char * p , int board,int portno,u16 data){
	p[0] = ntoalpha(board);
	p[1] = ntoalpha(portno);
	sprintf(p+2 , "%04x" , data);
	return 6;
}

void eth_read_data_ssi_handler(char * p , int len){
	int board,portno,os = 0;
	if(web_read_eth()<0){
		strcpy(p , FAIL_STR);
		return;
	}
	for(board = 0 ; board<MAX_BOARD_NO;++board){
		for(portno = 0;portno<web_control->eth[board].ge_amount;++portno){
			os += eth_make_data(p+os,
			                    board,
			                    portno,
			                    web_control->eth[board].ge[portno].half_word);
		}
	}
	os += sprintf(p+os , "_FE_");
	for(board = 0 ; board<MAX_BOARD_NO;++board){
		for(portno = 0;portno<web_control->eth[board].fe_amount;++portno){
			os += eth_make_data(p+os,
			                    board,
			                    portno,
			                    web_control->eth[board].fe[portno].half_word);
		}
	}
}

int 
eth_write_frame(const char * cginame,
                     int iNumParams,
                     char *pcParam[],
                     char *pcValue[],
                     union web_eth_config* buf,
                     int board,
                     int portno,
                     int (*pfunc)(union web_eth_config *,int,int,const char *)){
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

int eth_write_frame_switch(union web_eth_config * buf,int board,int portno,const char *pcValue){
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

int eth_write_frame_auto_nego(union web_eth_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_AUTO_NEGO)){
		buf[portno].bit.auto_nego = 1;
		debug_printf(DBG_DEBUG, "<auto_nego>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_FORCE)){
		buf[portno].bit.auto_nego = 0;
		debug_printf(DBG_DEBUG, "<force>\r\n");
		return 0;
	}
	return -1;
}

int eth_write_frame_bandwidth(union web_eth_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,"10M")){
		buf[portno].bit.bandwidth = 0;
		debug_printf(DBG_DEBUG, "<10M>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,"100M")){
		buf[portno].bit.bandwidth = 1;
		debug_printf(DBG_DEBUG, "<100M>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,"1000M")){
		buf[portno].bit.bandwidth = 2;
		debug_printf(DBG_DEBUG, "<1000M>\r\n");
		return 0;
	}
	return -1;
}

int eth_write_frame_duplex(union web_eth_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_DUPLEX_HALF)){
		buf[portno].bit.full_duplex = 0;
		debug_printf(DBG_DEBUG, "<duplex_half>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_DUPLEX_FULL)){
		buf[portno].bit.full_duplex = 1;
		debug_printf(DBG_DEBUG, "<duplex_full>\r\n");
		return 0;
	}
	return -1;
}

int eth_write_frame_flow_ctrl(union web_eth_config * buf,int board,int portno,const char *pcValue){
	if(!mystrncmp(pcValue,CGISTR_OPEN )){
		buf[portno].bit.flow_ctrl = 1;
		debug_printf(DBG_DEBUG, "<flow_ctrl>\r\n");
		return 0;
	}
	if(!mystrncmp(pcValue,CGISTR_CLOSE)){
		buf[portno].bit.flow_ctrl = 0;
		debug_printf(DBG_DEBUG, "<flow_no_ctrl>\r\n");
		return 0;
	}
	return -1;
}

int eth_write_frame_trap_mask(union web_eth_config * buf,int board,int portno,const char *pcValue){
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

const char* eth_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	int eth_config_ge,board,portno;
	union web_eth_config buf[16]={0};
	//cgi_print_submission(iNumParams, pcParam ,pcValue);
	if( iNumParams<2){
		return "/assets/ajax/eth_read.ajax";
	}
	if(*pcValue[FindCGIParameter("ge",pcParam,iNumParams)] =='G')
		eth_config_ge = 1;
	else
		eth_config_ge = 0;
	board = atoi(pcValue[FindCGIParameter("board",pcParam,iNumParams)]);
	portno= atoi(pcValue[FindCGIParameter("portno",pcParam,iNumParams)]);	
	if(eth_config_ge)
		memcpy(buf , web_control->eth[board].ge,sizeof(web_control->eth[0].ge));
	else
		memcpy(buf , web_control->eth[board].fe,sizeof(web_control->eth[0].fe));
	
	// 开关
	eth_write_frame("eth_switch",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_switch);
	// 自动协商
	eth_write_frame("eth_auto_nego",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_auto_nego);
	// 接口速率
	eth_write_frame("eth_bandwidth",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_bandwidth);
	// 双工
	eth_write_frame("eth_duplex",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_duplex);	
	// 流控
	eth_write_frame("eth_flow_ctrl",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_flow_ctrl);	
	// 告警抑制
	eth_write_frame("eth_trap_mask",iNumParams,pcParam,pcValue,buf,board,portno,eth_write_frame_trap_mask);	

	// configure it
	if(web_write_eth(eth_config_ge , board, buf)<0)
		eth_config_state = -1;
	else
		eth_config_state = 0;
	return "/assets/ajax/eth_write.ajax";
}

void eth_config_status_ssi_handler(char * pcInsert , int len){
	if(eth_config_state<0)
		strcpy(pcInsert,FAIL_STR);
	else
		strcpy(pcInsert,SUCCESS_STR);
}

