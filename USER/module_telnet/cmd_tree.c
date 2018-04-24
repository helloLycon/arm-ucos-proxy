#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "cmd_tree.h"


/*
	{"help",     "help     ---- show help \r\n", CMD_HELP},
	{"?",        "?        ---- show help \r\n", CMD_HELP},	
	{"reboot",   "reboot   ---- reboot the system\r\n",cmd_reboot},
	{"ifconfig", "ifconfig ---- show ip addr \r\n", cmd_ifconfig},
	{"info"    , "info     ---- show informations of device\r\n" , cmd_info},
	{"free",     "free     ---- memory free\r\n", cmd_mem_free},
	{"debug",    "debug xx ---- set debug level\r\n"\
	             "              0: critical\r\n"\
	             "              1: error\r\n"\
	             "              2: warning\r\n"\
	             "              3: informational\r\n"\
	             "              4: debug\r\n", cmd_debug},
	{"hjip",     "hjip [xx.xx.xx.xx.xx.xx] ---- set ip addr,mac addr,netmask\r\n", cmd_hjip},
	{"ping",     "ping <ip_addr>           ---- ping another host\r\n",cmd_ping},
*/

CMDTREE(ct_root)={
	CT_OBJ("?", NULL),
	CT_OBJ("help", NULL),
	CT_OBJ("reboot", NULL),
	CT_OBJ("ifconfig", NULL),
	CT_OBJ("info", NULL),
	CT_OBJ("free", NULL),
	CT_OBJ("debug", NULL),
	CT_OBJ("hjip", NULL),
	CT_OBJ("ping", NULL),
	CT_OBJ("port", ct_port),
	CT_OBJ("exit", NULL),
	CT_OBJ(NULL, NULL),
};

CMDTREE(ct_port) = {
	CT_OBJ("opt", NULL),
	CT_OBJ("eth", NULL),
	CT_OBJ("e1", NULL),
	CT_OBJ(NULL, NULL),
};

/*
const struct cmd_tree ct_info[]={
	{"abc" , ct_info_abc},
	{"bcd" , ct_info_bcd},
	{NULL},
};

const struct cmd_tree ct_info_abc[]={
	{"abc1" , ct_info_abc_abc1},
	{"abc2" , NULL},
	{"abc3" , NULL},
	{NULL},
};
const struct cmd_tree ct_info_abc_abc1[]={
	{"abc100" , NULL},
	{NULL},
};

const struct cmd_tree ct_info_bcd[]={
	{"abc100" , NULL},
	{NULL},
};
*/

void cmd_tree_print_one_match(
	send_t send_method,
	struct netconn * conn,
	unsigned char * buf,
	u16_t * plen,
	const char **argv,
	const struct cmd_tree * lvl_root,
	int lvl,
	int index)
{
	int i , os = 0;
	char tmp[128],raw[128]={0};
	for(i=0;i<lvl;++i){
		os += sprintf((char*)buf+os,"%s " , argv[i]);
	}
	os += sprintf((char*)buf+os , "%s ",lvl_root[index].self);
	*plen = os;
	// display
	strcpy(raw,"> ");
	memcpy(raw+2,buf,*plen);
	clr_line_insert(tmp, raw);
	send_method(tmp,conn);
}

void cmd_tree_print_all_match(
	send_t send_method,
	struct netconn * conn,
	unsigned char * buf,
	u16_t * plen,
	const struct cmd_tree ** tr,
	int n)
{
	char tmpbuf[256]={0};
	int os = 0,i;
	if(!n){
		// NO MATCH
		char raw[128] = {0};
		--(*plen);
		strcpy(raw,"> ");
		memcpy(raw+2,buf,*plen);
		clr_line_insert(tmpbuf, raw);
		send_method(tmpbuf,conn);
		return;
	}
	os += sprintf(tmpbuf+os , "\r\n");
	for(i=0;i<n;++i){
		os += sprintf(tmpbuf+os,"  %s\r\n" , tr[i]->self);
	}
	os += sprintf(tmpbuf+os , "> ");
/*	for(i=0;i<argc;++i){
		os += sprintf(buf+os , i==(argc-1)?"%s":"%s " , argv[i]);
	}*/
	memcpy(tmpbuf+os , buf,--(*plen));
	send_method(tmpbuf,conn);
}

/*
20171108
使用递归遍历命令树
2种情况{
	完全匹配 => 进入next level
	部分匹配或不匹配 => 显示匹配项
}*/
int traverse_cmd_tree(
	send_t send_method,
	struct netconn * conn,
	unsigned char * buf,
	u16_t * plen,
	int argc,
	const char **argv,
	const struct cmd_tree * lvl_root,
	int lvl)
{
	int i , save_index = 0 , for_one_match_index;
	static const struct cmd_tree * match_save[ARRAY_SIZE(ct_root)];
	if( !lvl_root ){
		//printf("jump out...\r\n");
		//cmd_tree_print_one_match(send_method,conn,buf,plen,argv,lvl_root,lvl,for_one_match_index);
		return -1;
	}
	if(lvl>= argc)
		argv[lvl] = "";
	// traverse one level
	for(i=0; lvl_root[i].self ;++i){
		if( !strncmp(lvl_root[i].self , argv[lvl],strlen(argv[lvl])) ){
			if( !strcmp(lvl_root[i].self,argv[lvl]) ){
				if( lvl==argc-1 ){
					// last argument
					if( ' '==buf[*plen-2] ){
						// tailed with ' '
						return
						traverse_cmd_tree(send_method,conn,buf,plen,argc,argv,lvl_root[i].next,lvl+1);
					}
				}
				else{
					// (not last arg)totally match, goto next level
					return 
					traverse_cmd_tree(send_method,conn,buf,plen,argc,argv,lvl_root[i].next,lvl+1);
				}
			}
			match_save[save_index++] = lvl_root+i;
			for_one_match_index = i;
		}
	}
	// NO match
/*	if(!save_i)
		return 0;*/
	if(1 == save_index){
		// MATCH ONE ONLY
		cmd_tree_print_one_match(send_method,conn,buf,plen,argv,lvl_root,lvl,for_one_match_index);
		return 1;
	}
	/* NO MATCH or MATCH MORE THAN 1 */
	cmd_tree_print_all_match(send_method,conn,buf,plen,match_save,save_index);
	return save_index;
}

