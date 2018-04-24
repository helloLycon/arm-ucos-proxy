#ifndef  __WEBSERVER_UPDATE_H
#define  __WEBSERVER_UPDATE_H




const char* update_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void update_status_ssi_handler(char * pcInsert , int len);
//static int update_filename_check(void * vp);
//static int update_cgi_handler(void * vp);
//void update_refresh_time_ssi_handler(char * pcInsert , int len);
//void update_progress_ssi_handler(char *p , int len);
//void update_gray_ssi_handler(char * p,int len);
//void update_ban_ssi_handler(char* p,int len);
//void update_count_down_ssi_handler(char *p,int len);
//void update_ban_countdown_ssi_handler(char* p,int len);
//extern u16 update_refresh_time;

extern enum ssi_status update_ssi_status;
extern char update_filename[];






#endif
