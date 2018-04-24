/*************************************************  
  File name:         socketutil.h
  Author:            txli
  Version:           1.0.0 
  Created Date:      20121205
  Description:       网络操作基本工具
  Others:             
  History:
    1. Date:         20121205       
       Author:       txli  
       Modification: 起始版本
    2. 
*************************************************/

#ifndef __SOCKETUTIL_H__
#define __SOCKETUTIL_H__

//#include "hj_nm.h"

#include "types.h"
//#include "arch/lpc18xx_43xx_emac.h"
//#include "arch/lpc_arch.h"
//#include "arch/sys_arch.h"
#include "lwip/opt.h"
//#include "lwip/sys.h"
#include "lwip/sockets.h"


#define SOCKET_TCP 0
#define SOCKET_UDP 1

#define DEFAULT_KEEPALIVETIME   10      /*default keep alive time */

#define TCPCLIENTSOCKETBUFSIZE      8192    /*定义每个客户连接请求缓冲区的大小      */


#ifdef LINUX
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#endif


/*************************************************                                                     
  Function:        createsocket                                                                        
  Description:     创建socket
  Calls:           无                                                                             
  Input:        	 s32_t socktype : socket类型                                                                                                                                                       
  Output:          无                                                                                  
  Return:                                                                                              
                   s32_t sockfd   : socket句柄
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/ 
s32_t createsocket(s32_t socktype);
#if 0
/*************************************************                                                     
  Function:        closesocket                                                                        
  Description:     关闭socket
  Calls:           无                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                                      
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/ 
status_t closesocket(s32_t sockfd);

#endif
/*************************************************                                                     
  Function:        bindsocket                                                                        
  Description:     绑定socket
  Calls:           无                                                                             
  Input:        	 s32_t sockfd   : socket句柄   
                   u8_t ipaddr    : IP 地址
                   s32_t port     : 端口                                                                                                                                                   
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/ 
status_t bindsocket(s32_t sockfd,u8_t ipaddr, s32_t port);

/*************************************************                                                     
  Function:        listensocket                                                                        
  Description:     监听socket
  Calls:           无                                                                             
  Input:        	 s32_t sockfd   : socket句柄   
                   s32_t MaxNum   : 同时最大个数                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/ 
status_t listensocket(s32_t sockfd,s32_t MaxNum);

/*************************************************                                                     
  Function:        settcpserversocketoption                                                                        
  Description:     设置tcp服务器socket选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t settcpserversocketoption(s32_t sockfd);

/*************************************************                                                     
  Function:        settcpclientsocketoption                                                                        
  Description:     设置tcp客户端请求socket选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t settcpclientsocketoption(s32_t sockfd);

/*************************************************                                                     
  Function:        setsockopt_reuse                                                                        
  Description:     设置端口重用选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t setsockopt_reuse(s32_t sockfd);

/*************************************************                                                     
  Function:        setsockopt_keepalive                                                                        
  Description:     设置socket keepalive
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄      
                   s32_t min      : 分钟                                                                                                                              
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
*************************************************/
status_t setsockopt_keepalive(s32_t sockfd, s32_t min);


//int clientSocket(short send_from_port, short send_to_port, const char *interface_name);
int route_add_host(int type);
status_t setsockopt_broadcast(s32_t sockfd);
#endif



