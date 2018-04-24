#ifndef __FLASH_H
#define __FLASH_H			    
#include "sys.h"  


/*** <lycon> 2014.4.7 ***/
//#define Update_Flash_Block_Pos_Start  4
//#define Update_Flash_Block_Pos_End    7
//#define Update_Flash_Memory_size     (FLASH_BLOCK_SIZE*(Update_Flash_Block_Pos_End-Update_Flash_Block_Pos_Start))
//20171025
//#define Update_Flash_Block_Pos_Start  1
//#define Update_Flash_Block_Pos_End    8
//20171110
#define Update_Flash_Block_Pos_Start  16
#define Update_Flash_Block_Pos_End    32
#define Update_Flash_Memory_size     (FLASH_BLOCK_SIZE*(Update_Flash_Block_Pos_End-Update_Flash_Block_Pos_Start))



#define FLASH_PAGE_SIZE		256
#define FLASH_SECTOR_SIZE	4096
#define FLASH_SECTOR_COUNT	512
#define FLASH_BLOCK_SIZE	65536
#define FLASH_PAGES_PER_SECTOR	FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE

#define	SPI_FLASH_CS PAout(4)  //ѡ��FLASH					 
////////////////////////////////////////////////////////////////////////////
//W25X16��д
#define FLASH_ID 0XEF14
//ָ���
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

uint8_t SPI_Flash_Init(void);
u16  SPI_Flash_ReadID(void);  	    //��ȡFLASH ID
u8	 SPI_Flash_ReadSR(void);        //��ȡ״̬�Ĵ��� 
void SPI_FLASH_Write_SR(u8 sr);  	//д״̬�Ĵ���
void SPI_FLASH_Write_Enable(void);  //дʹ�� 
void SPI_FLASH_Write_Disable(void);	//д����
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//д��flash
void SPI_Flash_Erase_Chip(void);    	  //��Ƭ����
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//��������
void SPI_Flash_Wait_Busy(void);           //�ȴ�����
void SPI_Flash_PowerDown(void);           //�������ģʽ
void SPI_Flash_WAKEUP(void);			  //����

void W25X_Read_Sector(uint32_t nSector, u8* pBuffer);
void W25X_Write_Sector(uint32_t nSector, u8* pBuffer);
#endif
















