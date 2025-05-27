#include "score7_registers.h"

// Used to reference start of exception vector
extern void norm_debug_vec(void);

// Max/Min number of IRQs
// For now we will only attach handlers for 24~63 but
// will later add support for the CPU exceptions and 
// custom handling of those (for now those just freeze
// with a jump to self at intmsg)
#define MAX_IRQ 63
#define MIN_IRQ 24

// Type for an ISR callback
typedef void (*isr_handler)(void);

// Dispatch table, all entries == NULL
static isr_handler isr_table[MAX_IRQ] = { 0 };

// Fallback when no handler is attached
static void handler_placeholder(void) { HS_LEDS(0xFF); }

// Hook IRQ ISR
void attach_isr(unsigned int irq, isr_handler handler) {
	
	// Only attach handler if our irq is within the appropriate IRQ firing range
    if (irq >= MIN_IRQ && irq <= MAX_IRQ) {
    	
    	// Disable interrupts
		asm("li r4, 0x0");
		asm("mtcr r4, cr0");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		
		// Install handler to isr_table
    	isr_table[irq] = handler;
    
		// There seems to possibly be a bug in SPG290 (maybe other score7 based SoC?) where it seems
		// like the vector table won't properly relocate (I could be wrong) but this is fine (I think?) 
		// as we can just copy only the vectors that we expect to use over the fixed default vector
		// address space (0xA00001FC) and use the attach_isr to hook into the vector table with
		// our handler
		
		// Patch the irq_dispatch vector from our current one, to the fixed address (0xA00001FC area)
		// to use our custom isr dispatch for specified isr. This could possibly only work on Hyperscan
		// since I haven't seen any examples of other score7 based systems and their memory map, but if so
		// this could easily be fixed to accomodate those systems as well with our -D(platform) configuration switch
		unsigned int *current_irq_dispatch = (unsigned int *)&norm_debug_vec + 1;
		unsigned int *fixed_irq_dispatch = (unsigned int *)0xA00001FC + 1;
		
		unsigned int irq_offset = irq;
		
		// Swap handler at fixed vector with the one from our (unused) current vector
		fixed_irq_dispatch[irq_offset] = current_irq_dispatch[irq_offset];
		
		// Enable interrupts
		asm("li r4, 0x1");
		asm("mtcr r4, cr0");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
    }
}

// Get cause and handle IRQ
void irq_dispatch(unsigned int cp0_cause) {
    // bits [23:18] of cause register gives the IRQ number
    unsigned int intvec = (cp0_cause & 0x00FC0000u) >> 18;

	// Only hook between MIN_IRQ and MAX_IRQ (24~63) for now, until we add custom CPU exception handling
	if (intvec >= MIN_IRQ && intvec <= MAX_IRQ) {
    	// look it up in the table
    	isr_handler h = isr_table[intvec];
    	if (h) h(); else handler_placeholder();
	}
}

