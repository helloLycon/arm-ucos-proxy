#ifndef __CMD_TREE_H
#define __CMD_TREE_H


#ifndef  NULL
#define  NULL ((void*)0)
#endif


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif
#define CT_OBJ(str,nxt)  {str,nxt}

#define CMDTREE(x)  const struct cmd_tree x[]


struct cmd_tree{
	const char * self;
	const struct cmd_tree * next;
};


extern CMDTREE(ct_port);
extern const struct cmd_tree ct_root[];
extern const struct cmd_tree ct_info[];
extern const struct cmd_tree ct_info_abc[];
extern const struct cmd_tree ct_info_abc_abc1[];
extern const struct cmd_tree ct_info_bcd[];

int traverse_cmd_tree(send_t send_method,struct netconn * conn,unsigned char * buf,u16_t * plen,int argc,const char **argv,const struct cmd_tree * lvl_root,int lvl);



#endif

