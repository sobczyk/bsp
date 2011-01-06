/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: crt0_gnu.asm 738 2008-05-09 21:35:39Z wellsk                   $
; 
; Project: GNU C runtime startup code
;
; Description:
;     This code sets up the basic code runtime environment for and
;     application example started with the GNU toolset. This code
;     clears out the ZI segment, initialize the stacks, and branches
;     to the c_entry() function.
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

.EQU MODE_USR,     0x010
.EQU MODE_FIQ,     0x011
.EQU MODE_IRQ,     0x012
.EQU MODE_SVC,     0x013
.EQU MODE_SVC_NI,  0x0D3
.EQU MODE_ABORT,   0x017
.EQU MODE_UNDEF,   0x01b
.EQU MODE_SYSTEM,  0x01f
.EQU MODE_BITS,    0x01f
.EQU I_MASK,       0x080
.EQU F_MASK,       0x040
.EQU IF_MASK,      0x0C0

.EQU USR_STK_SIZE, 0x400    /* User mode stack size */
.EQU FIQ_STK_SIZE, 0x400    /* FIQ mode stack size */
.EQU IRQ_STK_SIZE, 0x800    /* IRQ mode stack size */
.EQU ABT_STK_SIZE, 0x400    /* Abort exception stack size */
.EQU UND_STK_SIZE, 0x400    /* Undefined exception stack size */
.EQU SYS_STK_SIZE, 0x1000   /* System mode stack size */
.EQU SVC_STK_SIZE, 0x4000   /* Supervisor mode stack size */

    .text
    .code 32
    .align 0
    .extern c_entry         /* Reference to c entry point */
    .extern __gnu_bssstart  /* Start of the bss segment */
    .extern __gnu_bssend    /* End of the bss segment */
    .extern __end           /* End of segments, needed to define stack
                               area */
    .global __gccmain       /* public method */
    .global __start         /* public code entry point */

__start:
    STMFD sp!, {lr}
    STMFD sp!, {r0 - r12, lr}

    LDR r0, =__gnu_bssstart /* Get start of bss segment */
    LDR r1, =__gnu_bssend   /* Get end of bss segment */

/* Clear out the xero-init (ZI) data segment */
    LDR r2, =0              /* r2 = 0 */
zi_clear:
    CMP r0, r1              /* Compare current and last clear address */
    BGE zi_clear_done       /* Was this the last address? */
    STR r2, [r0]            /* Not last address, clear data area */
    ADD r0, r0, #4          /* Increment to next word address */
    B zi_clear              /* Continue */

/* After the ZI segment is cleared, set up the stacks needed for the
   examples */
zi_clear_done:

c_start:
    BL c_entry              /* Jump to the c_entry() */

exit:
    LDMFD sp!, {r0 - r12, lr, pc}

/* The function gccmain() is needed only for GNU toolsets */
__gccmain:
    MOV pc, lr              /* Just return - gnu specific	*/

    .end
