/***********************************************************************
 * $Id:: sys_hw.c 3545 2010-05-19 21:58:15Z usb10132                   $
 *
 * Project: NXP PHY3250 startup code for stage 1
 *
 * Description:
 *     This file provides initialization code for the S1L.
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

#include "lpc_types.h"
#include "s1l_sys_inf.h"
#include "s1l_sys.h"
#include "sys.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_emc.h"
#include "startup.h"

/***********************************************************************
 *
 * Function: clock_adjust
 *
 * Purpose: Safely adjust or readjust system clocks from saved values
 *
 * Processing:
 *     Safely adjust the system clocks by placing DRAM in sleep mode,
 *     adjusting clocks, and restoring DRAM state.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: New clock frequency in Hz
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clock_adjust(void)
{
	CLKPWR_HCLK_PLL_SETUP_T pllcfg;
	UNS_32 freqr;

	/* Place DRAM into self refresh mode */
	CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_UPD_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_UPD_SDRAM_SELF_RFSH;
	CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_AUTO_SDRAM_SELF_RFSH;
	while ((EMC->emcstatus & EMC_DYN_SELFRESH_MODE_BIT) == 0);

	/* Enter direct-run mode */
	clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_STOP, 1, 2);
	clkpwr_set_mode(CLKPWR_MD_DIRECTRUN);

	/* Find and set new PLL frequency */
	clkpwr_pll_dis_en(CLKPWR_HCLK_PLL, 0);
	clkpwr_find_pll_cfg(MAIN_OSC_FREQ, s1l_board_cfg.armclk,
		10, &pllcfg);
    freqr = clkpwr_hclkpll_setup(&pllcfg);
	if (freqr != 0) 
	{
		/* Wait for PLL to lock before switching back into RUN mode */
		while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

		/* Switch out of direct-run mode and set new dividers */
		clkpwr_set_mode(CLKPWR_MD_RUN);
		clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_STOP,
			s1l_board_cfg.pclkdiv, s1l_board_cfg.hclkdiv);

		/* Take DRAM out of self-refresh */
		CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_SDRAM_SELF_RFSH;
		CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_UPD_SDRAM_SELF_RFSH;
		CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_UPD_SDRAM_SELF_RFSH;
		while ((EMC->emcstatus & EMC_DYN_SELFRESH_MODE_BIT) != 0);

		/* Optimize timings */
		sdram_adjust_timing(clkpwr_get_clock_rate(CLKPWR_SDRAMDDR_CLK));
		term_setbaud(syscfg.baudrate);
	}

	return freqr;
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
 *
 * Processing:
 *     See function.
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
void c_entry(void) {
	BOOL_32 goto_prompt = TRUE;

	while (1) 
	{
		/* Go to boot manager */
		boot_manager(goto_prompt);
		goto_prompt = FALSE;
	}
}
