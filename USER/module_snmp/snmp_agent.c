#include <stdio.h>
#include <string.h>
#include "types.h"
#include "mainconfig.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/udp.h"
#include "snmp.h"
#include "snmp_agent.h"
#include "snmp_msg.h"
#include "snmp_asn1.h"
#include "webserver_control_comm.h"

/*------ refer to sys_tem_scalar plz ------*/



static struct snmp_base_struct snmp_base = {
	"sysDescr",8,
	"sysName" ,7,
	"sysContact",10,
	"sysLocation",11,
	2,
};

//static struct snmp_devinfo_struct snmp_devinfo;


void __snmp_agent_systime_inc(void){
/*	Exactly every 10 msec the SNMP uptime timestamp must be updated with
	snmp_inc_sysuptime(). You should call this from a timer interrupt
	or a timer signal handler depending on your runtime environment.
	*/
	static int cnter = 0;
	cnter += 100; // which means 10ms
	if( cnter >=  OS_TICKS_PER_SEC){
		snmp_inc_sysuptime();
		cnter = 0;
	}
}

int snmp_agent_init(void){
/*	Before starting the agent you should supply pointers
	to non-volatile memory for sysContact, sysLocation,
	and snmpEnableAuthenTraps. You can do this by calling
	*/
	snmp_set_sysdesr(snmp_base.sysdescr_str , &snmp_base.sysdescr_len);
	snmp_set_syscontact(snmp_base.syscontact_str,&snmp_base.syscontact_len);
	snmp_set_syslocation(snmp_base.syslocation_str,&snmp_base.syslocation_len);
	snmp_set_sysname(snmp_base.sysname_str,&snmp_base.sysname_len);
	snmp_set_snmpenableauthentraps(&snmp_base.snmpauthentraps_set);
	//(if you have a private MIB)
	//snmp_set_sysobjid(); 
	
/*	Also before starting the agent you need to setup
	one or more trap destinations using these calls:
	*/
	//snmp_trap_dst_ip_set(0,&trap_addr);
	//snmp_trap_dst_enable(0,trap_flag);

	/* init snmp and private mib */
	snmp_init();
	return 0;
}



/**
 * Returns systems object definitions.
 *
 * @param ident_len the address length (2)
 * @param ident points to objectname.0 (object id trailer)
 * @param od points to object definition.
 */
void devinfo_get_object_def(u8_t ident_len, s32_t *ident, struct obj_def *od)
{
  u8_t id;

  /* return to object name, adding index depth (1) */
  ident_len += 1;
  ident -= 1;
  if (ident_len == 2)
  {
    od->id_inst_len = ident_len;
    od->id_inst_ptr = ident;

    LWIP_ASSERT("invalid id", (ident[0] >= 0) && (ident[0] <= 0xff));
    id = (u8_t)ident[0];
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("get_object_def system.%"U16_F".0\n",(u16_t)id));
    switch (id)
    {
      case 1: /* devip */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(s32_t);
        break;
      case 2: /* devport */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(s32_t);
        break;
      case 3: /* mask */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(s32_t);
        break;
      case 4: /* gateway */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(s32_t);
        break;
      case 5: /* mac */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = (6*2+5);
        break;
      case 6: /* mng ip */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_APPLIC | SNMP_ASN1_PRIMIT | SNMP_ASN1_IPADDR);
        od->v_len = sizeof(s32_t);
        break;
      case 7: /* mng port */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(s32_t);
        break;
      case 8: /* dev id */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_INTEG);
        od->v_len = sizeof(s32_t);
        break;
      case 9: /* boot version */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = 4;
        break;
      case 10: /* app version */
        od->instance = MIB_OBJECT_SCALAR;
        od->access = MIB_OBJECT_READ_ONLY;
        od->asn_type = (SNMP_ASN1_UNIV | SNMP_ASN1_PRIMIT | SNMP_ASN1_OC_STR);
        od->v_len = 4;
        break;
      default:
        LWIP_DEBUGF(SNMP_MIB_DEBUG,("system_get_object_def: no such object\n"));
        od->instance = MIB_OBJECT_NONE;
        break;
    };
  }
  else
  {
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("system_get_object_def: no scalar\n"));
    od->instance = MIB_OBJECT_NONE;
  }
}

