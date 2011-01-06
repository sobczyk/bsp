/***********************************************************************
 * $Id:: phy3250_board.c 4873 2010-09-09 21:04:43Z usb10132            $
 *
 * Project: Phytec LPC3250 board functions
 *
 * Description:
 *     This file contains driver support for various Phytect LPC3250
 *     board functions.
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
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "phy3250_board.h"

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_board_init
 *
 * Purpose: Initializes basic board functions
 *
 * Processing:
 *     Does nothing
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
void phy3250_board_init(void)
{
  ;
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
 * Function: phy3250_toggle_led
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
void phy3250_toggle_led(BOOL_32 on)
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
 * Function: phy3250_button_state
 *
 * Purpose: Read a button state
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     button : PHY3250_BTN1 or PHY3250_BTN2
 *
 * Outputs: None
 *
 * Returns: 1 if the button is pressed, or 0 if depressed
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 phy3250_button_state(PHY_BUTTON_T button)
{
  UNS_32 mask = 0;
  INT_32 pressed = 0;

  if (button == PHY3250_BTN1)
  {
    mask = P3_IN_STATE_GPI_03;
  }
  else if (button == PHY3250_BTN2)
  {
    mask = P3_IN_STATE_GPI_02;
  }

  if ((gpio_get_inppin_states() & mask) != 0)
  {
    pressed = 1;
  }

  return pressed;
}

/***********************************************************************
 *
 * Function: phy3250_sdpower_enable
 *
 * Purpose: Enable or disable SDMMC power
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
void phy3250_sdpower_enable(BOOL_32 enable)
{
  UNS_32 set, clr;

  if (enable == FALSE)
  {
    set = 0;
    clr = P3_STATE_GPO(5);
  }
  else
  {
    set = P3_STATE_GPO(5);
    clr = 0;
  }

  gpio_set_gpo_state(set, clr);
}

/***********************************************************************
 *
 * Function: phy3250_sdmmc_card_inserted
 *
 * Purpose: Returns card inserted status
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if a card is inserted, otherwise FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 phy3250_sdmmc_card_inserted(void)
{
  BOOL_32 ins = FALSE;

  if ((gpio_get_inppin_states() & P3_IN_STATE_GPIO_01) == 0)
  {
    ins = TRUE;
  }

  return ins;
}

/***********************************************************************
 *
 * Function: phy3250_lcd_backlight_enable
 *
 * Purpose: Enables or disables the LCD backlight
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     enable : TRUE ro enable, FALSE to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void phy3250_lcd_backlight_enable(BOOL_32 enable)
{
  if (enable != FALSE)
  {
    /* Enable LCD backlight */
#if PHY3250_LCD_1307_X > 0
    /* Active high to enable */
	gpio_set_gpo_state(_BIT(4), 0);
#else
    /* Active low to enable */
    gpio_set_gpo_state(0, _BIT(4));
#endif
  }
  else
  {
    /* Disable LCD backlight */
#if PHY3250_LCD_1307_X > 0
    /* Active low to disable */
	gpio_set_gpo_state(0, _BIT(4));
#else
    /* Active high to disable */
    gpio_set_gpo_state(_BIT(4), 0);
#endif
  }
}

/***********************************************************************
 *
 * Function: phy3250_lcd_power_enable
 *
 * Purpose: Enables or disables the LCD power
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     enable : TRUE ro enable, FALSE to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void phy3250_lcd_power_enable(BOOL_32 enable)
{
  if (enable != FALSE)
  {
    /* Active high to enable */
	gpio_set_gpo_state(_BIT(0), 0);
  }
  else
  {
    /* Active low to disable */
	gpio_set_gpo_state(0, _BIT(0));
  }
}
