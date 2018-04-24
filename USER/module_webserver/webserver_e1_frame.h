#ifndef  __WEBSERVER_E1_FRAME_H
#define  __WEBSERVER_E1_FRAME_H


#define  MAX_E1_PORT_AMOUNT        8u



// e1配置参数
union web_e1_config {
	u8 byte;
	struct {
		u8 trap_mask:1;
		u8 circ_loop:1;
		u8 dev_loop:1;
		u8 __rsv3:1;
		u8 __rsv4:1;
		u8 __rsv5:1;
		u8 __rsv6:1;
		u8 enable:1;
	} bit;
};

//e1状态参数
union web_e1_state {
	u8 byte;
	struct {
		u8 los:1;
		u8 ais:1;
		u8 loop:1;
		u8 __rsv3:1;
		u8 __rsv4:1;
		u8 __rsv5:1;
		u8 __rsv6:1;
		u8 __rsv7:1;
	} bit ;
};

struct web_e1 {
	union web_e1_config config[8];
	union web_e1_state  state[8];
	u8 __rsv[16];
	u8 amount;
};




#endif

