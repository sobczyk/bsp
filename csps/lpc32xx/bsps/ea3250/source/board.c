/***********************************************************************
 * $Id:: board.c 3380 2010-05-05 23:54:23Z usb10132   $
 *
 * Project: EA3250 Board related functions
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

#include "lpc32xx_i2c.h"
#include "lpc32xx_wdt.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "board.h"

/***********************************************************************
 *
 * Function: ea3250_toggle_led
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
void ea3250_toggle_led(BOOL_32 on)
{
  UNS_32 set, clr;

  if (on == FALSE)
  {
    set = 0;
    clr = P3_STATE_GPO(1);
  }
  else
  {
    set = P3_STATE_GPO(1);
    clr = 0;
  }

  /* Set LED2 on GPO_O1 */
  gpio_set_gpo_state(set, clr);
}

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
 * Function: ea3250_vcore_set
 *
 * Purpose: Sets the ea3250 core voltage to the passed value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     vcore : Voltage in mV to set the core to
 *
 * Outputs: None
 *
 * Returns: 0 on pass, or -1 on failure
 *
 * Notes: Holds the vcore value across reset cycles
 *
 **********************************************************************/
int ea3250_vcore_set(UNS_32 vcore) {
	UNS_16 progval[2];

	/* Convert voltage to program value */
	if ((vcore < 690) || (vcore > 1399)) {
		return -1;
	}
	vcore = vcore - 690;
	vcore = vcore * 10;

	/* Enable I2C1 clock */
	CLKPWR->clkpwr_i2c_clk_ctrl |= CLKPWR_I2CCLK_I2C1CLK_EN;

	/* I2C write order = START I2C_ADDRESS-W ACK DATA ACK STOP */
	progval[0] = (0x66 << 1) | 0;
	progval[1] = ((UNS_16) (vcore / 216));

	/* Use around 100KHz (266MHz) for the I2C clock */
	I2C1->i2c_clk_hi = I2C1->i2c_clk_lo = 220;

	/* I2C reset and then setup */
	I2C1->i2c_ctrl |= I2C_RESET;
	I2C1->i2c_ctrl = 0;

	/* Transfer 2 bytes */
	I2C1->i2c_txrx = progval[0] | I2C_START;
	I2C1->i2c_txrx = progval[1] | I2C_STOP;

	/* Wait for either a NAK or complete */
	while ((I2C1->i2c_stat & (I2C_TDI | I2C_NAI)) == 0);

	if (I2C1->i2c_stat & I2C_NAI) {
		return -1;
	}

	return 0;
}
