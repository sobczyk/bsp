 /*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 
 ; $Id:: ldscript_ram_iar.ld 1001 2008-08-06 18:07:23Z wellsk          $
 ; 
 ; Project: Phytec LPC3250 example code linker file for IAR
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
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

/* 32-bit addressable space */
define memory mem with size = 4G;

define symbol __region_SDRAM_start__   = 0x80000000;
define symbol __region_SDRAM_end__     = 0x83FFFFFF;

/* SDRAM only */
define region RAM_region   = mem:[from __region_SDRAM_start__   to __region_SDRAM_end__];

/*-Sizes-*/
define symbol __size_cstack__   = 0x500;
define symbol __size_svcstack__ = 0x200;
define symbol __size_irqstack__ = 0x100;
define symbol __size_fiqstack__ = 0x100;
define symbol __size_undstack__ = 0x80;
define symbol __size_abtstack__ = 0x80;
define symbol __size_heap__     = 0x10000;

define block CSTACK    with alignment = 8, size = __size_cstack__   { };
define block SVC_STACK with alignment = 8, size = __size_svcstack__ { };
define block IRQ_STACK with alignment = 8, size = __size_irqstack__ { };
define block FIQ_STACK with alignment = 8, size = __size_fiqstack__ { };
define block UND_STACK with alignment = 8, size = __size_undstack__ { };
define block ABT_STACK with alignment = 8, size = __size_abtstack__ { };
define block HEAP      with alignment = 8, size = __size_heap__     { };

initialize by copy { readwrite };
do not initialize  { section .noinit };

place in RAM_region   { ro, ro section .text, ro section CODESEG, rw, rw section .data,
                        block CSTACK, block SVC_STACK, block IRQ_STACK, block FIQ_STACK,
                        block UND_STACK, block ABT_STACK, block HEAP };








