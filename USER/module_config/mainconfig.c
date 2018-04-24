/*
*
*  Filename        : config.c
*  Version         : 1.0.0.0
*  Author          : Xiaole
*  Description     :
*
*  History         :
*  1.  Date        : 2010/10/11
*      Author      : Xiaole
*      Modification: Created file
*/




#include "mainconfig.h"
#include "malloc.h"
#include "usr_prio.h"
#include "usart.h"
#include "useriap.h"
#include "lwip/inet.h"
#include "webserver_account.h"
//#include "crc16.h"

//static MAINCONFIG_S Manager;
static MAINCONFIG_S * hManager = NULL;



static int32_t ConfigLoad(uint8_t *filename, MAINCONFIG_S *pConfig);
static int32_t ConfigSave(uint8_t *filename, MAINCONFIG_S *pConfig);
static int32_t ConfigReset(MAINCONFIG_S *pConfig);

#define CONFIG_VERSION_SIZE 20


/*
*   Function :  ConfigReadFile
*   Describer:  config read frome file
*   Parameter:  filename        config file name
*               buf             read data point
*               size            the size that max read and return really read size
*   Return   :  OK_T            success
*               ERROR_T         failed
*/
int32_t ConfigReadSpiflash(uint8_t *filename, uint8_t *buf, uint32_t size)
{
    
    //spifilib_mainconfig_read(buf, MAINCONFIG_ADDR, size);
    SPI_Flash_Read(buf, MAINCONFIG_ADDR, size);    
    return OK_T;
}


/*
*   Function :  ConfigWriteFile
*   Describer:  config data write to file
*   Parameter:  filename        config file name
*               buf             config data point
*               size            the size that max write 
*   Return   :  OK_T            success
*               ERROR_T         failed
*/
int32_t ConfigWriteSpiflash(uint8_t *filename, uint8_t *buf, uint32_t size)
{
    //spifilib_mainconfig_write(buf, size); 
    SPI_Flash_Write(buf, MAINCONFIG_ADDR, size);
    return OK_T;
}



int32_t main_config_init_f(void_t)
{   
    //hMutex = mutex_create_f();
    //if (hMutex == NULL)
    //    return ERROR_T;
    u16_t ncrc;
    struct tagGlobalConfigS gloabl_cfg2;
    uint8_t err = 0;  
    //hManager = &Manager;
    
    
    hManager = mymalloc(SRAMCCM,sizeof(MAINCONFIG_S));
    
    hManager->hMutex = OSMutexCreate(MAINCONFIG_MUTEX_PRIO, &err);
	/* do check */
	os_obj_create_check("hManager->hMutex","main_config_init_f",err);

    
    printf("main_config_init_f::hManager=%x\n", hManager);
    if (hManager == NULL) {
        //xprintf(DEBUG_ERROR, "malloc failed (%d)\n", sizeof(MAINCONFIG_S));
        //mutex_destroy_f(hMutex);
        return ERROR_T;
    }   
    if (ConfigLoad(MAIN_CFG_FILE_NAME, hManager) == ERROR_T) {  //这里没有进行CRC校验
        printf("configload load error\n");
        ConfigReset(hManager);

        //printf("config reset\n");
        ConfigSave(MAIN_CFG_FILE_NAME, hManager);
        //printf("config save\n");
    }else{
        
        memcpy(&gloabl_cfg2, &hManager->Global, sizeof(struct tagGlobalConfigS));
        
        //debug_dump(&gloabl_cfg2, sizeof(struct tagGlobalConfigS), "main_config_read_gloabl_f:");
        ncrc = crc16_calc_f(&gloabl_cfg2, sizeof(struct tagGlobalConfigS)-2);        
        //printf("crc=%x,gloabl_cfg.crc=%x\n", ncrc, gloabl_cfg2.crc);
    //    printf("pData=%x\n", pData);
        if(gloabl_cfg2.crc == ncrc){
            printf("main config load success\n");    
            
        }else{
            printf("main config load error\n");
            ConfigReset(hManager);
            //printf("config reset\n");
            ConfigSave(MAIN_CFG_FILE_NAME, hManager);
        }  
    } 
//    debug_dump(hManager, sizeof(MAINCONFIG_S), "appp::main_config_init_f:");
//    debug_dump(&(hManager->Update), sizeof(struct tagUpdateConfigS), "appp::Update:");
    //printf("host_ip=%x\n", hManager->Global.host_ip);
    //printf("need_update_flag=%x\n", hManager->Update.need_update_flag);
    return OK_T;
}

