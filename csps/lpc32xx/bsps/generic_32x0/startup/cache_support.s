;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; $Id:: cache_support.s 3375 2010-05-05 22:25:10Z usb10132             $
; 
; Project: Generic startup code ARM926EJS cache support functions
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

    export dcache_flush
    export dcache_inval
    export icache_inval

	AREA CACHE, CODE   ; Startup code	

dcache_flush
	MRC  p15, 0, r15, c7, c10, 3
	BNE  dcache_flush
	MCR  p15, 0, r0, c7, c10, 4 ; drain write buffer
	MOV  pc, lr

dcache_inval
	MRC  p15, 0, r15, c7, c14, 3
	BNE  dcache_inval
	MOV  pc, lr

icache_inval
	MCR  p15, 0, r0, c7, c5, 0 ; invalidate icache
	MOV  pc, lr

    END
