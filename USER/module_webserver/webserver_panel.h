#ifndef  __WEBSERVER_PANEL_H
#define  __WEBSERVER_PANEL_H


#define  SVG_COLOR_GREEN  "#00FF00"



//void panel_led_ssi_handler(char * pcInsert , int len);
//const char* panel_refresh_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
//void panel_refresh_select_ssi_handler(char* p,int len);
//void panel_refresh_meta_ssi_handler(char* p,int len);


void panel_bulb_c1_ssi_handler(char * pcInsert , int len);
void panel_bulb_c2_ssi_handler(char * pcInsert , int len);
void panel_bulb_c3_ssi_handler(char * pcInsert , int len);
void panel_bulb_c4_ssi_handler(char * pcInsert , int len);
const char* panel_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void panel_ssi_handler(char * p , int len);





#endif

