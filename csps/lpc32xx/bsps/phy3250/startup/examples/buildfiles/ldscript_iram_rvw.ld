 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
 ; $Id:: ldscript_iram_rvw.ld 3392 2010-05-06 16:05:49Z usb10132       $
 ; 
 ; Project:
 ;     32x0 linker file for Realview/Keil toolsets for IRAM boot
 ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
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

FLASH 0x00000000
{
	; The executable region starts at the load region
    	ER_RO +0
        {
                startup_entry.o (STARTUP, +FIRST) 
                * (+RO) 
        }
        ER_RW +0
        {
                * (+RW)
        }
        ER_ZI +0        
        {
                * (+ZI)
        }
}
