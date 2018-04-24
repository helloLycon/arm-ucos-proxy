#ifndef  __WEBSERVER_NET_CONFIG_H
#define  __WEBSERVER_NET_CONFIG_H





/* declarations */
const char* net_cgi_interface(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
int dev_ip_cgi_handler(void * str_get);
int dev_mask_cgi_handler(void * str_get);
int dev_gateway_cgi_handler(void * str_get);
int mng_ip_cgi_handler(void * str_get);
void ip_ssi_handler(char * pcInsert , int len);
void port_ssi_handler(char * pcInsert , int len);
void mask_ssi_handler(char * pcInsert , int len);
void gateway_ssi_handler(char * pcInsert , int len);
void dev_ip_ssi_handler(char * pcInsert , int len);
void dev_port_ssi_handler(char * pcInsert , int len);
void dev_mask_ssi_handler(char * pcInsert , int len);
void dev_gateway_ssi_handler(char * pcInsert , int len);
void dev_mac_ssi_handler(char * pcInsert , int len);
void mng_ip_ssi_handler(char * pcInsert , int len);
void mng_port_ssi_handler(char * pcInsert , int len);
void device_id_ssi_handler(char * pcInsert , int len);
void boot_version_ssi_handler(char * pcInsert , int len);
void app_version_ssi_handler(char * pcInsert , int len);
int inet_check(void * vp_str);
int inet_check_host_ip(void * vp_str);
void net_status_ssi_handler(char * p , int len);
void net_config_status_ssi_handler(char * p , int len);
void net_read_data_ssi_handler(char * p, int len);


#endif

