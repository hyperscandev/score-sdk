.extern _start
.section .hardware_init,"ax",@progbits

.global _hardware_init
.ent _hardware_init
_hardware_init:

	// Setup SDRAM
	la	r9, 0x88070060
	la r10, 0x95404b04
	sw r10, [r9, 0]
	
	// Init/Clear BSS segment
	la      r8, __bss_start
	la      r9, _bss_end__
	ldi     r10, 0

clear_bss_loop:
	sb		r10, [r8]+, 1
	cmp!	r8, r9
	ble!	clear_bss_loop
	nop
				
	la r0, _stack
		
	j _start

.end _hardware_init

