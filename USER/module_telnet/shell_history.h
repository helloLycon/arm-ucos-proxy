#ifndef __SHELL_HISTORY_H
#define __SHELL_HISTORY_H


#define SHELL_HIST_CMD_LEN  64u
#define SHELL_HIST_NO       8u

#define HIST_RING_PLUS(x)  do{(x)=((x)+1)&(SHELL_HIST_NO-1);} while(0)
#define HIST_RING_DEC(x)   do{(x)=((x)+SHELL_HIST_NO-1)&(SHELL_HIST_NO-1);} while(0)


typedef char cmd_his_t[SHELL_HIST_CMD_LEN];

struct shell_history {
	cmd_his_t cmd[SHELL_HIST_NO];
	int top , bottom;
	int current;
};


int sh_hist_init(void);
void sh_hist_destroy(void);
int sh_hist_insert(const char * cmd,int len);
cmd_his_t * sh_hist_search(void * ,int);
void sh_hist_reset_current(void);



#endif

