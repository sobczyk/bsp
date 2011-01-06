/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: startup_entry.asm 4994 2010-09-21 22:39:30Z usb10132          $
; 
; Project: Generic 32x0 startup code
;
; Notes:
;     Realview 3.x and Keil MDK toolchain version
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

    .global arm926ejs_reset

    .global board_init
    .global mmu_setup
    .global __gnu_bssstart
    .global __gnu_bssend
    .global __gnu_roend
    .global __gnu_rwstart
    .global __gnu_rwend

    /*; This is the user application that is called by the startup code
    ; once board initialization is complete */
    .global c_entry

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Private defines and data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

.include "setup_gnu.inc"

.EQU MODE_USR,	0x010
.EQU MODE_FIQ,	0x011
.EQU MODE_IRQ,	0x012
.EQU MODE_SVC,	0x013
.EQU MODE_ABORT,	0x017
.EQU MODE_UNDEF,	0x01b
.EQU MODE_SYSTEM,	0x01f
.EQU MODE_BITS,	0x01f
.EQU I_MASK,	0x080
.EQU F_MASK,	0x040
.EQU IF_MASK,   0x0C0
.EQU MODE_SVC_NI, 0x0D3

/* End of internal RAM */
.EQU END_OF_IRAM, IRAM_SIZE

/*; Masks used to disable and enable the MMU and caches */
.EQU MMU_DISABLE_MASK,	0xFFFFEFFA
.EQU MMU_ENABLE_MASK,	0x00001005
.EQU MMU_ICACHE_BIT,	0x1000

	.text
	.code 32   /*; Startup code*/	
	.align 2

arm926ejs_reset:
	B     arm926ejs_entry
    B     .
    B     .
    B     .
    B     .
    B     .
    B     .
    B     .

/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function: arm926ejs_entry
;
; Purpose: Reset vector entry point
;
; Description:
;     Various support functions based on defines.
;
; Parameters: NA
;
; Outputs; NA
;
; Returns: NA
;
; Notes: NA
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/
arm926ejs_entry:
.ifdef	USE_MMU
    /*; Put the processor is in system mode with interrupts disabled */
    MOV   r0, #MODE_SVC_NI
    MSR   cpsr_cxsf, r0

    /*; Ensure the MMU is disabled */
    MRC   p15, 0, r6, c1, c0, 0
    LDR   r2,=MMU_DISABLE_MASK
    AND   r6, r6, r2
    MCR   p15, 0, r6, c1, c0, 0

    /*; Invalidate TLBs and invalidate caches */
    MOV   r7,#0
    MCR   p15, 0, r7, c8, c7, 0
    MCR   p15, 0, r7, c7, c7, 0

    /*; Enable instruction cache */
    ORR   r6, r6, #MMU_ICACHE_BIT
    MCR   p15, 0, r6, c1, c0, 0
.endif

    LDR   sp, =END_OF_IRAM
    /*; Get end of internal memory and set aside 16K for the page table
    ; SVC stack is under page table */
    SUB   sp, sp, #(16*1024)

    /*; Clear ZI segment */
    LDR   r0, =__gnu_bssstart
    LDR   r1, =__gnu_bssend
    MOV   r7, #0
clearzi:
    CMP   r0, r1
    BEQ   clearzi_exit
    STRB  r7, [r0], #1  /*; r7 previously set to 0 */
    B clearzi
clearzi_exit:

	/* ; Relocate read/write data segment before using it */
.ifdef RW_RELOC
	/* ; Get the start of the RW segment located in image */
    LDR   r7, =__gnu_roend

	/* ; Get address to move image too */
    LDR   r0, =__gnu_rwstart
    LDR   r1, =__gnu_rwend

rwmove:
	CMP   r0, r1
	BEQ   rwmove_exit
	LDRB  r2, [r7], #1
    STRB  r2, [r0], #1
    B     rwmove
rwmove_exit:
.endif

.ifdef	USE_BOARD_INIT
    /*; Initialize board */
    BL    board_init
.endif

.ifdef	USE_MMU
    /* ; MMU page table is at the end of IRAM, last 16K */
    LDR   r0, =END_OF_IRAM
    SUB   r0, r0, #(16*1024)
    MCR   p15, 0, r0, c2, c0, 0
	bl    mmu_setup

    /*; Setup the Domain Access Control as all Manager
    ; Make all domains open, user can impose restrictions */
    MVN   r7, #0
    MCR   p15, 0, r7, c3, c0, 0

    /*; Setup jump to run out of virtual memory at location inVirtMem */
    LDR   r5, =inVirtMem

    /*; Enable the MMU with instruction and data caches enabled */
    LDR   r2,=MMU_ENABLE_MASK
    ORR   r6, r6, r2
    MCR   p15, 0, r6, c1, c0, 0

    /*; Jump to the virtual address */
    MOV   pc, r5

    /*; The following NOPs are to clear the pipeline after the MMU virtual
    ; address jump */
    NOP
    NOP
    NOP
inVirtMem:
.endif

.ifdef	USE_ALL_STACKS

    /*; The code is operating out of virtual memory now - register R3
    ; contains the virtual address for the top of stack space */

	/*; SVC stack was previously setup, use it's location as the
	; reference for the other stacks */
    MOV   r3, sp
    SUB   r3, r3, #SVC_STACK_SIZE

    /*; All interrupts disabled at core for all modes */
    MOV   r1, #IF_MASK /*; No Interrupts */

    /*; Enter FIQ mode and setup the FIQ stack pointer */
    ORR   r0, r1, #MODE_FIQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #FIQ_STACK_SIZE

    /*; Enter IRQ mode and setup the IRQ stack pointer */
    ORR   r0, r1, #MODE_IRQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #IRQ_STACK_SIZE

    /*; Enter Abort mode and setup the Abort stack pointer */
    ORR   r0, r1, #MODE_ABORT
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #ABORT_STACK_SIZE

    /*; Enter Undefined mode and setup the Undefined stack pointer */
    ORR   r0, r1, #MODE_UNDEF
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #UNDEF_STACK_SIZE

    /*; Enter System mode and setup the User/System stack pointer */
    ORR   r0, r1, #MODE_SYSTEM
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SYSTEM_STACK_SIZE

    /*; Re-enter SVC mode for runtime initialization */
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0

.endif

    /*; 1-way jump to application */
    LDR  pc, =c_entry

    .END