int32_t main_config_release_f(void_t)
{
    printf("main_config_release_f\n");
    //if(hManager)
    //    free(hManager);
    //if (hMutex)
    //    mutex_destroy_f(hMutex);
    //hMutex = NULL;
    hManager = NULL;
    return OK_T;
}

////// crc为什么计算不正确 ?  ?
int32_t main_config_read_gloabl_f(uint8_t *pData, uint32_t len)
{
    uint8_t err = 0; 
    struct tagGlobalConfigS gloabl_cfg2;
    u16_t ncrc;

    int32_t nret = ERROR_T;
    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    OSMutexPend(hManager->hMutex,0,&err); 
    if (ConfigLoad(MAIN_CFG_FILE_NAME, hManager) == ERROR_T) {
        ConfigReset(hManager);
        //printf("read ConfigLoad ok\n");
        ConfigSave(MAIN_CFG_FILE_NAME, hManager);
		/**
		  * LEAD TO DEADLOCK WITHOUT IT !!!
		  */
	    OSMutexPost(hManager->hMutex);  
        return ERROR_T;
    }
    //printf("main_config_read_gloabl_f::hManager=%x\n", hManager);
    //printf("hManager->Global::ip=%x\n",hManager->Global.host_ip);
    memcpy(&gloabl_cfg2, &hManager->Global, sizeof(struct tagGlobalConfigS));
    //memcpy(pData, (u8_t *)&hManager->Global, sizeof(struct tagGlobalConfigS));
    
    //debug_dump(&gloabl_cfg2, sizeof(struct tagGlobalConfigS), "main_config_read_gloabl_f:");
    ncrc = crc16_calc_f(&gloabl_cfg2, sizeof(struct tagGlobalConfigS)-2);
//    printf("mac[0]=%x\n", hManager->Global.mac[0]);
    //printf("crc=%x,gloabl_cfg.crc=%x\n", ncrc, gloabl_cfg2.crc);
//    printf("pData=%x\n", pData);
    if(gloabl_cfg2.crc == ncrc){
        memcpy(pData, &gloabl_cfg2, len);
        nret = OK_T;
    }else{
        ConfigReset(hManager);
        memcpy(&gloabl_cfg2, &hManager->Global, sizeof(struct tagGlobalConfigS));
        memcpy(pData, &gloabl_cfg2, len);
    }
    
    OSMutexPost(hManager->hMutex);  
//    printf("nret=%x\n", nret);
    return nret;
}


int32_t main_config_write_gloabl_f(uint8_t *pData, uint32_t len)
{
    uint8_t err = 0; 
    u16_t ncrc;
    uint32_t       ret_b;
    struct tagGlobalConfigS gloabl_cfg1;
    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    OSMutexPend(hManager->hMutex,0,&err); 
    //printf("len=%x\n ",len);
    memcpy(&gloabl_cfg1, pData, len);
    //printf("sdflen=%x\n ",len);
    debug_dump(&gloabl_cfg1, sizeof(struct tagGlobalConfigS), "main_config_write_gloabl_f:");
    ncrc = crc16_calc_f(&gloabl_cfg1, len-2);
    gloabl_cfg1.crc = ncrc;
    //printf("main_config_write_gloabl_f::hManager=%x\n", hManager);
    //printf("ncrc=%x\n ",ncrc);
    memcpy((u8_t *)&hManager->Global, (u8_t *)&gloabl_cfg1, len);
    //printf("aaaahManager->Global.com1_baud=%x\n", hManager->Global.com1_baud);
    ConfigSave(MAIN_CFG_FILE_NAME, hManager);

    OSMutexPost(hManager->hMutex);  
    return OK_T;
}



