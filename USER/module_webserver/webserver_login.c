#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/udp.h"
#include "ucos_ii.h"
#include "mainconfig.h"
#include "webserver_login.h"
#include "httpd_cgi.h"
#include "webserver_account.h"



#if   0
static struct login_control login_ctrl = {0};


static uint32_t auth_get_os_time(void){
	OS_CPU_SR cpu_sr;
	uint32_t time;
	OS_ENTER_CRITICAL();
	time = OSTime;
	OS_EXIT_CRITICAL();
	return time;
}

static const char * auth_time_ntoa(uint32_t time,char * buf){
	uint32_t secs = time/OS_TICKS_PER_SEC;
	uint32_t h,m,s;
	h = secs/3600;
	m = (secs%3600)/60;
	s = secs%60;
	sprintf(buf , "%d:%02d:%02d",h,m,s);
	return buf;
}

static void auth_print_logged(const struct login_node * set,int size){
	int i , head = 0;
	char buf[16],timebuf[16];
	printf("----------LOGIN LIST----------\r\n");
	for(i=0;i<size;++i){
		if(set[i].ip.addr){
			printf("%d. %s[at %s]\r\n",++head,inet_ntoa_r(set[i].ip , buf,sizeof(buf)),auth_time_ntoa(set[i].time,timebuf));
		}
	}
	printf("------------------------------\r\n");
}

static int auth_add_node(struct login_node * set,int size,ip_addr_t ip){
	int i;
	char buf[16],timebuf[16];
	uint32_t oldest_time = set->time;
	int oldest_index = 0;

	if(!ip.addr)
		return 0;
	for(i=0 ; i<size;++i){
		if( set[i].ip.addr == ip.addr){
			/* already logged in, just update login time */
			set[i].time = auth_get_os_time();
			printf("[%s] %s(repeated) logged in.\r\n" ,auth_time_ntoa(auth_get_os_time(),timebuf), inet_ntoa_r(ip,buf,sizeof(buf)));
			auth_print_logged(login_ctrl.login_clients,LOGIN_CLIENT_NO);
			return 0;
		}
	}
	for(i=0;i<size;++i){
		if( 0 == set[i].ip.addr ){
			// node added
			set[i].ip = ip;
			set[i].time = auth_get_os_time();
			printf("[%s] %s(new) logged in!\r\n" ,auth_time_ntoa(auth_get_os_time(),timebuf), inet_ntoa_r(ip,buf,sizeof(buf)));
			auth_print_logged(login_ctrl.login_clients,LOGIN_CLIENT_NO);
			return 1;
		}
		if( set[i].time < oldest_time){
			oldest_time = set[i].time;
			oldest_index = i;
		}
	}
	// node added and substitute oldest node
	set[oldest_index].ip = ip;
	set[oldest_index].time = auth_get_os_time();
	printf("[%s] %s(new) logged in!\r\n" ,auth_time_ntoa(auth_get_os_time(),timebuf), inet_ntoa_r(ip,buf,sizeof(buf)));
	auth_print_logged(login_ctrl.login_clients,LOGIN_CLIENT_NO);
	return 1;
}

static int auth_remove_node(struct login_node * set,int size,ip_addr_t ip){
	int i;
	char buf[16],timebuf[16];
	if( !ip.addr )
		return 0;
	for(i=0 ; i<size;++i){
		if(set[i].ip.addr == ip.addr){
			set[i].ip.addr = 0;
			printf("[%s] %s logged out.\r\n" ,auth_time_ntoa(auth_get_os_time(),timebuf),inet_ntoa_r(ip,buf,sizeof(buf)));
			auth_print_logged(login_ctrl.login_clients,LOGIN_CLIENT_NO);
			return 1;
		}
	}
	return 0;
}

static int auth_check(const struct login_node * set,int size,ip_addr_t ip){
	int i;
	if(!ip.addr)
		return 0;
	for(i=0;i<size;++i){
		if(set[i].ip.addr == ip.addr)
			return 1;
	}
	return 0;
}

void auth_accept(ip_addr_t client_ip){
	login_ctrl.current_client = client_ip;
}

void auth_clear_all_login(void){
	memset(&login_ctrl , 0 , sizeof(login_ctrl));
}

void auth_logged_timeout_poll(void){
	uint32_t timeout = TIMEOUT_6_HOUR;
	struct login_node * set = login_ctrl.login_clients;
	uint32_t now = auth_get_os_time();
	int i;

	for(i=0;i<LOGIN_CLIENT_NO;++i){
		if( set[i].ip.addr ){
			if( now < set[i].time ){
				// OsTime overflow
				auth_remove_node(set,LOGIN_CLIENT_NO,set[i].ip);
				continue;
			}
			if( (now-set[i].time) > timeout ){
				auth_remove_node(set,LOGIN_CLIENT_NO,set[i].ip);
			}
		}
	}
}
#endif

const char* login_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
#if  0
	struct web_account acct;
	const char * login_page = "/index.shtml";
	int usr_index,passwd_index;
	//cgi_print_submission(iNumParams, pcParam,pcValue);
	if( !iNumParams)
		return "/index.shtml";
	if((usr_index = FindCGIParameter("login_user",pcParam,iNumParams))<0){
		printf("ERR: no \"login_user\" field!\r\n");
		return login_page;
	}
	if((passwd_index = FindCGIParameter("login_passwd",pcParam,iNumParams))<0){
		printf("ERR: no \"login_passwd\" field!\r\n");
		return login_page;
	}
	web_account_read(&acct);
	if( strcmp(acct.username,pcValue[usr_index]) || strcmp(acct.passwd,pcValue[passwd_index])){
		login_ctrl.login_wrong = 1;
		auth_remove_node(login_ctrl.login_clients,LOGIN_CLIENT_NO,login_ctrl.current_client);
		return login_page;
	}
	// an ip logged in.
	auth_add_node(login_ctrl.login_clients,LOGIN_CLIENT_NO,login_ctrl.current_client);
	return "/main.shtml";
#else
	return "/assets/ajax/login.ajax";	
#endif
}

void login_usrpasswd_ssi_handler(char * p , int len){
	struct web_account acct;
	web_account_read(&acct);
	sprintf(p,"username=%s;\npasswd=%s;\n" , acct.username,acct.passwd);
}

