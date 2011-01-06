/***********************************************************************
 * $Id:: board.c 3380 2010-05-05 23:54:23Z usb10132   $
 *
 * Project: FDI3250 Board related functions
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

#include "lpc32xx_wdt.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "board.h"

/***********************************************************************
 *
 * Function: board_reset
 *
 * Purpose: Reset the board
 *
 * Processing:
 *     Reset the board via the watchdog timer.
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
void board_reset(void)
{
  /* Enable watchdog clocking */
  CLKPWR->clkpwr_timer_clk_ctrl = CLKPWR_PWMCLK_WDOG_EN;

  /* Instant assertion of RESETOUT_N with pulse length 1mS */
  WDT->wdtim_pulse = 13000;
  WDT->wdtim_mctrl = WDT_M_RES2 | WDT_RESFRC1 | WDT_RESFRC2;
 
  while (1);
}

/***********************************************************************
 *
 * Function: fdi3250_toggle_led
 *
 * Purpose: Toggles LED
 *
 * Processing:
 *     Toggles LED on the board based on the on value.
 *
 * Parameters:
 *     on : TRUE to enable LED, FALSE to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void fdi3250_toggle_led(BOOL_32 on)
{
  UNS_32 set, clr;

  /* D3 LED is on GPO */
  if (on == FALSE)
  {
    set = 0;
    clr = P3_STATE_GPO(3);
  }
  else
  {
    set = P3_STATE_GPO(3);
    clr = 0;
  }

  /* Set D3 on GPO_03 */
  gpio_set_gpo_state(set, clr);
}
