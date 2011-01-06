/*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: val3250_2_startup_entry.s 3288 2010-04-14 18:31:44Z usb10132   $
; 
; Project: Setup macros for the generic 32x0 startup code
;
; Notes:
;     CodeSourcery GNU toolchain version
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

/* LPC3220 users should changes to following define to 0x20000, while
   other LCP32x0 users should keep this at 0x40000 */
.EQU IRAM_SIZE,              0x40000

/* Stack sizes, change as needed, don't make them too big if they are
  located in IRAM */
.EQU FIQ_STACK_SIZE,         64
.EQU IRQ_STACK_SIZE,         512
.EQU ABORT_STACK_SIZE,       64
.EQU UNDEF_STACK_SIZE,       64
.EQU SYSTEM_STACK_SIZE,      64
.EQU SVC_STACK_SIZE,         2048
