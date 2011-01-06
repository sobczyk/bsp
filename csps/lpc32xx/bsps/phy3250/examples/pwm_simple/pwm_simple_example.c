/***********************************************************************
 * $Id:: pwm_simple_example.c 1120 2008-08-21 21:03:20Z stefanovicz    $
 *
 * Project: NXP PHY3250 Simple PWM example
 *
 * Description:
 *     This file contains a simple pwm code example that
 *     generates several waveforms. The output can be observed on
 *     the expansion board pin 44F (PWM1) and 44E (PWM2).
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
#include "lpc32xx_clkpwr_driver.h"
#include "lpc3xxx_spwm_driver.h"

/***********************************************************************
 * Simple PWM example private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: delay
 *
 * Purpose: generate a delay
 *
 * Processing:
 *     A local software counter counts up to the specified count.
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

/* Simple PWM device handles */
static INT_32 spwmdev1, spwmdev2;

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
  SPWM_SETUP_T spwmsetup[2];

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

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Enable simple PWM1/2 clocks*/
  clkpwr_clk_en_dis(CLKPWR_PWM1_CLK, 1);
  clkpwr_clk_en_dis(CLKPWR_PWM2_CLK, 1);

  /* Loop forever and let the demo syscle thru different steps */
  while (1)
  {
    /* open PWM1 */
    spwmdev1 = spwm_open(PWM1, 0);
    /* Select PERIPH_CLK for PWM1 and set divider to 4 */
    clkpwr_setup_pwm(1, 1, 4);

    /* open PWM2 */
    spwmdev2 = spwm_open(PWM2, 0);
    /* Select 32768 Hz for PWM2 and set divider to 1 */
    clkpwr_setup_pwm(2, 0, 1);

    /* PWM1 duty cycle = 30%, reload = 8       */
    /* fout1 = 13 MHz / (4*8*256) = 1.587 kHz  */
    spwmsetup[0].reload = 8;
    spwmsetup[0].duty_per = 30;
    spwm_ioctl(spwmdev1, SPWM_SETUP, (INT_32) &spwmsetup[0]);

    /* PWM2 duty cycle = 85%, reload = 2       */
    /* fout2 = 32768 Hz / (1*2*256) = 64 Hz */
    spwmsetup[1].reload = 2;
    spwmsetup[1].duty_per = 85;
    spwm_ioctl(spwmdev2, SPWM_SETUP, (INT_32) &spwmsetup[1]);
    delay(2500000);

    /* Drive PWM1 low <=> 0% duty cycle */
    spwm_ioctl(spwmdev1, SPWM_DUTY_PER, 0);
    /* Drive PWM2 low <=> 0% duty cycle */
    spwm_ioctl(spwmdev2, SPWM_DUTY_PER, 0);
    delay(2500000);

    /* PWM1 duty cycle 10% */
    spwm_ioctl(spwmdev1, SPWM_DUTY_PER, 10);
    /* Drive PWM2 high <=> 100% duty cycle */
    spwm_ioctl(spwmdev2, SPWM_DUTY_PER, 100);
    delay(1000000);

    /* PWM1 duty cycle 50% */
    spwm_ioctl(spwmdev1, SPWM_DUTY_PER, 50);
    /* Drive PWM2 low <=> 0% duty cycle */
    spwm_ioctl(spwmdev2, SPWM_DUTY_PER, 0);
    delay(1000000);

    /* PWM1 duty cycle 75% */
    spwm_ioctl(spwmdev1, SPWM_DUTY_PER, 75);
    /* Drive PWM2 high <=> 100% duty cycle */
    spwm_ioctl(spwmdev2, SPWM_DUTY_PER, 100);
    delay(1000000);

    /* Drive PWM1 low <=> 0% duty cycle */
    spwm_ioctl(spwmdev1, SPWM_DUTY_PER, 0);
    /* Drive PWM2 low <=> 0% duty cycle */
    spwm_ioctl(spwmdev2, SPWM_DUTY_PER, 0);

    /* close both simple PWMs: PWM1 & PWM2 */
    spwm_close(spwmdev1);
    spwm_close(spwmdev2);
    delay(2500000);
  }
}
