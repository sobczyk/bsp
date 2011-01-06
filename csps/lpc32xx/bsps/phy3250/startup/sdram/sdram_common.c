/***********************************************************************
 * $Id:: sdram_common.c 3439 2010-05-10 16:42:51Z usb10132             $
 *
 * Project: Common SDRAM code for SDR and DDR systems
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

#include "startup.h"
#include "dram_configs.h"
#include "lpc32xx_emc.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * Startup code private data
 **********************************************************************/

struct SDRAM_CFG_MAP
{
	UNS_8 bank_bits;
	UNS_8 rows;
	UNS_8 cols;
	UNS_8 cfgval;
};

#define SDRAM_LP_MASK_BIT (1 << 5)

/* Note: In some cases, the configuration may not completely match
   what is in the User's guide (ie, 32Mx16 uses value 0x11, while
   32Mx8 uses 0x08). This is ok as long as the Row, column, and
   banks are the same. So..it's possible for a 32Mx configuration
   to use value 0x11 instead and it will work fine. */

/* 32-bit bus mappings */
const int bus32 = 1;
#define SDRAM_MAX_CONFIGS 13
const struct SDRAM_CFG_MAP sdram_map[SDRAM_MAX_CONFIGS] =
{
	/* 1Gb (32Mx32), 4 banks, row length = 13, column length = 10 */
	/* 512Mb (32Mx16), 4 banks, row length = 13, column length = 10 */
	/* 256Mb (32Mx8), 4 banks, row length = 13, column length = 10 */
	{2, 13, 10, 0x91},
	/* 512Mb (64Mx8), 4 banks, row length = 13, column length = 11 */
	{2, 13, 11, 0x90},
	/* 256Mb (8Mx32), 4 banks, row length = 13, column length = 8 */
	{2, 13, 8,  0x8e},
	/* 512Mb (16Mx32), 4 banks, row length = 13, column length = 9 */
	/* 256Mb (16Mx16), 4 banks, row length = 13, column length = 9 */
	{2, 13, 9,  0x8d},
	/* 128Mb (4Mx32), 4 banks, row length = 12, column length = 8 */
	/* 64Mb (4Mx16), 4 banks, row length = 12, column length = 8 */
	{2, 12, 8,  0x8a},
	/* 256Mb (8Mx32), 4 banks, row length = 12, column length = 9 */
	/* 128Mb (8Mx16), 4 banks, row length = 12, column length = 9 */
	/* 64Mb (8Mx8), 4 banks, row length = 12, column length = 9 */
	{2, 12, 9,  0x89},
	/* 128Mb (16Mx8), 4 banks, row length = 12, column length = 10 */
	{2, 12, 10, 0x88},
	/* 64Mb (2Mx32), 4 banks, row length = 11, column length = 8 */
	{2, 11, 8,  0x86},
	/* 16Mb (1Mx16), 2 banks, row length = 11, column length = 8 */
	{1, 11, 8,  0x81},
	/* 16Mb (2Mx8), 2 banks, row length = 11, column length = 9 */
	{1, 11, 9,  0x80},
};

int modeshift;
int bankshift;

#ifdef ENABLE_DEBUG
UNS_32 modeaddr, extmodeaddr;
#endif

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: sdram_adjust_timing
 *
 * Purpose: Optimizes DRAM interface timing for the current clock speed
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk : SDRAM interface clock rate
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *    For every value, you can specify a clock count instead of a time.
 *
 **********************************************************************/
