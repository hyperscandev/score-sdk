#ifdef hyperscan
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "score7_registers.h"
#include "score7_constants.h"

#include "hyperscan/fatfs/ff.h"		/* Obtains integer types */
#include "hyperscan/fatfS/diskio.h"		/* Declarations of disk functions */
#include "hyperscan/hyperscan.h"

/* Definitions of physical drive number for each drive */
#define DEV_USB 0

int DiskB_Initial(void){
	
	if(drvusbh_init()){
		return 1;
	}
	if(drvusbh_luninit(0)){
		return 1;
	}

	return 0;
}

int USB_disk_initialize(void){

	// USB_Init	
	*P_CLK_PLLAU_CONF |= C_PLLU_CLK_EN;
	*P_INT_MASK_CTRL1 &= ~C_INT_USB_DIS;
	
	while(DiskB_Initial());
	
	return 0;
}

int DiskB_ReadSector(LBA_t block, UINT blocknum, BYTE *inaddr){
	drvusbh_readsector(block, blocknum, inaddr, 0);
	return 0;
}

int DiskB_WriteSector(LBA_t block, UINT blocknum, BYTE *outaddr){
	drvusbh_writesector(block, blocknum, outaddr, 0);
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	USB_disk_initialize();
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DiskB_ReadSector(sector, count, buff);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	BYTE *buff,			/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DiskB_WriteSector(sector, count, buff);
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}
#endif
