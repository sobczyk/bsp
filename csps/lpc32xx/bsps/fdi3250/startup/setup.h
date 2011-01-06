/***********************************************************************
 * $Id:: setup.h 3437 2010-05-10 16:40:57Z usb10132                    $
 *
 * Project: Setup macros for the generic 32x0 startup code
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only  
 * which provides customers with programming information regarding the  
 * products. This software is supplied "AS IS" without any warranties.  
 * NXP Semiconductors assumes no responsibility or liability for the 
 * use of the software, conveys no license or title under any patent, 
 * copyright, or mask work right to the product. NXP Semiconductors 
 * reserves the right to make changes in the software without 
 * notification. NXP Semiconductors also make no representation or 
 * warranty that such application will be suitable for the specified 
 * use without further testing or modification. 
 **********************************************************************/

#ifndef SETUP_H
#define SETUP_H

/* Module rev can be 1, 2, or 3 - effects the amount of DRAM */
#ifndef SOMDIMM_LPC3250_REV
    #define SOMDIMM_LPC3250_REV 1
#endif

/* The code can be made slightly smaller by disabling the debug code.
   This code integrates with S1L and provides early help for debug of
   SDRAM initialization. Comment out this code to save space once
   your system is stable or if your image size is too big. */
#define ENABLE_DEBUG

/* LPC3220 users should changes to following define to 0x20000, while
   other LCP32x0 users should keep this at 0x40000 */
#define IRAM_SIZE 0x40000

/* CPU speed in Hz */
#define CPU_CLOCK_RATE 266000000

/* HCLK bus divider rate - this can be 1, 2 or 4 and defines the HCLK
  bus rate in relation to the CPU speed. This can never exceed
  133MHz. DDR systems only work with a value of 2 or 4. */
#define HCLK_DIVIDER 2

/* PCLK divider rate - this can be 1 to 32 and defines PCLK bus rate
   in relation to CPU speed. This value should be picked to allow the
   PCLK to run as close as possible to 13MHz. Slightly exceeding 13Mhz
   is ok */
#define PCLK_DIVIDER 20

/* Examples for clock settings:
   208MHz CPU, 104MHz bus, 13MHz PCLK
     CPU_CLOCK_RATE = 208000000
     HCLK_DIVIDER   = 2
     PCLK_DIVIDER   = 16
   266MHz CPU, 133MHz bus, 13.325MHz PCLK
     CPU_CLOCK_RATE = 266000000
     HCLK_DIVIDER   = 2
     PCLK_DIVIDER   = 20
   100MHz CPU, 100MHz bus, 13.325MHz PCLK
     CPU_CLOCK_RATE = 100000000
     HCLK_DIVIDER   = 1
     PCLK_DIVIDER   = 7
   90MHz CPU, 90MHz bus, 12.857MHz PCLK
     CPU_CLOCK_RATE = 900000000
     HCLK_DIVIDER   = 1
     PCLK_DIVIDER   = 7
*/

/* Clock switching allows the clock speed to be changed from the
   S1L command line. The clock rates are saved as part of the S1L
   config and are restored the next time the board is booted. On
   DDR systems, the board will reset when you issue this command
   to re-initialize memory. This command is entirely experimental
   and currently does not work correctly. Use at your own risk. */
//#define ENABLE_CKLSWITCHING

#endif /* SETUP_H */
