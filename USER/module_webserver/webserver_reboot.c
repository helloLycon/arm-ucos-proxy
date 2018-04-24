#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ucos_ii.h"
#include "webserver_reboot.h"
#include "httpd_cgi.h"



static int reboot_set = 0;

const char* reboot_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	if(FindCGIParameter("reboot",pcParam,iNumParams) != -1){
		//printf("rebooting...\r\n");
		//control_set_sysreset();
		reboot_set = 1;
	}
	else{
		printf("WARNING: there is no parameter called \"reboot\"!\r\n");
	}
	return "/assets/ajax/reboot.ajax";
}



void reboot_poll(void){
	int control_set_sysreset(void);
	if(reboot_set){
		OSTimeDlyHMSM(0,0,1,0);
		printf("rebooting...\r\n");
		control_set_sysreset();
	}
}

