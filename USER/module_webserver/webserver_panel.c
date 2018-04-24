#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "mainconfig.h"
#include "lwip_comm.h"
#include "httpd_cgi.h"
#include "httpd_ssi.h"
#include "webserver_control_comm.h"
#include "webserver_panel.h"


const char* panel_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	return "/assets/ajax/panel.ajax";
}

void panel_ssi_handler(char * p , int len){
	int i;
	if(web_read_panel()<0){
		strcpy(p  , FAIL_STR);
		return;
	}
	for(i=0;i<16;++i){
		p[i] = web_control->bulb_ver[0].frame.bulb&(1<<i) ?'1':'0';
	}
	p[i]=0;
}
