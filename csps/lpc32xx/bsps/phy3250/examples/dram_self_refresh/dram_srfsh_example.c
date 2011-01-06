/***********************************************************************
 * $Id:: dram_srfsh_example.c 3257 2010-04-12 17:26:38Z usb10132       $
 *
 * Project: NXP PHY3250 System DRAM self refresh example
 *
 * Description:
 *     This example demonstrates how to safely place the system into
 *     sleep mode by bringing down the clock, entering self refresh
 *     mode on the SDRAM devices, and then entering sleep mode. The
 *     system will wake up after 10 seconds and restore clocks and
 *     normal SDRAM operation.
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
#include "lpc32xx_rtc_driver.h"
#include "lpc32xx_gpio_driver.h"

/* Where in IRAM the position independent code is loaded */
#define IRAM_CODE_ADDR 0x08028000

/* Number of seconds this example will sleep before waking up and
   testing the DRAM pattern */
#define SLEEP_TIMEOUT_SEC 180

/* Start at end of position independent code in SDRAM (for copy) */
extern UNS_32 srfsh_start;
extern UNS_32 srfsh_end;

/* Start or memory and size (bytes) of where to test SDRAM data. This
   whould not overlap with where this example is executed in SDRAM */
#define SDRAM_START 0x81000000
#define SDRAM_SIZE  0x01000000 /* Must be a 32-bit aligned size */

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
  UNS_32 sz, *src, *dst;
  BOOL_32 onoff = TRUE;
  int del;
  PFV func = (PFV) IRAM_CODE_ADDR;

  /* Disable interrupts in ARM core */
  disable_irq();

#if 0
  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Setup inverse address test data in SDRAM */
  src = (UNS_32 *) SDRAM_START;
  sz = SDRAM_SIZE;
  while (sz > 0) {
    *src = ~(UNS_32) src;
    src++;
    sz -= 4;
  }
#endif

  /* Setup RTC to interrupt */
  RTC->ctrl = RTC_CNTR_DIS;
  RTC->ucount = 0;
  RTC->dcount = 0xFFFFFFFF;
  RTC->match0 = SLEEP_TIMEOUT_SEC;
  RTC->match1 = SLEEP_TIMEOUT_SEC + 1;
  RTC->intstat = (RTC_MATCH0_INT_STS | RTC_MATCH1_INT_STS |
    RTC_ONSW_INT_STS);
  RTC->ctrl = RTC_MATCH0_EN;

  /* Setup wakeup event for RTC match */
  clkpwr_clear_event(CLKPWR_EVENT_INT_RTC);
  clkpwr_set_event_pol(CLKPWR_EVENT_INT_RTC, 1);
  clkpwr_wk_event_en_dis(CLKPWR_EVENT_INT_RTC, TRUE);

#if 0
  /* Both LEDs initially on */
  phy3250_toggle_led(TRUE);
  gpio_set_gpo_state(P3_STATE_GPO(14), 0);
#endif

  /* At this point, any peripheral that uses DMA to transfer data
     to/from SDRAM should be disabled. It might be a good idea to
     also disable interrupts just in case one happens and somehow does
     soemthing with SDRAM */

  /* If your code is running from SDRAM, you'll need to make sure it is
     running in IRAM. After the SDRAM chips are placed into self
     refresh mode, the SDRAM won't be usable and there will still be
     a few instructions left to process. For this example, a small
     piece of position independent code is copied to IRAM and then
     executed. the code should be placed in uncached IRAM with the
	 same virtual and physical address. */
  src = (UNS_32 *) &srfsh_start;
  dst = (UNS_32 *) IRAM_CODE_ADDR;
  sz = ((UNS_32) &srfsh_end);
  while ((UNS_32) src <= sz) {
    *dst = *src;
    src++;
    dst++;
  }

  /* Jump to code to place system into self refresh and sleep */
  func();

  clkpwr_clear_event(CLKPWR_EVENT_INT_RTC);
  clkpwr_wk_event_en_dis(CLKPWR_EVENT_INT_RTC, FALSE);

#if 0
  /* Both LEDs off */
  phy3250_toggle_led(FALSE);
  gpio_set_gpo_state(0, P3_STATE_GPO(14));

  /* Verify the test results */
  src = (UNS_32 *) SDRAM_START;
  sz = SDRAM_SIZE;
  while (sz > 0) {
	  if ((UNS_32) *src != ~(UNS_32) src) {
		  /* Green LED turns on if an error occurs */
		  gpio_set_gpo_state(P3_STATE_GPO(14), 0);
		  sz = 0;
	  }
	  else {
	    src++;
		sz -= 4;
	  }
  }

  while (1) {
	  /* Toggle red LED */
	  phy3250_toggle_led(onoff);
	  for (del = 0; del < 0xFFFF; del++);
	  if (onoff == TRUE) {
		  onoff = FALSE;
	  }
	  else {
		  onoff = TRUE;
	  }
  }
#endif
}
