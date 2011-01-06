 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
 ; $Id:: ldscript_ram_rvw.ld 453 2008-03-25 18:08:43Z wellsk           $
 ; 
 ; Project: Phytec LPC3250 example code linker file for ARM Realview
 ; 
 ; Description: 
 ;     This file implements a linker script for the Phytec LPC3250 board
 ;     when used with the example code.
 ;
 ; Notes: None
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

ISRAM 0x80000000
{
    	ER_RO +0
        {
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

