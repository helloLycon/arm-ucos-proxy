

#include "hj_nm.h"
//#include "usart1.h"	
#include "uart3_control.h"
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "useriap.h" 
#include "usart.h"

static hj_nm_cmd_frame_t ack_frame;

extern CONTROL_S * h_uartcontrol;

u32_t  hj_80_fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

unsigned int  hj15_crc_7e(unsigned char  * pdata, int nlen,unsigned char *p_crc)
{
    int i;
    unsigned int len=0;
    unsigned short int temp;
    unsigned char data;
    *p_crc = 0x7e;
    p_crc++;
    len++;
    temp = PPPINITFCS16;
    for(i = 0 ; i < nlen; ++i ){
        if((pdata[i]==0x7e)||(pdata[i]==0x7d)){
            *p_crc = 0x7d;
            p_crc++;
            *p_crc = pdata[i]^0x20;
            len++;
        }else{
            *p_crc = pdata[i];
        }
        len++;
        p_crc++;
        temp = (temp >> 8) ^ hj_80_fcstab[(temp ^ pdata[i]) & 0xff];
    }
    temp = temp^PPPINITFCS16;
    data = temp&0xff;
    if((data==0x7e)||(data==0x7d)){
        *p_crc = 0x7d;
        p_crc++;
        *p_crc = data^0x20;
        len++;
    }else{
        *p_crc = data;
    }
    len++;
    p_crc++;
    data = (temp>>8)&0xff;
    if((data==0x7e)||(data==0x7d)){
        *p_crc = 0x7d;
        p_crc++;
        *p_crc = data^0x20;
        len++;
    }else{
        *p_crc = data;
    }
    len++;
    p_crc++;
    *p_crc = 0x7e;
    len++;
    return len;
}

//不进行0x20异或，只计算CRC。
unsigned int  hj15_crc_7e_notxor_20(unsigned char  * pdata, int nlen,unsigned char *p_crc)
{
    int i;
    unsigned int len=0;
    unsigned short int temp;
	unsigned char data;
    *p_crc = 0x7e;
    p_crc++;
    len++;
    temp = PPPINITFCS16;
    for(i = 0 ; i < nlen; ++i ){
        //if((pdata[i]==0x7e)||(pdata[i]==0x7d)){
        //    *p_crc = 0x7d;
        //    p_crc++;
        //    *p_crc = pdata[i]^0x20;
        //    len++;
        //}else{
            *p_crc = pdata[i];
        //}
        len++;
        p_crc++;
        temp = (temp >> 8) ^ hj_80_fcstab[(temp ^ pdata[i]) & 0xff];
    }
    temp = temp^PPPINITFCS16;
    data = temp&0xff;
    //if((data==0x7e)||(data==0x7d)){
    //    *p_crc = 0x7d;
    //    p_crc++;
    //    *p_crc = data^0x20;
    //    len++;
    //}else{
        *p_crc = data;
    //}
    len++;
    p_crc++;
    data = (temp>>8)&0xff;
    //if((data==0x7e)||(data==0x7d)){
    //    *p_crc = 0x7d;
    //    p_crc++;
    //    *p_crc = data^0x20;
    //    len++;
    //}else{
        *p_crc = data;
    //}
    len++;
    p_crc++;
    *p_crc = 0x7e;
    len++;
    return len;
}



unsigned int hj_calc_crc(unsigned char  * pdata, int nlen)
{
  int i;
  unsigned short int temp;

  temp = PPPINITFCS16;

  for(i = 0 ; i < nlen; ++i )
    temp = (temp >> 8) ^ hj_80_fcstab[(temp ^ pdata[i]) & 0xff];
	temp = temp^PPPINITFCS16;
  return temp;
}
#if 1
unsigned int FCS(unsigned short int fcs,unsigned char x)
{
	fcs = (fcs >> 8) ^ hj_80_fcstab[(fcs ^ x) & 0xff];
	return(fcs);
}
#endif


/*************************************************                                                     
  Function:        debug_dump                                                                        
  Description:     调试打印
  Calls:           无                                                                                                                                        
  Input:           
                   u8_t *buf    : 源缓冲区
                   s32_t len    : 数据长度
                   u8_t *tip    : 提示信息                                                        
  Output:   	     无                  
  Return:          无                          
  Others:          无                                                                                  
*************************************************/
void_t debug_dump(u8_t *buf, s32_t len, u8_t *tip)
{
	extern int cur_debug_level;
    s32_t i;

	if( DBG_DEBUG <= cur_debug_level){
	    printf("%s\r\n", tip);
	    for (i = 0; i < len; ++i) {
	        printf("%02X ", buf[i]);
	    }
	    printf("\r\n");
	}
}

void debug_puts(int dbg_level, const char * dbg_str){
	extern int cur_debug_level;
	if( dbg_level <= cur_debug_level){
		printf(dbg_str);
	}
}

int debug_printf(int dbg_level , const char * fmt,...){
	extern int cur_debug_level;
	if( dbg_level <= cur_debug_level){
		int ret;
		va_list ap;
		va_start(ap,fmt);
		ret = vprintf(fmt , ap);
		va_end(ap);
		return ret;
	}
	return 0;
}

#if 0

