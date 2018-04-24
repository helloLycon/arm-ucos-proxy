#ifndef __WEBSERVER_BULB_VER_FRAME_H
#define __WEBSERVER_BULB_VER_FRAME_H




//指示灯参数
struct web_bulb_ver_frm{
	u16 bulb;
	u8  __rsv1[4];
	u8  boot_ver;
	u8  cpu_app_ver;
	u16 fpga_ver;
	u8  pcb_ver;
	u8  __rsv6[5];
} __attribute__((packed));

struct web_bulb_ver{
	struct web_bulb_ver_frm frame;
	int set;
};



#endif

