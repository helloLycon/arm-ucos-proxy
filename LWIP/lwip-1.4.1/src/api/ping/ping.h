#ifndef __PING_H__
#define __PING_H__

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    0u /* LWIP_SOCKET */
#endif

/*
struct icmp_echo_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
  u16_t id;
  u16_t seqno;
};
*/

//void ping_init(void);
int ping_thread(send_t send_method ,struct netconn *conn, ip_addr_t host_ip);


#if !PING_USE_SOCKETS
//void ping_raw_init(void);
//void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#endif /* __PING_H__ */