static  __asm void boot_jump( uint32_t address )
{
	LDR SP, [R0]		;Load new stack pointer address
    LDR PC, [R0, #4]	;Load new program counter address
}

static  void ExecuteUserCode(uint32_t addr)
{
	SysTick->CTRL = 0;
	SCB->VTOR = addr & 0xFFFFFF80;
	boot_jump(addr);
}

#endif
//如何实现复位功能：
//1、使用函数指针跳转，
//2、使用寄存器配置地址跳转
//3、不再喂狗使之复位


status_t control_set_sysreset(void)
{
    s32_t nret = 0; 

	printf("system will restart in serval sec.\n");
	NVIC_DisableIRQ(USART2_IRQn);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_DisableIRQ(ETH_IRQn);
    __disable_irq();		
	//iap_load_app(FLASH_BOOT_ADDR);  
	NVIC_SystemReset();
    return nret;
}


//网管盘复位功能暂时无法实现
status_t control_jump_to_app(void)
{
    s32_t nret = 0; 

	printf("system will jump to app.\n");
	NVIC_DisableIRQ(USART2_IRQn);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_DisableIRQ(ETH_IRQn);
    __disable_irq();		
    //ExecuteUserCode(FLASH_APP_ADDR);    
	iap_load_app(FLASH_APP_ADDR);    
    return nret;
}



status_t control_read_device_cfg(u8_t * p_p_data)
{
    s32_t nret = 0;
    s32_t retb;
    struct tagDeviceConfigS  * p_device_cfg;
    
    nret = sizeof(struct tagDeviceConfigS)-2;
    p_device_cfg = (struct tagDeviceConfigS *)(p_p_data);    
    
    //retb = mutex_lock_f(h_control->device_mutex, -1);
    //if(main_config_read_device_f((u8_t *)p_device_cfg, sizeof(struct tagDeviceConfigS)) == OK_T){
        
    //    memset(p_device_cfg->unuse, 0, sizeof(p_device_cfg->unuse));

    //    nret = sizeof(struct tagDeviceConfigS)-2; //crc             
        //printf("read device_id=%x\n", p_global_cfg->device_id);
		//printf("main_config_read_gloabl_f ok\n");

    //}else{				//初始值
    	
    //	p_device_cfg->baudrate     = BAUDRATE_2400;
//		p_device_cfg->work_mode   =  MODE_TCP_SERVER;	
	//	printf("control_read_device_cfg error\n");
    //}
//    p_device_cfg->baudrate = control_get_scom2_baudrate();
    
    nret = sizeof(struct tagDeviceConfigS)-2; //crc  
    //printf("222nret=%x\n", nret);           
    //if (retb == OK_T)
    //    mutex_unlock_f(h_control->device_mutex);

    return nret;
}




status_t control_write_device_cfg(u8_t * p_buff)
{
    s32_t nret = 0; 
    struct tagDeviceConfigS  device_cfg;   
	struct tagDeviceConfigS  * p_device_cfg;
    p_device_cfg = (struct tagDeviceConfigS *)p_buff;
    
    //debug_dump(p_buff, sizeof(struct tagGlobalConfigS), "pcontrol");    
    
    main_config_read_device_f((uint8_t *)&device_cfg, sizeof(struct tagDeviceConfigS));
#if 0
	if(device_cfg.baudrate != p_device_cfg->baudrate){ //当IP地址不相等时

        device_cfg.baudrate = p_device_cfg->baudrate;
        //system_change_ip(global_cfg.host_ip, global_cfg.netmask);
        wgcmd_set_baud(device_cfg.baudrate);        //设置波特
        //system_change_gw(global_cfg.gateway, global_cfg.host_ip);
        main_config_write_device_f((uint8_t *)&device_cfg, sizeof(struct tagDeviceConfigS));
    }
	
    if(device_cfg.work_mode != p_device_cfg->work_mode){ //当IP地址不相等时

        device_cfg.work_mode = p_device_cfg->work_mode;          
        main_config_write_device_f((uint8_t *)&device_cfg, sizeof(struct tagDeviceConfigS));      
    }  
#endif  
    nret =  sizeof(struct tagDeviceConfigS);
	
  	//debug_dump(&global_cfg, sizeof(struct tagGlobalConfigS), "tagGlobalConfigS222");   

    return nret;
}


status_t control_read_global_cfg(u8_t * p_p_data)
{
    s32_t nret = 0;
    s32_t retb;
    struct tagGlobalConfigS  * p_global_cfg;
    
    nret = sizeof(struct tagGlobalConfigS)-2;
   
    p_global_cfg = (struct tagGlobalConfigS *)p_p_data;    
    
    //retb = mutex_lock_f(h_control->device_mutex, -1);
    if(main_config_read_gloabl_f((u8_t *)p_global_cfg, sizeof(struct tagGlobalConfigS)) == OK_T){
        //printf("control_read_global_cfg222222222\n");

        memset(p_global_cfg->unuse, 0, sizeof(p_global_cfg->unuse));
		p_global_cfg->arm_version = ARM_VERSION;
		p_global_cfg->host_port   =  NM_TCP_SERVER_PORT;
        nret = sizeof(struct tagGlobalConfigS)-2; //crc            
//        memcpy(p_buff,p_global_cfg,nret);   
        
        //printf("read device_id=%x\n", p_global_cfg->device_id);
		//printf("main_config_read_gloabl_f ok\n");

			
	    debug_dump((u8_t *)p_global_cfg, sizeof(struct tagGlobalConfigS), "control_read_global_cfg:");
    }else{				//???
    	
    	
		//printf("main_config_read_gloabl_f error\n");
		debug_puts(DBG_ERR,"main_config_read_gloabl_f error\r\n");
	      
    }
    
    nret = sizeof(struct tagGlobalConfigS)-2; //crc  
    nret = 64;                                //////
    //printf("222nret=%x\n", nret);           
    //if (retb == OK_T)
    //    mutex_unlock_f(h_control->device_mutex);

    return nret;
}





int ip_available(uint32_t ip){
	u8_t msbyte = (u8_t)(ip>>24);
	// value between 1 and 223
	return  (msbyte>=1 && msbyte<=223) ;
}

status_t control_write_global_cfg(u8_t * p_buff,int * result)
{
    s32_t nret = 0; 
    struct tagGlobalConfigS  global_cfg;   
	struct tagGlobalConfigS  * p_global_cfg;
    p_global_cfg = (struct tagGlobalConfigS *)p_buff;
    
    //debug_dump(p_buff, sizeof(struct tagGlobalConfigS), "pcontrol");    
    
    main_config_read_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
	    //debug_dump(&global_cfg, sizeof(struct tagGlobalConfigS), "tagGlobalConfigS");    

	*result = 0;
	if( !ip_msk_gw_validation(p_global_cfg->host_ip,p_global_cfg->netmask,p_global_cfg->gateway)){
		*result = -1;
		goto WRITE_GLOBAL_END;
	}

/*	
	printf("host_ip=%x\n", p_global_cfg->host_ip);
	printf("host_port=%x\n", p_global_cfg->host_port);
	
	printf("gateway=%x\n", p_global_cfg->gateway);
	printf("netmask=%x\n", p_global_cfg->netmask);
	printf("mac[0]=%x\n", p_global_cfg->mac[0]);
	printf("mac[1]=%x\n", p_global_cfg->mac[1]);
	printf("mac[2]=%x\n", p_global_cfg->mac[2]);
	printf("mac[3]=%x\n", p_global_cfg->mac[3]);
	printf("mac[4]=%x\n", p_global_cfg->mac[4]);
	printf("mac[5]=%x\n", p_global_cfg->mac[5]);
	printf("jump_ip=%x\n", p_global_cfg->jump_ip);
	printf("jump_port=%x\n", p_global_cfg->jump_port);
*/	
	if((global_cfg.host_ip != p_global_cfg->host_ip)&& ip_available(p_global_cfg->host_ip)){ //当IP地址不相等时

        global_cfg.host_ip = p_global_cfg->host_ip;
        //system_change_ip(global_cfg.host_ip, global_cfg.netmask);
        
        cmd_set_ipaddr(global_cfg.host_ip);
        cmd_set_netmask(global_cfg.netmask);
        cmd_set_gw(global_cfg.gateway);
        //system_change_gw(global_cfg.gateway, global_cfg.host_ip);
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
    }
	
    if(global_cfg.host_port != p_global_cfg->host_port){ //当IP地址不相等时

        global_cfg.host_port = p_global_cfg->host_port;          
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));      
    }  
    if(global_cfg.jump_ip != p_global_cfg->jump_ip){ //当IP地址不相等时

        global_cfg.jump_ip = p_global_cfg->jump_ip;  
        //printf("jump_ip=%x\n", global_cfg.jump_ip);           
        debug_printf(DBG_INFO,"jump_ip=%x\r\n", global_cfg.jump_ip);           
        //sprintf(h_control->p_jump->p_socket_data->remote_addr, "%d.%d.%d.%d"
        //    , (global_cfg.jump_ip >> 24) & 0xFF
        //    , (global_cfg.jump_ip >> 16) & 0xFF
        //    , (global_cfg.jump_ip >> 8) & 0xFF
        //    , global_cfg.jump_ip & 0xFF);          
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));     
        jump_update_udp_addr(); 
		/* +++ */
		st_update_udp_send_addr();
    }  

    if(global_cfg.jump_port != p_global_cfg->jump_port){ //当IP地址不相等时

        global_cfg.jump_port = p_global_cfg->jump_port;              
        //h_control->p_jump->p_socket_data->remote_port =  global_cfg.jump_port; 
        //printf("global_cfg.jump_port=%x\n", global_cfg.jump_port);   
        debug_printf(DBG_INFO,"global_cfg.jump_port=%x\r\n", global_cfg.jump_port);   
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
        jump_update_udp_addr();  
    }

    if(global_cfg.trap_ip != p_global_cfg->trap_ip){ //当IP地址不相等时

        global_cfg.trap_ip = p_global_cfg->trap_ip;      

        //printf("<set Trap IP = %#x, port = %d>\n", global_cfg.trap_ip, global_cfg.trap_port);
        debug_printf(DBG_INFO,"<set Trap IP = %#x, port = %d>\r\n", global_cfg.trap_ip, global_cfg.trap_port);
                  
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));      
    
    }  

    if(global_cfg.trap_port != p_global_cfg->trap_port){ //当IP地址不相等时

        global_cfg.trap_port = p_global_cfg->trap_port;    

        //printf("<set Trap IP = %#x, port = %d>\n", global_cfg.trap_ip, global_cfg.trap_port);       
        debug_printf(DBG_INFO,"<set Trap IP = %#x, port = %d>\r\n", global_cfg.trap_ip, global_cfg.trap_port);       
                
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
        
    } 

    if(global_cfg.gateway != p_global_cfg->gateway){ //当gateway不相等时
        global_cfg.gateway = p_global_cfg->gateway;      
        //system_change_gw(global_cfg.gateway, global_cfg.host_ip);
        cmd_set_ipaddr(global_cfg.host_ip);
        cmd_set_netmask(global_cfg.netmask);
        cmd_set_gw(global_cfg.gateway);
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
    }

    if(global_cfg.tserver_ip != p_global_cfg->tserver_ip){ //当IP地址不相等时
        global_cfg.tserver_ip = p_global_cfg->tserver_ip;
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
    }

    if(global_cfg.netmask != p_global_cfg->netmask){ //当IP地址不相等时
        global_cfg.netmask = p_global_cfg->netmask;
        cmd_set_ipaddr(global_cfg.host_ip);
        cmd_set_netmask(global_cfg.netmask);
        cmd_set_gw(global_cfg.gateway);
        //system_change_ip(global_cfg.host_ip, global_cfg.netmask);
       
        //system_change_gw(global_cfg.gateway, global_cfg.host_ip);
        main_config_write_gloabl_f((uint8_t *)&global_cfg, sizeof(struct tagGlobalConfigS));
    }        
