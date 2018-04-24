/*************************************************  
  File name:         crc16.h
  Author:            txli
  Version:           1.0.0 
  Created Date:      20121205
  Description:       16λCRCУ��ģ��
  Others:             
  History:
    1. Date:         20121205       
       Author:       txli  
       Modification: ��ʼ�汾
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
  Description:     ����crc16ֵ                                                                 
  Calls:           ��                                                                                  
  Input:        	                                                                                     
                   s8_t *buf  : Դ����ָ��                                                                 
                   u32_t size : ���ݳ���                                                                   
  Output:          ��                                                                                  
  Return:                                                                                              
                   u16_t crc16: ������                                                               
  Others:          ��                                                                                  
*************************************************/   
u16_t crc16_calc_f(s8_t *buf, u32_t size);

/*************************************************                                                     
  Function:        crc16_pack_f                                                                        
  Description:     ��֡�����е�0x7eת����0x7d����                                                                 
  Calls:           ��                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : ����ָ��                                                                 
                   s32_t *p_len  : ���ݳ���                                                                   
  Output:          
                   u8_t *p_data  : ����ָ��      
                   s32_t *p_len  : ���ݳ���                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          ��                                                                                  
*************************************************/ 
status_t crc16_pack_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        crc16_unpack_f                                                                        
  Description:     ��֡�����е�0x7eת����0x7d�����ķ�����                                                                 
  Calls:           ��                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : ����ָ��                                                                 
                   s32_t *p_len  : ���ݳ���                                                                   
  Output:          
                   u8_t *p_data  : ����ָ��      
                   s32_t *p_len  : ���ݳ���                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          ��                                                                                  
*************************************************/  
status_t crc16_unpack_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        buff_add_crc16_f                                                                        
  Description:     ��crc16���������뵽֡���ݵ�ĩβ                                                                
  Calls:           
                   u16_t crc16_calc_f(s8_t *buf, u32_t size)                                                                                  
  Input:        	                                                                                     
                   u8_t *p_data  : ����ָ��                                                                 
                   s32_t *p_len  : ���ݳ���                                                                   
  Output:          
                   u8_t *p_data  : ����ָ��      
                   s32_t *p_len  : ���ݳ���                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          ��                                                                                  
*************************************************/ 
status_t buff_add_crc16_f(u8_t *p_data, s32_t *p_len);

/*************************************************                                                     
  Function:        buff_dec_crc16_f                                                                        
  Description:     ��crc16���ȴ�֡���ݵĳ�����ȥ��                                                                
  Calls:           ��                   
  Input:        	                                                                 
                   s32_t *p_len  : ���ݳ���                                                                   
  Output:          
                   s32_t *p_len  : ���ݳ���                                                                                                                        
  Return:                                                                                              
                   OK_T          : success
                   ERROR_T       : failed
  Others:          ��                                                                                  
*************************************************/ 
status_t buff_dec_crc16_f(s32_t *p_len);


unsigned int  hj15_crc_7e_notxor_20(unsigned char  * pdata, int nlen,unsigned char *p_crc);
unsigned int hj_calc_crc(unsigned char  * pdata, int nlen);
//u16_t crc16_calc_f(void *buf, u32_t size);










#endif  /* _CRC16_H_ */
