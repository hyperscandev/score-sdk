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