WRITE_GLOBAL_END:
    nret =  sizeof(struct tagGlobalConfigS);
	
	//main_config_read_gloabl_f(&global_cfg, sizeof(struct tagGlobalConfigS));
  	//debug_dump(&global_cfg, sizeof(struct tagGlobalConfigS), "tagGlobalConfigS222");   

    return nret;
}

int control_write_nic_mac(u8_t cmdExt , u8_t * buf , int * p_status){
	static u8_t rand_mac[8] , last_success_cmd_ext;
	int i  ;
	uint32_t read_os_time;
	OS_CPU_SR cpu_sr;
	u8_t xor_mac_res[8];
	struct tagGlobalConfigS global_cfg;

	OS_ENTER_CRITICAL();
	read_os_time = OSTime;
	OS_EXIT_CRITICAL();

	/* os time exceeds 2 min */
	if( read_os_time > (120*OS_TICKS_PER_SEC)){
		last_success_cmd_ext = 0;
		*p_status = Error_UI_MAC_TIMEOUT;
		debug_printf(DBG_WARNING, "control_write_nic_mac: timed out\r\n");
		return 0;
	}

	main_config_read_gloabl_f((u8_t*)&global_cfg , sizeof(struct tagGlobalConfigS));
	switch(cmdExt){
		case CMDEXT_NIC_MAC_STEP1:
			memcpy(rand_mac , buf,sizeof(rand_mac));
			memcpy(buf ,global_cfg.mac , 6);
			last_success_cmd_ext = cmdExt;
			*p_status = Error_UI_OK;
			debug_printf(DBG_DEBUG,"CMDEXT_NIC_MAC_STEP1: succeed\r\n");
			return 8;
		case CMDEXT_NIC_MAC_STEP2:
			if( CMDEXT_NIC_MAC_STEP1 != last_success_cmd_ext ){
				last_success_cmd_ext = 0;
				*p_status = Error_UI_Error;
				debug_printf(DBG_ERR,"CMDEXT_NIC_MAC_STEP2: CMDEXT_NIC_MAC_STEP1 != last_success_cmd_ext\r\n");
				return 0;
			}
			memcpy(xor_mac_res , buf, sizeof(xor_mac_res));
			for(i=0 ;i<6;++i ){
				// xor check
				if( global_cfg.mac[i]^rand_mac[i] != xor_mac_res[i])
					break;
			}
			//*p_status = ((6==i)?OK_T:ERROR_T);
			if( 6==i ){
				last_success_cmd_ext = cmdExt;
				*p_status = Error_UI_OK;
				debug_printf(DBG_DEBUG,"CMDEXT_NIC_MAC_STEP2: succeed\r\n");
			}
			else{
				last_success_cmd_ext = 0;
				*p_status = Error_UI_Error;
				debug_printf(DBG_ERR,"CMDEXT_NIC_MAC_STEP2: xor check error\r\n");
			}
			return 0;
		case CMDEXT_NIC_MAC_STEP3:
			if( CMDEXT_NIC_MAC_STEP2 != last_success_cmd_ext ){
				last_success_cmd_ext = 0;
				*p_status = Error_UI_Error;
				debug_printf(DBG_ERR,"CMDEXT_NIC_MAC_STEP3: CMDEXT_NIC_MAC_STEP2 != last_success_cmd_ext\r\n");
				return 0;
			}
			memcpy(global_cfg.mac , buf , 6);
			main_config_set_main_mac(global_cfg.mac);
			//main_config_write_gloabl_f((u8_t*)&global_cfg , sizeof(struct tagGlobalConfigS));
			last_success_cmd_ext = cmdExt;
			*p_status = Error_UI_OK;
			debug_printf(DBG_DEBUG,"CMDEXT_NIC_MAC_STEP3: succeed:(mac: %02x:",global_cfg.mac[0]);
			debug_printf(DBG_DEBUG,"%02x:",global_cfg.mac[1]);
			debug_printf(DBG_DEBUG,"%02x:",global_cfg.mac[2]);
			debug_printf(DBG_DEBUG,"%02x:",global_cfg.mac[3]);
			debug_printf(DBG_DEBUG,"%02x:",global_cfg.mac[4]);
			debug_printf(DBG_DEBUG,"%02x)\r\n",global_cfg.mac[5]);
			return 0;
		default:
			last_success_cmd_ext = 0;
			*p_status = Error_UI_Error;
			debug_printf(DBG_ERR,"control_write_nic_mac: case error(default)\r\n" );
			return 0;
	}
	last_success_cmd_ext = 0;
	*p_status = Error_UI_Error;
	debug_printf(DBG_ERR,"control_write_nic_mac: case error(default)\r\n" );
	return 0;
}


