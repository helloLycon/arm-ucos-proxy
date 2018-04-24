/* Copyright (c) 2001, Swedish Institute of Computer Science.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  * 3. Neither the name of the Institute nor the names of its contributors
  *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * httpd.c
 *
 * Author : Adam Dunkels <adam@sics.se>
 *
 */


#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "webserver_iap.h"


static vu32 DataFlag=0;
static vu32 size =0;
static vu32 FlashWriteAddress;
static vu32 TotalReceived=0;
static char LeftBytesTab[4];
static vu8  LeftBytes=0;
static vu8  resetpage=0;
static vu32 ContentLengthOffset =0 , BrowserFlag=0;
static vu32 TotalData=0 ;
static int  recv_cnt = 0;


/* ATTENTION: must fit "struct http_state" in httpd.c */
struct http_state
{
  void * reserved;
  const char * file;
  u32_t  left;
};


static const char http_crnl_2[4] = 
/* "\r\n--" */
{0xd, 0xa,0x2d,0x2d};
static const char octet_stream[14] = 
/* "octet-stream" */
{0x6f, 0x63, 0x74, 0x65, 0x74, 0x2d, 0x73, 0x74, 0x72, 0x65, 0x61, 0x6d,0x0d, };
static const char Content_Length[17] = 
/* Content Length */
{0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x4c, 0x65, 0x6e, 0x67,0x74, 0x68, 0x3a, 0x20, };





static void clear_transmission(void){
	iap_dbg("clear transmission...\r\n");
	DataFlag = 0;
	size = 0;
	TotalReceived = 0;
	LeftBytes = 0;
	resetpage = 0;
	ContentLengthOffset = 0;
	BrowserFlag = 0;
	TotalData = 0 ;
	recv_cnt = 0;
}

/**
  * @brief  Extract the Content_Length data from HTML data  
  * @param  data : pointer on receive packet buffer 
  * @param  len  : buffer length  
  * @retval size : Content_length in numeric format
  */
static uint32_t Parse_Content_Length(const char *data, uint32_t len)
{
  uint32_t i=0,size=0, S=1;
  int32_t j=0;
  char sizestring[6];
  const char *ptr;
   
  ContentLengthOffset =0;
  
  /* find Content-Length data in packet buffer */
  for (i=0;i<len;i++)
  {
    if (strncmp (data+i, Content_Length, 16)==0)
    {
      ContentLengthOffset = i+16;
      break;
    }
  }
  /* read Content-Length value */
  if (ContentLengthOffset)
  {
    i=0;
    ptr = data + ContentLengthOffset;
    while(*(ptr+i)!=0x0d)
    {
      sizestring[i] = *(ptr+i);
      i++;
      ContentLengthOffset++; 
    }
    if (i>0)
    {
      /* transform string data into numeric format */
      for(j=i-1;j>=0;j--)
      {
        size += (sizestring[j]-0x30)*S;
        S=S*10;
      }
    }
  }
  return size;
}

/**
  * @brief  writes received data in flash    
  * @param  ptr: data pointer
  * @param  len: data length
  * @retval none 
  */
static void IAP_HTTP_writedata(char * ptr, uint32_t len)            
{
  uint32_t count, i=0, j=0;
  
  /* check if any left bytes from previous packet transfer*/
  /* if it is the case do a concat with new data to create a 32-bit word */
  if (LeftBytes)
  {
    while(LeftBytes<=3)
    {
      if(len>(j+1))
      {
        LeftBytesTab[LeftBytes++] = *(ptr+j);
      }
      else
      {
        LeftBytesTab[LeftBytes++] = 0xFF;
      }
      j++;
    }
    //FLASH_If_Write(&FlashWriteAddress, (u32*)(LeftBytesTab),1);
    LeftBytes =0;
    
    /* update data pointer */
    ptr = (char*)(ptr+j);
    len = len -j;
  }
  
  /* write received bytes into flash */
  count = len/4;
  
  /* check if remaining bytes < 4 */
  i= len%4;
  if (i>0)
  {
    if (TotalReceived != size)
    {
      /* store bytes in LeftBytesTab */
      LeftBytes=0;
      for(;i>0;i--)
      LeftBytesTab[LeftBytes++] = *(char*)(ptr+ len-i);  
    }
    else count++;
  }
  //FLASH_If_Write(&FlashWriteAddress, (u32*)ptr ,count);
}

/* file must be allocated by caller and will be filled in
   by the function. */
//static int fs_open(char *name, struct fs_file *file);
static int iap_fs_open(char *name, struct fs_file *file);

/**
  * @brief  callback function for handling connection errors
  * @param  arg: pointer to an argument to be passed to callback function
  * @param  err: LwIP error code   
  * @retval none
  */
