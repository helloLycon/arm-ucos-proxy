
#include <string.h>
#include "user_iap.h"



#define   OPEN_CORE_INT()     __enable_irq()
#define   CLOSE_CORE_INT()    __disable_irq()

/* ����IAP������ */ 
                                    //   ����           ���� 
#define     IAP_INIT            49  // ��ʼ��IAP    ���ޡ�
#define     IAP_SELECTOR        50  // ѡ������     ����ʼ�����š����������š�bank�š� 
#define     IAP_RAMTOFLASH      51  // ��������     ��FLASHĿ���ַ��RAMԴ��ַ��д���ֽ�����ϵͳʱ��Ƶ�ʡ� 
#define     IAP_ERASESECTOR     52  // ��������     ����ʼ�����š����������š�ϵͳʱ��Ƶ�ʡ� 
#define     IAP_BLANKCHK        53  // �������     ����ʼ�����š����������š� 
#define     IAP_READPARTID      54  // ������ID     ���ޡ� 
#define     IAP_BOOTCODEID      55  // ��Boot�汾�� ���ޡ� 
#define     IAP_COMPARE         56  // �Ƚ�����     ��Flash��ʼ��ַ��RAM��ʼ��ַ����Ҫ�Ƚϵ��ֽ����� 
 

//#define     IAP_ERROR            -1

/*********************************************************************************************************
** define variable related to iap
*********************************************************************************************************/
typedef void (* IAP) (uint32_t *, uint32_t *);
#define IAP_LOCATION	*(volatile unsigned int *)0x10400100
static IAP iap_entry;


static uint32_t  paramin[5] = {100};                         // IAP��ڲ���������    
static uint32_t  paramout[3] = {100};                        // IAP���ڲ���������

static uint32_t gSecStart = 0;
static uint32_t gSecEnd   = 0;