#if 0
status_t work_set_wg_board_ftp_cmd_data(u32_t cmdExt, u32_t thread_num, u8_t * buff, u32_t len)
{   
    work_set_ftp_cmd_data(cmdExt, thread_num, buff, len);
    return 0;
}
#endif

status_t control_transfer_cmd(s32_t board_index, s32_t interfaceIndex, u8_t * buff, u32_t len, u32_t thread_num)
{
    status_t nret = OK_T;
    s32_t i,framelen,socket;  
    static u8_t pwdata[256],pcrc[256];
    static struct hj_dyb_dmp_frame dmp_frame;
    static struct hj_dump_frame_head * p_head;
    u8_t * p_dst_char_buff, * p_src_char_buff;
    hj_nm_cmd_frame_t *ptxframe,*ptxback;
    struct hj_txback_hdr *ptxbackhdr;  
    s32_t pthread_id = h_uartcontrol->threaddata[thread_num].thread_id;
    static u8_t seq;
    uint8_t err = 0;  
	//printf("len=%x, pthread_id=%x,seq=%x\n",len, pthread_id,seq);
    //memcpy(pwdata, &buff[1], len);
    p_head = (struct hj_dump_frame_head *)buff;
    if(p_head->cmdExt == EXPORT_CMDEXT){
        socket = control_get_fd_from_thread_num(thread_num);
        work_set_ftp_cmd_data(p_head->cmdExt,buff, len, socket);
        //work_set_ftp_cmd_data(p_head->cmdExt, buff, len, h_control->threaddata[thread_num].p_socket_data);
    }else if(p_head->cmdExt == IMPORT_CMDEXT){
        
        socket = control_get_fd_from_thread_num(thread_num);
        work_set_ftp_cmd_data(p_head->cmdExt,buff, len, socket);
    }else if(p_head->cmdExt == UPDATE_CMDEXT){
        
        socket = control_get_fd_from_thread_num(thread_num);
        work_set_ftp_cmd_data(p_head->cmdExt,buff, len, socket);
    }else{      //如果是其它命令则下发
    
        seq++;
        hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
        
        dmp_frame.phead->server_seq = seq;
        //printf("dmp_frame.phead:::seq=%x\n", seq);
        dmp_frame.phead->dst_deviceid = p_head->dst_deviceid;
        dmp_frame.phead->src_deviceid = p_head->src_deviceid;
        dmp_frame.phead->protocolVer = p_head->protocolVer;
        dmp_frame.phead->thread_netid = pthread_id;
        //printf("cmdLen=%x\n",p_head->cmdLen);
        //memcpy(&dmp_frame.phead->boardIndex,&p_head->boardIndex,p_head->cmdLen+8);
        dmp_frame.phead->boardIndex = p_head->boardIndex;
        dmp_frame.phead->interfaceIndex = p_head->interfaceIndex;
        //dmp_frame.phead->server_seq = p_head->server_seq;
        dmp_frame.phead->cmdExt = p_head->cmdExt;
        dmp_frame.phead->cmdId = p_head->cmdId;
        dmp_frame.phead->cmdStatus = p_head->cmdStatus;
        dmp_frame.phead->cmdLen = p_head->cmdLen;
			p_dst_char_buff = (u8_t *)&(dmp_frame.phead->cmdLen);
			p_dst_char_buff++;
			p_dst_char_buff++;
			p_src_char_buff = (u8_t *)&(p_head->cmdLen);
			p_src_char_buff++;
			p_src_char_buff++;
        memcpy(p_dst_char_buff,p_src_char_buff,p_head->cmdLen);
                
        //hj_nm_build_dump_frame(&dmp_frame, pwdata,0);	
    
    	framelen = sizeof(struct hj_dyb_dmp_frame_head);
    	framelen += p_head->cmdLen;
    	
        framelen = hj15_crc_7e(pwdata,framelen,pcrc);
        //debug_dump(pcrc, sizeof(struct hj_txback_hdr), "pcrc:");
    	Board_UARTX_Inset_Data_To_Link(pcrc, framelen);
    	
    	OSMutexPend(h_uartcontrol->threaddata[thread_num].hmutex,0,&err);
    	ptxback = h_uartcontrol->threaddata[thread_num].txback;			
     	for(i=0;i<TXBACKSIZE;i++){
        	ptxbackhdr = (struct hj_txback_hdr *)(ptxback->data_buff);
    		if(ptxbackhdr->pre==0){
    		    //p_src_char_buff = buff;
    			//memcpy(ptxbackhdr,p_src_char_buff,sizeof(hj_txback_hdr_t));
    			ptxbackhdr->pre = p_head->pre;
    			ptxbackhdr->protocolVer = p_head->protocolVer;
    			ptxbackhdr->dst_netid = p_head->thread_netid;
    			ptxbackhdr->thread_netid = p_head->dst_netid;
    			ptxbackhdr->dst_deviceid = p_head->src_deviceid;
    			ptxbackhdr->src_deviceid = p_head->dst_deviceid;
    			ptxbackhdr->boardIndex = p_head->boardIndex;
    			ptxbackhdr->interfaceIndex = p_head->interfaceIndex;
    			ptxbackhdr->server_seq = p_head->server_seq;
    			ptxbackhdr->cmdExt = p_head->cmdExt;
    			ptxbackhdr->cmdId = p_head->cmdId;
    			ptxbackhdr->cmdStatus = p_head->cmdStatus;
    			ptxbackhdr->cmdLen = p_head->cmdLen;
                ptxbackhdr->thread_id = pthread_id;
    			ptxbackhdr->seq = seq;
    			//printf("ptxbackhdr:::seq=%x\n", seq);
    			ptxbackhdr->times = 0;		
    			//debug_dump((u8_t *)ptxbackhdr, sizeof(struct hj_txback_hdr), "ptxbackhdr:");
    	
    			break;
    		}		
    		ptxback++;
    	}
    	OSMutexPost(h_uartcontrol->threaddata[thread_num].hmutex); 	
    }
    return nret;
}