void sdram_adjust_timing(UNS_32 clk)
{
	/* Setup percharge command delay */
#if SDRAM_TRP_DELAY > 15
	EMC->emcdynamictrp = EMC_DYN_PRE_CMD_PER(clk / SDRAM_TRP_DELAY);

#else
	EMC->emcdynamictrp = EMC_DYN_PRE_CMD_PER(SDRAM_TRP_DELAY - 1);
#endif

	/* Setup Dynamic Memory Active to Precharge Command period */
#if SDRAM_TRAS_DELAY > 15
	EMC->emcdynamictras = EMC_DYN_ACTPRE_CMD_PER(clk / SDRAM_TRAS_DELAY);

#else
	EMC->emcdynamictras =
		EMC_DYN_ACTPRE_CMD_PER(SDRAM_TRAS_DELAY - 1);
#endif

	/* Dynamic Memory Self-refresh Exit Time */
#if SDRAM_TSREX_TIME > 15
	EMC->emcdynamictsrex =
		EMC_DYN_SELF_RFSH_EXIT(clk / SDRAM_TSREX_TIME);

#else
		EMC->emcdynamictsrex =
			EMC_DYN_SELF_RFSH_EXIT(SDRAM_TSREX_TIME - 1);
#endif

	/* Dynamic Memory Self-refresh Exit Time */
#if SDRAM_TWR_TIME > 15
	EMC->emcdynamictwr =
		EMC_DYN_WR_RECOVERT_TIME(clk / SDRAM_TWR_TIME);

#else
	EMC->emcdynamictwr =
		EMC_DYN_WR_RECOVERT_TIME(SDRAM_TWR_TIME - 1);
#endif

	/* Dynamic Memory Active To Active Command Period */
	EMC->emcdynamictrc = EMC_DYN_ACT2CMD_PER(7);

	/* Dynamic Memory Auto-refresh Period */
#if SDRAM_TRFC_TIME > 15
	EMC->emcdynamictrfc =
		EMC_DYN_AUTOREFRESH_PER(clk / SDRAM_TRFC_TIME);

#else
	EMC->emcdynamictrfc =
		EMC_DYN_AUTOREFRESH_PER(SDRAM_TRFC_TIME - 1);
#endif

	/* Dynamic Memory Active To Active Command Period */
#if SDRAM_TXSNR_TIME > 15
	EMC->emcdynamictxsr =
		EMC_DYN_EXIT_SRFSH_TIME(clk / SDRAM_TXSNR_TIME);

#else
		EMC->emcdynamictxsr =
			EMC_DYN_EXIT_SRFSH_TIME(SDRAM_TXSNR_TIME - 1);
#endif

	/* Dynamic Memory Active Bank A to Active Bank B Time */
#if SDRAM_TRRD_TIME > 15
	EMC->emcdynamictrrd = EMC_DYN_BANKA2BANKB_LAT(clk / SDRAM_TRRD_TIME);

#else
	EMC->emcdynamictrrd =
		EMC_DYN_BANKA2BANKB_LAT(SDRAM_TRRD_TIME - 1);
#endif

	/* Dynamic Memory Load Mode Register To Active Command Time */
#if SDRAM_TRRD_TIME > 15
	EMC->emcdynamictmrd =
		EMC_DYN_LM2ACT_CMD_TIME(clk / SDRAM_TMRD_TIME);

#else
	EMC->emcdynamictmrd = EMC_DYN_LM2ACT_CMD_TIME(SDRAM_TMRD_TIME);
#endif

	/* Dynamic Memory Last Data In to Read Command Time */
#if SDRAM_TCDLR_TIME > 15
	EMC->emcdynamictcdlr =
		EMC_DYN_LASTDIN_CMD_TIME(clk / SDRAM_TCDLR_TIME);

#else
	EMC->emcdynamictcdlr = EMC_DYN_LASTDIN_CMD_TIME(SDRAM_TCDLR_TIME);
#endif

	/* Note : Refresh timing needs to be performed outside this
	   function after this function exits. Something similar to the
	   following can be used:
           EMC->emcdynamicrefresh =
               EMC_DYN_REFRESH_IVAL(clk / SDRAM_RFSH_INTERVAL);
    */
}

/***********************************************************************
 *
 * Function: sdram_find_config
 *
 * Purpose: Returns the dynamic controller config based on config
 *
 * Processing:
 *    Finds the optimal address mapping for the current row, bank,
 *    column, and access mode (low pwoer or performance). This value is
 *    used to program the address mapping field in the SDRAM dynamic
 *    control register.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 sdram_find_config(void)
{
	int idx;
	UNS_32 cfg = 0;

	for (idx = 0; (idx < SDRAM_MAX_CONFIGS) && (cfg == 0); idx++)
	{
		if ((SDRAM_COLS == sdram_map[idx].cols) &&
			(SDRAM_ROWS == sdram_map[idx].rows) &&
			(SDRAM_BANK_BITS == sdram_map[idx].bank_bits))
		{
			cfg = (UNS_32) sdram_map[idx].cfgval;
#if SDRAM_USE_PERFORMANCE_MODE==0
			cfg |= SDRAM_LP_MASK_BIT;
#endif
		}
	}

	/* Compute shift value for mode word, we will need it later */
#if SDRAM_USE_PERFORMANCE_MODE==1
	/* Performance mode : Row - Bank - Col mapping */
	modeshift = SDRAM_COLS + bus32 + 1 + SDRAM_BANK_BITS;
	bankshift = SDRAM_COLS + bus32 + 1;
#else
	/* Low power mode : Bank - Row - Col mapping */
	modeshift = SDRAM_COLS + bus32 + 1;
	bankshift = SDRAM_COLS + SDRAM_ROWS + bus32 + 1;
#endif

	return cfg;
}

/***********************************************************************
 *
 * Function: sdram_get_bankmask
 *
 * Purpose: Returns offset to set BA0 and BA1 to the passed state
 *
 * Processing:
 *     Returns the offset value to the start of the address range for
 *     SDRAM to access a specific bank pattern.
 *
 * Parameters:
 *     ba1: state for BA1 signal (o or 1)
 *     ba0: state for BA0 signal (o or 1)
 *
 * Outputs: None
 *
 * Returns:
 *     An offet to address to the SDRAM base address to address the
 *     DRAM with the bank pins in the passed states.
 *
 * Notes:
 *     Assumes BA0 connected to A13 and BA1 connected to A14. In some
 *     cases, BA0 and BA1 are swapped. This accounts for those cases.
 *
 **********************************************************************/
UNS_32 sdram_get_bankmask(UNS_32 ba1, UNS_32 ba0)
{
	UNS_32 bankoffs = 0;

	if (bankshift & 1)
	{
		/* Swap bank pins */
		bankoffs =(ba0 << 1) | ba1;
	}
	else
	{
		/* If the bank shift is even, the BA0 and BA1 do not need to
		   be swapped. */
		bankoffs =(ba1 << 1) | ba0;
	}

	return (bankoffs << bankshift);
}
