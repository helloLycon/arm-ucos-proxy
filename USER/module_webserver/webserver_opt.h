#ifndef  __WEBSERVER_OPT_H
#define  __WEBSERVER_OPT_H


#define OPT_COLUMN_NO  ARRAY_SIZE(opt_column_table)


int opt_column1_portname(char * buf, int lineno, int board, int portno);
int opt_column2_port_pos(char * buf, int lineno, int board, int portno);
int opt_column3_port_comment(char * buf, int lineno, int board, int portno);
int opt_column4_port_enable(char * buf, int lineno, int board, int portno);
int opt_column5_loop(char * buf, int lineno, int board, int portno);
int opt_column6_trap_mask(char * buf, int lineno, int board, int portno);
int opt_column7_LOF(char * buf, int lineno, int board, int portno);
int opt_column8_NOP(char * buf, int lineno, int board, int portno);
int opt_column9_LOOP(char * buf, int lineno, int board, int portno);
int opt_column10_belonging_grp(char * buf, int lineno, int board, int portno);
int opt_column11_1p1_protect(char * buf, int lineno, int board, int portno);
int opt_column12_cfg_btn(char * buf, int lineno, int board, int portno);

void opt_line1_ssi_handler(char * pcInsert , int len);
void opt_line2_ssi_handler(char * pcInsert , int len);
void opt_line3_ssi_handler(char * pcInsert , int len);
void opt_line4_ssi_handler(char * pcInsert , int len);

void opt_config_name_ssi_handler(char * pcInsert, int len);
void opt_switch_enable_ssi_handler(char * pcInsert, int len);
void opt_switch_disable_ssi_handler(char * pcInsert, int len);
void opt_trap_nomask_ssi_handler(char * pcInsert, int len);
void opt_trap_mask_ssi_handler(char * pcInsert, int len);
void opt_loop_normal_ssi_handler(char * pcInsert, int len);
void opt_loop_circuit_ssi_handler(char * pcInsert, int len);
void opt_loop_device_ssi_handler(char * pcInsert, int len);
void opt_loop_double_ssi_handler(char * pcInsert, int len);
void opt_config_state_ssi_handler(char * pcInsert, int len);

const char* opt_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* opt_config_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void opt_read_data_ssi_handler(char * p , int len);
void opt_config_status_ssi_handler(char * p , int len);

#endif