status_t work_do_cmd_read_cfg(s8_t * buff, s32_t buff_len, int thread_num)
{
    s32_t len, ntotal, nret;
    struct hj_dump_frame_head * p_cmd_hdr, * p_cmd_ack_hdr;
    struct hj_dump_frame_tail  *p_cmd_tail;    
    
    u8_t board_index,interfaceIndex,cmdExt;        
    u16_t  temp_data;    
    u8_t * p_tmp_buff; 
    len = 0;
    p_cmd_hdr = (struct hj_dump_frame_head *)buff;

    ntotal = p_cmd_hdr->cmdLen+sizeof(struct hj_dump_frame_head)+3;		//crc2+7e1+datalen
   
	    if(buff_len >= ntotal){
	        len = ntotal;
	    }
	
    if(len>0)
    {
    	board_index = p_cmd_hdr->boardIndex;
    	interfaceIndex = p_cmd_hdr->interfaceIndex;
    	cmdExt = p_cmd_hdr->cmdExt;
    	//printf("dst_deviceid=%x,board_index=%x \n", p_cmd_hdr->dst_deviceid, board_index);
        //局端设备
                
            //printf("wgdevice::cmdExt=%x\n", cmdExt);
            
	        //if((cmdExt == CMDEXT_GLOBALDATA)||(cmdExt == CMDEXT_TIME)||(cmdExt == CMDEXT_DEVICEDATA)){	   	//读取全局参数或者时间
	        if((cmdExt == CMDEXT_GLOBALDATA)||(cmdExt == CMDEXT_TIME) /* ||(cmdExt == CMDEXT_DEVICEDATA)*/){	   	//读取全局参数或者时间
	        	
	        	if(cmdExt == CMDEXT_GLOBALDATA){   
	        	    struct tagGlobalConfigS global_cfg;
	        	    p_tmp_buff = (u8_t * )&global_cfg;
	        		nret = control_read_global_cfg(p_tmp_buff);   
	        	}else if(cmdExt == CMDEXT_TIME){
	        		//nret = control_get_time(&p_tmp_buff);   
	        		nret = 0;
	        	}
				/*else if((cmdExt == CMDEXT_DEVICEDATA)){
	        	    struct tagDeviceConfigS  device_cfg;
	        	    p_tmp_buff = (u8_t * )&device_cfg;
    	            nret = control_read_device_cfg(p_tmp_buff);  
    	       	
	        	}*/
				else{
	        	    nret = 0;
	        	}
	        	//printf("111* ack_buff\n");         	                     
	            ack_frame.n_data_len = sizeof(* p_cmd_ack_hdr)+3+nret;
	            
	            p_cmd_ack_hdr = (struct hj_dump_frame_head *)(ack_frame.data_buff);
	            p_cmd_ack_hdr->pre = 0x7e;
	            p_cmd_ack_hdr->protocolVer = PROTOCOL_VER;
				p_cmd_ack_hdr->dst_netid = p_cmd_hdr->dst_netid;
	            p_cmd_ack_hdr->thread_netid =p_cmd_hdr->thread_netid;
	            p_cmd_ack_hdr->src_deviceid = p_cmd_hdr->src_deviceid;
				p_cmd_ack_hdr->dst_deviceid = p_cmd_hdr->dst_deviceid;
	            p_cmd_ack_hdr->boardIndex = p_cmd_hdr->boardIndex;
	            p_cmd_ack_hdr->interfaceIndex = p_cmd_hdr->interfaceIndex;
	            p_cmd_ack_hdr->server_seq = SERVER_ID;
	            p_cmd_ack_hdr->cmdId = HJ80_CMD_GETCONFIGPARAMRESPONSE<<4;
	    		p_cmd_ack_hdr->cmdExt = cmdExt;
	            p_cmd_ack_hdr->cmdStatus = Error_UI_OK;
	            p_cmd_ack_hdr->cmdLen = nret;
	      
	            if(nret > 0)
	                memcpy(ack_frame.data_buff+sizeof(* p_cmd_ack_hdr), p_tmp_buff, nret);
	            	          
	            p_cmd_tail = (struct hj_dump_frame_tail *)(ack_frame.data_buff + sizeof(* p_cmd_ack_hdr)+nret);
	            temp_data  =  hj_calc_crc (ack_frame.data_buff+1,ack_frame.n_data_len -4);
	            p_cmd_tail -> crcl = temp_data & 0xff;
	            p_cmd_tail -> crch = (temp_data>>8) & 0xff;
	            p_cmd_tail -> suf = 0x7e;   
       
            }else{
                ack_frame.n_data_len = 0;
                control_transfer_cmd(board_index,interfaceIndex,buff, len, thread_num); 
            }       
           
    }   
    return len; 
}


