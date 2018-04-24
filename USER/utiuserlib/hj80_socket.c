#include "hj80_socket.h"


/*create a socket data structure*/
status_t create_socket_tcp_data(hj_socket_data_t ** pp_socket_data)
{
	(*pp_socket_data) = (hj_socket_data_t * ) malloc(sizeof(hj_socket_data_t));
	if((*pp_socket_data)==NULL_T){
		return ERROR_T;
	}
	memset((*pp_socket_data), 0, sizeof(hj_socket_data_t));
	(*pp_socket_data)->socket_fd = -1;
	(*pp_socket_data)->socket_type = ST_TCP;
	return OK_T;
}

/*create a socket data structure*/
status_t create_socket_udp_data(hj_socket_data_t ** pp_socket_data)
{
	(*pp_socket_data) = (hj_socket_data_t * ) malloc(sizeof(hj_socket_data_t));
	if((*pp_socket_data)==NULL_T){
		return ERROR_T;
	}
	memset((*pp_socket_data), 0, sizeof(hj_socket_data_t));
	(*pp_socket_data)->socket_fd = -1;
	(*pp_socket_data)->socket_type = ST_UDP;

	return OK_T;
}
/*create the socket file descriptor*/
status_t init_socket_data(hj_socket_data_t * p_socket_data)
{
	if(p_socket_data->socket_type == ST_TCP)
		p_socket_data->socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	else
		p_socket_data->socket_fd = socket(PF_INET, SOCK_DGRAM, 0);

	return OK_T;
}

/*close the socket file descriptor*/
status_t close_hj_socket(hj_socket_data_t * p_socket_data)
{
	if(p_socket_data->socket_fd > -1){
		shutdown(p_socket_data->socket_fd, SHUT_RDWR);
		close(p_socket_data->socket_fd);
		p_socket_data->socket_fd = -1;
	}

	return OK_T;
}

/*relase the socket data structure memory*/
status_t destroy_socket_data(hj_socket_data_t * p_socket_data)
{
	free(p_socket_data);

	return OK_T;
}

/*start listen*/
status_t listen_hj_socket(hj_socket_data_t * p_socket_data, s32_t num)
{
	if(p_socket_data->socket_type == ST_TCP)
		return listen(p_socket_data->socket_fd, num);
	else
		return OK_T;
}
/*connect to remote host*/
status_t connect_hj_socket(hj_socket_data_t * p_socket_data)
{
	struct sockaddr_in addr;

	if(isdigit(p_socket_data->remote_addr[0])){
		addr.sin_addr.s_addr = inet_addr(p_socket_data->remote_addr);
		addr.sin_family = PF_INET;
		addr.sin_port = htons(p_socket_data->remote_port);
	}
	if(p_socket_data->socket_type == ST_TCP)
		return connect(p_socket_data->socket_fd, (struct sockaddr *)&addr, sizeof(addr));
	else
		return OK_T;
}


/*send data*/
status_t send_hj_socket(hj_socket_data_t * p_socket_data,
						 u8_t * buff, s32_t datalen)
{
	return send(p_socket_data->socket_fd, buff, datalen, 0);
}


/*receive data*/
status_t recv_hj_socket(hj_socket_data_t * p_socket_data,
						 u8_t * buff, s32_t datalen)
{
	return recv(p_socket_data->socket_fd, buff, datalen, 0);
}