int32_t main_config_read_device_f(uint8_t *pData, uint32_t len)
{
    uint32_t       ret_b;
    struct tagDeviceConfigS device_cfg;
    u16_t ncrc;

    int32_t nret = ERROR_T;
    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    if (ConfigLoad(MAIN_CFG_FILE_NAME, hManager) == ERROR_T) {
        ConfigReset(hManager);
        //printf("read ConfigLoad ok\n");
        ConfigSave(MAIN_CFG_FILE_NAME, hManager);
        return ERROR_T;
    }
    //printf("main_config_read_device_f::hManager=%x\n", hManager);
    //printf("hManager->Global::ip=%x\n",hManager->Global.host_ip);
    memcpy(&device_cfg, &hManager->Device, sizeof(struct tagDeviceConfigS));
    //memcpy(pData, &device_cfg, len);
//    //debug_dump(&gloabl_cfg, sizeof(struct tagDeviceConfigS), "main_config_read_gloabl_f:");
    ncrc = crc16_calc_f(&device_cfg, sizeof(struct tagDeviceConfigS)-2);
//    //printf("mac[0]=%x\n", hManager->Global.mac[0]);
    printf("crc=%x,device_cfg.crc=%x\n", ncrc, device_cfg.crc);
    if(device_cfg.crc == ncrc){
        memcpy(pData, &device_cfg, len);
        nret = OK_T;
    }else{
        ConfigReset(hManager);
        memcpy(&device_cfg, &hManager->Device, sizeof(struct tagDeviceConfigS));
        memcpy(pData, &device_cfg, len);
    }
    //printf("nret=%x\n", nret);
    //if (ret_b == OK_T)
    //    mutex_unlock_f(hMutex);
    //xSemaphoreGive( hManager->hMutex );
    //printf("bbbhManager->Global.com1_baud=%x\n", hManager->Global.com1_baud);
    return OK_T;
}


int32_t main_config_write_device_f(uint8_t *pData, uint32_t len)
{
    u16_t ncrc;
    uint32_t       ret_b;
    struct tagDeviceConfigS device_cfg;
    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    //printf("len=%x\n ",len);
    memcpy(&device_cfg, pData, len);
    //printf("sdflen=%x\n ",len);
    //debug_dump(&gloabl_cfg, sizeof(struct tagDeviceConfigS), "main_config_write_gloabl_f:");
    ncrc = crc16_calc_f(&device_cfg, len-2);
    device_cfg.crc = ncrc;
    //printf("main_config_write_gloabl_f::hManager=%x\n", hManager);
    //printf("len=%x\n ",ncrc);
    memcpy(&hManager->Device, &device_cfg, len);
    //printf("aaaahManager->Global.com1_baud=%x\n", hManager->Global.com1_baud);
    ConfigSave(MAIN_CFG_FILE_NAME, hManager);
//    if (ret_b == OK_T)
//        mutex_unlock_f(hMutex);
    //xSemaphoreGive( hManager->hMutex );
    return OK_T;
}





int32_t main_config_read_update_f(uint8_t *pData, uint32_t len)
{
    uint32_t       ret_b;
    struct tagUpdateConfigS update_cfg;
    u16_t ncrc;

    int32_t nret = ERROR_T;
    //ret_b = mutex_lock_f(hMutex, 500);
    //printf("rd1:hManager=%x\n", hManager);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    if (ConfigLoad(MAIN_CFG_FILE_NAME, hManager) == ERROR_T) {
        ConfigReset(hManager);
        //printf("read ConfigLoad ok\n");
        ConfigSave(MAIN_CFG_FILE_NAME, hManager);
        return ERROR_T;
    }
//    printf("rd2:pData=%x,hManager=%x\n", pData, hManager);
    //memcpy(pData, (u8_t *)&hManager->Update, sizeof(struct tagUpdateConfigS));
    memcpy(&update_cfg, (u8_t *)&hManager->Update, sizeof(struct tagUpdateConfigS));
    //printf("rd3:hManager=%x\n", hManager);
    ncrc = crc16_calc_f(&update_cfg, sizeof(struct tagUpdateConfigS)-2);

    printf("crc=%x,update_cfg.crc=%x\n", ncrc, update_cfg.crc);
    if(update_cfg.crc == ncrc){
        memcpy(pData, &update_cfg, len);
        nret = OK_T;
    }else{
        ConfigReset(hManager);
        memcpy(&update_cfg, &hManager->Update, sizeof(struct tagUpdateConfigS));
        memcpy(pData, &update_cfg, len);
    }
    //xSemaphoreGive( hManager->hMutex );
    //printf("bbbhManager->Global.com1_baud\n");
    return nret;
}


