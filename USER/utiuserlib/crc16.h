/*************************************************  
  File name:         crc16.h
  Author:            txli
  Version:           1.0.0 
  Created Date:      20121205
  Description:       16位CRC校验模块
  Others:             
  History:
    1. Date:         20121205       
       Author:       txli  
       Modification: 起始版本
    2. 
*************************************************/

#ifndef _CRC16_H_
#define _CRC16_H_

#include "types.h"
#include "cc.h"
//#include "board.h"
//#include "arch/lpc18xx_43xx_emac.h"
//#include "arch/lpc_arch.h"
//#include "arch/sys_arch.h"



#define CRC16_VALUE_SIZE 2


/*************************************************                                                     
  Function:        crc16_calc_f                                                                        
  Description:     计算crc16值                                                                 
  Calls:           无                                                                                  
  Input:        	                                                                                     
                   s8_t *buf  : 源数据指针                                                                 
                   u32_t size : 数据长度                                                                   
  Output:          无                                                                                  
  Return:                                                                                              
                   u16_t crc16: 计算结果                                                               
  Others:          无                                                                                  
*************************************************/   
u16_t crc16_calc_f(s8_t *buf, u32_t size);

/*************************************************                                                     
  Function:        crc16_pack_f                                                                        
  Description:     对帧数据中的0x7e转化成0x7d操作                                                                 
  Calls:           无                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : 数据指针                                                                 
                   s32_t *p_len  : 数据长度                                                                   
  Output:          
                   u8_t *p_data  : 数据指针      
                   s32_t *p_len  : 数据长度                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          无                                                                                  
*************************************************/ 
status_t crc16_pack_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        crc16_unpack_f                                                                        
  Description:     对帧数据中的0x7e转化成0x7d操作的反运算                                                                 
  Calls:           无                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : 数据指针                                                                 
                   s32_t *p_len  : 数据长度                                                                   
  Output:          
                   u8_t *p_data  : 数据指针      
                   s32_t *p_len  : 数据长度                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          无                                                                                  
*************************************************/  
status_t crc16_unpack_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        buff_add_crc16_f                                                                        
  Description:     将crc16计算结果加入到帧数据的末尾                                                                
  Calls:           
                   u16_t crc16_calc_f(s8_t *buf, u32_t size)                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : 数据指针                                                                 
                   s32_t *p_len  : 数据长度                                                                   
  Output:          
                   u8_t *p_data  : 数据指针      
                   s32_t *p_len  : 数据长度                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          无                                                                                  
*************************************************/ 
status_t buff_add_crc16_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        buff_dec_crc16_f                                                                        
  Description:     将crc16长度从帧数据的长度中去除                                                                
  Calls:           无                   
  Input:        	                                                                 
                   s32_t *p_len  : 数据长度                                                                   
  Output:          
                   s32_t *p_len  : 数据长度                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          无                                                                                  
*************************************************/ 
status_t buff_dec_crc16_f(s32_t *p_len);


unsigned int  hj15_crc_7e_notxor_20(unsigned char  * pdata, int nlen,unsigned char *p_crc);
unsigned int hj_calc_crc(unsigned char  * pdata, int nlen);
//u16_t crc16_calc_f(void *buf, u32_t size);










#endif  /* _CRC16_H_ */