/*
static void conn_err(void *arg, err_t err)
{
  struct http_state *hs;

  hs = arg;
  mem_free(hs);
}
*/

/**
  * @brief  closes tcp connection
  * @param  pcb: pointer to a tcp_pcb struct
  * @param  hs: pointer to a http_state struct
  * @retval
  */
static void close_conn(struct tcp_pcb *pcb, struct http_state *hs)
{
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  mem_free(hs);
  tcp_close(pcb);
}

/**
  * @brief sends data found in  member "file" of a http_state struct
  * @param pcb: pointer to a tcp_pcb struct
  * @param hs: pointer to a http_state struct
  * @retval none
  */
static void send_data(struct tcp_pcb *pcb, struct http_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space available in the send
     buffer */
  if (tcp_sndbuf(pcb) < hs->left)
  {
    len = tcp_sndbuf(pcb);
  }
  else
  {
    len = hs->left;
  }
  err = tcp_write(pcb, hs->file, len, 0);
  if (err == ERR_OK)
  {
    hs->file += len;
    hs->left -= len;
  }
}

/**
  * @brief tcp poll callback function
  * @param arg: pointer to an argument to be passed to callback function
  * @param pcb: pointer on tcp_pcb structure
  * @retval err_t
  */
/*
static err_t http_poll(void *arg, struct tcp_pcb *pcb)
{
  if (arg == NULL)
  {
    tcp_close(pcb);
  }
  else
  {
    send_data(pcb, (struct http_state *)arg);
  }
  return ERR_OK;
}
*/

/**
  * @brief callback function called after a successfull TCP data packet transmission  
  * @param arg: pointer to an argument to be passed to callback function
  * @param pcb: pointer on tcp_pcb structure
  * @param len
  * @retval err : LwIP error code
  */
static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_state *hs;

  hs = arg;

  if (hs->left > 0)
  {
    send_data(pcb, hs);
  }
  else
  {
    close_conn(pcb, hs);
    if(resetpage ==1)
    { 
      /* Generate a software reset */
      NVIC_SystemReset();
      while(1);
    }
      
  }
  return ERR_OK;
}


/**
  * @brief callback function for handling TCP HTTP traffic
  * @param arg: pointer to an argument structure to be passed to callback function
  * @param pcb: pointer to a tcp_pcb structure
  * @param p: pointer to a packet buffer
  * @param err: LwIP error code
  * @retval err
  */

static void webserver_iap_debug(const char* ptr, int len){
	iap_dbg("handle data: %d bytes\r\n" , len);
}

