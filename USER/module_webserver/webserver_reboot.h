#ifndef  __WEBSERVER_REBOOT_H
#define  __WEBSERVER_REBOOT_H




const char* reboot_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void reboot_gray_ssi_handler(char * p , int len);
//void reboot_ban_ssi_handler(char * p,int len);
void reboot_go_ssi_handler(char * p,int len);
void reboot_poll(void);






#endif

