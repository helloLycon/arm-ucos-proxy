#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "httpd_cgi.h"
#include "httpd_ssi.h"
#include "lwip_comm.h"
#include "gan111_type.h"
#include "webserver_build_form.h"
#include "webserver_control_comm.h"
#include "webserver_form_string.h"
#include "webserver_login.h"
#include "mainconfig.h"

static int account_status;

const char* account_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
#if 0
	const char * acct_page = "/account.shtml";
	struct web_account buf;
	//cgi_print_submission(iNumParams,pcParam,pcValue);
	if(!iNumParams)
		return "/index.shtml";
	web_account_read(&buf);
	if( strcmp(pcValue[FindCGIParameter("cur_user",pcParam,iNumParams)],buf.username)||
		strcmp(pcValue[FindCGIParameter("cur_passwd",pcParam,iNumParams)],buf.passwd)){
		// incorrect old username and passwd
		account_status = 2;
		return acct_page;
	}
	// correct
	strcpy(buf.username , pcValue[FindCGIParameter("new_user",pcParam,iNumParams)]);
	strcpy(buf.passwd, pcValue[FindCGIParameter("new_passwd",pcParam,iNumParams)]);
	web_account_write(&buf);
	// Succeed
	account_status = 1;
	return acct_page;
#else
	const char * acct_ajax = "/assets/ajax/account.ajax";
	struct web_account buf;
	//cgi_print_submission(iNumParams,pcParam,pcValue);
	if(iNumParams<2){
		account_status = 1;
		return acct_ajax;
	}
	web_account_read(&buf);
	if( strcmp(pcValue[FindCGIParameter("cur_user",pcParam,iNumParams)],buf.username)||
		strcmp(pcValue[FindCGIParameter("cur_passwd",pcParam,iNumParams)],buf.passwd)){
		// incorrect old username and passwd
		account_status = 2;
		return acct_ajax;
	}
	// correct
	strcpy(buf.username , pcValue[FindCGIParameter("new_user",pcParam,iNumParams)]);
	strcpy(buf.passwd, pcValue[FindCGIParameter("new_passwd",pcParam,iNumParams)]);
	web_account_write(&buf);
	// Succeed
	account_status = 0;
	return acct_ajax;
#endif
}

void account_status_ssi_handler(char * p ,int len){
	switch(account_status){
		case 0:
		default:
			strcpy(p , SUCCESS_STR);
			//auth_clear_all_login();
			break;
		case 1:
			//strcpy(p,"\naccount_status(true);\n");
			strcpy(p , FAIL_STR);
			break;
		case 2:
			strcpy(p,INCORRECT_STR);
			break;
	}
}

