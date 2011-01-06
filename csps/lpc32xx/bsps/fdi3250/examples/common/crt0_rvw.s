;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: crt0_rvw.s 614 2008-04-18 19:25:38Z wellsk                     $
; 
; Project: Phytec LPC3250 example startup code
;
; Description:
;     Basic startup code for running examples
;
; Notes:
;     This version of the file is for the ARM Realview toolset.
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

    export __main

    ; This is the user application that is called by the startup code
    ; once the stacks are saved
    extern c_entry

    extern |Image$$ER_ZI$$ZI$$Base|
    extern |Image$$ER_ZI$$ZI$$Length|

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	PRESERVE8
	AREA STARTUP, CODE   ; Startup code
	ENTRY

__main
    STMFD sp!, {lr}
    STMFD sp!, {r0 - r12, lr}

; Get start and compute end of zero initialized data segment
    LDR r0, =|Image$$ER_ZI$$ZI$$Base| ; Get start of bss segment
    LDR r1, =|Image$$ER_ZI$$ZI$$Length|   ; Get end of bss segment
    ADD r1, r0, r1

; Clear out the xero-init (ZI) data segment
    LDR r2, =0              ; r2 = 0
zi_clear
    CMP r0, r1              ; Compare current and last clear address
    BGE zi_clear_done       ; Was this the last address?
    STR r2, [r0]            ; Not last address, clear data area
    ADD r0, r0, #4          ; Increment to next word address
    B zi_clear              ; Continue

; After the ZI segment is cleared, set up the stacks needed for the
; examples
zi_clear_done

c_start
    BL c_entry              ; Jump to the c_entry()

exit
    LDMFD sp!, {r0 - r12, lr, pc}

    END
