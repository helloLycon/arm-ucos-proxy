#ifndef  VALIDATION_H
#define  VALIDATION_H


#define IPV4_COMB(a,b,c,d) ((((u32_t)a & 0xff)<<24)|\
                            (((u32_t)b & 0xff)<<16)|\
                            (((u32_t)c & 0xff)<<8)|\
                            ((u32_t)d & 0xff))

#define VALI_OK    1
#define VALI_WRONG 0


int ip_msk_gw_validation(u32_t ip,u32_t msk,u32_t gw);



#endif

