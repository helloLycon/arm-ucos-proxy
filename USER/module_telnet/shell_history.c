#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "shell_history.h"


struct shell_history * sh_hist;


int sh_hist_init(void){
	sh_hist = (struct shell_history*)mymalloc(SRAMIN, sizeof(struct shell_history));
	if(sh_hist){
		memset(sh_hist , 0 , sizeof(struct shell_history));
		return 0;
	}
	printf("ERR: failed to allocate mem for shell history\r\n");
	return -1;
}

void sh_hist_destroy(void){
	myfree(SRAMIN,sh_hist);
}

void sh_hist_reset_current(void){
	sh_hist->current = sh_hist->top;
}

int sh_hist_insert(const char * cmd , int len ){
	int i;
	if(len < SHELL_HIST_CMD_LEN){
		int prv = sh_hist->top;
		char tmp[SHELL_HIST_CMD_LEN];
		memcpy(tmp,cmd,len);
		tmp[len] = 0;
		for(i=strlen(tmp)-1 ; i ;--i){
			if( '\r'==tmp[i] || '\n'==tmp[i])
				tmp[i] = 0;
			else
				break;
		}
		HIST_RING_DEC(prv);
		if(strcmp(tmp , sh_hist->cmd[prv])){
			strcpy(sh_hist->cmd[sh_hist->top] , tmp);
			HIST_RING_PLUS(sh_hist->top);
			//sh_hist->current = sh_hist->top;
			if( sh_hist->top == sh_hist->bottom){
				int tmp = sh_hist->top;
				HIST_RING_PLUS(sh_hist->top);
				sh_hist->bottom = sh_hist->top;
				sh_hist->top = tmp;
			}
		}
	}
	sh_hist_reset_current();
/*	for(i=0;i<SHELL_HIST_NO;++i){
		printf("%d. \"%s\"\r\n" , i,sh_hist->cmd[i]);
	}*/
	return 0;
}

cmd_his_t * sh_hist_search(void * buf,int up){
	int * current = &sh_hist->current;
	if(sh_hist->bottom == sh_hist->top){
		// empty
		//printf("empty\r\n");
		return (cmd_his_t*)buf;
	}
	if(up){
		if(*current == sh_hist->bottom){
			// current EARLIST command
			//printf("hit EARLIST\r\n");
			return sh_hist->cmd+(*current);
		}
		HIST_RING_DEC(*current);
		return sh_hist->cmd + (*current);
	}
	else{
		if(*current == sh_hist->top){
			// current user input
			//printf("hit INPUT\r\n");
			return (cmd_his_t*)buf;
		}
		HIST_RING_PLUS(*current);
		if(*current == sh_hist->top){
			//printf("now INPUT\r\n");
			return NULL;
		}
		return sh_hist->cmd+(*current);
	}
}

