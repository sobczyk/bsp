/***********************************************************************
 * $Id:: wdt_example.c 1118 2008-08-21 21:02:18Z stefanovicz           $
 *
 * Project: WDT driver example
 *
 * Description:
 *     A simple WDT driver example.
 *
 * Notes:
 *     This example controls the RESETOUT pin (testpoint 4E on the
 *     extension board). A single short low pulse can be observed on
 *     the pin running this demo. If RESET_CHIP_example is defined,
 *     the WDT will generate a 10 ms delay after the initial low
 *     pulse and a 3 ms low RESETOUT pulse will follow. At the same
 *     time a chip reset will reset the microcontroller itself.
 *
 *     RESET_CHIP_example define is uncommented by default.
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
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_ssp_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_wdt_driver.h"

/* WDT device handles */
static INT_32 wdtdev;

/* WDT device setup */
static WDT_SETUP_T wdtsetup;

/* Uncomment me to enable chip reset using M_RES2 option
   See the following code. Use with care!                */
//#define RESET_CHIP_example

/***********************************************************************
 * WDT example private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: delay
 *
 * Purpose: generate a delay
 *
 * Processing:
 *     A local software counter counts up to a specified count.
 *
 * Parameters:
 *    cnt : number to be counted
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void delay(UNS_32 cnt)
{
  UNS_32 i = cnt;
  while (i != 0) i--;
  return;
}

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/***********************************************************************
 *
 * Function: wdt_user_interrupt
 *
 * Purpose: WDT interrupt handler
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
void wdt_user_interrupt(void)
{
  wdt_ioctl(wdtdev, WDT_INT_CLEAR, 0);
  return;
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
 * Returns: Always returns 1, or <0 on an error
 *
 * Notes: None
 *
 **********************************************************************/
int c_entry(void)
{
  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Enable simple WDT clock */
  clkpwr_clk_en_dis(CLKPWR_WDOG_CLK, 1);

  /* Install WDT interrupt handler as an IRQ interrupt */
  int_install_ext_irq_handler(IRQ_WATCH,
                              (PFV) wdt_user_interrupt, ACTIVE_HIGH, 1);

  /* Open WDT */
  wdtdev = wdt_open(WDT, 0);
  if (wdtdev == 0)
  {
    /* Error */
    return -1;
  }

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Drive RESETOUT high/low/high using RESFRC1 */
  wdtsetup.initial_setup = TRUE;
  wdtsetup.pause = 1;
  wdtsetup.resfrc2 = 0;
  wdtsetup.resfrc1 = 0;
  wdtsetup.m_res2 = 0;
  wdtsetup.m_res1 = 0;
  wdtsetup.ext_match_setup = WDT_EXT_MATCH_IDLE;
  wdtsetup.match_setup = WDT_MATCH_IDLE;
  wdtsetup.match0_update = FALSE;
  wdtsetup.counter_update = FALSE;
  wdtsetup.pulse_update = FALSE;
  wdt_ioctl(wdtdev, WDT_SETUP, (UNS_32) &wdtsetup);
  delay(10000);

  wdtsetup.resfrc1 = 1;
  wdt_ioctl(wdtdev, WDT_SETUP, (UNS_32) &wdtsetup);
  delay(10000);

  wdtsetup.resfrc1 = 0;
  wdt_ioctl(wdtdev, WDT_SETUP, (UNS_32) &wdtsetup);
  delay(10000);

#ifdef RESET_CHIP_example
  /* generate a 10 ms delay followed by a 3 ms */
  /* RESETOUT pulse using M_RES2 = 1           */
  wdt_ioctl(wdtdev, WDT_TIMER_STOP, 0);
  wdt_ioctl(wdtdev, WDT_INT_DISABLE, 0);
  wdt_ioctl(wdtdev, WDT_INT_CLEAR, 0);

  wdtsetup.initial_setup = TRUE;
  wdtsetup.pause = 1;
  wdtsetup.resfrc2 = 0;
  wdtsetup.resfrc1 = 0;
  wdtsetup.m_res2 = 1;
  wdtsetup.m_res1 = 0;
  wdtsetup.ext_match_setup = WDT_EXT_MATCH_HIGH;
  wdtsetup.match_setup = WDT_MATCH_EN_INT_RESET;
  wdtsetup.match0 = 10 * 13000;
  wdtsetup.match0_update = TRUE;
  wdtsetup.counter_update = FALSE;
  wdtsetup.pulse = 3 * 13000 - 5;
  wdtsetup.pulse_update = TRUE;
  wdt_ioctl(wdtdev, WDT_SETUP, (UNS_32) &wdtsetup);

  wdt_ioctl(wdtdev, WDT_INT_CLEAR, 0);
  wdt_ioctl(wdtdev, WDT_INT_ENABLE, 0);
  int_enable(IRQ_WATCH);
  while (1);
#endif

  /* Close WDT */
  wdt_close(wdtdev);

  return 1;
}
