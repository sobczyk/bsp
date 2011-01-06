;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: seflrefresh.s 3257 2010-04-12 17:26:38Z usb10132               $
; 
; Project: NXP LPC32x0 system DRAM self refresh example
;
; Notes:
;     This example shows how to safely place DRAM into self-refresh and
;     power down the system (full sleep). This example works for DDR
;     and SDRAM based systems and uses the manual self-refresh exit
;     capability of the controller. Auto-exit can also be used, but adds
;     unnecessary complexity to the software.
;
;     Note comments on specific instructions for DDR and SDRAM based
;     systems. DDR systems are currently the default, but SDRAM can be
;     enabled by uncommenting SDRAM lines and commenting the DDR lines.
;
;     This code is intended to run in uncached IRAM as the DRAM
;     resources are not available during self-refresh. It is recommended
;     that the MMU and caches be flushed and disabled prior to calling
;     this function to prevent TLB accesses or cache flushes to DRAM
;     during this sequence. This code is position independent.
;
;     -Other things to consider when entering sleep mode-
;      Disabling interrupts to prevent PC jumps
;      Disabling the MMU to prevent TLB updates
;      Avoid stack pushes/pops if stack is in SDRAM
;      Any other chip bus masters (ie, DMA) should be disabled
;
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    export srfsh_start
    export srfsh_end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	PRESERVE8
	AREA REFRESH, CODE

    ;
    ;

srfsh_start
    ; The stack is assumed to be in IRAM
    STMFD sp!, {r0 - r12, lr}

    ;
    ; Flush data cache - may not be needed on all systems, but will help
    ; prevent cache line flushes during self-refresh and clock disable
    ; (which can cause system instability)
    ;
