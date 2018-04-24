#ifndef  __HTTPD_CGI_H
#define  __HTTPD_CGI_H


#include "httpd_ssi.h"
/* macros */
#define  CGI_ACTION(x)      ("/"x".cgi")
// 用该宏初始化一个cgi对象
#define  CGI_OBJ(act,hdl)   {CGI_ACTION(act),hdl}


#if  0
/* cgi data format definition: string or numeric? */
enum cgi_data_format {
	CGI_FORMAT_NUMERIC = 0,
	CGI_FORMAT_STRING , 
};

/* cgi class definition(correspond a cgi item) */
struct cgi_class {
	/* cgi parameter name */
	const char * name;
	/* value format: numeric or string? */
	enum cgi_data_format format;
	/* check validation, 0 on success, -1 on failure */
	int (*check)(void *);
	/* return result of check */
	enum ssi_status * status;
	/* where the value stores */
	void * store;
	/* how to handle the value received */
	int (*handler)(void *);
};
#endif


/* declarations */
int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams);
void httpd_cgi_init(void);
void cgi_print_submission(int iNumParams, char *pcParam[], char *pcValue[]);





#endif

