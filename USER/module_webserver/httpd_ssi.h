#ifndef  __HTTPD_SSI_H
#define  __HTTPD_SSI_H



/* httpd ssi class declaration */
struct httpd_ssi_class {
	const char * tag;
	void (* handler)(char * , int );
};

/* ssi status display declaration */
enum ssi_status {
	SSI_STATUS_READ_SUCCESS = 0,
	SSI_STATUS_WRITE_SUCCESS,
	SSI_STATUS_INVALID_VALUE,
	/* for update */
	SSI_STATUS_UPDATE_DOING,
	SSI_STATUS_UPDATE_SUCCEED,
	SSI_STATUS_UPDATE_FAILED,
	/* empty */
	SSI_STATUS_EMPTY,
	/* FAIL */
	SSI_STATUS_WRITE_FAILURE,
};


/* macros */
#define   SUCCESS_STR  "success"
#define   FAIL_STR     "fail"
#define   INCORRECT_STR "incorrect"

#define   HTTPD_SSI_MAX       128
/* 使用该宏初始化一个ssi对象 */
#define   SSI_OBJ(tag,hdl)    {tag,hdl}


/* declarations */
void httpd_ssi_init(void);


int ajax_get_fill(char * p  , const char * name,void(*call)(char*),const char * val);
char ntoalpha(int n);





#endif

