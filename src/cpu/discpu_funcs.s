#define    SP        r0
#define    LR        r3
#define    S0        r12
#define    S1        r13
#define    S2        r14
#define    S3        r15
#define    S4        r16
#define    S5        r17
#define    S6        r18
#define    S7        r19

// These are BOOTROM callbacks that exist on SPG29x core
//
// On the hyperscan they're in the internal BOOTROM, and in
// the external NOR flash at their respective offsets
//
// They're used when doing any operations with the P_CPU_CONF register (0x88210000)
// and best I can tell they're needed because the operations need to be done while
// certain MIU and CPU clocks are disabled

//9f000010:	 	b		 <discpu_clk_func>
//9f000014:	 	b		 <discpu_miu_clk_func>
//9f000018:	 	b		 <discpu_miu_clk_pll_func>
//9f00001c:	 	b		 <cpu_poweronn_func>
//9f000020:	 	b		 <cpu_warmrstn_func>
//9f000024:     b		 <change_clk_func>

.global jumpto_extrom_discpu_clk
jumpto_extrom_discpu_clk:
	ldis r8, 0x9f00
	ori	r8, 0x0010
	br	r8 // Go to discpu_clk_func in BOOTROM (P_CPU_CONF bit 1 trigger)
	nop
	nop

.global jumpto_extrom_discpu_miu_clk
jumpto_extrom_discpu_miu_clk:   
	ldis r8, 0x9f00
	ori	r8, 0x0014
	br	r8 // Go to discpu_miu_clk_func in BOOTROM (P_CPU_CONF bit 2 trigger)
	nop
	nop

.global jumpto_extrom_discpu_miu_clk_pll
jumpto_extrom_discpu_miu_clk_pll:   
	ldis r8, 0x9f00
	ori	r8, 0x0018
	br	r8 // Go to discpu_miu_clk_pll_func in BOOTROM (P_CPU_CONF bit 3 trigger)
	nop
	nop

.global jumpto_extrom_cpu_poweronn
jumpto_extrom_cpu_poweronn:   
	ldis r8, 0x9e00
	ori	r8, 0x001C
	br	r8 // Go to cpu_poweronn_func in BOOTROM (P_CPU_CONF bit 4 trigger)
	nop
	nop

.global jumpto_extrom_cpu_warmrstn
jumpto_extrom_cpu_warmrstn:   
	ldis r8, 0x9f00
	ori	r8, 0x0020
	br	r8 // Go to cpu_warmrstn_func in BOOTROM (P_CPU_CONF bit 6 and bit 5 trigger)
	nop
	nop

.global jumpto_extrom_change_cpuclk
jumpto_extrom_change_cpuclk:
   
// We save/restore these values because the SPG290 internal BOOTROM will change them

push_stack:	
	subi SP, 28
	sw	S0, [SP,0]
	sw	S1, [SP,4]
	sw	S2, [SP,8]
	sw	S3, [SP,12]
	sw	S6, [SP,16]
	sw	S7, [SP,20]
	sw	LR, [SP,24]
		
	li	S1,0x9f000024 // Go to change_clk_func in BOOTROM (P_CPU_CONF bit 0 trigger)
	brl	S1
	nop
	nop

pop_stack:		
	lw	S0, [SP,0]
	lw	S1, [SP,4]
	lw	S2, [SP,8]
	lw	S3, [SP,12]
	lw	S6, [SP,16]
	lw	S7, [SP,20]
	lw	LR, [SP,24]
	addi SP, 28
	br LR
	nop
	nop
	

