#ifndef  __WEBSERVER_BOARDS_MNG_H
#define  __WEBSERVER_BOARDS_MNG_H


#define BOARD_COLUMN_NO  ARRAY_SIZE(board_column_table)

int board_column1_slot(char * p , int lineno);
int board_column2_type(char * p,int lineno);
int board_column3_business_type(char * p, int lineno);
int board_column4_pcb_version(char * p , int lineno);
int board_column5_sw_version(char * p , int lineno);
int board_column6_fpga_version(char * p , int lineno);
int board_column7_boot_version(char * p , int lineno);
int board_column8_state(char * p,int lineno);

void board_line1_ssi_handler(char * p,int len);
void board_line2_ssi_handler(char * p,int len);
void board_line3_ssi_handler(char * p,int len);
void board_line4_ssi_handler(char * p,int len);
void board_line5_ssi_handler(char * p,int len);
void board_line6_ssi_handler(char * p,int len);
void board_line7_ssi_handler(char * p,int len);
void board_line8_ssi_handler(char * p,int len);
void board_line9_ssi_handler(char * p,int len);
void board_line10_ssi_handler(char * p,int len);

#endif