status_t work_do_cmd_write_cfg(s8_t * buff, s32_t buff_len, int thread_num)
{
    s32_t len, ntotal, nret, socket;
    struct hj_dump_frame_head * p_cmd_hdr, * p_cmd_ack_hdr;
    struct hj_dump_frame_tail  *p_cmd_tail; 
    u8_t board_index,interfaceIndex,cmdExt;
    u16_t  temp_data; 
    u8_t * p_tmp_buff = NULL; 
    len = 0;
    p_cmd_hdr = (struct hj_dump_frame_head *)buff;

	ntotal = p_cmd_hdr->cmdLen+sizeof(struct hj_dump_frame_head)+3;		//crc2+7e1+datalen

	    if(buff_len >= ntotal){
	        len = ntotal;
	    }

    if(len>0)
    {
    	board_index = p_cmd_hdr->boardIndex;
    	interfaceIndex = p_cmd_hdr->interfaceIndex;
    	cmdExt = p_cmd_hdr->cmdExt;
    	//printf("dst_deviceid=%x, board_index=%x, cmdExt=%x\n", p_cmd_hdr->dst_deviceid, board_index, cmdExt);
        	//局端设备
            //if((cmdExt == CMDEXT_TIME)||(cmdExt == CMDEXT_GLOBALDATA)||(cmdExt == UPDATE_CMDEXT)||(cmdExt == IMPORT_CMDEXT)||(cmdExt == EXPORT_CMDEXT)){
            if((cmdExt == CMDEXT_TIME)||(cmdExt == CMDEXT_GLOBALDATA)||(cmdExt == CMDEXT_SYSRESET)||((cmdExt & 0xF0) == CMDEXT_NIC_MAC)){
				int write_nic_mac_status;
				int write_global_status = 0;
				p_tmp_buff = (u8_t *)&(p_cmd_hdr->cmdLen);
	        	p_tmp_buff += sizeof(p_cmd_hdr->cmdLen);
	        	if(cmdExt == CMDEXT_TIME){
	        		//nret = control_set_time(&p_cmd_hdr->buf[0]);
	        	}else if(cmdExt == CMDEXT_GLOBALDATA){
	        	    
	        		nret = control_write_global_cfg((p_tmp_buff),&write_global_status);  //指向数据
	        	}else if(cmdExt == CMDEXT_SYSRESET){
	        	    nret = control_set_sysreset();	        	
	        	//}else if((cmdExt == UPDATE_CMDEXT)){
	                
    	        //}else if((cmdExt == IMPORT_CMDEXT)){
    	            //socket = control_get_fd_from_thread_num(thread_num);
    	            //work_set_ftp_cmd_data(cmdExt,buff, len, socket);
    	        //}else if((cmdExt == EXPORT_CMDEXT)){
    	            //socket = control_get_fd_from_thread_num(thread_num);
    	            //work_set_ftp_cmd_data(cmdExt,buff, len, socket);
    	        }
				else if( (cmdExt & 0xF0) == CMDEXT_NIC_MAC){
					nret = control_write_nic_mac(cmdExt , p_tmp_buff,&write_nic_mac_status);
				}
				/*else if((cmdExt == CMDEXT_DEVICEDATA)){
    	            nret = control_write_device_cfg((p_tmp_buff));  //指向数据
    	        }*/
    	        else if(cmdExt == CMDEXT_DEFAULT){
	        		nret = control_set_default();
	        	}else if(cmdExt == PARAM_ALTERID_EXT40){     
	        	    PARAM_UP_EXT40_S alterid_upext40;
			        p_tmp_buff = (u8_t * )&alterid_upext40;		
	        	    nret = control_set_alterid_ext40(p_tmp_buff);
	        	}
					
	            ack_frame.n_data_len = sizeof(* p_cmd_ack_hdr)+3+nret;
	            
	            p_cmd_ack_hdr = (struct hj_dump_frame_head *)(ack_frame.data_buff);
	            p_cmd_ack_hdr->pre = 0x7e;
	            p_cmd_ack_hdr->protocolVer = PROTOCOL_VER;
				p_cmd_ack_hdr->dst_netid = p_cmd_hdr->dst_netid;
	            p_cmd_ack_hdr->thread_netid =p_cmd_hdr->thread_netid;
	            p_cmd_ack_hdr->src_deviceid = p_cmd_hdr->src_deviceid;
				p_cmd_ack_hdr->dst_deviceid = p_cmd_hdr->dst_deviceid;
	            p_cmd_ack_hdr->boardIndex = p_cmd_hdr->boardIndex;
	            p_cmd_ack_hdr->interfaceIndex = p_cmd_hdr->interfaceIndex;
	            p_cmd_ack_hdr->server_seq = SERVER_ID;
	            p_cmd_ack_hdr->cmdId = HJ80_CMD_SETCONFIGRESPONSE<<4;
				p_cmd_ack_hdr->cmdExt = cmdExt;
				if((cmdExt & 0xF0) == CMDEXT_NIC_MAC)
					p_cmd_ack_hdr->cmdStatus = write_nic_mac_status;
				else
					p_cmd_ack_hdr->cmdStatus = write_global_status<0?Error_UI_Error:Error_UI_OK;
	            p_cmd_ack_hdr->cmdLen = nret;
	
	            if(nret > 0)
	                memcpy(ack_frame.data_buff+sizeof(* p_cmd_ack_hdr), p_tmp_buff, nret);
	            
	            p_cmd_tail = (struct hj_dump_frame_tail *)(ack_frame.data_buff + sizeof(* p_cmd_ack_hdr)+ nret);
	            temp_data  =  hj_calc_crc (ack_frame.data_buff+1,ack_frame.n_data_len -4);
	            p_cmd_tail -> crcl = temp_data & 0xff;
	            p_cmd_tail -> crch = (temp_data>>8) & 0xff;
	            p_cmd_tail -> suf = 0x7e;   
	        }else{
                ack_frame.n_data_len = 0;
                control_transfer_cmd(board_index, interfaceIndex, buff, len, thread_num);           
            }    
            
    }   
    return len; 
}


