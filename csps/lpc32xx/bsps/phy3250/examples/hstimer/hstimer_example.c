/***********************************************************************
 * $Id:: hstimer_example.c 2390 2009-10-28 00:17:36Z wellsk            $
 *
 * Project: NXP PHY3250 High Speed Timer example
 *
 * Description:
 *     This file contains a simple startup code example that will
 *     generate a 'heartbeat' indicator on the LED.
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
#include "lpc_arm922t_cp15_driver.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_chip.h"
#include "phy3250_board.h"
#include "lpc32xx_hstimer_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"


/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Timer device handles */
static INT_32 timerdev;

/* On/off GPIO state */
static UNS_32 onoff1 = 0;

/***********************************************************************
 *
 * Function: hstimer_user_interrupt
 *
 * Purpose: Timer interrupt handler
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
void hstimer_user_interrupt(void)
{
  UNS_32 set, clr;

  /* Clear latched timer interrupt */
  hstimer_ioctl(timerdev, HST_CLEAR_INTS, HSTIM_MATCH0_INT);

  /* Toggle LED1 connected to GPO_01 */
  onoff1 = 1 - onoff1;
  if (onoff1 == 0)
  {
    set = 0;
    clr = P3_STATE_GPO(1);
  }
  else
  {
    set = P3_STATE_GPO(1);
    clr = 0;
  }
  gpio_set_gpo_state(set, clr);
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
void c_entry(void)
{
  HST_PSCALE_SETUP_T pscale;
  HST_MATCH_SETUP_T msetup;
  int onoff = 0, idx;

  /* Disable interrupts in ARM core */
  disable_irq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Install timer handlers as a IRQ interrupts */
  int_install_irq_handler(IRQ_HSTIMER, (PFV) hstimer_user_interrupt);

  /* Open HS timer */
  timerdev = hstimer_open(HSTIMER, 0);

  /* Use a prescale count value of 1000 */
  pscale.ps_tick_val = 0;
  pscale.ps_us_val   = 1000;
  hstimer_ioctl(timerdev, HST_SETUP_PSCALE, (INT_32) &pscale);

  /* Use a match count value of 100 */
  msetup.timer_num = 0; 				/* Use match register set 0 (of 0..2) */
  msetup.use_match_int = TRUE; 		/* Generate match interrupt on match */
  msetup.stop_on_match = FALSE; 		/* Do not stop timer on match */
  msetup.reset_on_match = TRUE; 		/* Reset timer counter on match */
  msetup.match_tick_val = 100; 		/* Match is when timer count is 100 */
  hstimer_ioctl(timerdev, HST_SETUP_MATCH, (INT_32) &msetup);

  /* Clear any latched timer 0 interrupts and enable match
  interrupt */
  hstimer_ioctl(timerdev, HST_CLEAR_INTS,
                (HSTIM_MATCH0_INT | HSTIM_MATCH1_INT |
                 HSTIM_MATCH2_INT | HSTIM_GPI_06_INT |
                 HSTIM_RTC_TICK_INT));

  /* Enable timers (starts counting) */
  hstimer_ioctl(timerdev, HST_ENABLE, 1);

  /* Enable timer interrupts in the interrupt controller */
  int_enable(IRQ_HSTIMER);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Loop forever and toggle other GPIO in the background */
  while (1) {
    onoff = 1 - onoff;
	for (idx = 0; idx < 20000000; idx++);
    if (onoff == 0)
    {
      gpio_set_gpo_state(P3_STATE_GPO(14), 0);
    }
    else
    {
      gpio_set_gpo_state(0, P3_STATE_GPO(14));
    }
  }
}