flushcache
    MRC   p15, 0, r15, c7, c10, 3
    BNE   flushcache

    ;
    ; Load commonly used register addresses (physical) used during this
    ; seqeuence
    ; r8 = 0x40004000 (CLKPWR regs base phy)
    ; r9 = 0x31080000 (EMC regs base phy)
    ; r5 = 0x40028000 (GPIO regs base phy)
    ;
    MOV   r5, pc
    ADD   r5, r5, #16
    LDR   r8, [r5]
    LDR   r9, [r5, #4]
    LDR   r5, [r5, #8]
    B     startcycle
regs__
    DCD 0x40004000 ; CLKPWR regs base address
    DCD 0x31080000 ; SDRAM status reg
    DCD 0x40028000 ; GPIO regs base address

startcycle
    ;
    ; This patch of code simply enables automatic disable of DRAM clocks
    ; when self-refresh is started. This can be deleted if your init code
    ; already sets this up.
    ;
    LDR   r0, [r9, #0x20] ; 0x31080020
    ORR   r0, r0, #(1 << 3)
    STR   r0, [r9, #0x20] ; DRAM clocks will disable on self-refresh

    ;
    ; Wait for SDRAM to go busy and then idle - this offers more reliability
    ; than just checking for an idle condition
    ;
waitrdy1
    LDR   r0, [r9, #0x4]
    AND   r0, r0, #(1 << 0)
    CMP   r0, #(1 << 0)
    BNE   waitrdy1 ; Wait for busy
waitrdy2
    LDR   r0, [r9, #0x4]
    AND   r0, r0, #(1 << 0)
    CMP   r0, #(1 << 0)
    BEQ   waitrdy2 ; Wait for idle

    ;
    ; Setup self-refresh with support for manual exit of self-refresh mode.
    ; In manual exit mode, all thats needed to exit self-refresh mode is a
    ; few extra register operations.
    ;
    LDR   r2, [r8, #0x44]
    ORR   r2, r2, #(1 << 9)
    STR   r2, [r8, #0x44] ; Enable self-refresh bit
    ORR   r1, r2, #(1 << 8)
    STR   r1, [r8, #0x44] ; Activate self-refresh latch
    STR   r2, [r8, #0x44] ; Clear activation latch

    ;
    ; Wait for SDRAM acknowledge of self refresh
    ;
wait_sfrsh
    LDR   r0, [r9, #0x4]
    AND   r0, r0, #(1 << 2)
    CMP   r0, #(1 << 2)
    BNE   wait_sfrsh ; Wait for self-refresh

    ;
    ; Enter direct-run mode
    ;
    BIC   r0, r2, #(1 << 2)
    STR   r0, [r8, #0x44]

    LDR   r7, [r8, #0x40]
    ;
    ; If you use any PCLK based peripherals between here and the restart of
    ; run mode, you will also need to set the PCLK divider to 1 here in the
    ; HCLK divider control register or PCLK based peripherals will not get
    ; clock. The following code optionally performs this if needed.
    ;
    ;AND   r6, r7, #0xFFFFFF83
    ;STR   r6, [r8, #0x40] ; PCLK div = 1

    ; Disable the DRAM clock
    AND   r6, r7, #0xFFFFFE7F
    STR   r6, [r8, #0x40] ; Stop DRAM clock

    ;
    ; Disable HCLK PLL
    ;
    LDR   r10, [r8, #0x58]
    BIC   r0, r10, #(1 << 16)
    STR   r0, [r8, #0x58]

    ;
    ; Enter stop mode - an enabled event will have to wake us back up or we
    ; will sleep forever
    ;
    LDR   r0, [r8, #0x44]
    ORR   r0, r0, #(1 << 0)
    STR   r0, [r8, #0x44] ; Enter stop mode
    NOP
    NOP
    NOP
    NOP

    ;
    ; Restore HCLK PLL with original value and wait for PLL lock
    ;
    STR   r10, [r8, #0x58]
wait_for_lock
    LDR   r0, [r8, #0x58]
    AND   r0, r0, #(1 << 0)
    BNE wait_for_lock ; Wait for PLL lock

    ; If the PCLK divider was changed to 1, restore it's original value here,
    ;b ut keep the DRAM clocks disabled. The optional following code performs
    ; this if needed.
    ;AND   r6, r7, #0xFFFFFE80
    ;STR   r6, [r8, #0x40] ; PCLK div = 1

    ;
    ; Enter run mode to restore normal system clocking (minus DRAM clocking)
    ;
    STR   r2, [r8, #0x44]

    ;
    ; DDR systems only - resync DDR clocks (must be done with DRAM clock
    ; disabled)
    ;
    LDR   r0, [r8, #0x68] ; DDR only
    ORR   r1, r0, #(1 << 19) ; DDR only
;    STR   r1, [r8, #0x68] ; DDR only << THIS OPERATION CAUSES PROBLEMS
    STR   r0, [r8, #0x68] ; DDR only

    ;
    ; Restore original DRAM clock mode
    ;
    STR   r7, [r8, #0x40] ; DDR only

    ;
    ; clear self-refresh, but only after run mode has been restored on DDR
    ; systems. SDRAM systems can clear self-refresh prior to run mode or use
    ; the auto-exit function for self-refresh exit on exit from STOP mode.
    ; This manual deactivation works on DDR and SDRAM.
    ;
    LDR   r0, [r8, #0x44]
    BIC   r0, r0, #(1 << 9)
    STR   r0, [r8, #0x44] ; Clear self-refresh bit
    ORR   r1, r0, #(1 << 8)
    STR   r1, [r8, #0x44] ; Activate self-refresh latch
    STR   r0, [r8, #0x44] ; Clear activation latch

wait_drfsh
    LDR   r0, [r9, #0x4]
    AND   r0, r0, #(1 << 2)
    CMP   r0, #0
    BNE   wait_drfsh ; Wait for self-refresh exit

    ; DRAM is back up

edxit2222
    LDMFD sp!, {r0 - r12, lr}
    MOV   pc, lr

srfsh_end
    END