int32_t main_config_write_update_f(uint8_t *pData, uint32_t len)
{
    u16_t ncrc;
    uint32_t       ret_b;
    struct tagUpdateConfigS update_cfg;
    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    //printf("len=%x\n ",len);
    memcpy((u8_t *)&update_cfg, pData, len);
	//printf("wr1:hManager=%x,pData=%x\n", hManager, pData);
    //printf("sdflen=%x\n ",len);
    //debug_dump(&gloabl_cfg, sizeof(struct tagDeviceConfigS), "main_config_write_gloabl_f:");
    ncrc = crc16_calc_f(&update_cfg, len-2);
    update_cfg.crc = ncrc;
    //printf("main_config_write_gloabl_f::hManager=%x\n", hManager);
    //printf("len=%x\n ",ncrc);
    //printf("main_config_write_update_f len=%x, ncrc=%x\n ",len, ncrc);
    debug_printf(DBG_DEBUG,"main_config_write_update_f len=%x, ncrc=%x\r\n ",len, ncrc);
    memcpy((u8_t *)&hManager->Update,(u8_t *)&update_cfg, sizeof(struct tagUpdateConfigS));
    
    ConfigSave(MAIN_CFG_FILE_NAME, hManager);     //加上这个会死机
//    if (ret_b == OK_T)
//        mutex_unlock_f(hMutex);
    //xSemaphoreGive( hManager->hMutex );
    return OK_T;
}


int32_t main_config_save_f(MAINCONFIG_S *pConfig)
{
    uint32_t       ret_b;

    //ret_b = mutex_lock_f(hMutex, 500);
    //xSemaphoreTake( hManager->hMutex, portMAX_DELAY );
    ConfigSave(MAIN_CFG_FILE_NAME, pConfig);
    //if (ret_b == OK_T)
    //    mutex_unlock_f(hMutex);
    //xSemaphoreGive( hManager->hMutex );
    return OK_T;
}

void main_config_read_dev_ip(char * dst){
	uint8_t err;
	uint32_t uint32_ip;
	ip_addr_t ip_addr;
	OSMutexPend(hManager->hMutex,0,&err); 
	uint32_ip = hManager->Global.host_ip;
	OSMutexPost(hManager->hMutex);  
	ip_addr.addr = htonl(uint32_ip);
	inet_ntoa_r(ip_addr , dst , 32);
}

void main_config_read_dev_port(char * dst){
	uint8_t err;
	uint16_t port;
	OSMutexPend(hManager->hMutex,0,&err); 
	port = hManager->Global.host_port;
	OSMutexPost(hManager->hMutex);  
	sprintf(dst , "%d",port);
}
void main_config_read_dev_mask(char * dst){
	uint8_t err;
	uint32_t uint32_mask;
	ip_addr_t mask;
	OSMutexPend(hManager->hMutex,0,&err); 
	uint32_mask = hManager->Global.netmask;
	OSMutexPost(hManager->hMutex);  
	mask.addr = htonl(uint32_mask);
	inet_ntoa_r(mask , dst , 32);
}
void main_config_read_dev_gateway(char * dst){
	uint8_t err;
	uint32_t uint32_gateway;
	ip_addr_t gateway;
	OSMutexPend(hManager->hMutex,0,&err); 
	uint32_gateway = hManager->Global.gateway;
	OSMutexPost(hManager->hMutex);  
	gateway.addr = htonl(uint32_gateway);
	inet_ntoa_r(gateway , dst , 32);
}
void main_config_read_dev_mac(char * dst){
	uint8_t err;
	const uint8_t * p = hManager->Global.mac;
	
	OSMutexPend(hManager->hMutex,0,&err); 
	sprintf(dst,"%02X-%02X-%02X-%02X-%02X-%02X",
		p[0],p[1],p[2],p[3],p[4],p[5]);
	OSMutexPost(hManager->hMutex);  
}
void main_config_read_mng_ip(char * dst){
	uint8_t err;
	uint32_t uint32_ip;
	ip_addr_t ip_addr;
	OSMutexPend(hManager->hMutex,0,&err); 
	uint32_ip = hManager->Global.jump_ip;
	OSMutexPost(hManager->hMutex);  
	ip_addr.addr = htonl(uint32_ip);
	inet_ntoa_r(ip_addr , dst , 32);
}

