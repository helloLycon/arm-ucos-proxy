#ifndef  __SNMP_AGENT_H
#define  __SNMP_AGENT_H


#include "snmp_structs.h"


#define SNMP_HJ_ENTERPRISE_ID  26382


struct snmp_base_struct{
	u8_t sysdescr_str[16];
	u8_t sysdescr_len;
	u8_t sysname_str[16];
	u8_t sysname_len;
	u8_t syscontact_str[16];
	u8_t syscontact_len;
	u8_t syslocation_str[16];
	u8_t syslocation_len;
	/* enable == 1, disable == 2 */
	u8_t snmpauthentraps_set;
};


struct snmp_devinfo_struct{
	s32_t dev_ip;
	s32_t dev_port;
	s32_t mask;
	s32_t dev_gateway;
	char mac[16];
	s32_t mng_ip;
	s32_t mng_port;
	s32_t dev_id;
	s32_t boot_ver;
	s32_t app_ver;
};








void ocstrncpy(u8_t *dst, u8_t *src, u16_t n);
void system_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od);
void system_get_value(struct obj_def *od, u16_t len, void *value);
int snmp_agent_init(void);
extern const struct mib_array_node snmp_custom_root;





#endif

