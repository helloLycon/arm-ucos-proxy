
#include "socketutil.h"


#if 0
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
  Description:     关闭socket
  Calls:           无                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                                      
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
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
  Description:     设置tcp服务器socket选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
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
  Description:     设置tcp客户端请求socket选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
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
  Description:     设置端口重用选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
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
  Description:     设置广播选项
  Calls:           setsockopt();                                                                             
  Input:        	 s32_t sockfd   : socket句柄                                                                                                                                          
  Output:          无                                                                                  
  Return:                                                                                              
                   OK_T           : 执行成功
                   ERROR_T        : 执行失败
  Others:          无                                                                                  
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
status_t setsockopt_keepalive(s32_t sockfd, s32_t min)
{   
    s32_t keepAlive = 1; // 开启keepalive属性
    //s32_t keepIdle = 60*min; // 如该连接在min分钟内没有任何数据往来,则进行探测 

    s32_t keepIdle = 60 *min; 
    s32_t keepInterval = 20; // 探测时发包的时间间隔为20 秒
    s32_t keepCount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
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