void main_config_read_mng_port(char * dst){
	uint8_t err;
	uint16_t port;
	OSMutexPend(hManager->hMutex,0,&err); 
	port = hManager->Global.jump_port;
	OSMutexPost(hManager->hMutex);  
	sprintf(dst , "%d",port);
}

void main_config_read_v1(char * dst){
	u8 boot_v;
	boot_v = STMFLASH_ReadWord(FLASH_BOOT_VERSION_ADDR)>>24;
	version_ntoa(boot_v, dst);
}

void main_config_read_v2(char * dst){
	version_ntoa(ARM_VERSION, dst);
}


/*
configload过程：1、从文件中取得全部数据
2、CRC较验，无法通过CRC较验则返回错误
3、从数据中取得版本号
4、从数据中取得内容
*/
static int32_t ConfigLoad(uint8_t *filename, MAINCONFIG_S *pConfig)
{
    uint32_t               size;

    //  uint8_t                version[32];

    size = MainConfig_Size;
    //buf = malloc(size);
    //if (buf == NULL)
    //  return ERROR_T;

    if (ConfigReadSpiflash(filename, (uint8_t *)pConfig, size) == ERROR_T) {
        //  free(buf);
        return ERROR_T;
    }

	/**
	  * 2017.4.21
	  * 修正心跳端口错误
	  */
	if( (pConfig == hManager)&&(DEFAULT_JUMP_PORT != pConfig->Global.jump_port) )
	{
		pConfig->Global.jump_port = DEFAULT_JUMP_PORT;
		main_config_write_gloabl_f((uint8_t *)(&(pConfig->Global)),sizeof(struct tagGlobalConfigS));
		jump_update_udp_addr();
	}
	
	//ptr = buf;

    //if (ConfigCRCCheck(ptr, size) == ERROR_T) {
    //  free(buf);
    //  return ERROR_T;
    //}
    //size -= 4;

    /* get config version */
    //len = ConfigStringGet(version, ptr, size);
    //ptr += len;
    //size -= len;

    //len = ConfigMemeryGet((uint8_t *)pConfig, ptr, size);
    //if (len != size) {
    //  free(buf);
    //  return ERROR_T;
    //}

    //free(buf);
    return OK_T;
}

static int32_t ConfigSave(uint8_t *filename, MAINCONFIG_S *pConfig)
{
    //uint8_t              *buf, *ptr;
    //uint32_t             len, size;

    //buf = malloc(MainConfig_Size);
    //if (buf == NULL)
    //  return ERROR_T;
    //ptr = buf;

    //size = 0;

    //len = ConfigStringSet(MAIN_CONFIG_VERSION, ptr);
    //ptr += len;
    //size += len;

    //len = ConfigMemerySet((uint8_t *)pConfig, sizeof(MAIN_CONFIG_VERSION), ptr);
    //ptr += len;
    //size += len;

    //ConfigCRCSet(buf, size);
    //size += 4;
    if (pConfig == NULL){
        return ERROR_T;
    }
    //printf("mainconfig size =%x\n", MainConfig_Size);
    ConfigWriteSpiflash(filename, (uint8_t *)pConfig, MainConfig_Size);

    return OK_T;
}

