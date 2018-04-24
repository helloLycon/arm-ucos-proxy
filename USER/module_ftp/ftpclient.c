/**
 * DAISY TECHNOLOGY FTP CLIENT 
 *
 * Uses passive mode (control and data connection are locally initiated)
 * Based on the eCos ftp client
 *
 * Author: Iordan Neshev
 * Date:   28-03-2008
 */

/*
//==========================================================================
//
//      ftpclient.c
//
//      A simple FTP client
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Andrew Lunn.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    andrew.lunn@ascom.ch
// Contributors: andrew.lunn@ascom.ch
// Date:         2001-11-4
// Purpose:      
// Description:  
//              
//####DESCRIPTIONEND####
//
//==========================================================================
*/

/*
 * Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#include <stdio.h>	
#include <ctype.h>

//#include "../../apps/cbsockets/cbsockets.h"
#include "ftpclient.h" 
#include "ring_buffer.h"
#include "wgcmd_ftp.h"
#include "usart.h"



volatile u32_t TimeoutFTPC;			// FTP client timeout 

//#if CALLBACK_SOCKETS_ENABLED

/*
Note: functions, which declare, request or create cb_sockets are 
	  responsible to free them! Exceptions from this rule are documented.

to do:

	1. Check examples with invalid username/password/filename/path
	2. Add timeouts in all loops
	4. Check if all return values are correctly interpreted
	5. Match FTPClient's errcodes with the lwip's errcodes (ERR_)
	6. check:	receive_file_RAM, receive_file_MBF, FTP_BADFILENAME
*/

///////////////////////////////////////////////////////////////////////////////
// Global variables
u16_t last_file_size;	// size of the last received file

///////////////////////////////////////////////////////////////////////////////
// Static functions
static int get_line(int sockfd, char *buf, u16_t buf_size);
static int get_reply(int sockfd, char *reply, unsigned int reply_size);
static int send_cmd(int sockfd,char * msgbuf); 
static int command(char *cmd, char *arg, int sockfd, char *msgbuf, u16_t msgbuflen);
static int login(char * username, char *passwd, int sockfd, char *msgbuf, u16_t msgbuflen);
static u16_t GetDynPort(char* msg, char* ip_str, unsigned int len);

///////////////////////////////////////////////////////////////////////////////
// Public functions
int opendatasock(int ctrl_sfd, struct sockaddr_in* ptr_local_ipp_data, struct sockaddr_in* ptr_remote_ipp_data, char *msgbuf, u16_t msgbuflen);
int ftp_open_session(struct sockaddr_in* ptr_local_ipp, struct sockaddr_in *ptr_remote_ipp, char * username, char * passwd, int *ctrl_sockfd, char *msgbuf, unsigned msgbuflen);
int ftp_quit(int sockfd, char *msgbuf, u16_t msgbuflen);

int receive_file_MBF(int data_sockfd, char* flash_addr, int * p_start_address, int * p_checksum);
int send_file(int data_sockfd, char *buf, int buf_size);

int ftp_open_wr(const char* filename, int ctrl_s, int *data_s, char *msgbuf, u16_t buflen,
			    struct sockaddr_in *ptr_local_ipp_data, 
			    struct sockaddr_in *ptr_remote_ipp_data);

int ftp_append(int data_s, char *buf, int len);
int ftp_closefile(int ctrl_s, int data_s);

///////////////////////////////////////////////////////////////////////////////
// Examples







int ftp_put(char * hostname, char * username, char * passwd, 
            char * filename, char * buf, u16_t len);

int ftp_upload(char* src, u16_t len);
static int ftpc_debug_printf(const char * fmt , ...);
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/* Read one line from the server, being careful not to overrun the
   buffer. If we do reach the end of the buffer, discard the rest of the line.
   Note: buf_size must be >=5 in order to hold the return code
*/
static int get_line(int sockfd, char *buf, u16_t buf_size) 
{
	u16_t cnt = 0;
	u16_t lastelement;
	char ch;

	if ((buf == NULL) || (buf_size == 0))
		return -1;	

	lastelement = buf_size - 1;
	buf[lastelement] = '\0';

	while(1) 
	{
		#ifdef FTPC_GETLINE_TIMEOUT_ENABLE
		{
			TimeoutFTPC = FTPC_GETLINE_TIMEOUT;
			while ((cb_recv(sockfd, &ch, 0, CB_RECV_SIZE) == 0))	// check size of input data
			{	// wait some for new data on socket, then read it
				Idle_cbsocks(1);
				if (!TimeoutFTPC)
					return FTP_TIMEDOUT;
			}

       		if (read(sockfd,&ch,1) != 1)	// read 1 char from input
				return FTP_BAD;
		}
		#else

		if (read(sockfd,&ch,1) != 1)		// warning: blocking read!
			return FTP_BAD;

		#endif

		if (ch == '\n')		// if end of line
		{
			if ((buf[cnt-1] == '\r') && (cnt > 1))	// delete CR if any
				buf[cnt-1] = '\0';
		
			buf[cnt] = '\0';
			//FTPC_DEBUG("\n reached exit point of get_line(), returning OK\n");
			return ERR_OK;		/* OK - the one and only ERR_OK exit point from get_line() */
		}
		
		if (cnt < (lastelement)) 
		{
			buf[cnt++] = ch;
		}   // else loop read() eating all the unnecessary chars till the end of line
	}

//	return ERR_OK; // OK
}

///////////////////////////////////////////////////////////////////////////////

