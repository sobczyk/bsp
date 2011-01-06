;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id$
; 
; Project: Phytec 3250 board stack setup code
;
; Description:
;     Basic stack setup code for the Phytec 3250 board using Keil's JTAG
;
; Notes:
;     This version of the file is for the Realview 3.x toolset.
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

    export phy3250_stack_setup

    extern __main

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
END_OF_IRAM EQU 0x08040000

; Default stack sizes
FIQ_STACK_SIZE     EQU    512
IRQ_STACK_SIZE     EQU    1024
ABORT_STACK_SIZE   EQU    512
UNDEF_STACK_SIZE   EQU    512
SYSTEM_STACK_SIZE  EQU    512
SVC_STACK_SIZE     EQU    4096

	PRESERVE8
	AREA STACK_STARTUP, CODE

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Function: phy3250_stack_setup
;
; Purpose: Setup the stacks for each ARM modes
;
; Description:
;     Setup the stacks for each ARM modes: FIQ, IRQ, ABORT, UNDEF,
;     SYSTEM, SVC    
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
phy3250_stack_setup

    ; Setup current stack pointer to end of internal memory
    LDR   r3, =END_OF_IRAM

    ; Initialize stacks for all modes
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

    ; Enter SVC mode and setup the SVC stack pointer.
    ; This is the mode for runtime initialization.
    ORR   r0, r1, #MODE_SVC
    MSR   cpsr_cxsf, r0
    MOV   sp, r3
    SUB   r3, r3, #SVC_STACK_SIZE

    ; Get address of application to execute
    LDR   pc, =__main

    END
