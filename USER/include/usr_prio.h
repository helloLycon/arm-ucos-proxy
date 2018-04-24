#ifndef  __USR_PRIO_H
#define  __USR_PRIO_H

// get "MAXTHREADNUM"
#include "uart3_control.h"

//#define LWIP_DHCP_TASK_PRIO       		  7
//#define USERSCOM3_SEND_TASK_PRIORITY		  8
//#define LED_TASK_PRIO                       9
//#define START_TASK_PRIO                    10
//#define USERSCOM1_TASK_PRIORITY            11
//#define USERFTP_TASK_PRIORITY              12
//#define USER_TELNET_SERVER_TASK_PRIORITY   17

/**
  * ATTENTION: 所有互斥量及线程优先级不可重复
  * ATTENTION: 所有互斥量及线程优先级不可重复
  * ATTENTION: 所有互斥量及线程优先级不可重复
  * ATTENTION: 所有互斥量及线程优先级不可重复
  */

// 用户线程优先级
#define USER_THREAD_PRIO_START             30u
#define LWIP_DHCP_TASK_PRIO                (USER_THREAD_PRIO_START+0)
#define USERSCOM3_SEND_TASK_PRIORITY       (USER_THREAD_PRIO_START+1)
#define LED_TASK_PRIO                      (USER_THREAD_PRIO_START+2)
#define START_TASK_PRIO                    (USER_THREAD_PRIO_START+3)
#define USERSCOM1_TASK_PRIORITY            (USER_THREAD_PRIO_START+4)
#define USERFTP_TASK_PRIORITY              (USER_THREAD_PRIO_START+5)
#define USER_TELNET_SERVER_TASK_PRIORITY   (USER_THREAD_PRIO_START+6)
#define USER_TFTP_CLIENT_TASK_PRIORITY     (USER_THREAD_PRIO_START+7)

// 互斥量优先级
#define ALL_MUTEX_PRIO_START         10u

#define THREAD_MUTEX_PRIO_START      ALL_MUTEX_PRIO_START

#define USER_MUTEX_PRIO_START        (THREAD_MUTEX_PRIO_START+MAXTHREADNUM)
#define TCP_THREAD_MUTEX_PRIO        (USER_MUTEX_PRIO_START+0)
#define MAINCONFIG_MUTEX_PRIO        (USER_MUTEX_PRIO_START+1)
#define SCOM3_THREAD_MUTEX_PRIO      (USER_MUTEX_PRIO_START+2)

#endif

