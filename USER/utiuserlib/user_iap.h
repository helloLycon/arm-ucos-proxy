
#ifndef _USER_IAP_H
#define _USER_IAP_H

#ifndef uint32_t 
typedef unsigned int    uint32_t;
#endif
#ifndef uint8_t
typedef unsigned char   uint8_t;
#endif


/* ¶¨ÒåIAP·µ»Ø×´Ì¬×Ö */ 
#define     CMD_SUCCESS          0 
#define     INVALID_COMMAND      1 
#define     SRC_ADDR_ERROR       2  
#define     DST_ADDR_ERROR       3 
#define     SRC_ADDR_NOT_MAPPED  4 
#define     DST_ADDR_NOT_MAPPED  5 
#define     COUNT_ERROR          6 
#define     INVALID_SECTOR       7 
#define     SECTOR_NOT_BLANK     8 
#define     SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION 9 
#define     COMPARE_ERROR        10 
#define     BUSY                 11 

#define BANK0          0
#define BANK1          1

extern void iap_init (uint8_t bank);
extern uint32_t iap_erase_flash(uint32_t addrDst, uint32_t length);
extern uint32_t iap_write_flash(uint32_t addrDst, uint8_t *addrSrc, uint32_t length);
extern uint32_t iap_read_flash(uint8_t *addrDst, uint8_t *addrSrc, uint32_t length);
extern uint32_t iap_compare(uint32_t dst, uint8_t *src, uint32_t no);


#endif


