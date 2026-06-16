////////////////////////////////////////////////////
// Definition of registers
#define    SP        r0
#define    AT        r1
#define    BP        r2
#define    LR        r3
#define    A0        r4
#define    A1        r5
#define    A2        r6
#define    A3        r7
#define    T0        r8
#define    T1        r9
#define    T2        r10
#define    T3        r11
#define    S0        r12
#define    S1        r13
#define    S2        r14
#define    S3        r15
#define    S4        r16
#define    S5        r17
#define    S6        r18
#define    S7        r19
#define    S8        r20
#define    S9        r21
#define    T4        r22
#define    T5        r23
#define    T6        r24
#define    T7        r25
#define    T8        r26
#define    T9        r27
#define    R28       r28
#define    JP        r29
#define    K0        r30
#define    K1        r31

// CP0 register define
#define CP0_STATUS   cr0
#define CP0_COND     cr1
#define CP0_CAUSE    cr2
#define CP0_EPC      cr5
#define CP0_EXCPVEC  cr3
#define CP0_CCR      cr4
#define CP0_BADVA    cr6
#define CP0_WIRE     cr7
#define CP0_INDEX    cr8
#define CP0_CONTEXT  cr9
#define CP0_RANDOM   cr10
#define CP0_ENTRYHI  cr11
#define CP0_ENTRYLO  cr12
#define CP0_FMARLO   cr13
#define CP0_FMARHI   cr14
#define CP0_FMCR     cr15
#define CP0_FFMR     cr16
#define CP0_LLADDR   cr17
#define CP0_PREV     cr18
#define CP0_DREG     cr29
#define CP0_DEPC     cr30
#define CP0_DSAVE    cr31
#define SPR_HI       sr1
#define SPR_LO       sr2
#define SR_CNT       sr0
#define SR_LCR       sr1
#define SR_SCR       sr2

.extern main
.extern _gp
.extern _stack

.section .hardware_init,"ax",@progbits
.global _hardware_init
.ent    _hardware_init
_hardware_init:
    // Set up global pointer and stack pointer.
    la      r28, _gp
    la      r0,  _stack
    
    // Clear BSS
    la r8, __bss_start__
    la r9, __bss_end__
    ldi     r10, 0
    
clear_bss_check:
    cmp!    r8, r9
    bge!    clear_bss_done
    nop
    sb      r10, [r8]+, 1
    j       clear_bss_check
    nop
clear_bss_done:
    jl      main
    nop
end_loop:
    j       end_loop
    nop
.end    _hardware_init
