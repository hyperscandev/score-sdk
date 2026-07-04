#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tv/tv.h"
#include "irq/interrupts.h"
#include "hyperscan/hs_controller/hs_controller.h"

unsigned short *fb = (unsigned short *) 0xA0400000;

#define P_GPIO_JTAG_PORT  ((volatile unsigned int *)0x88200034)
#define P_GPIO_JCD_INPUT  ((volatile unsigned int *)0x88200070)

#define JTAG_MASK 0x1F

void jtag_disable_pulls_input(void)
{
    unsigned int v = *P_GPIO_JTAG_PORT;

    v &= ~(JTAG_MASK << 8);    // output enable off
    v &= ~(JTAG_MASK << 21);   // pull-ups off
    v &= ~(JTAG_MASK << 24);   // pull-downs off

    *P_GPIO_JTAG_PORT = v;
}

unsigned int jtag_read(void)
{
    return *P_GPIO_JCD_INPUT & JTAG_MASK;
}

void jtag_enable_pulldowns_input(void)
{
    unsigned int v = *P_GPIO_JTAG_PORT;

    v &= ~(JTAG_MASK << 8);    // input mode
    v &= ~(JTAG_MASK << 21);   // pull-ups off
    v |=  (JTAG_MASK << 24);   // pull-downs on

    *P_GPIO_JTAG_PORT = v;
}

void jtag_target_output_enable(void)
{
    unsigned int v = *P_GPIO_JTAG_PORT;

    v &= ~(JTAG_MASK << 21);
    v &= ~(JTAG_MASK << 24);

    v &= ~(JTAG_MASK << 0);
    v |=  (JTAG_MASK << 8);

    *P_GPIO_JTAG_PORT = v;
}

void jtag_target_write(unsigned int value)
{
//    unsigned int v = *P_GPIO_JTAG_PORT;

//    v &= ~(JTAG_MASK << 0);
//    v |= (value & JTAG_MASK);

//    v |= (JTAG_MASK << 8);

//    *P_GPIO_JTAG_PORT = v;
*P_GPIO_JTAG_PORT = value;
}

static void delay(unsigned int timez) {
	int i = 0;
	for(i=0;i<=timez;i++){
		__asm__("nop");
	}
}

int main(){
	
    jtag_target_output_enable();
	
	tv_init(RESOLUTION_640_480, COLOR_RGB565, 0xA0400000, 0xA0400000, 0xA0400000);
	
	volatile unsigned int *test = (volatile unsigned int *) 0x88200004;
	
	test[0] = 0x01000700u;
	
	while(1){
		tv_print(fb, 28, 2, "NOPEZ");
		jtag_target_write(0x1F00u);
        delay(0xffff);
        jtag_target_write(0x1F1Fu);
        delay(0xffff);
		//tv_printf(fb, 28, 5, "%x", jtag_read());
	}
		
	return 0;
}
