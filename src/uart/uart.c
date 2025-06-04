#include "score7_registers.h"
#include "score7_constants.h"
#include "uart/uart.h"

//=============================================================
//	void uart_Init(int baudrate)
//
//
//	void return
//=============================================================
void uart_init(int baudrate){
/*	
	For now I'll leave this blank, as UART is already set up
	by the firmware on the Mattel HyperScan, but having the 
	option to change configs for other SPG29x systems would be useful.
*/
}

//=============================================================
//	void uart_sendchar(char cdata)
//
//
//	void return
//=============================================================
void uart_sendchar(char cdata){
    while(*P_UART_TXRX_STATUS & C_UART_TXFIFO_FULL);
    *P_UART_TXRX_DATA = cdata;
	while(*P_UART_TXRX_STATUS & C_UART_BUSY_FLAG);  
}

//=============================================================
//	char uart_getchar();
//
//
//	char return (the character send to the device)
//=============================================================
char uart_getchar(){
    char cdata = 0;
    int i=0;
    
    while(*P_UART_TXRX_STATUS & C_UART_RXFIFO_EMPTY){
    	i++;
    	if(i==10000)break;
    }
    
    if(i==10000) cdata = 0xFF; else cdata = *P_UART_TXRX_DATA;
    
    return cdata;
}

//=============================================================
//	void uart_enable_interface();
//	
//	Enable UART
//
//	return: None
//=============================================================
void uart_enable_interface() {
	*P_UART_INTERFACE_SEL |= SW_UART_UART;
}

//=============================================================
//	void uart_wait_nonbusy();
//	
//	Wait for UART to be free
//
//	return: None
//=============================================================
static void uart_wait_nonbusy() {
	unsigned int status = *P_UART_TXRX_STATUS;
	while (status & UART_BUSY || status & UART_TRANSMIT_FULL || !(status & UART_TRANSMIT_EMPTY)) {
		status = *P_UART_TXRX_STATUS;
	}
}

//=============================================================
//	void uart_write_byte(unsigned int c);
//	
//	Write a byte over UART
//
//	return: None
//=============================================================
void uart_write_byte(unsigned int c) {
	*P_UART_TXRX_DATA = c;
	uart_wait_nonbusy();
}

//=============================================================
//	void uart_print_string(const char *str);
//	
//	Write a string over UART
//
//	return: None
//=============================================================
void uart_print_string(const char *str) {
	while (*str) {
		uart_write_byte(*str++);
	}
}

//=============================================================
//	unsigned char uart_read_byte();
//	
//	Gett a byte over UART
//
//	return: byte
//=============================================================
unsigned char uart_read_byte() {
	while (*P_UART_TXRX_STATUS & UART_RECEIVE_EMPTY);
	return *P_UART_TXRX_DATA;
}

