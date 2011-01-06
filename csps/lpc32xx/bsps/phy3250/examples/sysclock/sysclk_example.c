/***********************************************************************
 * $Id:: sysclk_example.c 1124 2008-08-22 20:40:11Z wellsk             $
 *
 * Project: NXP PHY3250 System clocking change example
 *
 * Description:
 *     This example demonstrates how to change the system clocks and
 *     then readjust the interface timing based on the new clocks.
 *
 * Overview of clock change process on the LPC3250
 *     The ARM CPU clock is driven from a HPLL which is driven from an
 *     external crystal or the internal 397x PLL. The HPLL directly
 *     drives the ARM clock and is programmable for a large range of
 *     operating frequencies. The bus and peripheral clocks are also
 *     derived from the HPLL clock.
 *
 *     Changing the system clock speeds consists of the following
 *     sequence of steps for SDRAM:
 *      1.  Switch to direct-run mode from run mode. This switches the
 *          internal system clocking from the HPLL to the crystal of
 *          397x PLL.
 *      1a. SDRAM refresh timing may need to be temporarily increased
 *          because of the lower clock speed in direct-run mode to
 *          prevent data loss.
 *      2.  Stop and HPLL analog block.
 *      3.  Setup the new HPLL timing values.
 *      4.  Restart the HPLL analog block.
 *      5.  Wait for the HPLL to lock.
 *      6.  Switch to run mode form direct-run mode.
 *      7.  Adjust system timing as necessary for the new system
 *          clocks.
 *
 *     Changing the system clock speeds consists of the following
 *     sequence of steps for DDR:
 *      1.  All the code for system clock change function must be
 *          located in IRAM. When the HPLL is brought down for DDR,
 *          the DDR clock is lost. Locate the code for the system
 *          change in IRAM and jump to it.
 *      2.  Place DDR into self-refresh mode to prevent data loss
 *          when the DDR clocks are stopped.
 *      3.  Switch to direct-run mode from run mode. This switches the
 *          internal system clocking from the HPLL to the crystal of
 *          397x PLL.
 *      4.  Stop and HPLL analog block.
 *      5.  Setup the new HPLL timing values.
 *      6.  Restart the HPLL analog block.
 *      7.  Wait for the HPLL to lock.
 *      8.  Switch to run mode form direct-run mode.
 *      9.  Place DDR into normal mode (exit self-refresh) to restore
 *          the DDR clocks.
 *      10. Adjust system timing as necessary for the new system
 *          clocks.
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
#include "lpc_irq_fiq.h"
#include "phy3250_board.h"
#include "lpc32xx_clkpwr_driver.h"

/* Select a clock speed for the ARM core here. Any speed between 33 to
   266 is acceptable. */
#define ARM_CLK_MHZ 266

/* Select a bus divider value for the system. This value can be 1, 2,
   or 4. This value is used to determine the internal bus speed and the
   speed of memory. When used with the ARM_CLK_MHZ value, the bus speed
   must not go too fast or the system will no longer be stable */
#define HCLK_DIV 2

/* Last of all, select the peripheral clock divider. The peripheral
   clock is generated from ARM_CLK_MHZ value. Ideally, a value should
   be selected to allow the peripheral clock to operate around 13MHz */
#define PCLK_DIV 11

/* Based on the following values:
   ARM_CLK_MHZ = 266
   HCLK_DIV    = 2
   PCLK_DIV    = 11
   The CPU will run at 266MHz, while SDRAM will run at 133MHz and the
   peripheral clock will run at 13.3MHz */

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
void c_entry(void)
{
  UNS_32 newarmclk, newhclk, newpclk, armclk, hclk, pclk;
  UNS_8 *p8;
  CLKPWR_HCLK_PLL_SETUP_T pllcfg;
  UNS_32 freqr;
  int idx;

  /* Disable interrupts in ARM core */
  disable_irq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Read SDRAM data from EEPROM - this step is unique to the Phytec
     board. SDRAM configuration data is retrived to determine the new
   SDRAM timing later */
  p8 = (UNS_8 *) & phyhwdesc;
  for (idx = 0; idx < sizeof(phyhwdesc); idx++)
  {
    *p8 = phy3250_sspread(PHY3250_SEEPROM_CFGOFS + idx);
    p8++;
  }

  /* Get the current ARM clock, HCLK, and PCLK speeds */
  armclk = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
  hclk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
  pclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);

  /* Compute the required ARM clock rate in MHz */
  newarmclk = 1000 * 1000 * ARM_CLK_MHZ;

  /* Set PCLK to '1' in direct-run mode */
  clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM, 1, 2);
  clkpwr_set_mode(CLKPWR_MD_DIRECTRUN);

  /* Find and set new PLL frequency with a .1% tolerance */
  clkpwr_pll_dis_en(CLKPWR_HCLK_PLL, 0);
  clkpwr_find_pll_cfg(MAIN_OSC_FREQ, newarmclk,
                      10, &pllcfg);
  freqr = clkpwr_hclkpll_setup(&pllcfg);
  if (freqr != 0)
  {
    /* Wait for PLL to lock before switching back into RUN mode */
    while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

    /* Switch out of direct-run mode and set new dividers */
    clkpwr_set_mode(CLKPWR_MD_RUN);
    clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_NORM,
                         PCLK_DIV, HCLK_DIV);

    /* Optimize timings */
    sram_adjust_timing();
    idx = (phyhwdesc.dramcfg & PHYHW_DRAM_SIZE_MASK) >> 2;
    sdram_adjust_timing(&dram_cfg[idx]);
  }

  /* Compute new system clock speeds */
  newarmclk = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
  newhclk = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
  newpclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);

  /* If running the example from S1L, S1L may restore the original
     system clock rates */
}
