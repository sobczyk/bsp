;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: startup_entry.s 4994 2010-09-21 22:39:30Z usb10132             $
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	INCLUDE setup.inc

    export arm926ejs_reset

    extern board_init
    extern mmu_setup
    extern |Image$$ER_ZI$$ZI$$Base|
    extern |Image$$ER_ZI$$ZI$$Length|
    extern |Image$$ER_RW$$RW$$Base|
    extern |Image$$ER_RW$$RW$$Length|
    extern |Load$$ER_RW$$Base|

    ; This is the user application that is called by the startup code
    ; once board initialization is complete
    extern c_entry

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Private defines and data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MODE_USR    EQU 0x010
MODE_FIQ    EQU 0x011
MODE_IRQ    EQU 0x012
MODE_SVC    EQU 0x013
MODE_ABORT  EQU 0x017
MODE_UNDEF  EQU 0x01b
MODE_SYSTEM EQU 0x01f
MODE_BITS   EQU 0x01f
I_MASK      EQU 0x080
F_MASK      EQU 0x040

; End of internal RAM
END_OF_IRAM EQU IRAM_SIZE

; Masks used to disable and enable the MMU and caches
MMU_DISABLE_MASK   EQU    0xFFFFEFFA
MMU_ENABLE_MASK    EQU    0x00001005
MMU_ICACHE_BIT     EQU    0x1000

	PRESERVE8
	AREA STARTUP, CODE   ; Startup code	
	ENTRY

arm926ejs_reset
	B     arm926ejs_entry
    B     .
    B     .
    B     .
    B     .
    B     .
    B     .
    B     .

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
arm926ejs_entry
	IF :DEF:USE_MMU
    ; Put the processor is in system mode with interrupts disabled
    MOV   r0, #MODE_SVC:OR:I_MASK:OR:F_MASK
    MSR   cpsr_cxsf, r0

    ; Ensure the MMU is disabled
    MRC   p15, 0, r6, c1, c0, 0
    LDR   r2,=MMU_DISABLE_MASK
    AND   r6, r6, r2
    MCR   p15, 0, r6, c1, c0, 0

    ; Invalidate TLBs and invalidate caches
    MOV   r7,#0
    MCR   p15, 0, r7, c8, c7, 0
    MCR   p15, 0, r7, c7, c7, 0

    ; Enable instruction cache
    ORR   r6, r6, #MMU_ICACHE_BIT
    MCR   p15, 0, r6, c1, c0, 0
	ENDIF

    LDR   sp, =END_OF_IRAM
    ; Get end of internal memory and set aside 16K for the page table
    ; SVC stack is under page table
    SUB   sp, sp, #(16*1024)

    ; Clear ZI segment
    LDR   r0, =|Image$$ER_ZI$$ZI$$Base|
    LDR   r1, =|Image$$ER_ZI$$ZI$$Length|
    MOV   r7, #0
clearzi
    CMP   r1, #0
    BEQ   clearzi_exit
    SUB   r1, r1, #1
    STRB  r7, [r0], #1 ; r7 previously set to 0
    B clearzi
clearzi_exit

	; Relocate read/write data segment before using it
	IF :DEF:RW_RELOC
	; Get the start of the RW segment located in image
    LDR   r7, =|Load$$ER_RW$$Base|

	; Get address to move image too
    LDR   r0, =|Image$$ER_RW$$RW$$Base|
    LDR   r1, =|Image$$ER_RW$$RW$$Length|
	ADD   r1, r0, r1

rwmove
	CMP   r0, r1
	BEQ   rwmove_exit
	LDRB  r2, [r7], #1
    STRB  r2, [r0], #1
    B     rwmove
rwmove_exit
	ENDIF

	IF :DEF:USE_BOARD_INIT
    ; Initialize board
    BL    board_init
	ENDIF

	IF :DEF:USE_MMU
    ; MMU page table is at the end of IRAM, last 16K
    LDR   r0, =END_OF_IRAM
    SUB   r0, r0, #(16*1024)
    MCR   p15, 0, r0, c2, c0, 0
	bl    mmu_setup

    ; Setup the Domain Access Control as all Manager
    ; Make all domains open, user can impose restrictions
    MVN   r7, #0
    MCR   p15, 0, r7, c3, c0, 0

    ; Setup jump to run out of virtual memory at location inVirtMem
    LDR   r5, =inVirtMem

    ; Enable the MMU with instruction and data caches enabled
    LDR   r2,=MMU_ENABLE_MASK
    ORR   r6, r6, r2
    MCR   p15, 0, r6, c1, c0, 0

    ; Jump to the virtual address
    MOV   pc, r5

    ; The following NOPs are to clear the pipeline after the MMU virtual
    ; address jump
    NOP
    NOP
    NOP
inVirtMem
	ENDIF

	IF :DEF:USE_ALL_STACKS

    ; The code is operating out of virtual memory now - register R3
    ; contains the virtual address for the top of stack space

	; SVC stack was previously setup, use it's location as the
	; reference for the other stacks
    MOV   r3, sp
    SUB   r3, r3, #SVC_STACK_SIZE

    ; All interrupts disabled at core for all modes
    MOV   r1, #I_MASK:OR:F_MASK ; No Interrupts

    ; Enter FIQ mode and setup the FIQ stack pointer
    ORR   r0, r1, #MODE_FIQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #FIQ_STACK_SIZE

    ; Enter IRQ mode and setup the IRQ stack pointer
    ORR   r0, r1, #MODE_IRQ
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #IRQ_STACK_SIZE

    ; Enter Abort mode and setup the Abort stack pointer
    ORR   r0, r1, #MODE_ABORT
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #ABORT_STACK_SIZE

    ; Enter Undefined mode and setup the Undefined stack pointer
    ORR   r0, r1, #MODE_UNDEF
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #UNDEF_STACK_SIZE

    ; Enter System mode and setup the User/System stack pointer
    ORR   r0, r1, #MODE_SYSTEM
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SYSTEM_STACK_SIZE

    ; Re-enter SVC mode for runtime initialization
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0

	ENDIF

    ; 1-way jump to application
    LDR  pc, =c_entry

    END