#define DEFAULT_HOST_IP 0xc0a8019b
/*
*   Function :  ConfigReset
*   Describer:  config reset
*   Parameter:  filename
*               pConfig
*   Return   :  OK_T            success
*               ERROR_T         failed
*/
static int32_t ConfigReset(MAINCONFIG_S *pConfig)
{
    u16_t ncrc;
    memset(pConfig, 0x00, sizeof(MAINCONFIG_S));

    pConfig->Global.host_ip     = DEFAULT_HOST_IP;
	pConfig->Global.host_port       =  NM_TCP_SERVER_PORT;
    pConfig->Global.gateway     = 0xc0a80401;
    pConfig->Global.jump_ip     = 0xc0a80401;
	pConfig->Global.jump_port       =  DEFAULT_JUMP_PORT;
    pConfig->Global.trap_ip     =  0xc0a80401;
    pConfig->Global.trap_port   =  DEFAULT_JUMP_PORT;

    pConfig->Global.netmask     =  0xffffff00;
	//pConfig->Global.device_id   =  0;
	pConfig->Global.tserver_ip  =  0xc0a80401;
	//pConfig->Global.ftp_ip  =  0xc0a80401;
    pConfig->Global.mac[0]      =  0x2c;
    pConfig->Global.mac[1]      =  0x06;
    pConfig->Global.mac[2]      =  0x23;
    pConfig->Global.mac[3]      =  0x00;
    pConfig->Global.mac[4]      =  0x00;
    pConfig->Global.mac[5]      =  0x01;       
    
    pConfig->Global.arm_version = 0x20; 

    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    

    //pConfig->Device.baud = 5; 
    //pConfig->Device.stop_bits = 1;           

    ncrc = crc16_calc_f(&hManager->Device, sizeof(struct tagDeviceConfigS)-2);  
    hManager->Device.crc = ncrc;   

    pConfig->Update.update_size = 0; 
    pConfig->Update.need_update_flag = 0;    
    pConfig->Update.boot_just_update_flag = 0; 
    pConfig->Update.boot_update_flash_powerdown0 = 0;    
    pConfig->Update.boot_update_flash_powerdown1 = 0;
    ncrc = crc16_calc_f(&hManager->Update, sizeof(struct tagUpdateConfigS)-2);  
    hManager->Update.crc = ncrc;

    return OK_T;
}
/*
更新main配置模块配置数据句柄
*/
int32_t main_config_refresh_h_main(void)
{
    if (ConfigLoad(MAIN_CFG_FILE_NAME, hManager) == ERROR_T) {
        ConfigReset(hManager);

        ConfigSave(MAIN_CFG_FILE_NAME, hManager);
        return ERROR_T;
    }
    return OK_T;
}
/*
取得main配置模块句柄
*/
void * main_config_get_h_main(void)
{
    return (void *)hManager;      
}


uint32_t main_config_get_main_host_ip(void_t)
{
    //uint32_t nret;
    //printf("dsfsdfs\n");  
    //printf("eeehManager=%x\n", hManager); 
    //if(hManager){
    //  printf("hManager=%x\n", hManager);  
    //}else{
    //main_config_init_f();
    //}
    //nret = hManager->Global.host_ip;
    //printf("get::host_ip=%x\n",hManager->Global.host_ip);

    return hManager->Global.host_ip;
}

uint32_t main_config_get_main_host_port(void_t)
{
    //uint32_t nret;
    //printf("dsfsdfs\n");  
    //printf("eeehManager=%x\n", hManager); 
    //if(hManager){
    //  printf("hManager=%x\n", hManager);  
    //}else{
    //main_config_init_f();
    //}
    //nret = hManager->Global.host_ip;
    //printf("get::host_ip=%x\n",hManager->Global.host_ip);

    return hManager->Global.host_port;
}

int32_t main_config_set_main_host_ip(uint32_t ip)
{
    u16_t ncrc;
    hManager->Global.host_ip = ip;
    //printf("main_config_set_main_host_ip::host_ip=%x\n",hManager->Global.host_ip);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_host_ip:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    
    ConfigSave(MAIN_CFG_FILE_NAME, hManager);
    //printf("set::host_ip=%x\n",hManager->Global.host_ip);
    return OK_T;
}


int32_t main_config_set_main_jump_ip(uint32_t ip)
{
    u16_t ncrc;
    hManager->Global.jump_ip = ip;
    //printf("main_config_set_main_host_ip::host_ip=%x\n",hManager->Global.host_ip);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_host_ip:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    
    ConfigSave(MAIN_CFG_FILE_NAME, hManager);
    //printf("set::host_ip=%x\n",hManager->Global.host_ip);
    return OK_T;
}

uint32_t main_config_get_main_host_mask(void_t)
{
    //uint32_t nret;
    //printf("dsfsdfs\n");  
    //printf("yyyhManager=%x\n", hManager); 
    //if(hManager){
    //  printf("hManager=%x\n", hManager);  
    //}else{
    //main_config_init_f();
    //}
    //nret = hManager->Global.netmask;
    //printf("nret=%x\n",nret);
    return hManager->Global.netmask;
}

