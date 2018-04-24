#ifndef  __WEBSERVER_ACCOUNT_H
#define  __WEBSERVER_ACCOUNT_H



struct web_account{
	char username[64];
	char passwd[64];
	u16  crc;
};




const char* account_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void account_status_ssi_handler(char * p ,int len);
void account_logged_name_ssi_handler(char * p , int len);



#endif

