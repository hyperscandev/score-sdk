#include "score7_registers.h"
#include "score7_constants.h"
#include "i2c/i2c.h"

//=============================================================
// void i2c_init(void)
//
// This initializes the I2C controller
//
//=============================================================
void i2c_init(void)
{
	*P_I2C_CLK_CONF = C_I2C_CLK_EN
					 | C_I2C_RST_DIS;			
	*P_I2C_INTERFACE_SEL |= C_I2C_PORT_SEL;		
	*P_I2C_RATE_SETUP = 0x258;						
	*P_I2C_SLAVE_ADDR = 0x54;		
}

//=============================================================
// unsigned int i2c_read8(int addr)
// 
// This reads (8) bits of data from the selected I2C addr (addr)
// 
//=============================================================
unsigned int i2c_read8(int addr)
{
	unsigned int a = 0;

	*P_I2C_DATA_ADDR = addr;
	*P_I2C_MODE_CTRL = C_I2C_RX_MODE | C_I2C_8BIT_START;

	a = *P_I2C_INT_STATUS;
	while(a==0)	a = *P_I2C_INT_STATUS;
	*P_I2C_INT_STATUS = C_I2C_INT_FLAG;

	a = *P_I2C_MODE_CTRL & C_I2C_8BIT_ACK;
	a = *P_I2C_RX_DATA;
	
	return a;
}

//=============================================================
// void i2c_write8(int addr, unsigned int value)
// 
// This writes an (8) bit value (value) to the I2C addr (addr)
// 
//=============================================================
void i2c_write8(int addr, unsigned int value)
{
	unsigned int a;

	*P_I2C_DATA_ADDR = addr;
	*P_I2C_TX_DATA = value;
	
	*P_I2C_MODE_CTRL = C_I2C_TX_MODE| C_I2C_8BIT_START;
	
	a = *P_I2C_INT_STATUS;
	while(a==0)	a = *P_I2C_INT_STATUS;
	*P_I2C_INT_STATUS = C_I2C_INT_FLAG;
	
	a = *P_I2C_MODE_CTRL & C_I2C_8BIT_ACK;
}


