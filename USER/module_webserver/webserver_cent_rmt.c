#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "httpd_cgi.h"
#include "lwip_comm.h"
#include "gan111_type.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"
#include "webserver_login.h"
#include "mainconfig.h"


#define  CGI_REMOTE_STRING    "%E8%BF%9C"

int cr_remote_chosen = 0;

const char* cent_rmt_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	const char * rajax = "/assets/ajax/cr_read.ajax";
	const char * wajax = "/assets/ajax/cr_write.ajax";
	//cgi_print_submission(iNumParams,pcParam,pcValue);
	if(iNumParams < 2){
		// READ
		return rajax;
	}
	if(!mystrncmp(pcValue[FindCGIParameter("cent_rmt",pcParam,iNumParams)],CGI_REMOTE_STRING)){
		cr_remote_chosen = 1;
	}
	else
		cr_remote_chosen = 0;
	return wajax;
}

void cent_rmt_read_ssi_handler(char * p , int len){
	if(cr_remote_chosen)
		strcpy(p , "remote");
	else
		strcpy(p,"central");
}


