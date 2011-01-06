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

#if SDRAM_32BIT_BUS==1
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

#else
/* 16-bit bus mappings */
const int bus32 = 0;
#define SDRAM_MAX_CONFIGS 8
const struct SDRAM_CFG_MAP sdram_map[SDRAM_MAX_CONFIGS] =
{
	/* 512Mb (32Mx16), 4 banks, row length = 13, column length = 10 */
	/* 256Mb (32Mx8), 4 banks, row length = 13, column length = 10 */
	{2, 13, 10, 0x11},
	/* 512Mb (64Mx8), 4 banks, row length = 13, column length = 11 */
	{2, 13, 11, 0x10},
	/* 256Mb (16Mx16), 4 banks, row length = 13, column length = 9 */
	{2, 13, 9,  0x0D},
	/* 128Mb (8Mx16), 4 banks, row length = 12, column length = 9 */
	/* 64Mb (8Mx8), 4 banks, row length = 12, column length = 9 */
	{2, 12, 9,  0x09},
	/* 128Mb (16Mx8), 4 banks, row length = 12, column length = 10 */
	{2, 12, 10, 0x08},
	/* 64Mb (4Mx16), 4 banks, row length = 12, column length = 8 */
	{2, 12, 8,  0x05},
	/* 16Mb (1Mx16), 2 banks, row length = 11, column length = 8 */
	{1, 11, 8,  0x01},
	/* 16Mb (2Mx8), 2 banks, row length = 11, column length = 9 */
	{1, 11, 9,  0x00}
};
#endif

int modeshift;
int bankshift;

/* Structures and types used for memory tests */
typedef void (*pfrvi)(UNS_32 *);
typedef int (*pfrii)(UNS_32 *);
struct _memtests
{
	pfrvi testsetup;
	pfrii testcheck;
};

/* Calibration sensitivity table */
static const UNS_8 dqs2calsen[32] = {
	7, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
};

int dqsstart, dqsend;

#ifdef ENABLE_DEBUG
UNS_32 modeaddr, extmodeaddr;
#endif

/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: *various*
 *
 * Purpose: Memory pattern and check tests
 *
 * Processing:
 *     A collection of small memory pattern generator and test functions
 *     that are used for DDR calibration.
 *
 * Parameters:
 *     base : Base address for test or check
 *
 * Outputs: None
 *
 * Returns:
 *     Check tests returns TRUE if passed, otherwise FALSE. Pattern
 *     generators return nothing.
 *
 * Notes: None
 *
 **********************************************************************/
static void walking0bitsetup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = ~(1 << i);
		base++;
	}
}
static int walking0bitcheck(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != ~(1 << i))
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static void walking1bitsetup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = (1 << i);
		base++;
	}
}
static int walking1bitcheck(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != (1 << i))
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static void invaddrsetup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = ~((UNS_32) base);
		base++;
	}
}
static int invaddrcheck(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != ~((UNS_32) base))
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static void noninvaddrsetup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = ((UNS_32) base);
		base++;
	}
}
static int noninvaddrcheck(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != ((UNS_32) base))
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static void aa55setup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = 0x55aa55aa;
		base++;
	}
}
static int aa55check(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != 0x55aa55aa)
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static void _55aasetup(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		*base = 0x55aa55aa;
		base++;
	}
}
static int _55aacheck(UNS_32 *base)
{
	int i;
	for (i = 0; i < 32; i++)
	{
		if (*base != 0x55aa55aa)
		{
			return FALSE;
		}

		base++;
	}

	return TRUE;
}
static struct _memtests testvecs[] = {
	{walking0bitsetup, walking0bitcheck},
	{walking1bitsetup, walking1bitcheck},
	{invaddrsetup, invaddrcheck},
	{noninvaddrsetup, noninvaddrcheck},
	{aa55setup, aa55check},
	{_55aasetup, _55aacheck},
	{NULL, NULL},
};

/***********************************************************************
 *
 * Function: dqsin_ddr_mod
 *
 * Purpose: Adjusts non-calibrated DQSIN delay and cal sensitivity
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ddl : New DQSIN delay value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void dqsin_ddr_mod(UNS_32 ddl)
{
	UNS_32 tmp;

	/* Adjust calibration sensitivity with DQS delay */
	tmp = CLKPWR->clkpwr_sdramclk_ctrl & ~(CLKPWR_SDRCLK_SENS_FACT(7) |
		CLKPWR_SDRCLK_DQS_DLY(0x1F));
	CLKPWR->clkpwr_sdramclk_ctrl = tmp | CLKPWR_SDRCLK_DQS_DLY(ddl) |
		CLKPWR_SDRCLK_SENS_FACT(dqs2calsen[ddl]);
}

