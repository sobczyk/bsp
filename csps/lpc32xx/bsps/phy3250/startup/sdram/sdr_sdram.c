/***********************************************************************
 * $Id:: sdr_sdram.c 3377 2010-05-05 22:29:38Z usb10132                $
 *
 * Project: Standard and low power SDRAM setup code
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
#include "lpc32xx_timer_driver.h"

/***********************************************************************
 *
 * Function: sdr_sdram_setup
 *
 * Purpose: Setup SDRAM
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk : Base SDRAM controller clock rate
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void sdr_sdram_setup(UNS_32 clk)
{
	volatile UNS_32 tmp32;
	UNS_32 tmp;

	/* Enables SDR clocking mode, (7*0.25nS) HCLK delay
	   Tests of HCLK delay have shown that voltage and frequency are
	   not driving factors for it's timing. Over testing, a working
	   value of 3 to 27 (average) appears to work and not effect other
	   operation. For this value, a hardcoded value of 7 is used. */
    CLKPWR->clkpwr_sdramclk_ctrl = CLKPWR_SDRCLK_HCLK_DLY(7);

	/* Setup slew rates */
#ifdef SDRAM_USE_SLOW_SLEW
	CLKPWR->clkpwr_sdramclk_ctrl |= (CLKPWR_SDRCLK_SLOWSLEW_CLK |
		CLKPWR_SDRCLK_SLOWSLEW | CLKPWR_SDRCLK_SLOWSLEW_DAT);

#else
	CLKPWR->clkpwr_sdramclk_ctrl &= ~(CLKPWR_SDRCLK_SLOWSLEW_CLK |
		CLKPWR_SDRCLK_SLOWSLEW | CLKPWR_SDRCLK_SLOWSLEW_DAT);
#endif

	tmp = sdram_find_config();
	if (!tmp)
	{
		/* Nothing matches, exit, DRAM won't work */
		return;
	}

	/* Setup address mapping, its ok to use low power SDRAM mode with
	   standard SDRAM devices */
	EMC->emcdynamicconfig0 = (tmp << 7) | EMC_DYN_DEV_LP_SDR_SDRAM;

	/* Setup CAS and RAS latencies */
	EMC->emcdynamicrascas0 =
		EMC_SET_CAS_IN_HALF_CYCLES(SDRAM_CAS_LATENCY) |
		EMC_SET_RAS_IN_CYCLES(SDRAM_RAS_LATENCY);

	/* Setup SDRAM read strategy */
	EMC->emcdynamicreadconfig = (EMC_SDR_CLK_NODLY_CMD_DEL |
	    EMC_SDR_READCAP_POS_POL);

	/* Setup SDRAM timing for current HCLK clock settings */
	sdram_adjust_timing(clk);

	/* Enable clocks and delay for at least 100uS to stabilize */
	tmp = (EMC_DYN_CLK_ALWAYS_ON | EMC_DYN_CLKEN_ALWAYS_ON |
		EMC_DYN_DIS_INV_MEMCLK);
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_NOP_MODE);
	timer_wait_us(TIMER_CNTR0, 100);

	/* Issue a precharge all command */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_PALL_MODE);
	EMC->emcdynamicrefresh = EMC_DYN_REFRESH_IVAL(4);
	timer_wait_us(TIMER_CNTR0, 10);

	/* Fast dynamic refresh for at least a few SDRAM clock cycles */
	timer_wait_us(TIMER_CNTR0, 10);

	/* Normal refresh timing */
	EMC->emcdynamicrefresh =
		EMC_DYN_REFRESH_IVAL(clk / SDRAM_RFSH_INTERVAL);

	/* Issue load mode command and normal mode word */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_CMD_MODE);
	tmp32 = * (volatile UNS_32 *) (EMC_DYCS0_BASE +
		(SDRAM_MODE_WORD << modeshift));
#ifdef ENABLE_DEBUG
		modeaddr = EMC_DYCS0_BASE + (SDRAM_MODE_WORD << modeshift);
#endif
	timer_wait_us(TIMER_CNTR0, 1);

	/* Normal SDRAM mode */
	EMC->emcdynamiccontrol = EMC_DYN_NORMAL_MODE |
		EMC_DYN_DIS_INV_MEMCLK | EMC_DYN_DIS_MEMCLK_IN_SFRSH;
	timer_wait_us(TIMER_CNTR0, 1);
}