/* Read the reply from the server and return the MSByte from the return
   code. This gives us a basic idea if the command failed/worked. The
   reply can be spread over multiple lines. When this happens the line
   will start with a - to indicate there is more

  http://www.wu-ftpd.org/rfc/rfc959.html

    The three digits of the reply each have a special significance.
	The first digit denotes whether the response is good, bad or incomplete.
    An unsophisticated user-process will be able to determine its next 
	action by simply examining this first digit ....

	If (reply == NULL) OR (reply_size < 5), the reply will not be copied in *reply,
	only the MSByte	of the reply string will be returned.

	If you need the full return code, use strtoul(reply); note: strlen(reply) >= 5 bytes
*/

static int get_reply(int sockfd, char *reply, unsigned int reply_size) 
{
	char local_buf[5];	// holds the beginning of the response when *reply is not supplied from upper-layer function

	char *dest;
	u16_t len;
	u8_t more = 0;
	u8_t first_line = 1;
	int ret, code = 0;

	if ( (reply != NULL) && (reply_size >= 5) )	
	{	
		dest = reply;		// if there is higher-level buffer for messages available, then use it
		len = reply_size;
	}
	else
	{
   		dest = local_buf;	// else use the small local buffer in stack (only for result code)
		len = sizeof(local_buf);
	}

	do 
	{
		if ( (ret = get_line(sockfd, dest, len)) != ERR_OK) 
		{
			return(ret);		// should be FTP_BAD which is (-2) or FTP_TIMEDOUT (-8)
		}

		//FTPC_DEBUG("\n FTPServer: [%s]\n", dest);	

		if (first_line) 
		{
			code = strtoul(dest, NULL, 0);
			first_line = 0;
			more = (dest[3] == '-');
		} 
		else 
		{
			if(isdigit(dest[0]) && isdigit(dest[1]) && isdigit(dest[2]) &&  
				(code == (int)strtoul(dest,NULL,0)) && dest[3]==' ') // code of current line == code of first line
			{ more = 0; } 
			else 
			{ more = 1; }
		}
	} while (more);

	return (dest[0] - '0');			// the MSByte of the return code
}

///////////////////////////////////////////////////////////////////////////////
// Send a command to the server 

static int send_cmd(int sockfd, char *msgbuf) 
{
  	int len;
	int slen = strlen(msgbuf);

	//if ((len = cb_send(sockfd,msgbuf,slen, DUMMY)) != slen)
	if ((len = send(sockfd,msgbuf,slen, 0)) != slen)
	{
		if (len < 0) 
		{
			FTPC_ERROR("\n!!! send_cmd(): write error: %d\n", len);
			return FTP_BAD;
		} 
		else
		{
			FTPC_ERROR("\n!!! send_cmd(): write truncated!\n");
			return FTP_TOOBIG;
		}
	}
	return ERR_OK; 
}

int cb_close(int sockfd, int how)
{
	close(sockfd);
	// ignore 'how'
	return 0;
}

#define cb_socket socket
#define cb_connect connect
#define cb_bind bind


//int connect_peer(struct sockaddr_in *ptr_local_ipp, struct sockaddr_in *ptr_remote_ipp)

// Creates a socket for a given local interface and connect through it to remote host
// return: sockfd of the created socket, or <0 when failed
// _ipp means (IP + port)

int connect_peer(struct sockaddr_in *ptr_local_ipp, struct sockaddr_in *ptr_remote_ipp)
{ 
	int sockfd;
	err_t err;
	

	if ((ptr_remote_ipp == NULL) || (ptr_local_ipp == NULL))
		return -1;
	
	sockfd = cb_socket(AF_INET, SOCK_STREAM, 0);			// TCP active connect
	if (sockfd < 0) 
	{
		// to do printf("socket: %s\n",strerror(errno));
		FTPC_DEBUG("\n connect_peer(): failed creating cb_socket! \n");
		return -1;
	}

	err = cb_bind(sockfd, (struct sockaddr *)ptr_local_ipp, sizeof(struct sockaddr));	// 0 -> addrlen, ignored
	//printf("bind err=%x, ptr_local_ipp=%x\n", err, ptr_local_ipp);
	if (err < 0)
	{
	    FTPC_DEBUG("\n could not bind socket #%d to local ip/port\n", sockfd);
		cb_close(sockfd, 0);
		return -1;
	}
	
	err = cb_connect(sockfd, (struct sockaddr *)ptr_remote_ipp, sizeof(struct sockaddr));
	
	if (err < 0)
	{
		FTPC_DEBUG("\n host not found: %s\n", inet_ntoa(ptr_remote_ipp->sin_addr));
		cb_close (sockfd, 0);
		return -1;	
	}

	FTPC_DEBUG("\n connect_peer(): Connected to %s:%d\n",	inet_ntoa(ptr_remote_ipp->sin_addr), ntohs(ptr_remote_ipp->sin_port));

	return (sockfd);	
}


