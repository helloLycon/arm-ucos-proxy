#ifndef __WEBSERVER_OPT_FRAME_H
#define  __WEBSERVER_OPT_FRAME_H


#define  MAX_OPT_PORT_AMOUNT       2u


// 光路配置参数
union web_opt_config{
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

// 光路状态参数
union web_opt_state{
	u8 byte;
	struct {
		u8 lof:1;
		u8 nop:1;
		u8 loop:1;
		u8 __rsv3:1;
		u8 __rsv4:1;
		u8 __rsv5:1;
		u8 __rsv6:1;
		u8 __rsv7:1;
	} bit;
};

// 光口
struct web_opt {
	union web_opt_config config[8];
	union web_opt_state  state[8];
	u8 amount;
} ;





#endif
