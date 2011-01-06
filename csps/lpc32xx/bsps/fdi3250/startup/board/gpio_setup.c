/***********************************************************************
 * $Id:: gpio_setup.c 3376 2010-05-05 22:28:09Z usb10132               $
 *
 * Project: GPIO and MUX code
 *
 * Description:
 *     Provides MUX and GPIO setup for the board
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
#include "lpc32xx_gpio_driver.h"
#include "ARM9DIMM_LPC3250_pinConfig.h"
#include "LPC3250_pinMacros.h"

/* FIXME: THIS IS BROKEN */

/***********************************************************************
 *
 * Function: gpio_setup
 *
 * Purpose: Setup GPIO and MUX states
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
 * Notes: Changes these as needed.
 *
 **********************************************************************/
void gpio_setup(void)
{
  GPIO->p0_mux_set = PINCFG_PINMUX(P0);
  GPIO->p0_mux_clr = ~PINCFG_PINMUX(P0);
  GPIO->p0_dir_set = PINCFG_PINDIR(P0);
  GPIO->p0_dir_clr = ~PINCFG_PINDIR(P0);
  GPIO->p0_outp_set = PINCFG_PINSET(P0);
  GPIO->p0_outp_clr = PINCFG_PINCLR(P0);

  GPIO->p1_mux_set = PINCFG_PINMUX(P1);
  GPIO->p1_mux_clr = ~PINCFG_PINMUX(P1);
  GPIO->p1_dir_set = PINCFG_PINDIR(P1);
  GPIO->p1_dir_clr = ~PINCFG_PINDIR(P1);
  GPIO->p1_outp_set = PINCFG_PINSET(P1);
  GPIO->p1_outp_clr = PINCFG_PINCLR(P1);

  GPIO->p2_mux_set = PINCFG_PINMUX(P2);
  GPIO->p2_mux_clr = ~PINCFG_PINMUX(P2);
//    GPIO->p2_dir_set = PINCFG_PINDIR(P2);  // This is probably not correct
//    GPIO->p2_dir_clr = ~PINCFG_PINDIR(P2);

  GPIO->p3_mux_set = PINCFG_PINMUX(P3);
  GPIO->p3_mux_clr = ~PINCFG_PINMUX(P3);

  GPIO->p_mux_set = PINCFG_PINMUX(P4);
  GPIO->p_mux_clr = ~PINCFG_PINMUX(P4);

  /* Clear reset of ethernet device (GPO_04) */
  gpio_set_gpo_state(P3_STATE_GPO(4), 0);
}
