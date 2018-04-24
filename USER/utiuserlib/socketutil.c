
#include "socketutil.h"


#if 0
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
s32_t createsocket(s32_t socktype)
{
    s32_t sockfd;
    sockfd = socket(PF_INET, socktype, 0);
    if(sockfd<0){
        return ERROR_T;
    } 
    return sockfd;
}
#if  0
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
status_t closesocket(s32_t sockfd)
{
    s32_t fd = sockfd;

    if(fd < 0){
        return ERROR_T;
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    fd = -1;
    return OK_T;
}
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
status_t bindsocket(s32_t sockfd,u8_t ipaddr, s32_t port)
{
    struct sockaddr_in addr;

    if(ipaddr == NULL_T){
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_family = PF_INET;
        addr.sin_port = htons(port);
    }

    if(sockfd >= 0)
        return bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    else
        return ERROR_T;    
}

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
status_t listensocket(s32_t sockfd,s32_t MaxNum)
{
    if(sockfd<0){
        return ERROR_T;
    }
    if(listen(sockfd, MaxNum)==ERROR_T){
        return ERROR_T;
    }
    return OK_T;
}
#endif

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
status_t settcpserversocketoption(s32_t sockfd)
{
    s32_t ReuseOn=1;
    s32_t ret;
    s32_t keepalivetime;
    ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void_t *)&ReuseOn,sizeof(ReuseOn));
    if(ret==ERROR_T){
        return ERROR_T;
    }
    keepalivetime = DEFAULT_KEEPALIVETIME;
    ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void_t *)&(keepalivetime),sizeof(keepalivetime));
    if(ret==ERROR_T){
        return ERROR_T;
    }   

    return OK_T;
}

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
status_t settcpclientsocketoption(s32_t sockfd)
{
    s32_t ret;
    s32_t ReuseOn = 1;  
    s32_t one = 1;
    s32_t sockbufsize = TCPCLIENTSOCKETBUFSIZE;

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == ERROR_T) {
        perror("tcpclient.c, fcntl");
    }

    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void_t *) &ReuseOn,sizeof(ReuseOn))) == ERROR_T){        
        return ERROR_T;
    }
    /* large buffers */
    if(setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(void_t *)&sockbufsize,sizeof(sockbufsize))==ERROR_T){
        return ERROR_T;
    }
    if(setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,(void_t *)&one,sizeof(one))==ERROR_T){         
        return ERROR_T;
    }

    return OK_T;
}

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
status_t setsockopt_reuse(s32_t sockfd)
{
    s32_t ret;
    s32_t ReuseOn = 1;      
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void_t *) &ReuseOn,sizeof(ReuseOn))) == ERROR_T){        
        return ERROR_T;
    }
    return OK_T;
}


/*************************************************                                                     
  Function:        setsockopt_broadcast                                                                        
  Description:     ���ù㲥ѡ��
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket���                                                                                                                                          
  Output:          ��                                                                                  
  Return:                                                                                              
                   OK_T           : ִ�гɹ�
                   ERROR_T        : ִ��ʧ��
  Others:          ��                                                                                  
*************************************************/
status_t setsockopt_broadcast(s32_t sockfd)
{
    s32_t ret;
    s32_t BroadcastOn = 1;      
    if ((setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void_t *) &BroadcastOn,sizeof(BroadcastOn))) == ERROR_T){        
        return ERROR_T;
    }
    return OK_T;
}


#define SOL_TCP IPPROTO_TCP
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
status_t setsockopt_keepalive(s32_t sockfd, s32_t min)
{   
    s32_t keepAlive = 1; // ����keepalive����
    //s32_t keepIdle = 60*min; // ���������min������û���κ���������,�����̽�� 

    s32_t keepIdle = 60 *min; 
    s32_t keepInterval = 20; // ̽��ʱ������ʱ����Ϊ20 ��
    s32_t keepCount = 3; // ̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.
    s32_t nret;
    nret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    //printf("SO_KEEPALIVE nret=%x\n",nret);
    setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    //printf("TCP_KEEPIDLE nret=%x\n",nret);
    setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    //printf("TCP_KEEPINTVL nret=%x\n",nret);
    setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
    //printf("TCP_KEEPCNT nret=%x\n",nret);
    return OK_T;
}




#if 0

int clientSocket(short send_from_port, short send_to_port, const char *interface_name)
{
	int n = 1;
	int client_socket;
	struct sockaddr_in client;

	client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(client_socket == -1){
		printf("clientSocket client_socket error\n");	
		return -1;
	}

	if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1){
		printf("clientSocket setsockopt error\n");	
		return -1;
	}

	setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n));

	bzero(&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(send_from_port);
	client.sin_addr.s_addr = INADDR_ANY;

	if(bind(client_socket,(struct sockaddr *)&client, sizeof(struct sockaddr))==-1){
		printf("clientSocket bind error\n");	
		return -1;
	}

	bzero(&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons(send_to_port);
	client.sin_addr.s_addr = INADDR_BROADCAST;

	if(connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1){
		printf("clientSocket connect error\n");	
		return -1;
	}
	//DEBUG(NM_DEBUG_LEVEL_INFO,"broadcast process\n");
	return client_socket;
}

#define INTERFACE_NAME  "eth0"
#define ADD			1
#define DEL			2

int route_add_host(int type)
{
	pid_t pid;
	char *argv[16];
	int s, argc = 0;

	/* route add -host 255.255.255.255 ethX */
	if((pid = fork()) == 0) { /* child */
		argv[argc++] = "/bin/route";
		if(type == ADD)
			argv[argc++] = "add";
		else if(type == DEL)
			argv[argc++] = "del";
		argv[argc++] = "-host";
		argv[argc++] = "255.255.255.255";
		argv[argc++] = INTERFACE_NAME;
		argv[argc] = NULL;
		execvp("/bin/route", argv);
		exit(0);
	} else if (pid > 0) {
		waitpid(pid, &s, 0);
	}
	return 0;
}
#endif
