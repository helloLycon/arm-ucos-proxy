
#include <string.h>
#include "user_iap.h"



#define   OPEN_CORE_INT()     __enable_irq()
#define   CLOSE_CORE_INT()    __disable_irq()

/* 定义IAP命令字 */ 
                                    //   命令           参数 
#define     IAP_INIT            49  // 初始化IAP    【无】
#define     IAP_SELECTOR        50  // 选择扇区     【起始扇区号、结束扇区号、bank号】 
#define     IAP_RAMTOFLASH      51  // 拷贝数据     【FLASH目标地址、RAM源地址、写入字节数、系统时钟频率】 
#define     IAP_ERASESECTOR     52  // 擦除扇区     【起始扇区号、结束扇区号、系统时钟频率】 
#define     IAP_BLANKCHK        53  // 查空扇区     【起始扇区号、结束扇区号】 
#define     IAP_READPARTID      54  // 读器件ID     【无】 
#define     IAP_BOOTCODEID      55  // 读Boot版本号 【无】 
#define     IAP_COMPARE         56  // 比较命令     【Flash起始地址、RAM起始地址、需要比较的字节数】 
 

//#define     IAP_ERROR            -1

/*********************************************************************************************************
** define variable related to iap
*********************************************************************************************************/
typedef void (* IAP) (uint32_t *, uint32_t *);
#define IAP_LOCATION	*(volatile unsigned int *)0x10400100
static IAP iap_entry;


static uint32_t  paramin[5] = {100};                         // IAP入口参数缓冲区    
static uint32_t  paramout[3] = {100};                        // IAP出口参数缓冲区

static uint32_t gSecStart = 0;
static uint32_t gSecEnd   = 0;

