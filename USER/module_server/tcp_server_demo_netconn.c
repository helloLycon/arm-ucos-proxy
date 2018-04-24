#include "tcp_server_demo.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"

//TCP�ͻ�������
#define TCPSERVER_PRIO		10
//�����ջ��С
#define TCPSERVER_STK_SIZE	300
//�����ջ
OS_STK TCPSERVER_TASK_STK[TCPSERVER_STK_SIZE];

u8 *tcp_server_sendbuf="����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456 ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456 ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz! ����TCP���������������յ� 01234567890123456789 01234567890123456789 01234567890123456789 01234567890123456789 abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz!\r\n";	

//tcp����������
void tcp_server_thread(void *arg)
{
	err_t err,recv_err;
	struct netconn *conn, *newconn;
	static ip_addr_t ipaddr;
	static u16_t 			port;
	
	conn = netconn_new(NETCONN_TCP);                 //����һ��TCP����
	netconn_bind(conn,IP_ADDR_ANY,TCP_SERVER_PORT);  //�󶨶˿� 
	netconn_listen(conn);                            //�������ģʽ
    conn->recv_timeout = 10;                         //��ֹ�����߳� �ȴ�10ms
	while (1) 
	{
		  err = netconn_accept(conn,&newconn);        //������������

		  if (err == ERR_OK)    //���������ӵ�����
		  { 
				struct netbuf *recvbuf;

				netconn_getaddr(newconn,&ipaddr,&port,0); //��ȡԶ��IP��ַ�Ͷ˿ں�			
			
				while(1)
				{				
					if((recv_err = netconn_recv(newconn,&recvbuf)) == ERR_OK)  	//���յ�����
					{		
						netconn_write(newconn ,tcp_server_sendbuf,strlen((char*)tcp_server_sendbuf),NETCONN_COPY);
						netbuf_delete(recvbuf);
					}
					else if(recv_err == ERR_CLSD)  //�ر�����
					{
						netconn_close(newconn);
						netconn_delete(newconn);
						break;
					}
				}
		  }
	}
}

//����TCP�������߳�
//����ֵ:0 TCP�����������ɹ�
//		���� TCP����������ʧ��
INT8U tcp_server_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//���ж�
	res = OSTaskCreate(tcp_server_thread,(void*)0,(OS_STK*)&TCPSERVER_TASK_STK[TCPSERVER_STK_SIZE-1],TCPSERVER_PRIO); //����TCP�������߳�
	OS_EXIT_CRITICAL();		//���ж�
	
	return res;
}