/*
static err_t http_recv(void *arg, struct tcp_pcb *pcb,  struct pbuf *p, err_t err)
*/
err_t webserver_iap_http_recv(void *arg, struct tcp_pcb *pcb,  struct pbuf *p, err_t err)
{
	u16_t i, len, not_found;
	uint32_t DataOffset=0 , FilenameOffset=0;
	const char * data;
	const char * ptr;
	char filename[64]; 
	struct fs_file file = {0};
	struct http_state * hs;
	
	data = p->payload;
	len  = p->tot_len;
#if  1
			iap_dbg("\r\n");
			for(i=0;i<p->tot_len;i+=16){
				int j;
				iap_dbg("%04x   ",i);
				for(j=0;j<16;++j){
					if( (i+j) < p->tot_len )
						iap_dbg("%02x ",data[i+j]);
				}
				iap_dbg("\r\n");
			}
#endif

	/* process POST request for file upload and incoming data packets after POST request*/
	if ((strncmp(data, "POST /upload.cgi",16)==0)||(DataFlag >=1))
	{ 
		if(strncmp(data, "POST /upload.cgi",16)==0){
			iap_dbg("match \"POST /upload.cgi\"...\r\n");
		}
		DataOffset = 0;

		/* 接收到新的POST packet */
		if (DataFlag == 0)
		{ 
			/* reset transmission */
			clear_transmission();

			/* parse packet for "Content-length" field */
			size = Parse_Content_Length(data, p->tot_len);
			iap_dbg("Content-Length=%d\r\n" , size);

			/* parse packet for the "octet-stream" field */
			for (i=0;i<len;i++) {
				if (strncmp ((char*)(data+i), octet_stream, 13)==0)
				{
					iap_dbg("match \"octet-stream\" in POST packet\r\n");
					DataOffset = i+16;
					break;
				}
			}
			/** 
			  * case of MSIE8 : we do not receive data in the POST packet 
			  * THE CASE MOSTLY
			  */
			if (DataOffset == 0)
			{
				iap_dbg("no \"octet_stream\" in POST packet\r\n");
				DataFlag++;
				BrowserFlag = 1;
				pbuf_free(p);
				/* POST packet 处理结束 */
				return ERR_OK;
			}
			/* case of Mozilla Firefox v3.6 : we receive data in the POST packet*/
			else {
				TotalReceived = len - (ContentLengthOffset + 4);
			}
		}

		/* 正在接收... */
		/**
		  * DataFlag==0 && BrowserFlag==0 : data in the POST packet
		  * DataFlag==1 && BrowserFlag==1 : data in the next packet after POST packet
		  */
		if (((DataFlag ==1)&&(BrowserFlag==1)) || ((DataFlag ==0)&&(BrowserFlag==0)))
		//if ( (DataFlag ==1) && (BrowserFlag==1) )
		{ 
			if ((DataFlag ==0)&&(BrowserFlag==0)) {
				DataFlag++;
			}
			else if ((DataFlag ==1)&&(BrowserFlag==1)) {
				not_found = 1;
				/* parse packet for the octet-stream field */
				for (i=0;i<len;i++) {
					if (strncmp ((char*)(data+i), octet_stream, 13)==0){
						not_found = 0;
						iap_dbg("match \"octet-stream\" in 2nd packet\r\n");
						DataOffset = i+16;
						iap_dbg("%d bytes before(besides) \"octet_stream\\r\\n\\r\\n\"\r\n",DataOffset);
						break;
					}
				}
//				if( not_found ) {
//					iap_dbg("no \"octet_stream\" in 2nd packet!\r\n");
//					iap_fs_open("/upload.html", &file);
//					hs->file = file.data;
//					hs->left = file.len;
//					pbuf_free(p);
//					send_data(pcb, hs);
//					/* Tell TCP that we wish be to informed of data that has been
//					successfully sent by a call to the http_sent() function. */
//					tcp_sent(pcb, http_sent); 
//					
//					/* reset transmission */
//					clear_transmission();
//					return ERR_OK;
//				}
				/** 
				  * TotalReceived从第二个包开始计算
				  * 最后与Content-Length比较
				  */
				TotalReceived += len;
				iap_dbg("TotalReceived = %lu...(%dth start from 2nd pkt)\r\n",TotalReceived,++recv_cnt);
				DataFlag++;  // equals 2 here
			}
			
			/**** parse packet for the filename field ****/
			FilenameOffset = 0;
			for (i=0;i<len;i++)
			{
				if (strncmp ((char*)(data+i), "filename=", 9)==0)
				{
					iap_dbg("match \"filename=\"\r\n");
					/* FilenameOffset指向第一个"的后一个位置 */
					FilenameOffset = i + 10;
					break;
				}
			}           
			i = 0;
			if (FilenameOffset)
			{
				while((*(data+FilenameOffset + i)!=0x22 )&&(i<sizeof(filename)))
				{
					filename[i] = *(data+FilenameOffset + i);
					i++;
				}
				filename[i] = 0x0;
			}
			/* no filename, in this case reload upload page */
			if (i==0)
			{
				iap_dbg("no filename!\r\n");
				iap_fs_open("/upload.html", &file);
				hs->file = file.data;
				hs->left = file.len;
				pbuf_free(p);
				send_data(pcb, hs);
				/* Tell TCP that we wish be to informed of data that has been
				successfully sent by a call to the http_sent() function. */
				tcp_sent(pcb, http_sent); 
				
				/* reset transmission */
				clear_transmission();
				return ERR_OK;
			}
			iap_dbg("filename=\"%s\"\r\n",filename);
			/**** end parsing packet for the filename field ****/

			/* 准备开始接收文件数据 */
			TotalData = 0 ;
			
//			/* init flash */
//			FLASH_If_Init();
//			/* erase user flash area */
//			FLASH_If_Erase(USER_FLASH_FIRST_PAGE_ADDRESS);
//			FlashWriteAddress = USER_FLASH_FIRST_PAGE_ADDRESS;
		}
		/** 
		  * DataFlag >=2 :
		  * the packet is data only 
		  */
		else {
			TotalReceived += len;
			iap_dbg("TotalReceived = %lu...(%dth start from 2nd pkt)\r\n",TotalReceived,++recv_cnt);
		}

		ptr  = data + DataOffset;
		len -= DataOffset;

		/* 数据包大小累计 */
		TotalData += len;
		iap_dbg("TotalData = %lu...\r\n" , TotalData);

		/* 是否为最后一个数据包 */
		//if (TotalReceived == size)
		if (TotalReceived >= size)
		{
			not_found = 1;
			if( TotalReceived == size)
				iap_dbg("MESSAGE: TotalReceived == Content-Length\r\n");
			else
				iap_dbg("WARNING: TotalReceived(%lu) > Content-Length(%lu) !\r\n",TotalReceived,size);
#if  0
			iap_dbg("\r\n");
			for(i=0;i<p->tot_len;i+=16){
				int j;
				iap_dbg("%04x   ",i);
				for(j=0;j<16;++j){
					if( (i+j) < p->tot_len )
						iap_dbg("%02x ",data[i+j]);
				}
				iap_dbg("\r\n");
//				for(j=0;j<16;++j){
//					if( (i+j) < p->tot_len )
//						iap_dbg("%c",data[i+j]);
//				}
//				iap_dbg("\r\n");
			}
#endif
			/* if last packet need to remove the http boundary tag */
			/* parse packet for "\r\n--" starting from end of data */
			for( i=4 ; i <= p->tot_len ; ++i){
				if(0 == strncmp(data+p->tot_len-i , http_crnl_2 , 4)){
					iap_dbg("%d bytes after(besides) http_crnl_2\r\n",i);
					not_found = 0;
					break;
				}
			}
			if( not_found ){
				iap_dbg("no http_crnl_2 string in last packet!\r\n");
				iap_fs_open("/upload.html", &file);
				hs->file = file.data;
				hs->left = file.len;
				pbuf_free(p);
				send_data(pcb, hs);
				/* Tell TCP that we wish be to informed of data that has been
				successfully sent by a call to the http_sent() function. */
				tcp_sent(pcb, http_sent); 
				
				/* reset transmission */
				clear_transmission();
				return ERR_OK;
			}
			
			len -= i;
			TotalData -= i;
			iap_dbg("TotalData = %lu(last packet)\r\n" , TotalData);

			/* write data in Flash */
			if (len){
				//IAP_HTTP_writedata(ptr,len);
				webserver_iap_debug(ptr,len);
			}

			/* reset transmission */
			clear_transmission();
			iap_dbg(".........end.........\r\n\r\n");
			/* send uploaddone.html page */
			iap_fs_open("/uploaddone.html", &file);
			hs->file = file.data;
			hs->left = file.len;
			send_data(pcb, hs);
			/* Tell TCP that we wish be to informed of data that has been
			successfully sent by a call to the http_sent() function. */
			tcp_sent(pcb, http_sent);  
			
			return ERR_OK;
		}
		/* not last data packet */
		else {
			/* write data in flash */
			if(len){
				//IAP_HTTP_writedata(ptr,len);
				webserver_iap_debug(ptr,len);
			}
		}
		pbuf_free(p);
	}
	else
	{
		/* 不是上传文件http请求 */
		iap_dbg("webserver_iap_http_recv returns ERR_DONOTHING\r\n");
		/* reset transmission */
		clear_transmission();
		return ERR_DONOTHING;
	}
	return ERR_OK;
}
/**
  * @brief  callback function on TCP connection setup ( on port 80)
  * @param  arg: pointer to an argument structure to be passed to callback function
  * @param  pcb: pointer to a tcp_pcb structure
  * &param  err: Lwip stack error code
  * @retval err
  */