/**
 * Returns system object value.
 *
 * @param ident_len the address length (2)
 * @param ident points to objectname.0 (object id trailer)
 * @param len return value space (in bytes)
 * @param value points to (varbind) space to copy value into.
 */
void devinfo_get_value(struct obj_def *od, u16_t len, void *value)
{
  u8_t id;
  LWIP_ASSERT("invalid id", (od->id_inst_ptr[0] >= 0) && (od->id_inst_ptr[0] <= 0xff));
  id = (u8_t)od->id_inst_ptr[0];
  switch (id)
  {
    char buf[32];
    case 1: /* devip */
      *((s32_t *)value) = htonl(main_config_get_main_host_ip());
	  break;
    case 2: /* devport */
      *((s32_t *)value) = main_config_get_main_host_port();
	  break;
    case 3: /* mask */
      *((s32_t *)value) = htonl(main_config_get_main_host_mask());
      break;
    case 4: /* gateway */
      *((s32_t *)value) = htonl(main_config_get_main_gateway());
	  break;
    case 5: /* mac */
      main_config_read_dev_mac(buf);
      ocstrncpy((u8_t *)value,(u8_t*)buf, (6*2+5));
      break;
    case 6: /* mngip */
      *((s32_t *)value) = htonl(main_config_get_jump_ip());
      break;
    case 7: /* mng port */
      *((s32_t *)value) = main_config_get_jump_port();
      break;
    case 8: /* dev id */
      web_read_device_id();
      *((s32_t *)value) = web_control->dev_id;
      break;
    case 9: /* boot version */
      main_config_read_v1(buf);
      ocstrncpy((u8_t *)value,(u8_t*)buf, 4);
      break;
    case 10: /* app version */
      main_config_read_v2(buf);
      ocstrncpy((u8_t *)value,(u8_t*)buf, 4);
      break;
	default:
      break;
  }
}

/* custom .1.3.6.1.4.1.26382.N.0 */
const mib_scalar_node snmp_system_scalar = {
  &system_get_object_def,
  &system_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_SC,
  0
};


/* ....26382.1.N.0 */
const mib_scalar_node snmp_devinfo_scalar = {
  &devinfo_get_object_def,
  &devinfo_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_SC,
  0
};


/* .....26382.1 */
const s32_t snmp_devinfo_ids[10] = {1,2,3,4,5,6,7,8,9,10};
struct mib_node* const snmp_devinfo_nodes[10] = {
  (struct mib_node*)&snmp_devinfo_scalar, (struct mib_node*)&snmp_devinfo_scalar,
  (struct mib_node*)&snmp_devinfo_scalar, (struct mib_node*)&snmp_devinfo_scalar,
  (struct mib_node*)&snmp_devinfo_scalar, (struct mib_node*)&snmp_devinfo_scalar,
  (struct mib_node*)&snmp_devinfo_scalar, (struct mib_node*)&snmp_devinfo_scalar,
  (struct mib_node*)&snmp_devinfo_scalar, (struct mib_node*)&snmp_devinfo_scalar,
};
const struct mib_array_node snmp_devinfo_arr = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  10,
  snmp_devinfo_ids,
  snmp_devinfo_nodes
};


/* ....26382 */
const s32_t snmp_custom_ids[7] = { 1, 2, 3, 4, 5, 6, 7 };
struct mib_node* const snmp_custom_nodes[7] = {
  (struct mib_node*)&snmp_devinfo_arr,   (struct mib_node*)&snmp_system_scalar,
  (struct mib_node*)&snmp_system_scalar, (struct mib_node*)&snmp_system_scalar,
  (struct mib_node*)&snmp_system_scalar, (struct mib_node*)&snmp_system_scalar,
  (struct mib_node*)&snmp_system_scalar
};
const struct mib_array_node snmp_custom_root = {
  &noleafs_get_object_def,
  &noleafs_get_value,
  &noleafs_set_test,
  &noleafs_set_value,
  MIB_NODE_AR,
  7,
  snmp_custom_ids,
  snmp_custom_nodes
};