/*  
*********************************************************************************************************  
** 函数名称：init_iap()  
** 函数功能：IAP初始化，命令代码49。  
** 入口参数：无 
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t init_iap(void)   
{	
    paramin[0] = IAP_INIT;               // 设置命令字    
    iap_entry(paramin, paramout);           // 调用IAP服务程序   
      
    return(paramout[0]);                     // 返回状态码    
}  

/*  
*********************************************************************************************************  
** 函数名称：pre_sector()  
** 函数功能：IAP操作扇区选择，命令代码50。  
** 入口参数：sec1        起始扇区  
**           sec2        终止扇区  
**           bank    0 = bank A, 1 = bank B
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t pre_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{
    paramin[0] = IAP_SELECTOR;               // 设置命令字    
    paramin[1] = sec1;                       // 设置参数    
    paramin[2] = sec2;   
	paramin[3] = bank; 
    iap_entry(paramin, paramout);           // 调用IAP服务程序   
      
    return(paramout[0]);                     // 返回状态码    
}  


#define IAP_FCCLK	  180000 
/*  
*******************************************************************************************************  
** 函数名称：copy_ram_to_flash()  
** 函数功能：复制RAM的数据到FLASH，命令代码51。  
** 入口参数：dst        目标地址，即FLASH起始地址。以512字节为分界  
**           src        源地址，即RAM地址。地址必须字对齐  
**           no         复制字节个数，为512/1024/4096 
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
static uint32_t  copy_ram_to_flash(uint32_t dst, uint32_t src, uint32_t no)   
{     
    paramin[0] = IAP_RAMTOFLASH;             // 设置命令字    
    paramin[1] = dst;                        // 设置参数    
    paramin[2] = src;   
    paramin[3] = no;   
    paramin[4] = IAP_FCCLK;   
    iap_entry(paramin, paramout);           // 调用IAP服务程序    
       
    return(paramout[0]);                     // 返回状态码    
} 


/*  
*******************************************************************************************************  
** 函数名称：erase_sector()  
** 函数功能：扇区擦除，命令代码52。  
** 入口参数：sec1       起始扇区  
**           sec2       终止扇区  
**           bank    0 = bank A, 1 = bank B
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t erase_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{     
    paramin[0] = IAP_ERASESECTOR;            // 设置命令字    
    paramin[1] = sec1;                       // 设置参数    
    paramin[2] = sec2;   
    paramin[3] = IAP_FCCLK;   
	paramin[4] = bank; 
    iap_entry(paramin, paramout);           // 调用IAP服务程序    
      
    return(paramout[0]);                     // 返回状态码    
}  

/*  
*******************************************************************************************************  
** 函数名称：blank_check_sector()  
** 函数功能：扇区查空，命令代码53。  
** 入口参数：sec1       起始扇区  
**           sec2       终止扇区  
**           bank    0 = bank A, 1 = bank B
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t  blank_check_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{     
    paramin[0] = IAP_BLANKCHK;               // 设置命令字    
    paramin[1] = sec1;                       // 设置参数    
    paramin[2] = sec2;   
	paramin[3] = bank;
    iap_entry(paramin, paramout);           // 调用IAP服务程序    
   
    return(paramout[0]);                     // 返回状态码    
} 

/*  
*******************************************************************************************************  
** 函数名称：compare()  
** 函数功能：校验数据，命令代码56。  
** 入口参数：dst        目标地址，即RAM/FLASH起始地址。地址必须字对齐  
**           src        源地址，即FLASH/RAM地址。地址必须字对齐  
**           no         复制字节个数，必须能被4整除  
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t  compare(uint32_t dst, uint32_t src, uint32_t no)   
{
	CLOSE_CORE_INT();	     
    paramin[0] = IAP_COMPARE;                // 设置命令字    
    paramin[1] = dst;                        // 设置参数    
    paramin[2] = src;   
    paramin[3] = no;   
    iap_entry(paramin, paramout);           // 调用IAP服务程序       
	OPEN_CORE_INT();

    return(paramout[0]);                     // 返回状态码    
} 

/*  
*******************************************************************************************************  
** 函数名称：compare()  
** 函数功能：校验数据，命令代码56。  
** 入口参数：dst        目标地址，即RAM/FLASH起始地址。地址必须字对齐  
**           src        源地址，即FLASH/RAM地址。地址必须字对齐  
**           no         复制字节个数，必须能被4整除  
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/   
uint32_t  iap_compare(uint32_t dst, uint8_t *src, uint32_t no)   
{
    return compare(dst, (uint32_t)src, no);                         
} 


static uint8_t g_bank_num = 0;

void iap_init (uint8_t bank)
{
	if (bank > 1)
	{
		bank = 1;
	}
	
	g_bank_num = bank;
	iap_entry = (IAP)IAP_LOCATION;
	
	init_iap();
}

/*  
*******************************************************************************************************  
** 函数名称：iap_erase_flash()  
** 函数功能：flash 擦除操作  
** 入口参数：addrDst       要擦除flash的起始地址  
**           length        要擦除flash的长度   
** 出口参数：IAP操作状态码  
**           IAP返回值(paramout缓冲区)  
*******************************************************************************************************  
*/ 
uint32_t iap_erase_flash(uint32_t addrDst, uint32_t length)
{
	uint32_t state;
	uint32_t baseAddr;
	
	if (length == 0)
	{
		return 101;
	}
	
	if (g_bank_num)
	{
		baseAddr = 0x1B000000;
	} else
	{
		baseAddr = 0x1A000000;
	}
	
	if ((addrDst < baseAddr) || ((addrDst + length) > (0x7FFFF + baseAddr)))
	{
		return 100;
	}

	// 计算起始扇区号
	if (addrDst < (0x10000 + baseAddr))
	{
		gSecStart = (addrDst - baseAddr) / (8 << 10);
		if ((addrDst + length) < (0x10000 + baseAddr))
		{
			gSecEnd = (addrDst - baseAddr + length) / (8 << 10);
		} else
		{
			gSecEnd = 8 + (addrDst - baseAddr - 0x10000 + length) / (64 << 10);
		}
	} else
	{
		gSecStart = 8 + (addrDst - baseAddr - 0x10000) / (64 << 10);
		gSecEnd = 8 + (addrDst - baseAddr - 0x10000 + length) / (64 << 10);
	}

//	debugPrint("bank %d, len %d,sector start %d, end %d\r\n", g_bank_num, length, gSecStart, gSecEnd);
		 
	CLOSE_CORE_INT();	

// 	if (g_bank_num)
// 	{
// 		state = pre_sector(gSecStart, gSecEnd, 1);
// 	} else
// 	{
// 		state = pre_sector(gSecStart, gSecEnd, 0);
// 	}
	state = pre_sector(gSecStart, gSecEnd, g_bank_num);

	if (state == CMD_SUCCESS) 
	{
// 		if (g_bank_num)
// 		{
// 			state =  erase_sector(gSecStart, gSecEnd, 1);	
// 		} else 
// 		{
// 			state =  erase_sector(gSecStart, gSecEnd, 0);	
// 		}	
		state =  erase_sector(gSecStart, gSecEnd, g_bank_num);		
	
		if (state == CMD_SUCCESS) 
		{
// 			if (g_bank_num)
// 			{
// 				state = blank_check_sector(gSecStart, gSecEnd, 1);
// 			} else
// 			{
// 				state = blank_check_sector(gSecStart, gSecEnd, 0);
// 			}
			state = blank_check_sector(gSecStart, gSecEnd, g_bank_num);
		}  
	}		   
	OPEN_CORE_INT();

	return state;
}