/*  
*********************************************************************************************************  
** �������ƣ�init_iap()  
** �������ܣ�IAP��ʼ�����������49��  
** ��ڲ������� 
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
uint32_t init_iap(void)   
{	
    paramin[0] = IAP_INIT;               // ����������    
    iap_entry(paramin, paramout);           // ����IAP�������   
      
    return(paramout[0]);                     // ����״̬��    
}  

/*  
*********************************************************************************************************  
** �������ƣ�pre_sector()  
** �������ܣ�IAP��������ѡ���������50��  
** ��ڲ�����sec1        ��ʼ����  
**           sec2        ��ֹ����  
**           bank    0 = bank A, 1 = bank B
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
uint32_t pre_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{
    paramin[0] = IAP_SELECTOR;               // ����������    
    paramin[1] = sec1;                       // ���ò���    
    paramin[2] = sec2;   
	paramin[3] = bank; 
    iap_entry(paramin, paramout);           // ����IAP�������   
      
    return(paramout[0]);                     // ����״̬��    
}  


#define IAP_FCCLK	  180000 
/*  
*******************************************************************************************************  
** �������ƣ�copy_ram_to_flash()  
** �������ܣ�����RAM�����ݵ�FLASH���������51��  
** ��ڲ�����dst        Ŀ���ַ����FLASH��ʼ��ַ����512�ֽ�Ϊ�ֽ�  
**           src        Դ��ַ����RAM��ַ����ַ�����ֶ���  
**           no         �����ֽڸ�����Ϊ512/1024/4096 
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
static uint32_t  copy_ram_to_flash(uint32_t dst, uint32_t src, uint32_t no)   
{     
    paramin[0] = IAP_RAMTOFLASH;             // ����������    
    paramin[1] = dst;                        // ���ò���    
    paramin[2] = src;   
    paramin[3] = no;   
    paramin[4] = IAP_FCCLK;   
    iap_entry(paramin, paramout);           // ����IAP�������    
       
    return(paramout[0]);                     // ����״̬��    
} 


/*  
*******************************************************************************************************  
** �������ƣ�erase_sector()  
** �������ܣ������������������52��  
** ��ڲ�����sec1       ��ʼ����  
**           sec2       ��ֹ����  
**           bank    0 = bank A, 1 = bank B
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
uint32_t erase_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{     
    paramin[0] = IAP_ERASESECTOR;            // ����������    
    paramin[1] = sec1;                       // ���ò���    
    paramin[2] = sec2;   
    paramin[3] = IAP_FCCLK;   
	paramin[4] = bank; 
    iap_entry(paramin, paramout);           // ����IAP�������    
      
    return(paramout[0]);                     // ����״̬��    
}  

/*  
*******************************************************************************************************  
** �������ƣ�blank_check_sector()  
** �������ܣ�������գ��������53��  
** ��ڲ�����sec1       ��ʼ����  
**           sec2       ��ֹ����  
**           bank    0 = bank A, 1 = bank B
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
uint32_t  blank_check_sector(uint32_t sec1, uint32_t sec2, uint8_t bank)   
{     
    paramin[0] = IAP_BLANKCHK;               // ����������    
    paramin[1] = sec1;                       // ���ò���    
    paramin[2] = sec2;   
	paramin[3] = bank;
    iap_entry(paramin, paramout);           // ����IAP�������    
   
    return(paramout[0]);                     // ����״̬��    
} 

/*  
*******************************************************************************************************  
** �������ƣ�compare()  
** �������ܣ�У�����ݣ��������56��  
** ��ڲ�����dst        Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���  
**           src        Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���  
**           no         �����ֽڸ����������ܱ�4����  
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
*******************************************************************************************************  
*/   
uint32_t  compare(uint32_t dst, uint32_t src, uint32_t no)   
{
	CLOSE_CORE_INT();	     
    paramin[0] = IAP_COMPARE;                // ����������    
    paramin[1] = dst;                        // ���ò���    
    paramin[2] = src;   
    paramin[3] = no;   
    iap_entry(paramin, paramout);           // ����IAP�������       
	OPEN_CORE_INT();

    return(paramout[0]);                     // ����״̬��    
} 

/*  
*******************************************************************************************************  
** �������ƣ�compare()  
** �������ܣ�У�����ݣ��������56��  
** ��ڲ�����dst        Ŀ���ַ����RAM/FLASH��ʼ��ַ����ַ�����ֶ���  
**           src        Դ��ַ����FLASH/RAM��ַ����ַ�����ֶ���  
**           no         �����ֽڸ����������ܱ�4����  
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
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
** �������ƣ�iap_erase_flash()  
** �������ܣ�flash ��������  
** ��ڲ�����addrDst       Ҫ����flash����ʼ��ַ  
**           length        Ҫ����flash�ĳ���   
** ���ڲ�����IAP����״̬��  
**           IAP����ֵ(paramout������)  
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

	// ������ʼ������
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
** �������ƣ�iap_write_flash()  
** �������ܣ�flash д����  
** ��ڲ�����addrDst        д��flash�����ݵ�Ŀ���ַ  
**           addrSrc        Ҫд��flash�����ݵ�Դ��ַ 
**           length         ���ݳ���
** ���ڲ�����д����״̬��   
*******************************************************************************************************  
*/ 
uint32_t iap_write_flash(uint32_t addrDst, uint8_t *addrSrc, uint32_t length)
{
	uint32_t state;
	uint8_t  lastBuffer[512];

	uint32_t remainder = length;
	uint32_t offset    = 0;

	/*
	 *	��������
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
	 *  ÿ�ζ�д512�ֽڣ�������0xff���
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
** �������ƣ�iap_write_flash()  
** �������ܣ�flash д����  
** ��ڲ�����addrDst        д��������ݵ�Ŀ���ַ  
**           addrSrc        Ҫ�������ݵ�Դ��ַ 
**           length         ���ݳ���
** ���ڲ�������ȡ����   
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



