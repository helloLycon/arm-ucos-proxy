#ifndef  __WEBSERVER_BUILD_FORM_H
#define  __WEBSERVER_BUILD_FORM_H


#define  FORM_EMTPY  "&nbsp;"
#define  OPTION_SELECTED  "<option selected>"
#define  OPTION_UNSELECTED "<option>"


int fill_unit(char * buf , const char * text);
int fill_row(char * buf , const char ** text_arr);
int form_cp_ret(char* buf,const char * str);
int form_insert_cfg_btn(char * buf,const char * name);
int mystrncmp(const char * s,const char * fix_str);



#endif

