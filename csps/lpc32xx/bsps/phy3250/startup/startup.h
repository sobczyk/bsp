/***********************************************************************
 * $Id:: startup.h 3374 2010-05-05 22:24:40Z usb10132                  $
 *
 * Project: Generic 32x0 startup code
 *
 * Description:
 *     This contains the startup code functions used to bring up a
 *     32x0 board using the generic 32x0 startup code.
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

#ifndef STARTUP_H
#define STARTUP_H

#include "lpc_types.h"
#include "lpc_arm922t_cp15_driver.h"
#include "setup.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
 * Cache support functions
 **********************************************************************/

void dcache_flush(void); /* Also flushes write buffer */
void dcache_inval(void); /* Invalidates entire dcache, careful! */
void icache_inval(void); /* Invalidates entire icache */

/***********************************************************************
 * Misc startup code support functions
 **********************************************************************/

void gpio_setup(void);
void clock_setup(UNS_32 clkrate, UNS_32 hdiv, UNS_32 pdiv);
void mem_setup(void);
void mmu_setup(TRANSTABLE_T *mmu_base_aadr);
void misc_setup(void);

/* Main startup code entry point, called from reset entry code */
void board_init(void);

/***********************************************************************
 * DDR and SDR SDRAM setup functions
 **********************************************************************/

void sdr_sdram_setup(UNS_32 clk);

/***********************************************************************
 * Generic SDRAM support functions (DDR and SDR configs)
 **********************************************************************/

/* Finds the optimal address mapping for the current row, bank, column,
   and access mode (low pwoer or performance). This value is used to
   program the address mapping field in the SDRAM dynamic control
   register. */
UNS_32 sdram_find_config(void);

/* Optimizes DRAM interface timing for the current clock speed */
void sdram_adjust_timing(UNS_32 clk);

/* Returns the offset value to the start of the address range for
   SDRAM to access a specific bank pattern */
UNS_32 sdram_get_bankmask(UNS_32 ba1, UNS_32 ba0);

/***********************************************************************
 * Diagnostic support functions for SDRAM
 **********************************************************************/

/* Computed modeshift, bankshift, and bus width values */
extern int modeshift;
extern int bankshift;
extern const int bus32;

/* Saved computed mode and extended mode program address */
extern UNS_32 modeaddr, extmodeaddr;

#ifdef __cplusplus
}
#endif

#endif /* STARTUP_H */
