#ifndef __WEBSERVER_LOGIN_H
#define __WEBSERVER_LOGIN_H


#define LOGIN_CLIENT_NO  4u

#define TIMEOUT_1_MIN       (OS_TICKS_PER_SEC*60U)
#define TIMEOUT_6_HOUR      (OS_TICKS_PER_SEC*60u*60*6)
//#define WEB_LOGGED_TIMEOUT  TIMEOUT_1_MIN

struct login_node{
	ip_addr_t ip;
	uint32_t time;
};

struct login_control {
	struct login_node login_clients[LOGIN_CLIENT_NO];
	ip_addr_t current_client;
	int login_wrong;
};


void auth_accept(ip_addr_t client_ip);
void auth_clear_all_login(void);
void auth_logged_timeout_poll(void);
const char* login_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void login_wrong_ssi_handler(char * p,int len);
void login_jump_ssi_handler(char * p , int len);
void login_usrpasswd_ssi_handler(char * p , int len);




#endif