//
///////////////////////////////////////////////////////////////////////////////
// Send a complete command to the server and receive the reply. Pass temp 
// buffer (*msgbuf) for commands and answers. Return the MSB of the reply code.
static int command(char *cmd, char *arg, int sockfd, char *msgbuf, u16_t msgbuflen)
{
	int err, cmd_len;

	if (arg) {
		cmd_len = snprintf(msgbuf, msgbuflen, "%s %s\r\n", cmd, arg); 
	} else {
		cmd_len = snprintf(msgbuf, msgbuflen, "%s\r\n", cmd);
	}
	//  cmd_len=number of characters that would have been written had 'msgbuflen'
	//	been sufficiently large, not counting the terminating null character

	if (cmd_len >= msgbuflen) 
	{
		FTPC_ERROR("\n ftpc error: [%s] command too long, pls increase msgbuflen\n", cmd);
		return FTP_TOOBIG;
	}

	if (cmd_len <= 0)
	{
		FTPC_ERROR("\n ftpc error: conversion error for command [%s]\n", cmd);
		return FTP_BAD;
	}
	
	/* now (0 < cmd_len < bufsize) is TRUE => command string is good */

	if ( (err = send_cmd(sockfd, msgbuf)) != ERR_OK ) 
	{
		FTPC_ERROR("\n!ftpc could not send command: [%s]\n", cmd);
		return(err);	// should be FTP_BAD or FTP_TOOBIG
	}

	//FTPC_DEBUG("\n ftpc command sent: [%s]\n", msgbuf);
	
	return (get_reply(sockfd, msgbuf, msgbuflen));
}

//////////////////////////////////////////////////////////////////////////////
// Perform a login to the server. Pass the username and password and
// put the connection into binary mode. 
// Control conection must be established in advance on sockfd
   