status_t work_term_cmd_excute(s8_t * buff, u32_t len, int thread_num)
{
    s32_t nret = 0;
    struct hj_dump_frame_head * p_cmd_hdr;
	u32_t cmdid;
    p_cmd_hdr = (struct hj_dump_frame_head *)buff;

	cmdid = p_cmd_hdr->cmdId;
	cmdid &= 0xf0;
	cmdid >>= 4;
    switch(cmdid){

    case HJ80_CMD_GETCONFIGPARAM:
        nret = work_do_cmd_read_cfg(buff, len, thread_num);
        break;
    case HJ80_CMD_SETCONFIG:
        nret = work_do_cmd_write_cfg(buff, len,thread_num);
        break;   
   
    default:  nret= len;
    }   
    return nret;
}




static status_t work_excute_hj_cmd(int socket, s8_t * buff, s32_t len)
{
    s32_t nret;
    s32_t thread_num;
    thread_num = control_get_thread_num_from_fd(socket);
    nret = work_term_cmd_excute(buff, len, thread_num);
//    printf("ack_len111=%x,nret=%x\n", ack_frame.n_data_len,nret);
    if(ack_frame.n_data_len > 0){
        
        write(socket, ack_frame.data_buff, ack_frame.n_data_len);
        ack_frame.n_data_len = 0;
//        debug_dump(ack_frame.data_buff,ack_frame.n_data_len, "ack net:");
    }
    return nret;
}

