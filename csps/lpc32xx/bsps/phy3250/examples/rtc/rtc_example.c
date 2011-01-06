/***********************************************************************
 * $Id:: rtc_example.c 1259 2008-10-28 23:20:29Z wellsk                $
 *
 * Project: RTC driver example
 *
 * Description:
 *     A simple RTC driver example with interrupts.
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
#include "lpc_swim.h"
#include "lpc_swim_font.h"
#include "lpc_lcd_params.h"
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_rtc_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clcdc_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* RTC timer example data */
static INT_32 rtcdev;
static UNS_32 onoff = 0;
static volatile INT_32 secs;
static INT_32 lsecs;
static RTC_MATCH_SETUP_T mstp;

#define PHY_LCD_FRAME_BUF 0x81000000
#define LCD_DISPLAY       hitachi_tx09d71

/***********************************************************************
 *
 * Function: rtc_user_interrupt
 *
 * Purpose: RTC interrupt handler
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
void rtc_user_interrupt(void)
{
  /* Toggle LED1 connected to GPO_01 */
  onoff = 1 - onoff;
  if (onoff == 0)
  {
    phy3250_toggle_led(FALSE);
  }
  else
  {
    phy3250_toggle_led(TRUE);
  }

  secs++;

  /* Set next match */
  mstp.match_tick_val = secs + 1;
  rtc_ioctl(rtcdev, RTC_SETUP_MATCH, (INT_32) &mstp);

  /* Clear latched RTC matcg interrupt */
  rtc_ioctl(rtcdev, RTC_CLEAR_INTS, RTC_MATCH0_INT_STS);
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
  SWIM_WINDOW_T win1;
  COLOR_T *fblog;
  INT_32 lcddev;

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

  /* Install RTC interrupt handler as a IRQ interrupts */
  int_install_irq_handler(IRQ_RTC, (PFV) rtc_user_interrupt);

  /* Open RTC */
  rtcdev = rtc_open(RTC, 0);
  if (rtcdev == 0)
  {
    /* Error */
    return -1;
  }

  /* Set a 1s match rate */
  secs = lsecs = 0;
  mstp.match_num      = 0;
  mstp.use_match_int  = TRUE;
  mstp.enable_onsw    = FALSE;
  mstp.match_tick_val = secs + 1;
  rtc_ioctl(rtcdev, RTC_ENABLE, 0);
  rtc_ioctl(rtcdev, RTC_SET_COUNT, 0);
  rtc_ioctl(rtcdev, RTC_CLEAR_INTS, RTC_MATCH0_INT_STS);
  rtc_ioctl(rtcdev, RTC_SETUP_MATCH, (INT_32) &mstp);

  /* Setup LCD muxing for STN Color 16BPP */
  clkpwr_setup_lcd(CLKPWR_LCDMUX_TFT16, 1);

  /* Enable clock to LCD block (HCLK_EN)*/
  clkpwr_clk_en_dis(CLKPWR_LCD_CLK, 1);

  /* Setup LCD paramaters in the LCD controller */
  lcddev = lcd_open(CLCDC, (INT_32) & LCD_DISPLAY);

  /* Upper Panel Frame Base Address register */
  lcd_ioctl(lcddev, LCD_SET_UP_FB, PHY_LCD_FRAME_BUF);

  /* Enable LCD controller and power signals */
  lcd_ioctl(lcddev, LCD_PWENABLE, 1);

  /* Enable LCD backlight */
  phy3250_lcd_backlight_enable(TRUE);

  /* Enable LCD power */
  phy3250_lcd_power_enable(TRUE);

  /* Set frame buffer address */
  fblog = (COLOR_T *) cp15_map_physical_to_virtual(PHY_LCD_FRAME_BUF);

  /* Create a SWIM window */
  swim_window_open(&win1, LCD_DISPLAY.pixels_per_line,
                   LCD_DISPLAY.lines_per_panel, fblog, 0, 0,
                   (LCD_DISPLAY.pixels_per_line - 1), (LCD_DISPLAY.lines_per_panel - 1),
                   1, WHITE, BLACK, BLACK);
  swim_put_ltext(&win1, "RTC example: This example will print the message "
                 "TICK whenever an RTC interrupt occurs (1 second intervals). It will "
                 "quit after 10 seconds\n");

  /* Enable RTC (starts counting) */
  rtc_ioctl(rtcdev, RTC_ENABLE, 1);

  /* Enable RTC interrupt in the interrupt controller */
  int_enable(IRQ_RTC);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Loop for 10 seconds and let interrupts toggle the LEDs */
  while (secs < 10)
  {
    if (lsecs < secs)
    {
      swim_put_ltext(&win1, "TICK\n");
      lsecs = secs;
    }
  }
  /* Disable RTC interrupt in the interrupt controller */
  int_disable(IRQ_RTC);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Prior to closing the RTC, the ONSW key value is set. This will
     allow the RTC to keep it's value across resets as long as RTC
   power is maintained */
  rtc_ioctl(rtcdev, RTC_SETCLR_KEY, 1);

  /* Close RTC and LCD */
  rtc_close(rtcdev);
  lcd_close(lcddev);

  return 1;
}
