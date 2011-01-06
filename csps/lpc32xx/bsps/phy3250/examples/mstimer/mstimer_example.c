/***********************************************************************
 * $Id:: mstimer_example.c 1003 2008-08-06 18:08:30Z wellsk            $
 *
 * Project: Millisecond timer driver example
 *
 * Description:
 *     A simple millisecond timer driver example.
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
#include "lpc32xx_mstimer_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Milklisecond timer device handles */
static INT_32 mstimerdev;

/* On/off GPIO state */
static UNS_32 onoff = 0;

/* Number of milliseconds */
static volatile INT_32 msecs;

/* Timeout for the example in milliSeconds */
#define MSTIMEOUT (10 * 1000)

/* Tick rate in mS for the timer interrupt */
#define MSTICKRATE 10

/***********************************************************************
 *
 * Function: mstimer_user_interrupt
 *
 * Purpose: Millisecond timer interrupt handler
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
void mstimer_user_interrupt(void)
{
  static INT_32 lcnt = 0;

  /* Clear latched mstimer interrupt */
  mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);

  /* Limit LED toggle rate to 10Hz */
  lcnt++;
  if (lcnt >= 10)
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

    lcnt = 0;
  }

  msecs += 10;
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
  MST_MATCH_SETUP_T mstp;

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

  /* Install mstimer interrupts handlers as a IRQ interrupts */
  int_install_irq_handler(IRQ_MSTIMER, (PFV) mstimer_user_interrupt);

  /* Open mstimer */
  mstimerdev = mstimer_open(MSTIMER, 0);
  if (mstimerdev == 0)
  {
    /* Error */
    return -1;
  }

  /* Set a 10mS match rate */
  mstp.timer_num      = 0;
  mstp.use_int        = TRUE;
  mstp.stop_on_match  = FALSE;
  mstp.reset_on_match = TRUE;
  mstp.tick_val       = 0;
  mstp.ms_val         = MSTICKRATE;
  mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);
  mstimer_ioctl(mstimerdev, MST_TMR_SETUP_MATCH, (INT_32) &mstp);

  /* Reset terminal count */
  mstimer_ioctl(mstimerdev, MST_TMR_RESET, 0);

  /* Enable mstimer (starts counting) */
  msecs = 0;
  mstimer_ioctl(mstimerdev, MST_TMR_ENABLE, 1);

  /* Enable mstimer interrupts in the interrupt controller */
  int_enable(IRQ_MSTIMER);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Loop for 10 seconds and let interrupts toggle the LEDs */
  while (msecs < MSTIMEOUT);

  /* Disable mstimer interrupts in the interrupt controller */
  int_disable(IRQ_MSTIMER);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close mstimer */
  mstimer_close(mstimerdev);

  return 1;
}