u32_t cmd_process_count;
#define MAX_CMD_PROCESS_COUNT 20
status_t work_do_hj_cmd(int socket, s8_t * buff, s32_t len)
{
    s32_t i, nret = 0;
    s32_t nresult = (sizeof(struct hj_dump_frame_head)+3);
    s32_t len_left = len;
    cmd_process_count = 0;
    while((nresult >= (sizeof(struct hj_dump_frame_head)+3)) &&(len_left >= (sizeof(struct hj_dump_frame_head)+3))){
        
        nresult = work_excute_hj_cmd(socket,buff,len_left);
        cmd_process_count++;
        if(cmd_process_count >= MAX_CMD_PROCESS_COUNT){
            break;
        }
        //printf("nresult=%x\n", nresult);
        len_left = len_left - nresult;
        if(nresult >= (sizeof(struct hj_dump_frame_head)+3)){
            nret = nret + nresult;
            for(i = 0; i<len_left; i++){
                buff[i] = buff[i + nresult];
            }
        }else{          //当命令有错时需要删除这个帧，否则这个帧一直在打转
            //nret = len_left;
        }
    }
    return nret;
}




#if 0
status_t proxy_control_build_jump_frame(void)
{
    u8_t pwdata[128], pcrc[128];
    u32_t framelen;
    u16_t src_deviceid;
    struct hj_dmp_frame_head * p_dmp_frame_head;
    
    p_dmp_frame_head = (struct hj_dmp_frame_head *)pwdata;
    
    //printf("proxy_control_build_jump_frame\n");
    //hj_nm_build_dump_frame(&dmp_frame, pwdata,0);
    p_dmp_frame_head->protocolVer = 0x01;
    p_dmp_frame_head->dst_netid = PC_DEVICE_ID;    
    //dmp_frame.phead->thread_netid = main_config_get_main_host_ip();
    p_dmp_frame_head->dst_deviceid = PC_DEVICE_ID;
    //src_deviceid = control_get_local_device_id();
	//src_deviceid &= 0xff00;
    src_deviceid = 0;
    p_dmp_frame_head->src_deviceid = src_deviceid;    
    p_dmp_frame_head->boardIndex = 0xff;
    p_dmp_frame_head->interfaceIndex = 0xff;
    p_dmp_frame_head->server_seq = 0x01;
    framelen = JUMP_CMD;
    framelen <<= 4;
    framelen |= (RESPBEAT_CMDEXT>>8);
    p_dmp_frame_head->cmdId = framelen;
    p_dmp_frame_head->cmdExt = RESPBEAT_CMDEXT;
    p_dmp_frame_head->cmdStatus = 0;
    p_dmp_frame_head->cmdLen = 0;
    
	//debug_dump(pwdata, sizeof(struct hj_dmp_frame_head), "jump:"); 
    framelen = sizeof(struct hj_dmp_frame_head);
    framelen = hj15_crc_7e_notxor_20(pwdata,framelen,pcrc);
    //printf("framelen=%x\n", framelen);
    
	//if(hj80_debug(DEBUG_BOARD_MAIN,  HJ80_DEBUG_PC_SEND)){
//        debug_dump(pcrc, framelen, "1111jump:"); 
	//}
    
    jump_send_buf(pcrc, framelen);              //写入trap缓冲区
    return OK_T;
}
#endif
