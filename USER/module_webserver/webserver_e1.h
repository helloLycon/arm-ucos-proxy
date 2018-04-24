#ifndef  __WEBSERVER_E1_H
#define  __WEBSERVER_E1_H


#define E1_COLUMN_NO   ARRAY_SIZE(e1_column_table)

int e1_column1_portname(char * buf , int lineno,int board,int portno);
int e1_column2_port_pos(char * buf , int lineno,int board,int portno);
int e1_column3_port_comment(char * buf , int lineno,int board,int portno);
int e1_column4_port_enable(char * buf , int lineno,int board,int portno);
int e1_column5_loop(char * buf , int lineno,int board,int portno);
int e1_column6_trap_mask(char * buf , int lineno,int board,int portno);
int e1_column7_LOS(char * buf , int lineno,int board,int portno);
int e1_column8_AIS(char * buf , int lineno,int board,int portno);
int e1_column9_LOOP(char * buf , int lineno,int board,int portno);
int e1_column10_LOF(char * buf , int lineno,int board,int portno);
int e1_column11_error(char * buf , int lineno,int board,int portno);
int e1_column12_alarm(char * buf , int lineno,int board,int portno);
int e1_column13_CAS(char * buf , int lineno,int board,int portno);
int e1_column14_MLOF(char * buf , int lineno,int board,int portno);
int e1_column15_cfg_btn(char * buf , int lineno,int board,int portno);

void e1_line1_ssi_handler(char * pcInsert , int len);
void e1_line2_ssi_handler(char * pcInsert , int len);
void e1_line3_ssi_handler(char * pcInsert , int len);
void e1_line4_ssi_handler(char * pcInsert , int len);
void e1_line5_ssi_handler(char * pcInsert , int len);
void e1_line6_ssi_handler(char * pcInsert , int len);
void e1_line7_ssi_handler(char * pcInsert , int len);
void e1_line8_ssi_handler(char * pcInsert , int len);
void e1_line9_ssi_handler(char * pcInsert , int len);
void e1_line10_ssi_handler(char * pcInsert , int len);
void e1_line11_ssi_handler(char * pcInsert , int len);
void e1_line12_ssi_handler(char * pcInsert , int len);
void e1_line13_ssi_handler(char * pcInsert , int len);
void e1_line14_ssi_handler(char * pcInsert , int len);
void e1_line15_ssi_handler(char * pcInsert , int len);
void e1_line16_ssi_handler(char * pcInsert , int len);

void e1_config_name_ssi_handler(char * pcInsert , int len);
void e1_switch_enable_ssi_handler(char * pcInsert , int len);
void e1_switch_disable_ssi_handler(char * pcInsert , int len);
void e1_trap_nomask_ssi_handler(char * pcInsert , int len);
void e1_trap_mask_ssi_handler(char * pcInsert , int len);
void e1_loop_normal_ssi_handler(char * pcInsert , int len);
void e1_loop_circuit_ssi_handler(char * pcInsert , int len);
void e1_loop_device_ssi_handler(char * pcInsert , int len);
void e1_config_state_ssi_handler(char * pcInsert , int len);
const char* e1_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* e1_config_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void e1_read_data_ssi_handler(char * p , int len);
void e1_config_status_ssi_handler(char * pcInsert , int len);


#endif