/*  
*******************************************************************************************************  
** 函数名称：iap_write_flash()  
** 函数功能：flash 写操作  
** 入口参数：addrDst        写入flash的数据的目标地址  
**           addrSrc        要写入flash的数据的源地址 
**           length         数据长度
** 出口参数：写操作状态码   
*******************************************************************************************************  
*/ 
uint32_t iap_write_flash(uint32_t addrDst, uint8_t *addrSrc, uint32_t length)
{
	uint32_t state;
	uint8_t  lastBuffer[512];

	uint32_t remainder = length;
	uint32_t offset    = 0;

	/*
	 *	参数过滤
	 */
	if (length == 0)
	{
		return 100;
	}

	if (addrDst % 256 != 0)
	{
		return 101;
	}

	if (addrDst < 64)
	{
		return 102;
	}

	if ((uint32_t)addrSrc % 4 != 0)
	{
		return 103;
	}

    /*
	 *  每次都写512字节，不够用0xff填充
	 */
	while (remainder > 0)
	{
		CLOSE_CORE_INT();
// 		if (g_bank_num)
// 		{
// 			state = pre_sector(gSecStart, gSecEnd, 1);
// 		} else
// 		{
// 			state = pre_sector(gSecStart, gSecEnd, 0);
// 		}
		state = pre_sector(gSecStart, gSecEnd, g_bank_num);
		if (state != CMD_SUCCESS) 
		{
			OPEN_CORE_INT();
			return state;
		}

		if (remainder >= 512)
		{
			state = copy_ram_to_flash(addrDst+offset, (uint32_t)(addrSrc+offset), 512);
		} else
		{
			memcpy(lastBuffer, addrSrc+offset, remainder);
			memset(lastBuffer+remainder, 0xff, 512-remainder);
			state = copy_ram_to_flash(addrDst+offset, (uint32_t)lastBuffer, 512);
		}
		OPEN_CORE_INT();

		if (state != CMD_SUCCESS) 
		{
			return state;
		}

		offset += 512;
		if (remainder >= 512)
		{
			remainder -= 512;
		} else
		{
			remainder = 0;
		}
	}

	return state;
}


/*  
*******************************************************************************************************  
** 函数名称：iap_write_flash()  
** 函数功能：flash 写操作  
** 入口参数：addrDst        写保存的数据的目标地址  
**           addrSrc        要保存数据的源地址 
**           length         数据长度
** 出口参数：读取长度   
*******************************************************************************************************  
*/ 
uint32_t iap_read_flash(uint8_t *addrDst, uint8_t *addrSrc, uint32_t length)
{
	uint32_t i = 0;

	 if ((addrDst == NULL) || (addrSrc == NULL) || (length == 0))
	 {
	 	return 0;
	 }

	 while (length--)
	 {
	 	addrDst[i] = addrSrc[i];
		i++;
	 }

	 return i;
}

/*********************************************************************************************************
** the end
*********************************************************************************************************/