/***********************************************************************
 *
 * Function: ddr_memtst
 *
 * Purpose: Performs various quick integrity tests of the DDR memory
 *
 * Processing:
 *     This function simply performs a series of small memory tests
 *     on subsections of the passed memory range. The intended use of
 *     this function is to test DDR at different DQSIN delay values
 *     to find an optimal value for the current device/system's
 *     process, voltage, and temperature.
 *
 * Parameters:
 *     seed  : Test seed value
 *     start : Starting range for test (ie, start of DDR memory)
 *     size  : Size of test range in bytes (ie, 64MB)
 *
 * Outputs: None
 *
 * Returns: TRUE if integrity tests passed, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
static int ddr_memtst(UNS_32 seed, UNS_32 start, UNS_32 size)
{
	int testnum;
	UNS_32 inc, *base = (UNS_32 *) start;

	/* Offset test areas so we don't accidently get the results
	   from a previous test */
	base += (seed * 0x4000) + (seed * 4);
	inc = size / sizeof(UNS_32);
	inc = inc / 256; /* 256 test sections over test range */

	/* The DDR test is performed on a number of sections. Sections are
	   small areas of DDR memory seperated by untested areas. The
	   sections tested are spread out over the entire range of the
	   device. Testing the entire DDR would take a long time, so this
	   is a good alternative. */
	while ((UNS_32) base < ((start + size) - (32 * sizeof(UNS_32))))
	{
		/* Loop through each test */
		testnum = 0;
		while (testvecs[testnum].testsetup != NULL)
		{
			testvecs[testnum].testsetup(base);
			if (testvecs[testnum].testcheck(base) == FALSE)
			{
				/* Test failed */
				return FALSE;
			}

			testnum++;
		}

		base += inc;
	}

	/* Test passed */
	return TRUE;
}

/***********************************************************************
 * Public functions
 **********************************************************************/


/***********************************************************************
 *
 * Function: ddr_find_dqsin_delay
 *
 * Purpose: Finds the DQS range for an uncalibrated DDR system
 *
 * Processing:
 *     This function goes through a range of DQSIN delay values and
 *     performs memory tests with the DQSIN delay values. A working
 *     range of DQSIN delay values is generated and the optimal range
 *     is selected for the device/system.
 *
 * Parameters:
 *     start : Starting range for test (ie, start of DDR memory)
 *     size  : Size of test range in bytes (ie, 64MB)
 *
 * Outputs: None
 *
 * Returns: TRUE if integrity tests passed, otherwise FALSE
 *
 * Notes: Calibration must be disabled for this function.
 *
 **********************************************************************/
int ddr_find_dqsin_delay(UNS_32 start, UNS_32 size)
{
	UNS_32 dqsindly;
	int ppass = 0, pass = FALSE;

	/* Most devices fail DDR in the DQSIN range of 2 or less to about
	   20+ or more, so the test will start with a DQSIN delay value of
	   1 up to 30 (max - 1) */
	dqsindly = 1;
	dqsstart = dqsend = 0xFF;

	/* At this point, DDR is initialized and opeational with the
	   exception of the DQSIN delay value and calibrarion sensitivity.
	   While adjusting the DQSIN delay between a range of values,
	   perform spot checks on uncached DDR memory to determine if DDR
	   is working with specific DQSIN delay values. By mapping out a
	   range of working values, we can determine the optimal DQSIN
	   delay value and calibration sensitivity. */
	while (dqsindly <= 31)
	{
		/* Modify the DQSIN delay and appropriate calibration sensitivity
		   before running the test */
		dqsin_ddr_mod(dqsindly);

		/* Perform some memory write-read checks of uncached DDR memory
		   to determine if the values seem to work */
		if (ddr_memtst(dqsindly, start, size) == TRUE)
		{
			/* Test passed */
			if (dqsstart == 0xFF)
			{
				dqsstart = dqsindly;
			}

			dqsend = dqsindly;
			ppass = 1;
		}
		else
		{
			/* Test failed */
			if (ppass == 1)
			{
				pass = TRUE;
				ppass = 0;
			}
		}

		/* Try next value */
		dqsindly++;
	}

	/* If the test passed, the we can use the average of the min and 
	   max values to get an optimal DQSIN delay */
	if (pass == TRUE)
	{
		dqsindly = (dqsstart + dqsend) / 2;
	}
	else
	{
		/* A working value couldn't be found, just pick something safe
		   so the system doesn't become unstable */
		dqsindly = 0xF;
	}

	/* Set calibration sensitivity based on nominal delay */
	dqsin_ddr_mod(dqsindly);

	return pass;
}

/***********************************************************************
 *
 * Function: ddr_resync_clocks
 *
 * Purpose: Resyncs the DDR clocks (DDR only)
 *
 * Processing:
 *     Resyncs DDR clocks after clocks have been changed. This is
 *     needed if the clock base for the DDR is halted or changed.
 *
 * Parameters:
 *     cfg: Dynamic config regster setting value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: DDR resync, must be done when the EMC is in DDR mode.
 *
 **********************************************************************/
void ddr_clock_resync(UNS_32 cfg)
{
	/* Note this procedure doesn't match the User's guide DDR reset
	   procedure */

	/* Reset DDR interface to resync DDR clocks, note clocks in the
	   HCLK_DIV register for DDR must be enabled. */
	CLKPWR->clkpwr_sdramclk_ctrl |= CLKPWR_SDRCLK_SW_DDR_RESET;
	CLKPWR->clkpwr_sdramclk_ctrl &= ~CLKPWR_SDRCLK_SW_DDR_RESET;

	/* Configure dynamic configuration, this gets reset when the DDR
	   clocks are resync'd, so it needs to be setup after sync. Note
	   the lower power DDR mode can be used for both standard and
	   low power DDR devices. */
	EMC->emcdynamicconfig0 = (cfg << 7) | EMC_DYN_DEV_LP_DDR_SDRAM;

#ifdef USE_DUAL_SDRAM_DEVICES
	EMC->emcdynamicconfig1 = (cfg << 7) | EMC_DYN_DEV_LP_DDR_SDRAM;
#endif
}

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
