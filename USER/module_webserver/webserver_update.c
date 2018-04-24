#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpd_cgi.h"
#include "httpd_ssi.h"
#include "mainconfig.h"
#include "malloc.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/udp.h"
#include "types.h"
#include "err.h"
#include "user_iap.h"
#include "mainconfig.h"
#include "w25xxx.h" 
#include "wgcmd_ftp.h"
#include "tftp_client.h"
#include "webserver_update.h"



/* filename user type in browser */
char       update_filename[64];
/* show status in web page according to this */
enum ssi_status update_ssi_status = SSI_STATUS_EMPTY;


static int update_go(void){
	/* Éý¼¶¿ªÊ¼ */
	printf("It's going to update(filename: %s)...\r\n",update_filename);
	update_ssi_status = SSI_STATUS_UPDATE_DOING;
	OSSemPost(tftp_manager->start_sem);
	return 0;
}

const char * update_filename_crop(char * raw){
	// behind the last "%5C"
	char tmp[128];
	const char * src = tmp;
	int i;
	strcpy(tmp , raw);
	for(i=strlen(tmp) ; i>=0;--i){
		if(!strncmp("%5C" , tmp+i , 3)){
			src = tmp+i+3;
			break;
		}
	}
	strcpy(raw , src);
	//printf("crop: %s\r\n",raw);
	return raw;
}

const char* update_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	const char * ajax = "/assets/ajax/update_status.ajax";
	char tmp[128];
	//cgi_print_submission(iNumParams,pcParam,pcValue);
	if( iNumParams<2){
		// read update status
		return ajax;
	}
	iIndex = FindCGIParameter("filename",pcParam,iNumParams);
	
	/* get file size for progress display */
	tftp_manager->total_file_len = atoi(pcValue[iIndex]);
	printf("totfilelen=%d\r\n",tftp_manager->total_file_len);
	tftp_manager->file_len = 0;
	
	strcpy(tmp , pcValue[iIndex]);
	strcpy(update_filename , update_filename_crop(tmp));
	update_go();
	return ajax;
}


void update_status_ssi_handler(char * pcInsert , int len){
	switch(update_ssi_status){
		case SSI_STATUS_UPDATE_DOING:
			sprintf(pcInsert , "proc=%d;\n" , tftp_get_progress_percent());
			return;
		case SSI_STATUS_UPDATE_SUCCEED:
			strcpy(pcInsert , SUCCESS_STR);
			return;
		case SSI_STATUS_UPDATE_FAILED:
			strcpy(pcInsert , FAIL_STR);
			return;
		default:
			*pcInsert = 0;
			return;
	}
}

