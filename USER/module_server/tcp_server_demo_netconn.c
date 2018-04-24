#include "tcp_server_demo.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"

//TCP客户端任务
#define TCPSERVER_PRIO		10
//任务堆栈大小
#define TCPSERVER_STK_SIZE	300
//任务堆栈
OS_STK TCPSERVER_TASK_STK[TCPSERVER_STK_SIZE];

u8 *tcp_server_sendbuf="我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! 我是TCP服务器，数据已收到 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz!\r\n";	

//tcp服务器任务
void tcp_server_thread(void *arg)
{
	err_t err,recv_err;
	struct netconn *conn, *newconn;
	static ip_addr_t ipaddr;
	static u16_t 			port;
	
	conn = netconn_new(NETCONN_TCP);                 //创建一个TCP链接
	netconn_bind(conn,IP_ADDR_ANY,TCP_SERVER_PORT);  //绑定端口 
	netconn_listen(conn);                            //进入监听模式
    conn->recv_timeout = 10;                         //禁止阻塞线程 等待10ms
	while (1) 
	{
		  err = netconn_accept(conn,&newconn);        //接收连接请求

		  if (err == ERR_OK)    //处理新连接的数据
		  { 
				struct netbuf *recvbuf;

				netconn_getaddr(newconn,&ipaddr,&port,0); //获取远端IP地址和端口号			
			
				while(1)
				{				
					if((recv_err = netconn_recv(newconn,&recvbuf)) == ERR_OK)  	//接收到数据
					{		
						netconn_write(newconn ,tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf),NETCONN_COPY);
						netbuf_delete(recvbuf);
					}
					else if(recv_err == ERR_CLSD)  //关闭连接
					{
						netconn_close(newconn);
						netconn_delete(newconn);
						break;
					}
				}
		  }
	}
}

//创建TCP服务器线程
//返回值:0 TCP服务器创建成功
//		其他 TCP服务器创建失败
INT8U tcp_server_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//关中断
	res = OSTaskCreate(tcp_server_thread,(void*)0,(OS_STK*)&TCPSERVER_TASK_STK[TCPSERVER_STK_SIZE-1],TCPSERVER_PRIO); //创建TCP服务器线程
	OS_EXIT_CRITICAL();		//开中断
	
	return res;
}


