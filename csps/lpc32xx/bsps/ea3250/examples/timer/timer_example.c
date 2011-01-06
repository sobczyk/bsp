/***********************************************************************
 * $Id:: timer_example.c 963 2008-07-28 17:36:58Z wellsk               $
 *
 * Project: Timer driver example
 *
 * Description:
 *     A simple timer driver example.
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
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "board.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Timer device handles */
static INT_32 timer0dev, timer1dev;

/* Number of milliSeconds */
static volatile INT_32 msecs;

/***********************************************************************
 *
 * Function: timer0_user_interrupt
 *
 * Purpose: Timer 0 interrupt handler
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
void timer0_user_interrupt(void)
{
  /* Clear latched timer interrupt */
  timer_ioctl(timer0dev, TMR_CLEAR_INTS, TIMER_CNTR_MTCH_BIT(0));

  /* Turn on LED1 */
  ea3250_toggle_led(TRUE);

  msecs += 100;
}

/***********************************************************************
 *
 * Function: timer1_user_interrupt
 *
 * Purpose: Timer 1 interrupt handler
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
void timer1_user_interrupt(void)
{
  /* Clear latched timer interrupt */
  timer_ioctl(timer1dev, TMR_CLEAR_INTS, TIMER_CNTR_MTCH_BIT(0));

  /* Turn off LED1 */
  ea3250_toggle_led(FALSE);
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
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/
int c_entry(void)
{
  TMR_PSCALE_SETUP_T pscale;
  TMR_MATCH_SETUP_T msetup;

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Install timer interrupts handlers as a IRQ interrupts */
  int_install_irq_handler(IRQ_TIMER0, (PFV) timer0_user_interrupt);
  int_install_irq_handler(IRQ_TIMER1, (PFV) timer1_user_interrupt);

  /* Open timers - this will enable the clocks for all timers when
     match control, match output, and capture control functions
     disabled. Default clock will be internal. */
  timer0dev = timer_open(TIMER_CNTR0, 0);
  timer1dev = timer_open(TIMER_CNTR1, 0);

  /******************************************************************/
  /* Setup timer 0 for a 10Hz match rate                            */

  /* Use a prescale count time of 100uS                             */
  pscale.ps_tick_val = 0; /* Use ps_us_val value */
  pscale.ps_us_val = 100; /* 100uS */
  timer_ioctl(timer0dev, TMR_SETUP_PSCALE, (INT_32) &pscale);

  /* Use a match count value of 1000 (1000 * 100uS = 100mS (10Hz))  */
  msetup.timer_num = 0; /* Use match register set 0 (of 0..3) */
  msetup.use_match_int = TRUE; /* Generate match interrupt on match */
  msetup.stop_on_match = FALSE; /* Do not stop timer on match */
  msetup.reset_on_match = TRUE; /* Reset timer counter on match */
  msetup.match_tick_val = 999; /* Match is when timer count is 1000 */
  timer_ioctl(timer0dev, TMR_SETUP_MATCH, (INT_32) &msetup);

  /* Clear any latched timer 0 interrupts and enable match
   interrupt */
  timer_ioctl(timer0dev, TMR_CLEAR_INTS,
              (TIMER_CNTR_MTCH_BIT(0) | TIMER_CNTR_MTCH_BIT(1) |
               TIMER_CNTR_MTCH_BIT(2) | TIMER_CNTR_MTCH_BIT(3) |
               TIMER_CNTR_CAPT_BIT(0) | TIMER_CNTR_CAPT_BIT(1) |
               TIMER_CNTR_CAPT_BIT(2) | TIMER_CNTR_CAPT_BIT(3)));
  /******************************************************************/

  /******************************************************************/
  /* Setup timer 1 for a 4.9Hz match rate                           */

  /* Use a prescale count time of 100uS                             */
  pscale.ps_tick_val = 0; /* Use ps_us_val value */
  pscale.ps_us_val = 10; /* 100uS */
  timer_ioctl(timer1dev, TMR_SETUP_PSCALE, (INT_32) &pscale);

  /* Use a match value of 490 (490 * 100uS)                         */
  msetup.timer_num = 0; /* Use match register set 0 (of 0..3) */
  msetup.use_match_int = TRUE; /* Generate match interrupt on match */
  msetup.stop_on_match = FALSE; /* Do not stop timer on match */
  msetup.reset_on_match = TRUE; /* Reset timer counter on match */
  msetup.match_tick_val = 489;
  timer_ioctl(timer1dev, TMR_SETUP_MATCH, (INT_32) &msetup);

  /* Clear any latched timer 1 interrupts and enable match
   interrupt */
  timer_ioctl(timer1dev, TMR_CLEAR_INTS,
              (TIMER_CNTR_MTCH_BIT(0) | TIMER_CNTR_MTCH_BIT(1) |
               TIMER_CNTR_MTCH_BIT(2) | TIMER_CNTR_MTCH_BIT(3) |
               TIMER_CNTR_CAPT_BIT(0) | TIMER_CNTR_CAPT_BIT(1) |
               TIMER_CNTR_CAPT_BIT(2) | TIMER_CNTR_CAPT_BIT(3)));
  /******************************************************************/

  /* Enable timers (starts counting) */
  msecs = 0;
  timer_ioctl(timer0dev, TMR_ENABLE, 1);
  timer_ioctl(timer1dev, TMR_ENABLE, 1);

  /* Enable timer interrupts in the interrupt controller */
  int_enable(IRQ_TIMER0);
  int_enable(IRQ_TIMER1);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Loop for 20 seconds and let interrupts toggle the LEDs */
  while (msecs < (10 * 1000));

  /* Disable timer interrupts in the interrupt controller */
  int_disable(IRQ_TIMER0);
  int_disable(IRQ_TIMER1);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close timers */
  timer_close(timer0dev);
  timer_close(timer1dev);

  return 1;
}
