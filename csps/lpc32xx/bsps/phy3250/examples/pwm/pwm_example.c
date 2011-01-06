/***********************************************************************
 * $Id:: pwm_example.c 2442 2009-11-05 18:50:01Z wellsk                $
 *
 * Project: NXP PHY3250 PWM example
 *
 * Description:
 *     This file contains a pwm code example that
 *     generates several waveforms. PWM3 output can be observed on
 *     the SD/MMC pins MS_DIO0/1/2/3, MS_BS, and MS_SCLK and
 *     LCDLP, LCDAC, LCDFP, LCDCP, LCDLE, and LCDPWR for
 *     PWM3.1/2/3/4/5/6 respectively (X24 connector pins 5, 6, 1, 2, 4,
 *     and 3; expension board pins 49A, 49B, 48B, 49E, 49D, and 48E).
 *     PWM4 output can be observed on the LCD9/8/18 pins for PWM4.1/2/3
 *     respectively (expansion board pins 46A, 46B, and 44B)
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
#include "lpc32xx_pwm_driver.h"

/***********************************************************************
 * PWM example private functions
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

/* PWM device handles */
static INT_32 pwmdev1, pwmdev2;

/***********************************************************************
 *
 * Function: pwm3_user_interrupt
 *
 * Purpose: PWM3 interrupt handler
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
void pwm3_user_interrupt(void)
{
  UNS_32 flags;

  flags = pwm_ioctl(pwmdev1, PWM_INT_FLAGS_READ, 0);
  pwm_ioctl(pwmdev1, PWM_INT_FLAGS_CLEAR, flags);

  return;
}

/***********************************************************************
 *
 * Function: pwm4_user_interrupt
 *
 * Purpose: PWM4 interrupt handler
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
void pwm4_user_interrupt(void)
{
  UNS_32 flags;

  flags = pwm_ioctl(pwmdev2, PWM_INT_FLAGS_READ, 0);
  pwm_ioctl(pwmdev2, PWM_INT_FLAGS_CLEAR, flags);

  return;
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
  static PWM_CHANNEL_SETUP_T pwm_channel_setup[2][7];
  static PWM_SYSTEM_SETUP_T pwm_system_setup[2];
  static PWM_UPDATE_CONTROL_T pwm_update_control[2];

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

  /* Enable PWM3/4 clocks */
  clkpwr_clk_en_dis(CLKPWR_PWM3_CLK, 1);
  clkpwr_clk_en_dis(CLKPWR_PWM4_CLK, 1);

  /* Install PWM3 and PWM4 interrupt handlers as a IRQ interrupts */
  int_install_ext_irq_handler(IRQ_PWM3,
                              (PFV) pwm3_user_interrupt, ACTIVE_LOW, 1);
  int_install_ext_irq_handler(IRQ_PWM4,
                              (PFV) pwm4_user_interrupt, ACTIVE_LOW, 1);

  /* Disable SD/MMC output and select PWM3.x on the SD/MMC pins */
  CLKPWR->clkpwr_ms_ctrl = CLKPWR_MSCARD_MSDIO_PIN_DIS;
  GPIO->p_mux_set = P_MAT21 | P_MAT20 |
                    P_MAT03 | P_MAT02 |
                    P_MAT01 | P_MAT00;

  /* Select PWM3.x on the LCD pins */
  GPIO->p3_mux_set = P3_GPO10_MC2B | P3_GPO12_MC2A |
                     P3_GPO13_MC1B | P3_GPO15_MC1A |
                     P3_GPO16_MC0B | P3_GPO18_MC0A;

  /* Select PWM4.x on the LCD pins */
  GPIO->p3_mux_set = P3_GPO6 | P3_GPO8 |
                     P3_GPO9;

  /* Loop forever and let the demo syscle thru different steps */
  while (1)
  {

    /* PWM3/4 clock is the Peripheral clock (13 MHz)           */
    /* PWM3 and PWM4 are not synchronized                      */
    /*                                                         */
    /* Seven single edge independently controlled outputs      */
    /*                                                         */
    /* PWM3 prescaler: 2, period: 10000 ticks => fout = 650 Hz */
    /* channel 1: single edge, duty cycle 14%                  */
    /* channel 2: single edge, duty cycle 28%                  */
    /* channel 3: single edge, duty cycle 42%                  */
    /* channel 4: single edge, duty cycle 56%                  */
    /* channel 5: single edge, duty cycle 70%                  */
    /* channel 6: single edge, duty cycle 84%                  */
    /*                                                         */
    /* PWM4 prescaler: 2, period: 5000 ticks => fout = 1300 Hz */
    /* channel 1: single edge, toggle @ 2000                   */
    /* channel 2: single edge, toggle @ 3000                   */
    /* channel 3: single edge, toggle @ 4000                   */


    /* open PWM3 */
    pwmdev1 = pwm_open(PWM3, 0);

    pwm_ioctl(pwmdev1, PWM_SYSTEM_CONTROL, PWM_TIMER_RESET);

    pwm_system_setup[0].clock = PWM_CLOCK_PERIPHERAL;
    pwm_system_setup[0].prescale = 2;
    pwm_system_setup[0].period = 10000;
    pwm_system_setup[0].update = PWM_NO_UPDATE;
    pwm_ioctl(pwmdev1, PWM_SYSTEM_SETUP,
              (INT_32) &pwm_system_setup[0]);

    pwm_channel_setup[0][1].channel     = 1;
    pwm_channel_setup[0][1].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][1].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][1].duty        = 14;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][1]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 1);

    pwm_channel_setup[0][2].channel     = 2;
    pwm_channel_setup[0][2].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][2].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][2].duty        = 28;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][2]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 2);

    pwm_channel_setup[0][3].channel     = 3;
    pwm_channel_setup[0][3].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][3].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][3].duty        = 42;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][3]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 3);

    pwm_channel_setup[0][4].channel     = 4;
    pwm_channel_setup[0][4].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][4].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][4].duty        = 56;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][4]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 4);

    pwm_channel_setup[0][5].channel     = 5;
    pwm_channel_setup[0][5].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][5].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][5].duty        = 70;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][5]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 5);

    pwm_channel_setup[0][6].channel     = 6;
    pwm_channel_setup[0][6].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[0][6].duty_option = PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][6].duty        = 84;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][6]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 6);

    pwm_update_control[0].update    = PWM_UPDATE;
    pwm_update_control[0].channels  = 0x7E;
    pwm_ioctl(pwmdev1, PWM_UPDATE_CONTROL,
              (INT_32) &pwm_update_control[0]);
    pwm_ioctl(pwmdev1, PWM_SYSTEM_CONTROL, PWM_TIMER_RESTART);

    /* open PWM4 */
    pwmdev2 = pwm_open(PWM4, 0);

    pwm_system_setup[1].clock = PWM_CLOCK_PERIPHERAL;
    pwm_system_setup[1].prescale = 2;
    pwm_system_setup[1].period = 5000;
    pwm_system_setup[1].update = PWM_NO_UPDATE;
    pwm_ioctl(pwmdev2, PWM_SYSTEM_SETUP, (INT_32) &pwm_system_setup[1]);

    pwm_channel_setup[1][1].channel     = 1;
    pwm_channel_setup[1][1].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[1][1].duty_option = PWM_WAV_DUTY_ABS;
    pwm_channel_setup[1][1].duty        = 2000;
    pwm_ioctl(pwmdev2, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[1][1]);
    pwm_ioctl(pwmdev2, PWM_CHANNEL_OUT_ENABLE, 1);

    pwm_channel_setup[1][2].channel     = 2;
    pwm_channel_setup[1][2].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[1][2].duty_option = PWM_WAV_DUTY_ABS;
    pwm_channel_setup[1][2].duty        = 3000;
    pwm_ioctl(pwmdev2, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[1][2]);
    pwm_ioctl(pwmdev2, PWM_CHANNEL_OUT_ENABLE, 2);

    pwm_channel_setup[1][3].channel     = 3;
    pwm_channel_setup[1][3].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[1][3].duty_option = PWM_WAV_DUTY_ABS;
    pwm_channel_setup[1][3].duty        = 4000;
    pwm_ioctl(pwmdev2, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[1][3]);
    pwm_ioctl(pwmdev2, PWM_CHANNEL_OUT_ENABLE, 3);

    pwm_update_control[1].update    = PWM_UPDATE;
    pwm_update_control[1].channels  = 0x0E;
    pwm_ioctl(pwmdev2, PWM_UPDATE_CONTROL,
              (INT_32) &pwm_update_control[1]);
    pwm_ioctl(pwmdev2, PWM_SYSTEM_CONTROL, PWM_TIMER_RESTART);

    delay(2500000);

    /* close PWM3 & PWM4 */
    pwm_close(pwmdev1);
    pwm_close(pwmdev2);
    delay(2500000);


    /* PWM3/4 clock is the Peripheral clock (13 MHz)                */
    /* PWM3 and PWM4 are synchronized                               */
    /*                                                              */
    /* Four dual edge controlled outputs PWM3.2/4/6 and PWM4.2      */
    /* (as in a stepper motor control application) and a single     */
    /* edge independently controlled output PWM4.3                  */
    /* PWM3.1/3/5 are disabled                                      */
    /*                                                              */
    /* PWM3 prescaler: 5, period: 10000 ticks => fout = 260 Hz      */
    /* channel 1: disabled                                          */
    /* channel 2: dual edge, duty cycle 80%, offset 5%              */
    /* channel 3: disabled                                          */
    /* channel 4: dual edge, duty cycle 80%  offset 30%             */
    /* channel 5: disabled                                          */
    /* channel 6: dual edge, duty cycle 80%  offset 55%             */
    /*                                                              */
    /* PWM4 prescaler: 5, period: 10000 ticks => fout = 260 Hz      */
    /* channel 1: disabled                                          */
    /* channel 2: dual edge, duty cycle 80%  offset 80%             */
    /* channel 3: single edge, toggle @ 500                         */

    /* open PWM3 */
    pwmdev1 = pwm_open(PWM3, 0);

    pwm_ioctl(pwmdev1, PWM_SYSTEM_CONTROL, PWM_TIMER_RESET);

    pwm_system_setup[0].clock = PWM_CLOCK_PERIPHERAL;
    pwm_system_setup[0].prescale = 5;
    pwm_system_setup[0].period = 10000;
    pwm_system_setup[0].update = PWM_NO_UPDATE;
    pwm_ioctl(pwmdev1, PWM_SYSTEM_SETUP, (INT_32) &pwm_system_setup[0]);

    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_DISABLE, 1);

    pwm_channel_setup[0][2].channel     	= 2;
    pwm_channel_setup[0][2].mode        	= PWM_DUAL_EDGE;
    pwm_channel_setup[0][2].duty_option 	= PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][2].duty        	= 80;
    pwm_channel_setup[0][2].offset_option	= PWM_WAV_OFFSET_PER;
    pwm_channel_setup[0][2].offset        	= 5;
    pwm_channel_setup[0][2].ini_state      	= 1;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][2]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 2);

    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_DISABLE, 3);

    pwm_channel_setup[0][4].channel     	= 4;
    pwm_channel_setup[0][4].mode        	= PWM_DUAL_EDGE;
    pwm_channel_setup[0][4].duty_option 	= PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][4].duty        	= 80;
    pwm_channel_setup[0][4].offset_option	= PWM_WAV_OFFSET_PER;
    pwm_channel_setup[0][4].offset        	= 30;
    pwm_channel_setup[0][4].ini_state      	= 1;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][4]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 4);

    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_DISABLE, 5);

    pwm_channel_setup[0][6].channel     	= 6;
    pwm_channel_setup[0][6].mode        	= PWM_DUAL_EDGE;
    pwm_channel_setup[0][6].duty_option 	= PWM_WAV_DUTY_PER;
    pwm_channel_setup[0][6].duty        	= 80;
    pwm_channel_setup[0][6].offset_option	= PWM_WAV_OFFSET_PER;
    pwm_channel_setup[0][6].offset        	= 55;
    pwm_channel_setup[0][6].ini_state      	= 1;
    pwm_ioctl(pwmdev1, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[0][6]);
    pwm_ioctl(pwmdev1, PWM_CHANNEL_OUT_ENABLE, 6);

    pwm_update_control[0].update    = PWM_UPDATE;
    pwm_update_control[0].channels  = 0x7E;
    pwm_ioctl(pwmdev1, PWM_UPDATE_CONTROL,
              (INT_32) &pwm_update_control[0]);

    /* open PWM4 */
    pwmdev2 = pwm_open(PWM4, 0);

    pwm_system_setup[1].clock = PWM_CLOCK_PERIPHERAL;
    pwm_system_setup[1].prescale = 5;
    pwm_system_setup[1].period = 10000;
    pwm_system_setup[1].update = PWM_NO_UPDATE;
    pwm_ioctl(pwmdev2, PWM_SYSTEM_SETUP, (INT_32) &pwm_system_setup[1]);

    pwm_channel_setup[1][2].channel     	= 2;
    pwm_channel_setup[1][2].mode        	= PWM_DUAL_EDGE;
    pwm_channel_setup[1][2].duty_option 	= PWM_WAV_DUTY_PER;
    pwm_channel_setup[1][2].duty        	= 80;
    pwm_channel_setup[1][2].offset_option	= PWM_WAV_OFFSET_PER;
    pwm_channel_setup[1][2].offset        	= 80;
    pwm_channel_setup[1][2].ini_state      	= 1;
    pwm_ioctl(pwmdev2, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[1][2]);
    pwm_ioctl(pwmdev2, PWM_CHANNEL_OUT_ENABLE, 2);

    pwm_channel_setup[1][3].channel     = 3;
    pwm_channel_setup[1][3].mode        = PWM_SINGLE_EDGE;
    pwm_channel_setup[1][3].duty_option = PWM_WAV_DUTY_ABS;
    pwm_channel_setup[1][3].duty        = 500;
    pwm_ioctl(pwmdev2, PWM_CHANNEL_SETUP,
              (INT_32) &pwm_channel_setup[1][3]);
    pwm_ioctl(pwmdev2, PWM_CHANNEL_OUT_ENABLE, 3);

    pwm_update_control[1].update    = PWM_UPDATE;
    pwm_update_control[1].channels  = 0x0E;
    pwm_ioctl(pwmdev2, PWM_UPDATE_CONTROL,
              (INT_32) &pwm_update_control[1]);

    pwm_ioctl(pwmdev1, PWM_SYSTEM_CONTROL, PWM_TIMER_SYNC);
    pwm_ioctl(pwmdev2, PWM_SYSTEM_CONTROL, PWM_TIMER_RESTART);
    pwm_ioctl(pwmdev1, PWM_SYSTEM_CONTROL, PWM_TIMER_GO);

    delay(2500000);

    /* close PWM3 & PWM4 */
    pwm_close(pwmdev1);
    pwm_close(pwmdev2);
    delay(2500000);
  }
}
