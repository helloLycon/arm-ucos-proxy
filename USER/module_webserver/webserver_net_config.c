#include <stdio.h>
#include <string.h>
#include "webserver_net_config.h"
#include "httpd_cgi.h"
#include "mainconfig.h"
#include "lwip_comm.h"
#include "lwip/inet.h"
#include "webserver_control_comm.h"




/* status display */
static int net_config_status;


/* this is the interface to "httpd_cgi.c" */
const char* net_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
	struct tagGlobalConfigS  global_cfg;   
	const char * read_ajax = "/assets/ajax/net_read.ajax";
	const char * write_ajax = "/assets/ajax/net_write.ajax";
	//cgi_print_submission(iNumParams,pcParam, pcValue);
	if( iNumParams<2){
		// READ
		return read_ajax;
	}
	// WRITE
	/* 应用网络设置更改 */
	main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
	global_cfg.host_ip = ntohl(inet_addr(pcValue[FindCGIParameter("dev_ip",pcParam,iNumParams)]));
	global_cfg.netmask = ntohl(inet_addr(pcValue[FindCGIParameter("dev_mask",pcParam,iNumParams)]));
	global_cfg.gateway = ntohl(inet_addr(pcValue[FindCGIParameter("dev_gateway",pcParam,iNumParams)]));
	global_cfg.jump_ip = ntohl(inet_addr(pcValue[FindCGIParameter("mng_ip",pcParam,iNumParams)]));
	// validation
	if(!ip_msk_gw_validation(global_cfg.host_ip,global_cfg.netmask,global_cfg.gateway)){
		net_config_status = -1;
	}
	else{
		/* 设置本地网络参数 */
		cmd_set_ipaddr (global_cfg.host_ip);
		cmd_set_netmask(global_cfg.netmask);
		cmd_set_gw     (global_cfg.gateway);
		main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
		/* 设置网关主机参数 */
		jump_update_udp_addr();
		/* +++ */
		st_update_udp_send_addr();
		net_config_status = 0;  // must success
	}
	return write_ajax;
}



void net_read_data_ssi_handler(char * p, int len){
	int os = 0 ;
	os += ajax_get_fill(p+os,"dev_ip",main_config_read_dev_ip,NULL);
	os += ajax_get_fill(p+os,"dev_port",main_config_read_dev_port,NULL);
	os += ajax_get_fill(p+os,"dev_mask",main_config_read_dev_mask,NULL);
	os += ajax_get_fill(p+os,"dev_gateway",main_config_read_dev_gateway,NULL);
	os += ajax_get_fill(p+os,"dev_mac",main_config_read_dev_mac,NULL);
	os += ajax_get_fill(p+os,"mng_ip",main_config_read_mng_ip,NULL);
	os += ajax_get_fill(p+os,"mng_port",main_config_read_mng_port,NULL);
	os += sprintf(p+os , "dev_id=%d;\n" ,web_control->dev_id);
	os += ajax_get_fill(p+os,"boot_ver",main_config_read_v1,NULL);
	os += ajax_get_fill(p+os,"app_ver",main_config_read_v2,NULL);
}

void net_config_status_ssi_handler(char * p , int len){	
	if( 0 == net_config_status )
		strcpy(p , SUCCESS_STR);
	else
		strcpy(p, FAIL_STR);
}


