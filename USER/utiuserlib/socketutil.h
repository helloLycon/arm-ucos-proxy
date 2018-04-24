/*************************************************  
  File name:         socketutil.h
  Author:            txli
  Version:           1.0.0 
  Created Date:      20121205
  Description:       ���������������
  Others:             
  History:
    1. Date:         20121205       
       Author:       txli  
       Modification: ��ʼ�汾
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

#define TCPCLIENTSOCKETBUFSIZE      8192    /*����ÿ���ͻ��������󻺳����Ĵ�С      */


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
  Description:     ����socket
  Calls:           ��                                                                             
  Input:        	 s32_t socktype : socket����                                                                                                                                                       
  Output:          ��                                                                                  
  Return:                                                                                              
                   s32_t sockfd   : socket���
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/ 
s32_t createsocket(s32_t socktype);
#if 0
/*************************************************                                                     
  Function:        closesocket                                                                        
  Description:     �ر�socket
  Calls:           ��                                                                             
  Input:        	 s32_t sockfd   : socket���                                                                                                                                                      
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/ 
status_t closesocket(s32_t sockfd);

#endif
/*************************************************                                                     
  Function:        bindsocket                                                                        
  Description:     ��socket
  Calls:           ��                                                                             
  Input:        	 s32_t sockfd   : socket���   
                   u8_t ipaddr    : IP ��ַ
                   s32_t port     : �˿�                                                                                                                                                   
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/ 
status_t bindsocket(s32_t sockfd,u8_t ipaddr, s32_t port);

/*************************************************                                                     
  Function:        listensocket                                                                        
  Description:     ����socket
  Calls:           ��                                                                             
  Input:        	 s32_t sockfd   : socket���   
                   s32_t MaxNum   : ͬʱ������                                                                                                                                          
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/ 
status_t listensocket(s32_t sockfd,s32_t MaxNum);

/*************************************************                                                     
  Function:        settcpserversocketoption                                                                        
  Description:     ����tcp������socketѡ��
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket���                                                                                                                                          
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/
status_t settcpserversocketoption(s32_t sockfd);

/*************************************************                                                     
  Function:        settcpclientsocketoption                                                                        
  Description:     ����tcp�ͻ�������socketѡ��
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket���                                                                                                                                          
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/
status_t settcpclientsocketoption(s32_t sockfd);

/*************************************************                                                     
  Function:        setsockopt_reuse                                                                        
  Description:     ���ö˿�����ѡ��
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket���                                                                                                                                          
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/
status_t setsockopt_reuse(s32_t sockfd);

/*************************************************                                                     
  Function:        setsockopt_keepalive                                                                        
  Description:     ����socket keepalive
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket���      
                   s32_t min      : ����                                                                                                                              
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/
status_t setsockopt_keepalive(s32_t sockfd, s32_t min);


//int clientSocket(short send_from_port, short send_to_port, const char *interface_name);
int route_add_host(int type);
status_t setsockopt_broadcast(s32_t sockfd);
#endif