int32_t main_config_set_main_host_mask(uint32_t netmask)
{
    u16_t ncrc;

    hManager->Global.netmask = netmask;
    //printf("p_mac[0]=%x\n", p_mac[0]);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_host_mask:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    

    ConfigSave(MAIN_CFG_FILE_NAME, hManager);  
  	return OK_T;
}


uint32_t main_config_get_main_gateway(void_t)
{
    //uint32_t nret;
    //printf("uuuhManager=%x\n", hManager);
    //if(hManager){
    //  printf("hManager=%x\n", hManager);  
    //}else{
    //  //main_config_init_f();
    //}
    //nret = hManager->Global.gateway;
    //printf("nret=%x\n",nret);
    return hManager->Global.gateway;
}

int32_t main_config_set_main_gateway(uint32_t gateway)
{
    u16_t ncrc;

    hManager->Global.gateway = gateway;
    //printf("p_mac[0]=%x\n", p_mac[0]);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_gateway:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    

    ConfigSave(MAIN_CFG_FILE_NAME, hManager);  
  	return OK_T;
}

uint8_t * main_config_get_main_mac(void_t)
{
    //printf("kkkhManager=%x\n", hManager);
    //if(hManager){
    //printf("hManager=%x\n", hManager);    
    //}else{
    //main_config_init_f();
    //}
//    printf("p_mac[0]=%x\n", hManager->Global.mac[0]);
//    printf("p_mac[1]=%x\n", hManager->Global.mac[1]);
//    printf("p_mac[2]=%x\n", hManager->Global.mac[2]);
//    printf("p_mac[3]=%x\n", hManager->Global.mac[3]);
    return hManager->Global.mac;
}


int32_t main_config_set_main_mac(u8_t * p_mac)
{
    u16_t ncrc;

    memcpy(hManager->Global.mac, p_mac, sizeof(hManager->Global.mac));
//    printf("p_mac[0]=%x\n", hManager->Global.mac[0]);
//    printf("p_mac[1]=%x\n", hManager->Global.mac[1]);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_mac:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    

    ConfigSave(MAIN_CFG_FILE_NAME, hManager);
    return OK_T;
}

//trap地址就是为jump地址，原trap地址暂时没有用
int32_t main_config_get_trap(uint32_t * p_trap_ip, uint32_t * p_trap_port)
{
    //int i;
    //p_trap_ip[0] = hManager->Global.trap_ip;
    //p_trap_port[0] = hManager->Global.trap_port;
    *p_trap_ip = hManager->Global.jump_ip;
    *p_trap_port = hManager->Global.jump_port;
    return OK_T;

}



int32_t main_config_get_jump(uint32_t * p_jump_ip, uint32_t * p_jump_port)
{
    //int i;
    *p_jump_ip = hManager->Global.jump_ip;
    *p_jump_port = hManager->Global.jump_port;
    return OK_T;
}
uint32_t main_config_get_jump_ip(void)
{
    return hManager->Global.jump_ip;
}

uint32_t main_config_get_jump_port(void)
{
    return hManager->Global.jump_port;
}

int32_t main_config_get_send_st(uint32_t * p_st_ip, uint32_t * p_st_port)
{
    //int i;
    *p_st_ip = hManager->Global.jump_ip;
    *p_st_port = UDP_ST_SEND_PORT;
    return OK_T;
}



int32_t main_config_get_recv_st(uint32_t * p_st_ip, uint32_t * p_st_port)
{
    //int i;
    *p_st_ip = hManager->Global.jump_ip;
    *p_st_port = UDP_ST_RECV_PORT;
    return OK_T;
}


int32_t main_config_get_ftp(uint32_t * p_ftp_ip)
{
    //int i;
    //*p_ftp_ip = hManager->Global.ftp_ip;

    return OK_T;

}



uint32_t main_config_get_main_device_id(void_t)
{

    return hManager->Global.device_id;
}

uint32_t main_config_get_bootversion(void){
	return STMFLASH_ReadWord(FLASH_BOOT_VERSION_ADDR)>>24;
}

uint32_t main_config_get_appversion(void){
	//return STMFLASH_ReadWord(FLASH_BOOT_VERSION_ADDR)>>24;
	return ARM_VERSION;
}


