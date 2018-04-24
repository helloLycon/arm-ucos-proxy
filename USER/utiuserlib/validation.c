#include <stdio.h>
#include <string.h>
#include "hj_nm.h"
#include "validation.h"

/*
A类: 1.0.0.0 ~ 126.255.255.255
     (127.x.x.x为本地环回地址)
B类: 128.0.0.0 ~ 191.255.255.255
C类: 192.0.0.0 ~ 223.255.255.255
D类: 组播(224.0.0.0 ~ 239.255.255.255)
E类: 研究用途(240.0.0.0 ~ 255.255.255.255)
*/
#define  CLASS_A_START  IPV4_COMB(1,0,0,0)
#define  CLASS_A_END    IPV4_COMB(126,255,255,255)

#define  CLASS_B_START  IPV4_COMB(128,0,0,0)
#define  CLASS_B_END    IPV4_COMB(191,255,255,255)

#define  CLASS_C_START  IPV4_COMB(192,0,0,0)
#define  CLASS_C_END    IPV4_COMB(223,255,255,255)



int ip_is_class_A(u32_t ip){
/*
A类: 1.0.0.0 ~ 126.255.255.255
     (127.x.x.x为本地环回地址)
*/
	return  ip>=CLASS_A_START && ip<=CLASS_A_END;
}
int ip_is_class_B(u32_t ip){
	return  ip>=CLASS_B_START && ip<=CLASS_B_END;
}
int ip_is_class_C(u32_t ip){
	return  ip>=CLASS_C_START && ip<=CLASS_C_END;
}
int ip_is_class_ABC(u32_t ip){
	int ret = (ip_is_class_A(ip)||ip_is_class_B(ip)||ip_is_class_C(ip));
	if(!ret)
		printf("VALIDATION: IP is not class of A/B/C\r\n");
	return ret;
}

int msk_is_valid(u32_t msk){
	int i , host_flag = 0 , zero_cnter = 0;
	for(i=31 ; i>=0;--i){
		if(msk & (1ul<<i)){	
			// 1
			if(host_flag){
				// 出现0和1交叉
				printf("VALIDATION: mask 0/1 cross\r\n");
				return VALI_WRONG;
			}
		}
		else{
			// 0
			host_flag = 1;
			zero_cnter++;
		}
	}
	if(zero_cnter > 24){
		// 首字节不是255
		printf("VALIDATION: mask must be 255.x.x.x\r\n");
		return VALI_WRONG;
	}
	if(zero_cnter < 2){
		printf("VALIDATION: host nr can not be shorter than 2 bits\r\n");
		return VALI_WRONG;
	}
	return VALI_OK;
}

int ip_msk_gw_validation(u32_t ip,u32_t msk,u32_t gw){
	u32_t host_nr;
	if(!ip_is_class_ABC(ip)){
		printf("VALIDATION: host ip error\r\n");
		return VALI_WRONG;
	}
	if(!msk_is_valid(msk)){
		return VALI_WRONG;
	}
	if(!ip_is_class_ABC(gw)){
		printf("VALIDATION: gateway error\r\n");
		return VALI_WRONG;
	}
	// combine hostip and mask
	host_nr = ip & (~msk);
	if( !host_nr ){
		printf("VALIDATION: host_nr can not be all 0\r\n");
		return VALI_WRONG;
	}
	if( host_nr == (~msk) ){
		printf("VALIDATION: host_nr can not be all 1\r\n");
		return VALI_WRONG;
	}
	return VALI_OK;
}