/*
static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct http_state *hs;

  // Allocate memory for the structure that holds the state of the connection
  hs = mem_malloc(sizeof(struct http_state));

  if (hs == NULL)
  {
    return ERR_MEM;
  }

  // Initialize the structure. 
  hs->file = NULL;
  hs->left = 0;

  // Tell TCP that this is the structure we wish to be passed for our
  // callbacks. 
  tcp_arg(pcb, hs);

  // Tell TCP that we wish to be informed of incoming data by a call
  // to the http_recv() function. 
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);

  tcp_poll(pcb, http_poll, 10);
  return ERR_OK;
}
*/

/**
  * @brief  intialize HTTP webserver  
  * @param  none
  * @retval none
  */
/*
static void IAP_httpd_init(void)
{
  struct tcp_pcb *pcb;
  // create new pcb
  pcb = tcp_new();
  // bind HTTP traffic to pcb 
  tcp_bind(pcb, IP_ADDR_ANY, 80);
  // start listening on port 80 
  pcb = tcp_listen(pcb);
  // define callback function for TCP connection setup 
  tcp_accept(pcb, http_accept);
}
*/
	
/**
  * @brief  Opens a file defined in fsdata.c ROM filesystem 
  * @param  name : pointer to a file name
  * @param  file : pointer to a fs_file structure  
  * @retval  1 if success, 0 if fail
  */
static int iap_fs_open(char *name, struct fs_file *file)
{
  struct fsdata_file_noconst *f;

  for (f = (struct fsdata_file_noconst *)FS_ROOT; f != NULL; f = (struct fsdata_file_noconst *)f->next)
  {
    if (!strcmp(name, f->name))
    {
      file->data = f->data;
      file->len = f->len;
      return 1;
    }
  }
  return 0;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
