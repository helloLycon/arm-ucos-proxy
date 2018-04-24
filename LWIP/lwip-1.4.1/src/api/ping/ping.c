/**
 * @file
 * Ping sender module
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 */

/** 
 * This is an example of a "ping" sender (with raw API and socket API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include "lwip/opt.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "lwip/inet.h"
#include "usart.h"
#include "ping.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "lwip/inet_chksum.h"

#if PING_USE_SOCKETS
#include "lwip/sockets.h"
#include "lwip/inet.h"
#endif /* PING_USE_SOCKETS */


/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping target - should be a "ip_addr_t" */
/*
#ifndef PING_TARGET
#define PING_TARGET   (netif_default?netif_default->gw:ip_addr_any)
#endif
*/

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 2000 /* 1000 */
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     2000 /* 1000 */
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

/* ping variables */
static u16_t  ping_seq_num;
static u32_t  ping_time;
static u16_t  ping_sent,ping_recvd;
//static send_t ping_telnet_send_method;
//static struct netconn * ping_telnet_conn;

#if !PING_USE_SOCKETS
static ip_addr_t  ping_target;
static struct raw_pcb * ping_pcb;
OS_EVENT * ping_recv_sem;
#endif /* PING_USE_SOCKETS */

/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  iecho->type   = ICMP_ECHO;
  iecho->code   = 0;
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }
  // why?
  //iecho->chksum = inet_chksum(iecho, len);
}

//#if PING_USE_SOCKETS
#if  0

/* Ping using the socket ip */
static err_t
ping_send(int s, ip_addr_t *addr)
{
  int err;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
  if (!iecho) {
    return ERR_MEM;
  }

  ping_prepare_echo(iecho, (u16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;
  inet_addr_from_ipaddr(&to.sin_addr, addr);

  err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

  mem_free(iecho);

  return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)
{
  char buf[64];
  int fromlen, len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;

  while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
    if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) {
      ip_addr_t fromaddr;
      inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
      LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
      ip_addr_debug_print(PING_DEBUG, &fromaddr);
      LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now() - ping_time)));

      iphdr = (struct ip_hdr *)buf;
      iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
      if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
        /* do some ping result processing */
        PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
        return;
      } else {
        LWIP_DEBUGF( PING_DEBUG, ("ping: drop\n"));
      }
    }
  }

  if (len == 0) {
    LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %"U32_F" ms - timeout\n", (sys_now()-ping_time)));
  }

  /* do some ping result processing */
  PING_RESULT(0);
}

int ping_thread(ip_addr_t host_ip)
{
  int s , i;
  int timeout = PING_RCV_TIMEO;
  ip_addr_t ping_target;

  //LWIP_UNUSED_ARG(arg);

  if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
    return -1;
  }

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  for(i=0 ; i< 4 ; ++i) {
    //ping_target = PING_TARGET;
    ping_target = host_ip;

    if (ping_send(s, &ping_target) == ERR_OK) {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, ("\r\n"));

      ping_time = sys_now();
      ping_recv(s);
    } else {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
    }
	OSTimeDlyHMSM(0,0,2,0);
    //sys_msleep(PING_DELAY);
  }
  return 0;
}

#else /* PING_USE_SOCKETS */
/* Ping using the raw ip */
static u8_t
ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, ip_addr_t *addr)
{
  struct icmp_echo_hdr *iecho;
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(addr);
  LWIP_ASSERT("p != NULL", p != NULL);

  if ((p->tot_len >= (PBUF_IP_HLEN + sizeof(struct icmp_echo_hdr))) &&
      pbuf_header( p, -PBUF_IP_HLEN) == 0) {
    iecho = (struct icmp_echo_hdr *)p->payload;

    if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
//      LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
//      ip_addr_debug_print(PING_DEBUG, addr);
//      LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now()-ping_time)));

//      char tmp_ip_buf[32];
//      u32 time = sys_now()-ping_time;
//      if( time>0 )
//        snprintf(buf,sizeof buf,"Reply from %s: bytes=32 time=%dms\r\n",inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)),time);
//      else
//        snprintf(buf,sizeof buf,"Reply from %s: bytes=32 time<1ms\r\n",inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)));
      /* send message... */
	  //ping_telnet_send_method(buf,ping_telnet_conn);
	  
	  /* do some ping result processing */
      PING_RESULT(1);
      pbuf_free(p);

      //printf("sem post in ping_recv\r\n");
      ping_recvd++;
	  OSSemPost(ping_recv_sem);
      return 1; /* eat the packet */
    }
  }

  /* packet is not completed here! */
  //printf("sem post in ping_recv\r\n");
  //OSSemPost(ping_recv_sem);
  return 0; /* don't eat the packet */
}

