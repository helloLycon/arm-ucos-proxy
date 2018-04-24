#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "httpd_cgi.h"
#include "lwip_comm.h"
#include "gan111_type.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"
#include "webserver_boards_mng.h"


int (* const board_column_table[])(char*,int)={
	board_column1_slot,
	board_column2_type,
	board_column3_business_type,
	board_column4_pcb_version,
	board_column5_sw_version,
	board_column6_fpga_version,
	board_column7_boot_version,
	board_column8_state,
};

//ÁÙÊ±ÓÃ
const char * board_get_type_name(u32 type , int longname){
	if(longname){
		switch(type){
			case GAN111_MAIN_CTRL:
				return "GAN111&#20027;&#25511;&#26495;";
			case GAN111_8E1:
				return "8E1&#25509;&#21475;&#30424;";
			default:
				return FORM_EMTPY;
		}
	}
	switch(type){
		case GAN111_MAIN_CTRL:
			return "GAN111";
		case GAN111_8E1:
			return "8E1";
		default:
			return FORM_EMTPY;
	}
}

int board_get_boardno_according_to_lineno(int lineno,int * board){
	int i , sum=0;
	const struct web_bulb_ver * p_bv = web_control->bulb_ver;
	for(i=0 ; i<MAX_BOARD_NO ;++i){
		if(p_bv[i].set){
			sum++;
			if( sum == lineno ){
				// found
				*board = i;
				return 1;
			}
		}
	}
	// not found
	return 0;
}

int board_column1_slot(char * p , int lineno){
	int board;
	board_get_boardno_according_to_lineno(lineno , &board);
	return 1+sprintf(p , "%d" , board+1);
}

int board_column2_type(char * p,int lineno){
	int board;
	board_get_boardno_according_to_lineno(lineno , &board);
	return 1+sprintf(p , board_get_type_name(web_control->boards_exist.type[board],0));
}

int board_column3_business_type(char * p, int lineno){
	int board;
	board_get_boardno_according_to_lineno(lineno , &board);
	return 1+sprintf(p , board_get_type_name(web_control->boards_exist.type[board],1));
}

int board_column4_pcb_version(char * p , int lineno){
	int board;
	char buf[8];
	u8 ver;
	board_get_boardno_according_to_lineno(lineno , &board);
	ver = web_control->bulb_ver[board].frame.pcb_ver;
	if(!ver)
		return form_cp_ret(p, FORM_EMTPY);
	return 1+sprintf(p , "V%s",version_ntoa(ver,buf));
}


int board_column5_sw_version(char * p , int lineno){
	int board;
	char buf[8];
	u8 ver;
	board_get_boardno_according_to_lineno(lineno , &board);
	ver = web_control->bulb_ver[board].frame.cpu_app_ver;
	if(!ver)
		return form_cp_ret(p, FORM_EMTPY);
	return 1+sprintf(p , "V%s",version_ntoa(ver,buf));
}


int board_column6_fpga_version(char * p , int lineno){
	int board;
	u16 ver;
	board_get_boardno_according_to_lineno(lineno , &board);
	ver = web_control->bulb_ver[board].frame.fpga_ver;
	if(!ver)
		return form_cp_ret(p,FORM_EMTPY);
	return 1+sprintf(p , "V%d.%d%d%d",
	(ver&0x00F0)>>4,
	(ver&0x000F)>>0,
	(ver&0xF000)>>12,
	(ver&0x0F00)>>8
	);
}

int board_column7_boot_version(char * p , int lineno){
	int board;
	char buf[8];
	u8 ver;
	board_get_boardno_according_to_lineno(lineno , &board);
	ver = web_control->bulb_ver[board].frame.boot_ver;
	if(!ver)
		return form_cp_ret(p, FORM_EMTPY);
	return 1+sprintf(p , "V%s",version_ntoa(ver,buf));
}

int board_column8_state(char * p,int lineno){
	return form_cp_ret(p,"&#27491;&#24120;");
}


void board_common_line_ssi_handler(int lineno,char * pcInsert , int len){
	const char * text_arr[BOARD_COLUMN_NO+1]={0};
	char buf[512];
	int board;
	int display_needed; 
	
	display_needed = board_get_boardno_according_to_lineno(lineno,&board);
	if(display_needed){
		int i , os=0;
		for( i=0 ; i< BOARD_COLUMN_NO ;++i){
			int len = board_column_table[i](buf+os , lineno);
			text_arr[i] = buf+os;
			os += len;
		}
		fill_row(pcInsert,text_arr);
	}
	else{
		strcpy(pcInsert," ");
	}
}

void board_line1_ssi_handler(char * p,int len){
	web_read_boards_version();
	board_common_line_ssi_handler(1,p,len);
}

void board_line2_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(2,p,len);
}

void board_line3_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(3,p,len);
}

void board_line4_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(4,p,len);
}

void board_line5_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(5,p,len);
}

void board_line6_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(6,p,len);
}

void board_line7_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(7,p,len);
}

void board_line8_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(8,p,len);
}

void board_line9_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(9,p,len);
}

void board_line10_ssi_handler(char * p,int len){
	board_common_line_ssi_handler(10,p,len);
}

