#ifndef __WEBSERVER_ETH_FRAME_H
#define __WEBSERVER_ETH_FRAME_H



#define  MAX_ETH_GE_PORT_AMOUNT    2u
#define  MAX_ETH_FE_PORT_AMOUNT    6u



// 以太网配置参数
union web_eth_config {
	u16 half_word;
	struct {
		u8 __rsv0:1;
		u8 __rsv1:1;
		u8 __rsv2:1;
		u8 auto_nego:1;
		u8 full_duplex:1;
		u8 bandwidth:2;
		u8 link:1;
		u8 __rsv8:1;
		u8 __rsv9:1;
		u8 __rsv10:1;
		u8 __rsv11:1;
		u8 __rsv12:1;
		u8 trap_mask:1;
		u8 flow_ctrl:1;
		u8 enable:1;
	} bit;
};


// 以太网端口
struct web_eth {
	union web_eth_config ge[4];
	u8 ge_amount;
	union web_eth_config fe[16];
	u8 fe_amount;
};





#endif