static int login(char *username, char *passwd, int sockfd, char *msgbuf, u16_t msgbuflen)
{
  	int ret;

	ret = command("USER", username, sockfd, msgbuf, msgbuflen);	// returns the first digit of the response code , e.g 331 -> 3 ==User OK, need pass

	if ((msgbuf[0] == '2') && (msgbuf[1] == '3') && (msgbuf[2] == '0')) // if answer is  "230 Logged on" (without password)
		goto _logged_in;			// hardly possible

	if (ret != 3) 
	{
		FTPC_DEBUG("\nFTPC: User %s not accepted\n", username);
		return (FTP_BADUSER);
	}

	ret = command("PASS", passwd, sockfd, msgbuf, msgbuflen);
	
	if (ret != 2)	
	{
    	if (ret < 0)
			return (ret);	// socket or other error
	
		FTPC_DEBUG("\nFTP: Login failed for User %s\n", username);
		return (FTP_BADUSER);
	}

_logged_in:	  
	FTPC_DEBUG("\nFTP: Login sucessfull\n");
	  
	ret = command("TYPE", "I", sockfd, msgbuf, msgbuflen);	// set binary transfer mode
	
	if (ret != 2) 
	{
    	if (ret < 0) 
			return (ret);

		FTPC_ERROR("\nFTP: TYPE command failed!\n");
		return (FTP_BAD);
	}
	
	return (ret);	// 2
}
///////////////////////////////////////////////////////////////////////////////
// Returns the port on which the server waits for data connection or 0 if error occured
// by parsing the response of the PASV command, 
// e.g "227 Entering Passive Mode (192,168,1,18,5,8)"
// If (ip_str != NULL) the IP is returned there as string ("192.168.1.18")
static u16_t GetDynPort(char* msg, char* ip_str, unsigned int len)
{
	//	Note: strtok() will alter the IP/Port string by placing NULLS at 
	//	the delimiter positions. A good habit is to copy the string 
	//	to a temporary string and use the copy in the strtok() call.
	//	ip_str needs 17 bytes!

	unsigned int port = 20;

	char local_buf[27];				// "(000,000,000,000,000,000)"
	char *pToken;
	u8_t i;
	u8_t ip1, ip2, ip3, ip4, p1, p2;
	char delimiter[2];

	//FTPC_DEBUG("\n Extracting server IP and port from msg [%s]\n", msg);

	i = 0;
	ip1 = ip2 = ip3 = ip4 = p1 = p2 = 0;
	
	while (i < len)
	{
		if ((msg[i] == '(') || (msg[i] == '[') || (msg[i] == '<') || (msg[i] == '{'))
		{
			memcpy(local_buf, (msg + i + 1), sizeof(local_buf));	
			// local_buf gets string after the parenthesis (to digit or space)
			break;
		}
		i++;
		if (i >= len) {
			return 0;	// Could not parse string! Invalid server response
		}
	}
	// Question: Does anybody know if the delimiter is '.' or ',' ???
	// find delimiter (colon/point)
	delimiter[0] = ',';
	delimiter[1] = 0;
	
	if(!strchr(local_buf, delimiter[0]))
	{
		delimiter[0] = '.';		// try point
		if(!strchr(local_buf, delimiter[0]))
		{
			FTPC_ERROR("\n Could not find delimiter! \n\n");
			return 0;	// return 0, FTP_BAD is interpreted as valid port number!
		}
	}

	i = 0;
	pToken = (char *)strtok(local_buf, delimiter);
	while (pToken)
	{		
		switch(i++)
		{
			case 0: ip1 = atoi(pToken); break;
			case 1: ip2 = atoi(pToken); break;
			case 2: ip3 = atoi(pToken); break;
			case 3: ip4 = atoi(pToken); break;
			case 4: p1  = atoi(pToken); break;
			case 5: p2  = atoi(pToken); break;
			default:	
				FTPC_DEBUG("\n ERROR! COULD NOT PARSE PASV ANSWER! - [%s]\n", msg);
				return 0;
		}
		pToken = (char *)strtok(NULL, delimiter);
	} 

	port = (p1*256 + p2);

	if (ip_str)									// report IP
		sprintf(ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	//FTPC_DEBUG("\n FTP Server IP: \"%d.%d.%d.%d\"", ip1, ip2, ip3, ip4);
	//FTPC_DEBUG("\n FTP Server port: %d\n", port);

 	return port;
}
///////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
//  Open a data socket. User should be already logged in
//  input: 
//		socket file descriptor of aleady established control connection
//		pointer to existing and initialized sockaddr_in struct for the data connection (local)
//		pointer to existing and initialized sockaddr_in struct for the data connection (remote)
//		message buffer and its length, used for temporary storage of server responses
//	returns:
//		sockfd of data connection if succesfull. Upper-layer app must release sockfd when finished or on error
//		<0 if failed

int opendatasock(int ctrl_sfd, struct sockaddr_in* ptr_local_ipp_data, struct sockaddr_in* ptr_remote_ipp_data, char *msgbuf, u16_t msgbuflen) 
{
	int ret;
	int data_sockfd;
	unsigned int dynport = 22;			// server-side accept port, it's dynamic!
	
    ret = command("PASV", NULL, ctrl_sfd, msgbuf, msgbuflen);		
	// response should be "227 Entering passive mode <192,168,0,18,0,20>"

	if (ret < 0) 
		return (ret);

	if (ret != 2) 
	{
		FTPC_ERROR("\nFTP: PASV command failed!\n");
		return (FTP_BAD);
	} 

	// calculate remote port waiting to be connected
	dynport = GetDynPort(msgbuf, NULL, msgbuflen);	// NULL -> ignore remote IP string
	
	if (dynport == 0)
	{
    	FTPC_ERROR("\n Could not parse PASV answer\n");
		return FTP_BAD;
	}

	ptr_remote_ipp_data->sin_port = htons(dynport);	// update remote port for data connection

	// increase the priority of control connection because if there is no space for PCBs, the new data connection will kill the control's PCB and take it's place
	// to do: cb_setprio(ctrl_sfd, (cb_getprio(ctrl_sfd) + 1) );

	data_sockfd = connect_peer(ptr_local_ipp_data, ptr_remote_ipp_data);

	if( data_sockfd < 0)
		{ FTPC_DEBUG("\n could not establish data connection! \n"); }
	else
		{ FTPC_DEBUG("\n Data connection established \n"); }

	return (data_sockfd);
}


///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ftp_open_session() - connect to remote host, login, create control connection

int ftp_open_session(struct sockaddr_in* ptr_local_ipp, struct sockaddr_in *ptr_remote_ipp, char * username, char * passwd, int *ctrl_sockfd, char *msgbuf, unsigned msgbuflen)
{
	int ctrl_s;				// new control sockets file descriptor
	int ret;
	u8_t retry_cnt = 5;	
	
	ctrl_s = connect_peer(ptr_local_ipp, ptr_remote_ipp);

	if (ctrl_sockfd == NULL)
		return FTP_BAD_ARGUMENT;

	*ctrl_sockfd = ctrl_s;		// pass back the freshly created ctrl socket

	if (ctrl_s < 0) 
	{
		FTPC_DEBUG("\n Could not connect to server (control connection)!\n");
		return (ctrl_s);
	}
	
	/* Read the welcome message from the server */
  	ret = get_reply(ctrl_s, msgbuf, msgbuflen);	// should be 2 (OK), FTP_BAD or FTP_TIMEDOUT

	if (ret != 2) 
	{	// sometimes (due to bugs in socket's API) it can happen that the input buffer
		// contains only the beginning of the message and get_line() is stuck on waiting
		// CR/LF. If timeout handling is enabled, this piece of code solves the problem
		if (ret == FTP_TIMEDOUT)
		{
			if (!strstr(msgbuf, "220"))	// try to recover message
			{
				FTPC_DEBUG("\nFTPC: Welcome message not received (even with timeout)\n");
                cb_close(ctrl_s, 0);
				return FTP_BAD;
			} // else Welcome message "220 Hello..." recovered, continue with log in
		}
		else
		{
			FTPC_DEBUG("\nFTPC: Server refused connection\n");
			cb_close(ctrl_s, 0);
			return FTP_BAD;
		}
	}

	_retry_login:
	ret = login(username, passwd, ctrl_s, msgbuf, msgbuflen);
	
	if (ret != 2)					// "230 Logged on<CR><LF>" expected
	{	
		if (strstr(msgbuf, "331"))	// "331 Password required for 'user'<CR><LF>"
		{
			// This happens when the last part of the Welcome message comes after
			// its timeout expires (FTPC_GETLINE_TIMEOUT).
			// At this point server has received the USER command and expects password
			if (retry_cnt-- > 0)
				goto _retry_login;	
		}
		cb_close(ctrl_s, 0);
        FTPC_DEBUG("\n FTPC: Login failed!");
		return FTP_BAD;
	}

	FTPC_DEBUG("\n FTPC: Login successfull, server expects commands");
	return ERR_OK;
}



///////////////////////////////////////////////////////////////////////////////
/* All done, quit */
int ftp_quit(int sockfd, char *msgbuf, u16_t msgbuflen)
{
	int ret;
	
	ret = command("QUIT", NULL, sockfd, msgbuf, msgbuflen);
    
	if (ret != 2) 
	{
        if (ret < 0) 
		{
			FTPC_DEBUG("\nFTPC: Quit failed, rs < 0!\n");
			return (ret);
		}
        FTPC_DEBUG("\nFTPC: Quit failed!\n");
		return FTP_BAD;
	}
	
	FTPC_DEBUG("FTPC: QUIT command OK.\n");
	return (ERR_OK);
} 


/* Transmit and receive ring buffers */
extern RINGBUFF_T tx_trap_ring;
/* Transmit and receive buffers */
extern uint8_t tx_trap_buff[TX_TRAP_SIZE];

u8_t temp_buff[256];

u8_t ftp_file_buff[512];
u32_t ftp_file_len;

//u32_t testbyte900=0;

#define REMAIN_BYTES  256
#define SPI_FLASH_PAGE_SIZE 256
///////////////////////////////////////////////////////////////////////////////
/* Receive the file into the buffer and close the data socket afterwards */
int receive_file_RAM(int data_sockfd, u8_t type, int * p_start_address, int * p_checksum, UpdateFilehead_S * p_filehead)
{
	int i,nret;
	int finished = 0;
	int total_size = 0;
	u32_t flashAddr, checksum = 0;
    u32_t down_load_count = 0;
    u32_t filelen = 0;
//    UpdateFilehead_S * p_filehead;
	char *bufp = temp_buff;
	int len;
	
	if (data_sockfd < 0) 
	{
		FTPC_DEBUG("\nInvalid datasock_fd!\n");
		return FTP_BAD;   
	}
	do 
	{
		//FTPC_DEBUG("\n remaining %d bytes \n\n", remaining);
		
        //read() loops until data is received or returns 0 if disconnected
		len = read(data_sockfd,bufp, REMAIN_BYTES);
		if (len < 0) 
		{
			return FTP_BAD;   
		}
		if (len == 0) 
		{
		    printf("down_load_count=%x\n", down_load_count);
			//FTPC_DEBUG("\n zaiavili sme pove4e, ot kolkoto ima (iskame %d pove4e)\n", remaining);
			finished = 1;
		} 
		else 
		{
		    //这里需要延迟一段时间可能是因为spi flash写入太慢，而网络传输快。当改成16/32/128/256时经常会导致死机。
		    //printf("len=%x\n", len);
		    //debug_dump(bufp, 1024, "ftpget:");   
		    //debug_dump(bufp, 16, "\nftpget:\r\n");   
		    if(type == DEST_RAM_UPDATE){
        		if(down_load_count == 0){   //当第一次下载时，则读出文件长度，并擦除相应的区块
                    //p_filehead = (UpdateFilehead_S *)bufp;
                    if(len >= sizeof(UpdateFilehead_S)){   //当第一次下载时，读取长度需要超过文件头长度
                        filelen = p_filehead->size+sizeof(UpdateFilehead_S);
            					
                        printf("filelen=%x\n", filelen);
                        
                        //flashAddr = User_spifi_Erase(filelen);            //flashaddr固定为起始地址		//////
                        flashAddr = SPI_Flash_Get_User_Code_Addr();
                        printf("flashAddr=%x\n", flashAddr);
                        *p_start_address = flashAddr;
                        for(i=sizeof(UpdateFilehead_S);i<len;i++){          //校验和只计算真实的文件内容
                            checksum += bufp[i];
                        }
                        memcpy((char *)p_filehead, bufp,sizeof(UpdateFilehead_S));
                    }
                }else{
                    for(i=0;i<len;i++){
                        checksum += bufp[i];
                    }
                    * p_checksum = checksum;
                }
            }else if(type == DEST_RAM_IMPORT){
                if(down_load_count == 0){   //当第一次下载时，则读出文件长度，并擦除相应的区块
                    //filelen = p_filehead->size+sizeof(UpdateFilehead_S);
                    //printf("filelen=%x\n", filelen);
                    //flashAddr = User_spifi_Erase(filelen);          //flashaddr固定为起始地址		//////
                    flashAddr = SPI_Flash_Get_Import_Data_Addr();
                    printf("flashAddr=%x\n", flashAddr);
                    *p_start_address = flashAddr;
                    
                    memcpy((char *)p_filehead, bufp,sizeof(PortFilehead_S));
                }
            }
            down_load_count++;
		    
	        
	        if(RingBuffer_InsertMult(&tx_trap_ring, bufp, len)==0){
	            printf("trap buffer have no space\n");
	        }
	        
	        ftp_file_len = RingBuffer_GetCount(&tx_trap_ring);
	        printf("flashAddr=%x, ftp_file_len=%x\r\n", flashAddr, ftp_file_len);
	          
	        if(ftp_file_len >= SPI_FLASH_PAGE_SIZE){
	            
	            RingBuffer_PopMult(&tx_trap_ring, ftp_file_buff, SPI_FLASH_PAGE_SIZE);
	            
	            nret = SPI_Flash_Write_Page_Nret(ftp_file_buff, flashAddr, SPI_FLASH_PAGE_SIZE);      //一次只写入一页256 bytes 
                
                
                SPI_Flash_Read(temp_buff,flashAddr,SPI_FLASH_PAGE_SIZE);//读出整个扇区的内容
//                debug_dump(temp_buff, 256, "\nread flash:\r\n");  
//                testbyte900 = 0;
//                for(i=0;i<SPI_FLASH_PAGE_SIZE;i++){
//                    if(ftp_file_buff[i]!= temp_buff[i]){
//                        testbyte900++;
//                    }                    
//                }
                
	            flashAddr += nret;
	        }else{
	            printf("ftp_file_len size is small\n");
	        }
			total_size += len;	// now len is > 0
		}
	} while (!finished);
	
	ftp_file_len = RingBuffer_GetCount(&tx_trap_ring);  //最后一帧数据不超过256字节时需要再次写入
	printf("final::ftp_file_len=%x, flashAddr=%x\n", ftp_file_len, flashAddr);
	
	RingBuffer_PopMult(&tx_trap_ring, ftp_file_buff, SPI_FLASH_PAGE_SIZE);
	            
    nret = SPI_Flash_Write_Page_Nret(ftp_file_buff, flashAddr, SPI_FLASH_PAGE_SIZE);      //一次只写入一页256 bytes 
        
    SPI_Flash_Read(temp_buff,flashAddr,ftp_file_len);//读出整个扇区的内容
//    debug_dump(temp_buff, ftp_file_len, "read flash:\n");  

	
	FTPC_DEBUG("\n file (maybe partially) received, size = %d \n", total_size);
	return total_size;          //整个文件的长度
}



///////////////////////////////////////////////////////////////////////////////
/* Send the file to the server and close the (data) connection afterwards */
// podhodiashto e za failove, po-malki ot buf_size.
int send_file(int data_sockfd, char *buf, int buf_size)
{
	int remaining	=	buf_size;
	char *bufp = buf;
	int len;
	int sent = 0;

	if (data_sockfd<0) 
	{
		//FTPC_DEBUG("\ninvalid data_sockfd: %s\n",strerror(errno));
		return FTP_BAD;   
	}

	while (1)	
	{ 
		//len = cb_send(data_sockfd,bufp,remaining, DUMMY);
		len = send(data_sockfd,bufp,remaining, 0);
		if (len < 0) 
		{
			FTPC_ERROR("\nsend_file() error: %d\n", len);
            cb_close (data_sockfd, 0);	// signal EndOfFile
			return FTP_BAD;   
		}

		sent += len;
		if (len == remaining) 
			break;
		
		remaining -= len;
		bufp += len;
	}

	cb_close (data_sockfd, 0);	// signal EndOfFile
	return sent;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// FTP Client file operations
///////////////////////////////////////////////////////////////////////////////
// create a file for writing
// existing files will be overwritten!
// if return value is < 0, caller MUST close the data_sock (QUESTION: Is this right?)
// return 0 if ok
// uses the same hardware interface as the control connection
int ftp_open_wr(const char* filename, int ctrl_s, int *data_s, char *msgbuf, u16_t buflen, 
			    struct sockaddr_in *ptr_local_ipp_data, 
			    struct sockaddr_in *ptr_remote_ipp_data )
{
	int ret;
	int data_sock;

    if(data_s == NULL)
		return FTP_BAD_ARGUMENT;

	/*  We are now logged in and ready to transfer the file.
		Open the data socket ready to receive the file.
		It also builds the PASV command ready to send */

	data_sock = opendatasock(ctrl_s,ptr_local_ipp_data, ptr_remote_ipp_data, msgbuf,buflen);

	if (data_sock < 0) 
		return (data_sock);

	*data_s = data_sock;				// report data socket to the caller func
	
	/* Ask for the file */
	ret = command("STOR",(char*)filename,ctrl_s,msgbuf,buflen);
	if (ret < 0) {
		return (ret);
	}
	
	if (ret != 1) {
		FTPC_DEBUG("\nFTP: STOR failed!\n");
		return (FTP_BADFILENAME);
	}

	return ERR_OK;
}

////////////////////////////////////////////////////////////////////////////////
// append data to an already created file with control and data connections active
// returns number of bytes appended
// Does _NOT_ close any sockets!
int ftp_append(int data_s, char *buf, int len)
{
	char *bufp = buf;
	int bytes = 0;

	if (data_s < 0)	{
		FTPC_DEBUG("\nFTPC: ftp_append(): invalid data socket!\n");
		return FTP_BAD;
	}

	//bytes = cb_send(data_s, bufp, len, DUMMY);
	bytes = send(data_s, bufp, len, 0);
	if (bytes < 0) 
	{
		//FTPC_DEBUG("\nFTPC ftp_append() failed: write: %s\n",strerror(errno));
		return bytes;   
	}

	FTPC_DEBUG("\n ftp_append(): appended %d bytes\n",  bytes);
	
	return bytes;
}

///////////////////////////////////////////////////////////////////////////////
// Close remote file by closing the data connection, used by high-level functions
//
int ftp_closefile(int ctrl_s, int data_s)
{
	cb_close(data_s, 0);		// close data connection to indicate end of file

	if (get_reply(ctrl_s, NULL, 0) != 2) 
	{
		FTPC_DEBUG("\nFTPC: File transfer failed!\n");	// expected 226 Transfer OK
		return (FTP_BAD);
	}
	FTPC_DEBUG("\nTransfer OK, file sent :) \n");

	return ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////
// Examples 


// Set address structure
// Pass NULL ip_addr to set default addr ("0.0.0.0"), 
void SetIPPort(struct sockaddr_in *ip_addr, const char* ip_str, int port)
{
	memset ((char *)ip_addr, 0, sizeof(struct sockaddr_in));
	ip_addr->sin_family = AF_INET;       
	if (ip_str != NULL)
	{
		ip_addr->sin_addr.s_addr = inet_addr(ip_str);//inet_addr("10.12.110.57"); (returns in Network bytre order :))) 
		if (ip_addr->sin_addr.s_addr == INADDR_NONE)
		{
			if (strcmp(ip_str, "255.255.255.255") == 0){
			//	CBSOCK_ERROR("\nASSERT: SetIPPort gets invalid value!\n");
			}
		}
	}
	else 
	{
		ip_addr->sin_addr.s_addr = 0;	// inet_addr("0.0.0.0"); // use default output interface
	}

	ip_addr->sin_port = htons(port);

/*
	FTPC_DEBUG("\n arg ipstring is: [%s], converted: [%s]", ip_str, inet_ntoa(ip_addr->sin_addr)); 
	FTPC_DEBUG("\n addr->sin_addr.s_addr:  %ld, hex: 0x%08X", ip_addr->sin_addr.s_addr, ip_addr->sin_addr.s_addr);
	FTPC_DEBUG("\n addr->sin_port = %d, argument: %d", ntohs(ip_addr->sin_port), port);
*/
}

///////////////////////////////////////////////////////////////////////////////
/* Get a file from an FTP server. 
	"filename" should be
   the full pathname of the file. buf is a pointer to a buffer the
   contents of the file should be placed in and buf_size is the size
   of the buffer.
   ///////////
   Is this right?
	 If the file is bigger than the buffer, buf_size
   bytes will be retrieved and an error code returned. 
   //////////
	 On success the number of
   bytes received is returned. On error a negative value is returned
   indicating the type of error. */
 

int ftp_get(char * hostname, char * username, char * passwd, 
            char * filename, u8_t type, u32_t * p_start_address, int * p_checksum, void * p_filehead)
{
	char msgbuf[FTPC_MSG_BUFF]; 
	int ctrl_s,data_s;
	int ret, bytes;

	struct sockaddr_in local_ipp_ctrl;
	struct sockaddr_in local_ipp_data;
	struct sockaddr_in remote_ipp_ctrl;
	struct sockaddr_in remote_ipp_data;

	SetIPPort(&local_ipp_ctrl, NULL, 0);		// use default local if and random port
	SetIPPort(&local_ipp_data, NULL, 0);		// use default local if and random port
	SetIPPort(&remote_ipp_ctrl, hostname, FTP_SERVER_CTRL_PORT);	// port should be 21
    SetIPPort(&remote_ipp_data, hostname, 0); // port number is known after the PASV command	

	
    ret = ftp_open_session(&local_ipp_ctrl, &remote_ipp_ctrl, username, passwd, &ctrl_s, msgbuf, sizeof(msgbuf));
	if (ret != ERR_OK) {
		cb_close(ctrl_s, 0);
		return (ret);
	}

	/* We are now logged in and ready to transfer the file. Open the data socket 
	ready to receive the file. It also builds the PASV command ready to send */

	data_s = opendatasock(ctrl_s,&local_ipp_data, &remote_ipp_data, msgbuf,sizeof(msgbuf));
	if (data_s < 0) 
	{
		cb_close (ctrl_s, 0);
		return (data_s);
	}
		
	/* Ask for the file */
	ret = command("RETR",filename,ctrl_s,msgbuf,sizeof(msgbuf));
	if (ret < 0) 
	{
		cb_close(ctrl_s, 0);
		cb_close(data_s, 0);
		return (ret);
	}
	
	if (ret != 1)			// expected: 150 Connection accepted<CR><LF>
	{
		FTPC_ERROR("\nFTPC: RETR failed!\n");
		cb_close (data_s, 0);
		cb_close(ctrl_s, 0);
		return (FTP_BADFILENAME);
	}
	

        	if ((bytes = receive_file_RAM(data_s, type, p_start_address, p_checksum, p_filehead)) < 0) // Receive the file into the buffer and close the data socket afterwards
			{
				FTPC_ERROR("FTPC: Receiving file failed (RAM)\n");
				cb_close(data_s, 0);
				cb_close(ctrl_s, 0);
				return (bytes);
			}
	

	// Note: at this point the data connection should be closed (by receive_file_RAM() or receive_file_MBF())

	FTPC_DEBUG("\nRECEIVED FILE SIZE: %d\n", bytes);
	
	if (get_reply(ctrl_s, NULL, 0) != 2)	// expected: "226 Transfer OK\r\n"
	{
		FTPC_ERROR("FTP: Transfer failed!\n");
		cb_close (data_s, 0); 
		cb_close(ctrl_s, 0);
		return (FTP_BAD);
	}

	//FTPC_DEBUG("\nTransfer OK! \n");
		
	ret = ftp_quit(ctrl_s, msgbuf, sizeof(msgbuf));
	if (ret != ERR_OK) 
	{
		cb_close(ctrl_s, 0);
		cb_close(data_s, 0);
		return (ret);
	}
	
	cb_close (data_s, 0);
	cb_close(ctrl_s, 0);
	return bytes;
}

///////////////////////////////////////////////////////////////////////////////
// ftp_put() - connects to server, login, open/create remote file <"filename">,
// send <buf_size> bytes from <buf>, log out, close data and control connections.
// Returns number of bytes stored (if >0)
// or errno if <0

int ftp_put(char * hostname, char * username, char * passwd, 
            char * filename, char * buf, u16_t len)
{
	char msgbuf[81];
	int ctrl_s, data_s;							// control and data sockets
	int ret, bytes;

	struct sockaddr_in local_ipp_ctrl;
	struct sockaddr_in local_ipp_data;
	struct sockaddr_in remote_ipp_ctrl;
	struct sockaddr_in remote_ipp_data;

	SetIPPort(&local_ipp_ctrl, NULL, 0);		// use default local interface and random port
	SetIPPort(&local_ipp_data, NULL, 0);		// use default local if and random port
	SetIPPort(&remote_ipp_ctrl, hostname, 21);
	SetIPPort(&remote_ipp_data, hostname, 0);

	ret = ftp_open_session(&local_ipp_ctrl, &remote_ipp_ctrl, username, passwd, &ctrl_s, msgbuf, sizeof(msgbuf));
	if (ret != ERR_OK) {
		cb_close(ctrl_s, 0);
		return (ret);
	}

	data_s = opendatasock(ctrl_s,&local_ipp_data, &remote_ipp_data, msgbuf,sizeof(msgbuf));
	if (data_s < 0) 
	{
		cb_close(ctrl_s, 0);
		return (data_s);
	}
	
	/* Send STORe command */

	ret = command("STOR", filename, ctrl_s, msgbuf, sizeof(msgbuf));
	if (ret < 0) // 
	{
		cb_close(data_s, 0);
		cb_close(ctrl_s, 0);
		return ret;
	}

	if (ret != 1) 
	{
		FTPC_DEBUG("FTP: STOR failed!\n");
		cb_close(data_s, 0);
		cb_close(ctrl_s, 0);
		return FTP_BADFILENAME;
	}

	if (( bytes = send_file(data_s,buf,len)) < 0) 
	{
		FTPC_DEBUG("FTP: Sending file failed\n");
		cb_close(data_s, 0);
		cb_close(ctrl_s, 0);
		return bytes;
	}


	FTPC_DEBUG("\n NUMBER OF BYTES IN SENT FILE : %d\n",  bytes);
	

	if ((ret = get_reply(ctrl_s, NULL, 0)) != 2)
	{
		FTPC_DEBUG("FTP: Transfer failed, code %d, answer:[%s]\n", ret, msgbuf);
		cb_close(data_s, 0);
		cb_close(ctrl_s, 0);
		return FTP_BAD;
	}
	
	FTPC_DEBUG("\nTransfer OK, file sent :) \n");
 
	ret = ftp_quit(ctrl_s, msgbuf, sizeof(msgbuf));
	if (ret != ERR_OK)
	{
		cb_close(data_s, 0);
		cb_close(ctrl_s, 0);
		return ret;
	}

	cb_close(data_s, 0);
	cb_close(ctrl_s, 0);

	return bytes;		// if OK, ret == bytes sent
}

///////////////////////////////////////////////////////////////////////////////
// Example with ftp_open_session(), ftp_open_wr(), ftp_append()
//				ftp_closefile(), ftp_quit();

int ftp_upload(char* src, u16_t len)
{
	struct sockaddr_in local_ipp_ctrl;		// local interface (_ipp means ip+port)
	struct sockaddr_in local_ipp_data;		// local interface
	struct sockaddr_in	remote_ipp_ctrl;	// remote ip+port
	struct sockaddr_in	remote_ipp_data;

	char msgbuf[81];	// this buffer is used by low-level ftp client funcs to communicate with the ftp server over the control connection
	char databuf[20];
	int ctrl_s, data_s;		// control and data sockets
	int i, ret;

	SetIPPort(&local_ipp_ctrl, NULL, 0);			// use default local interface+random port
	SetIPPort(&local_ipp_data, NULL, 0);			// use default local interface+ port 6000
	SetIPPort(&remote_ipp_ctrl, FTP_SRV_IP, 21);
	SetIPPort(&remote_ipp_data, FTP_SRV_IP, 0);

	ret = ftp_open_session(&local_ipp_ctrl, &remote_ipp_ctrl, FTP_USER, FTP_PASS, &ctrl_s, msgbuf, sizeof(msgbuf));

	if (ret != ERR_OK) {
		cb_close(ctrl_s, 0);
		return (ret);
	}

	// open data tx connection and create remote file "upload.dat", write access
	ret = ftp_open_wr("upload.dat", ctrl_s, &data_s, msgbuf, sizeof(msgbuf), &local_ipp_data, &remote_ipp_data);

	if (ret < 0) {
		FTPC_ERROR("\n ret ftp_open error: %d\n", ret);
		cb_close(ctrl_s, 0);
		if (data_s != NULL)
			cb_close(data_s, 0);
		return (ret);
	}
	
	// on every iteration add one line (contained in databuf) to the file
	for (i = 0; i < 100; i++)
	{
	   	sprintf(databuf, "CURRENT line-> %d;\n", i);
		ret = ftp_append(data_s, databuf, strlen(databuf));
		if (ret < 0)
			break;
	}

	ret = ftp_append(data_s, src, len);		// add external text
	ret = ftp_closefile(ctrl_s, data_s);	// 0 if ok; closes only data connection
	ret = ftp_quit(ctrl_s, msgbuf, sizeof(msgbuf));
	cb_close (ctrl_s, 0);					// should be already closed by server
	//cb_sock_free(ctrl_s);
    //cb_sock_free(data_s);

	return ERR_OK;
}

//#endif // #if CALLBACK_SOCKETS_ENABLED
static int ftpc_debug_printf(const char * fmt , ...){
	extern int cur_debug_level;
	if( DBG_DEBUG <= cur_debug_level){
		int ret;
		va_list ap;
		va_start(ap,fmt);
		ret = vprintf(fmt , ap);
		va_end(ap);
		return ret;
	}
	return 0;
}