static void
ping_send(struct raw_pcb *raw, ip_addr_t *addr)
{
  struct pbuf *p;
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

//  LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
//  ip_addr_debug_print(PING_DEBUG, addr);
//  LWIP_DEBUGF( PING_DEBUG, ("\r\n"));
  LWIP_ASSERT("ping_size <= 0xffff", ping_size <= 0xffff);

  p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
  if (!p) {
  	printf("ERROR: failed to allocate pbuf in ping_send\r\n");
    return;
  }
  if ((p->len == p->tot_len) && (p->next == NULL)) {
    iecho = (struct icmp_echo_hdr *)p->payload;

    ping_prepare_echo(iecho, (u16_t)ping_size);

    raw_sendto(raw, p, addr);
    ping_sent++;
    ping_time = sys_now();
  }
  pbuf_free(p);
}

/*
static void
ping_timeout(void *arg)
{
  struct raw_pcb *pcb = (struct raw_pcb*)arg;
  //ip_addr_t ping_target = PING_TARGET;
  
  LWIP_ASSERT("ping_timeout: no pcb given!", pcb != NULL);

  ping_send(pcb, &ping_target);

  // set timeout handler 
  sys_timeout(PING_DELAY, ping_timeout, pcb);

  printf("sem post in ping_timeout\r\n");
  OSSemPost(ping_recv_sem);
}
*/

static void ping_raw_init(void)
{
  ping_pcb = raw_new(IP_PROTO_ICMP);
  LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);

  /* set receive handler */
  raw_recv(ping_pcb, ping_recv, NULL);
  raw_bind(ping_pcb, IP_ADDR_ANY);
  
  /* set timeout handler */
  //sys_timeout(PING_DELAY, ping_timeout, ping_pcb);
}

static void ping_raw_remove(void){
  raw_remove(ping_pcb);
}

static void ping_send_now(void)
{
  LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);
  ping_send(ping_pcb, &ping_target);
}

int ping_thread(send_t send_method ,struct netconn *conn, ip_addr_t host_ip)
{
  static int first_exec = 1;
  int i , loop;
  char tmp_ip_buf[32],buf[128];

  if( first_exec ){
  	first_exec = 0;
    ping_recv_sem = OSSemCreate(0);
  }

  /* allocate raw pcb */
  ping_raw_init();
  ping_target = host_ip;
  ping_recvd = 0;
  ping_sent  = 0;
//  ping_telnet_send_method = send_method;
//  ping_telnet_conn = conn;
  snprintf(buf,sizeof(buf),"Pinging %s with %d bytes of data:\r\n\r\n",inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)),PING_DATA_SIZE);
  send_method(buf,conn);

  loop = 4;
  for(i=0 ; i<loop ; ++i) {
  	INT8U err;
    ping_send_now();
	OSSemSet(ping_recv_sem,0, &err);
	/* wait recv semaphore for 3 seconds*/
	OSSemPend( ping_recv_sem, OS_TICKS_PER_SEC*3, &err);
	
	/* ping timed out */
	if( OS_ERR_TIMEOUT == err ){
		send_method("Request timed out.\r\n",conn);
	}
	/* ping received a rely */
	else{
		u32 time = sys_now()-ping_time;
		if( time>0 )
			snprintf(buf,sizeof buf,"Reply from %s: bytes=32 time=%dms\r\n",inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)),time);
		else
			snprintf(buf,sizeof buf,"Reply from %s: bytes=32 time<1ms\r\n",inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)));
		send_method(buf,conn);
	}
	if( i<(loop-1) )
		OSTimeDlyHMSM(0,0,2,0);
	else
		OSTimeDlyHMSM(0,0,0,200);
  }
  /* remove raw pcb */
  ping_raw_remove();
  snprintf(buf,sizeof(buf),
    "\r\nPing statistics for %s:\r\n"
    "  Packets: Sent = %d, Received = %d, Lost = %d (%d%% loss)\r\n",
    inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)),
    ping_sent,
    ping_recvd,
    ping_sent-ping_recvd,
    (ping_sent-ping_recvd)*100/ping_sent);
  /* 用回调函数打印不出百分号 */
  if( uart1_send == send_method ){
    printf(
      "\r\nPing statistics for %s:\r\n"
      "  Packets: Sent = %d, Received = %d, Lost = %d (%d%% loss)\r\n",
      inet_ntoa_r(ping_target, tmp_ip_buf, sizeof(tmp_ip_buf)),
      ping_sent,
      ping_recvd,
      ping_sent-ping_recvd,
      (ping_sent-ping_recvd)*100/ping_sent);
    return 0;
  }
  send_method(buf,conn);
//  snprintf(buf,sizeof(buf),"  Packets: Sent = %d, Received = %d, Lost = %d (%d%% loss)\r\n",ping_sent,ping_recvd,ping_sent-ping_recvd,(ping_sent-ping_recvd)*100/ping_sent);
//  send_method(buf,conn);
  return 0;
}

#endif /* PING_USE_SOCKETS */



#endif /* LWIP_RAW */
