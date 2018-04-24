#ifndef  __WEBSERVER_ETH_H
#define  __WEBSERVER_ETH_H


#define ETH_COLUMN_NO   ARRAY_SIZE(eth_column_table)

int eth_column1_board_name(int ge,char * buf , int lineno,int board,int portno);
int eth_column2_portname(int ge,char * buf , int lineno,int board,int portno);
int eth_column3_port_pos(int ge,char * buf , int lineno,int board,int portno);
int eth_column4_port_type(int ge,char * buf , int lineno,int board,int portno);
int eth_column5_port_comment(int ge,char * buf , int lineno,int board,int portno);
int eth_column6_port_enable(int ge,char * buf , int lineno,int board,int portno);
int eth_column7_link_state(int ge,char * buf , int lineno,int board,int portno);
int eth_column8_auto_nego(int ge,char * buf , int lineno,int board,int portno);
int eth_column9_bandwidth(int ge,char * buf , int lineno,int board,int portno);
int eth_column10_full_duplex(int ge,char * buf , int lineno,int board,int portno);
int eth_column11_flow_ctrl(int ge,char * buf , int lineno,int board,int portno);
int eth_column12_trap_mask(int ge,char * buf , int lineno,int board,int portno);
int eth_column13_cfg_btn(int ge,char * buf , int lineno,int board,int portno);

void eth_ge_line1_ssi_handler(char * pcInsert , int len);
void eth_ge_line2_ssi_handler(char * pcInsert , int len);
void eth_ge_line3_ssi_handler(char * pcInsert , int len);
void eth_ge_line4_ssi_handler(char * pcInsert , int len);
void eth_fe_line1_ssi_handler(char * pcInsert , int len);
void eth_fe_line2_ssi_handler(char * pcInsert , int len);
void eth_fe_line3_ssi_handler(char * pcInsert , int len);
void eth_fe_line4_ssi_handler(char * pcInsert , int len);
void eth_fe_line5_ssi_handler(char * pcInsert , int len);
void eth_fe_line6_ssi_handler(char * pcInsert , int len);

void eth_config_name_ssi_handler(char * pcInsert , int len);
void eth_switch_enable_ssi_handler(char * pcInsert , int len);
void eth_switch_disable_ssi_handler(char * pcInsert , int len);
void eth_force_ssi_handler(char * pcInsert , int len);
void eth_auto_nego_ssi_handler(char * pcInsert , int len);
void eth_10M_ssi_handler(char * pcInsert , int len);
void eth_100M_ssi_handler(char * pcInsert , int len);
void eth_1000M_full_ssi_handler(char * pcInsert , int len);
void eth_duplex_half_ssi_handler(char * pcInsert , int len);
void eth_duplex_full_ssi_handler(char * pcInsert , int len);
void eth_flow_ctrl_ssi_handler(char * pcInsert , int len);
void eth_flow_noctrl_ssi_handler(char * pcInsert , int len);
void eth_trap_nomask_ssi_handler(char * pcInsert , int len);
void eth_trap_mask_ssi_handler(char * pcInsert , int len);
void eth_config_state_ssi_handler(char * pcInsert , int len);


const char* eth_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* eth_config_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void eth_read_data_ssi_handler(char * p , int len);
void eth_config_status_ssi_handler(char * pcInsert , int len);

#endif

