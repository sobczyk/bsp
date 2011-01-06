/***********************************************************************
 * $Id:: clock_setup.c 3376 2010-05-05 22:28:09Z usb10132              $
 *
 * Project: Initial clock setup code
 *
 * Description:
 *     Initial clock setup for the board
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
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_timer_driver.h"

/***********************************************************************
 *
 * Function: clock_setup
 *
 * Purpose: Setup system clocking
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clkrate : Rate in Hz to set the CPU clock too
 *     hdiv    : Bus clock (HCLK) divider
 *     pdiv    : Peripheral clock (PCLK) divider
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     This function uses the clock and power driver. A lot of code
 *     space can be recovered not using that driver and setting the
 *     clock registers with direct values.
 *
 **********************************************************************/
void clock_setup(UNS_32 clkrate, UNS_32 hdiv, UNS_32 pdiv)
{
	CLKPWR_HCLK_PLL_SETUP_T pllcfg;

	clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_STOP, 1, 2);
	clkpwr_set_mode(CLKPWR_MD_DIRECTRUN);
	clkpwr_pll_dis_en(CLKPWR_HCLK_PLL, 0);
	timer_wait_ms(TIMER_CNTR0, 2);

	/* Is the PLL397 or the oscillator being used for SYSCLK? */
	if (clkpwr_get_osc() == CLKPWR_PLL397_OSC)
	{
		/* PLL397 is being used, try to switch to the main oscillator */

		/* Enable the main oscillator */
		clkpwr_mainosc_setup(0, 1);

		/* Wait 100mS to allow the oscillator to power up */
		timer_wait_ms(TIMER_CNTR0, 100);

		/* Switch over the main oscillator and disable PLL397 */
		clkpwr_sysclk_setup(CLKPWR_MAIN_OSC, 0x50);
		clkpwr_pll397_setup(0, 0, 0);
	}
	else 
	{
	    /* Set bad phase timing only */
		clkpwr_sysclk_setup(CLKPWR_MAIN_OSC, 0x50);
	}

	/* Setup the HCLK PLL for 208MHz operation, but if a configuration
	   can't be found, stay in direct run mode */
	clkrate = clkpwr_find_pll_cfg(MAIN_OSC_FREQ, clkrate, 5, &pllcfg);
	if (clkrate != 0)
	{
		/* PLL configuration is valid, so program the PLL with the
		   computed configuration data */
		clkpwr_hclkpll_setup(&pllcfg);

		/* Wait for PLL to lock */
		while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

		/* Setup intial dividers, DDR clocking will only be enabled
		   if configured for DDR mode */
		clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_STOP, pdiv, hdiv);

		/* Switch to run mode */
		clkpwr_force_arm_hclk_to_pclk(0);
		clkpwr_set_mode(CLKPWR_MD_RUN);
	}
}
