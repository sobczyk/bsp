/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: seflrefresh_gnu.asm 2897 2010-02-09 11:45:59Z ing02124           $
; 
; Project: NXP PHY3250 System DRAM self refresh example
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; Software that is described herein is for illustrative purposes only  
 ; which provides customers with programming information regarding the  
 ; products. This software is supplied "AS IS" without any warranties.  
 ; NXP Semiconductors assumes no responsibility or liability for the 
 ; use of the software, conveys no license or title under any patent, 
 ; copyright, or mask work right to the product. NXP Semiconductors 
 ; reserves the right to make changes in the software without 
 ; notification. NXP Semiconductors also make no representation or 
 ; warranty that such application will be suitable for the specified 
 ; use without further testing or modification. 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

    .global srfsh_start
    .global srfsh_end

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

	.text
	.code 32
	
srfsh_start:
    STMFD sp!, {r0 - r12, lr}

    /* Flush data cache */
flushcache:
	MRC   p15, 0, r15, c7, c10, 3
	BNE   flushcache

	/* Other things to consider
	Disabling interrupts to prevent PC jumps
	Disabling the MMU to prevent TLB updates (better
	be a physical address)
	Avoid stack pushes/pops if stack is in SDRAM
	This code just loads some registers with pointers using
	relative addresses */
	MOV   r5, pc
	ADD   r5, r5, #16
	LDR   r8, [r5]
	LDR   r9, [r5, #4]
	LDR   r5, [r5, #8]
	B     waitrdy1

regs__:
	.word 0x40004000 /* CLKPWR regs base address */
	.word 0x31080004 /* SDRAM status reg */
	.word 0x40028000 /* GPIO regs base address */

    /* Wait for SDRAM ready */
waitrdy1:
    LDR   r0, [r9]
    AND   r0, r0, #(1 << 0)
    CMP   r0, #(1 << 0)
    BEQ   waitrdy1

    /* Setup self-refresh */
    LDR   r0, [r8, #0x44]
    ORR   r0, r0, #(1 << 9)
    STR   r0, [r8, #0x44]
    ORR   r0, r0, #(1 << 7)
    STR   r0, [r8, #0x44]
    ORR   r1, r0, #(1 << 8)
    STR   r1, [r8, #0x44]
    STR   r0, [r8, #0x44]
    BIC   r0, r0, #(1 << 9)
    STR   r0, [r8, #0x44]

    /* Wait for SDRAM acknowledge of self refresh */
waitrdy2:
    LDR   r0, [r9]
    AND   r0, r0, #(1 << 2)
    CMP   r0, #(1 << 2)
    BNE   waitrdy2

    /* Before entering direct-run mode, set the PCLK
    divider to '1'*/
    LDR   r7, [r8, #0x40]
    AND   r6, r7, #0xFFFFFF83
    STR   r6, [r8, #0x40]

    /* Enter direct-run mode */
    LDR   r2, [r8, #0x44]
    BIC   r0, r2, #(1 << 2)
	STR   r0, [r8, #0x44]

	/* Enter stop mode */
    LDR   r0, [r8, #0x44]
    ORR   r0, r0, #(1 << 0)
    STR   r0, [r8, #0x44]

    /* Enter run mode on stop exit, restore divider */
	STR   r2, [r8, #0x44]
    STR   r7, [r8, #0x40]

	/* SDRAM is back up now
	 On exit from stop mode, clear auto-exit bit
	 NOTE: Auto-exit is cleared as part of the stop to
	 direct-run mode change. If the bit is left set,
	 SDRAM will still work properly, but should be unset
	 before being re-set again in the future. */
    BIC   r0, r0, #((1 << 0) | (1 << 7))
    STR   r0, [r8, #0x44]

	/* Restore your system status a to pre-STOP state */
    LDMFD sp!, {r0 - r12, lr}
    MOV   pc, lr

srfsh_end:
    .end
