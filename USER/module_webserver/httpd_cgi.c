#include <string.h>
#include <stdlib.h>
#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "httpd_cgi.h"
#include "webserver_panel.h"
#include "webserver_cent_rmt.h"
#include "webserver_login.h"
#include "webserver_account.h"
#include "webserver_net_config.h"
#include "webserver_reboot.h"
#include "webserver_update.h"
#include "webserver_opt.h"
#include "webserver_eth.h"
#include "webserver_e1.h"

/*
 * initializations of cgi objects in webpage...
 * use CGI_OBJ(act,hdl) to initialize a cgi object
 * act: value of "action" in form(eg. /action.cgi)
 * hdl: handler of obtaining a form from browser
 */
static const tCGI ppcURLs[]=
{
	/* LOGIN */
	CGI_OBJ("login", login_cgi_interface),
	/* panel+¾Ö¶Ë/Ô¶¶Ë+reboot(main page) */
	CGI_OBJ("panel"   , panel_cgi_interface),
	CGI_OBJ("cent_rmt", cent_rmt_cgi_interface),
	CGI_OBJ("reboot"  , reboot_cgi_interface),
	/* account */
	CGI_OBJ("account", account_cgi_interface),
	/* net config */
	CGI_OBJ("net"    , net_cgi_interface),
	/* tftp update */
	CGI_OBJ("update"   , update_cgi_interface),
	/* ¹â¿ÚÅäÖÃ */
	CGI_OBJ("opt"   , opt_cgi_interface),
	/* ethÅäÖÃ */
	CGI_OBJ("eth"   , eth_cgi_interface),
	/* e1ÅäÖÃ */
	CGI_OBJ("e1"   , e1_cgi_interface),
};

#if   0
/* 
 * name: 
 *   httpd_cgi_common_handler:
 * function:
 *   general method of handling a struct cgi_class object
 * retval:
 *   0           : NO CHANGE or ERROR
 *   more than 0 : param has been CHANGED
 */
int httpd_cgi_common_handler(const struct cgi_class * obj,int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	int found_index , changed = 0;
	unsigned int num_get;
	unsigned int * p_uint;
	char * p_string;
	
	found_index = FindCGIParameter(obj->name,pcParam,iNumParams);
	/* found a fit param */
	if( found_index != -1 ){
		switch(obj->format){
			case CGI_FORMAT_NUMERIC:
				num_get = atoi(pcValue[found_index]);
				p_uint = (unsigned int *)obj->store;
				/* the uint value changed */
				if( num_get != (*p_uint)){
					/* if it has a check method */
					if( obj->check ){
						if( -1 != obj->check(&num_get) ){
						}
						else{
							/* invalid !!! */
							printf("ERROR: %d is invalid!\r\n",num_get);
							if(obj->status)
								*(obj->status) = SSI_STATUS_INVALID_VALUE;
							return 0;
						}
					}
					/* VALID */
					++changed;
					/* store the new uint num */
					*p_uint = num_get;
					/* handle the num */
					obj->handler(p_uint);
				}
				/* do nothing if not changed */
				break;
			case CGI_FORMAT_STRING:
				p_string = (char *)obj->store;
				/* the string value changed */
				if( strcmp(pcValue[found_index],p_string) ){
					/* if it has a check method */
					if( obj->check ){
						if( -1 != obj->check(pcValue[found_index]) ){
						}
						else{
							/* invalid !!! */
							printf("ERROR: %s is invalid!\r\n",pcValue[found_index]);
							if(obj->status)
								*(obj->status) = SSI_STATUS_INVALID_VALUE;
							return 0;
						}
					}
					/* VALID */
					++changed;
					/* store the new string */
					strcpy(p_string , pcValue[found_index]);
					/* handle the string */
					obj->handler(p_string);
				}
				/* do nothing if not changed */
				break;
		}
		/* end */
		return changed;
	}
	printf("WARNING: there is no parameter called \"%s\"!\r\n",obj->name);
	return 0;
}
#endif

/* find the index of the FIRST fit parameter */
int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop);
		}
	}
	return (-1);
}

void cgi_print_submission(int iNumParams, char *pcParam[], char *pcValue[]){
	int i;
	printf("[iNumParams = %d]\r\n",iNumParams);
	for(i=0;i<iNumParams;++i){
		printf("[%s]=%s\r\n",pcParam[i],pcValue[i]);
	}
}

//CGI¾ä±ú³õÊ¼»¯
void httpd_cgi_init(void)
{ 
	http_set_cgi_handlers(ppcURLs,
		(sizeof(ppcURLs) / sizeof(tCGI)));
}