int32_t main_config_set_main_baud(uint32_t baud)
{
    u16_t ncrc;

//    hManager->Global.baud = baud;
    //printf("p_mac[0]=%x\n", p_mac[0]);
    //debug_dump(&hManager->Global, sizeof(struct tagGlobalConfigS), "main_config_set_main_gateway:");
    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
    hManager->Global.crc = ncrc;    

    ConfigSave(MAIN_CFG_FILE_NAME, hManager);  
  	return OK_T;
}

//uint32_t main_config_set_main_device_id(uint32_t deviceid)
//{
//    u16_t ncrc;
//    hManager->Global.device_id = deviceid ;
//
//    ncrc = crc16_calc_f(&hManager->Global, sizeof(struct tagGlobalConfigS)-2);  
//    hManager->Global.crc = ncrc;    
//    ConfigSave(MAIN_CFG_FILE_NAME, hManager);
//    return OK_T;
//}


//baudrate:: 1:1200,2:2400,3:4800,4:9600:5:19200,6:38400,7:57600,8:115200,9:230400
uint32_t main_config_get_device_baudrate(void_t)
{
    uint32_t nret = 19200;
    switch(hManager->Device.baud){
        case 1:
            nret = 1200;
            break;
        case 2:
            nret = 2400; 
            break;   
        case 3:
            nret = 4800;  
            break; 
        case 4:
            nret = 9600;
            break;
        case 5:
            nret = 19200; 
            break;   
        case 6:
            nret = 38400;  
            break; 
        case 7:
            nret = 57600;  
            break;   
        case 8:
            nret = 115200;   
            break;     
        case 9:
            nret = 230400;  
            break; 
        default:
            nret = 19200;   
            break;    
    }  
	nret = 115200;   		//////
    return nret;
}

uint32_t main_config_set_main_baudrate(uint32_t baudrate)
{
    u16_t ncrc;
    uint32_t nret = 0;
    switch(baudrate){
        case 1200:
            nret = 1;
            break;
        case 2400:
            nret = 2; 
            break;   
        case 4800:
            nret = 3;  
            break; 
        case 9600:
            nret = 4;
            break;
        case 19200:
            nret = 5; 
            break;   
        case 38400:
            nret = 6;  
            break; 
        case 57600:
            nret = 7;  
            break;   
        case 115200:
            nret = 8;   
            break;     
        case 230400:
            nret = 9;  
            break; 
        default:
            nret = 0;
            break;    
    }
    if(nret > 0){
        hManager->Device.baud = nret;
        ncrc = crc16_calc_f(&hManager->Device, sizeof(struct tagDeviceConfigS)-2);  
        hManager->Device.crc = ncrc;    
    
        ConfigSave(MAIN_CFG_FILE_NAME, hManager);   
        return OK_T;
    }else{
        return ERROR_T;
    }
}

void web_account_write(struct web_account * acct){
	INT8U err;
	acct->crc = crc16_calc_f((s8_t*)acct,(u32_t)&((struct web_account*)NULL)->crc);
    OSMutexPend(hManager->hMutex,0,&err); 
	SPI_Flash_Write((u8*)acct,WEB_ACCOUNT_PASSWD_ADDR,sizeof(struct web_account));
	//ConfigSave(MAIN_CFG_FILE_NAME, hManager);
    OSMutexPost(hManager->hMutex);  
}

int web_account_read(struct web_account * acct){
	INT8U err;
	u16 crc;
	struct web_account tmp;
    OSMutexPend(hManager->hMutex,0,&err); 
	SPI_Flash_Read((u8*)&tmp,WEB_ACCOUNT_PASSWD_ADDR,sizeof(struct web_account));
    OSMutexPost(hManager->hMutex);  
	crc = crc16_calc_f((s8_t*)&tmp,(u32_t)&((struct web_account*)NULL)->crc);
	if(crc == tmp.crc){
		memcpy(acct , &tmp , sizeof(tmp));
		return 0;
	}
	else{
		printf("ACCT: web account was reset(admin,admin)!\r\n");
		strcpy(acct->username , "admin");
		strcpy(acct->passwd , "admin");
		web_account_write(acct);
		return -1;
	}
}

