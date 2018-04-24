#ifndef HJ_SOCKET_H
#define HJ_SOCKET_H


#include "types.h"
#include "cc.h"
//#include "board.h"
//#include "arch/lpc18xx_43xx_emac.h"
//#include "arch/lpc_arch.h"
#include "arch/sys_arch.h"
#include "lwip/opt.h"
//#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"

#define SP_ERR ((status_t) -1)
#define SP_DATAIN ((status_t) 1)
#define SP_TIMEOUT ((status_t) 0)

#define ST_TCP 0
#define ST_UDP 1

#define SERIAL_WORK_THREAD 1		//表示是串口转发线程
#define HJ_WORK_THREAD  2		//表示是处理hj线程	

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))


typedef struct taghj_socket_data
{
	u8_t local_addr[32];
	status_t local_port;

	status_t socket_type;

	u8_t remote_addr[32];
	status_t remote_port;

	status_t socket_fd;
//	struct pollfd socket_poll_fd;
}hj_socket_data_t;

/*create a socket data structure*/
status_t create_socket_data(hj_socket_data_t ** pp_socket_data);
/*create a socket data structure*/
status_t create_socket_udp_data(hj_socket_data_t ** pp_socket_data);
/*create the socket file descriptor*/
status_t init_socket_data(hj_socket_data_t * p_socket_data);

/*close the socket file descriptor*/
status_t destroy_socket_data(hj_socket_data_t * p_socket_data);

/*relase the socket data structure memory*/
status_t close_hj_socket(hj_socket_data_t * p_socket_data);

/*bind to local address and port*/
status_t bind_hj_socket(hj_socket_data_t * p_socket_data);

/*start listen*/
status_t listen_hj_socket(hj_socket_data_t * p_socket_data, s32_t num);
/*connect to remote host*/
status_t connect_hj_socket(hj_socket_data_t * p_socket_data);

/*accept a incoming connection*/
status_t accept_hj_socket(hj_socket_data_t * p_socket_data,
						   hj_socket_data_t * p_New);


/*send data over udp*/
status_t send_hj_socket(hj_socket_data_t * p_socket_data,
						 u8_t * buff, s32_t datalen);

/*receive data*/
status_t recv_hj_socket(hj_socket_data_t * p_socket_data,
						 u8_t * buff, s32_t datalen);

/*poll the socket status: incoming connect or data*/
status_t poll_hj_socket(hj_socket_data_t * p_socket_data);


#endif/*hj_SOCKET_H*/
