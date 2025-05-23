#include "score7_registers.h"
#include "score7_constants.h"
#include "norflash/norflash.h"

//========================================================================
//	void norflash_init(void)
//
//
//
//
//	void return
//========================================================================

void norflash_init(void){
	/*
		On the Mattel HyperScan after it boots it disables the NorFlash chip,
		and so we need to re-enable that on init if we're going to be able
		to do any flash related operations
	*/
	*P_ROMCSN_INTERFACE_SEL |= 1 << 24;
	*P_DRAM_GPIO_SETUP = 0x0F000100;
}

//========================================================================
//	void norflash_chiperase(void)
//
//	WARNING: THIS ERASES THE NORFLASH CHIP - DO NOT USE UNLESS YOU KNOW
// 	EXACTLY WHAT YOU'RE DOING
//
//	void return
//========================================================================
void norflash_chiperase(void){
	*P_NOR_COMMAND_CTRL = 1;

	*C_CMD1_Addr = C_CMD_Data1;
	*C_CMD2_Addr = C_CMD_Data2;
	*C_CMD3_Addr = C_CMD_Data3;
	*C_CMD4_Addr = C_CMD_Data4;
	*C_CMD5_Addr = C_CMD_Data5;
	*C_CMD6_Addr = C_CHIP_ERASE;

	norflash_delay(C_CHIP_ERASE_TIME);			// delay 100ms, 100000000ns/40ns = 2500000;

	*P_NOR_COMMAND_CTRL = 0;
}

//========================================================================
//	void norflash_blockErase(unsigned int addr)
//
//	This erases a NorFlash block at specified address (addr)
//
// 	void return
//========================================================================
void norflash_blockerase(unsigned int addr){
	addr -= C_Flash_BaseAddr;
	addr = addr / 2;
	addr = addr & (~(C_BLOCK_SIZE - 1));
	addr = norflash_setaddress(addr);
	addr = C_Flash_BaseAddr + (addr << 4);
	*P_NOR_COMMAND_CTRL = 1;

	*C_CMD1_Addr = C_CMD_Data1;
	*C_CMD2_Addr = C_CMD_Data2;
	*C_CMD3_Addr = C_CMD_Data3;
	*C_CMD4_Addr = C_CMD_Data4;
	*C_CMD5_Addr = C_CMD_Data5;
	*(unsigned int *)addr = C_BLOCK_ERASE;

	norflash_delay(C_BLOCK_ERASE_TIME);

	*P_NOR_COMMAND_CTRL = 0;
}

//========================================================================
//	void norflash_sectorerase(unsigned int addr)
//
//	This erases a NorFlash sector starting at specified address (addr)
//
// 	void return
//========================================================================
void norflash_sectorerase(unsigned int addr){
	addr -= C_Flash_BaseAddr;
	addr = addr / 2;
	addr = addr & (~(C_SECTOR_SIZE - 1));
	addr = norflash_setaddress(addr);
	addr = C_Flash_BaseAddr + (addr << 4);
	*P_NOR_COMMAND_CTRL = 1;

	*C_CMD1_Addr = C_CMD_Data1;
	*C_CMD2_Addr = C_CMD_Data2;
	*C_CMD3_Addr = C_CMD_Data3;
	*C_CMD4_Addr = C_CMD_Data4;
	*C_CMD5_Addr = C_CMD_Data5;
	*(unsigned int *)addr = C_SECTOR_ERASE;

	norflash_delay(C_BLOCK_ERASE_TIME);

	*P_NOR_COMMAND_CTRL = 0;
}

//========================================================================
//	void norflash_write32(unsigned int addr, unsigned int data)
//
//	This writes a 32bit value (data) to a NorFlash address (addr)
//
//
// 	void return
//========================================================================
void norflash_write32(unsigned int addr, unsigned int data){

	unsigned int uiTemp;

	if(addr >= C_Flash_BaseAddr)
	{
		addr -= C_Flash_BaseAddr;
		uiTemp = addr;
		//write low 16 bits
		addr >>= 1;
		addr = norflash_setaddress(addr);
		addr = C_Flash_BaseAddr + (addr << 4);
		norflash_wordwrite(addr, data & 0x0000ffff);
		//write high 16 bits
		addr = uiTemp;
		addr >>= 1;
		addr++;
		addr = norflash_setaddress(addr);
		addr = C_Flash_BaseAddr + (addr << 4);
		norflash_wordwrite(addr, (data >> 16) & 0x0000ffff);
	}
}

//========================================================================
//	void norflash_wordwrite(unsigned int addr, unsigned data)
//
//	This writes a word (data) to a NorFlash at address (addr)
//
//
//	void return
//========================================================================
void norflash_wordwrite(unsigned int addr, unsigned int data){
	*P_NOR_COMMAND_CTRL = 1;

	data &= 0x0000ffff;
	*C_CMD1_Addr = C_CMD_Data1;
	*C_CMD2_Addr = C_CMD_Data2;
	*C_CMD3_Addr = C_WORD_WRITE;
	*(volatile unsigned int *)addr = data;

	norflash_delay(C_WRITE_DELAY_TIME);			// delay 20us, 20000ns/40ns = 500;

	*P_NOR_COMMAND_CTRL = 0;
}

//========================================================================
//	unsigned int norflash_setaddress(unsigned int addr)
//
//
//
//	returns address (addr)
//========================================================================
unsigned int norflash_setaddress(unsigned int addr){
	unsigned i = 0;
	while(addr > 0x0fffff){
		addr -= 0x100000;
		i++;
	}
	*P_NOR_BANK_SETUP = i;
	return(addr);
}

//========================================================================
//	void norflash_delay(unsigned int clk)
//
//	This is to create delay cycles for NorFlash write operations
//
//	void return
//========================================================================
void norflash_delay(unsigned int clk){
	unsigned int i;
	for(i = 0; i < clk; i++);
}
