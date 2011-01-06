/***********************************************************************
 * $Id:: lpc32xx_gpio_driver.h 1018 2008-08-06 18:36:25Z wellsk        $
 *
 * Project: LPC32xx GPIO driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx GPIO block.
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
 *********************************************************************/

#ifndef LPC32XX_GPIO_DRIVER_H
#define LPC32XX_GPIO_DRIVER_H

#include "lpc32xx_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * GPIO driver functions for PIO group #1
 **********************************************************************/

/* Returns current input states of the input pin state register */
#define gpio_get_inppin_states() GPIO->p3_inp_state

/* Sets and clears GPO or GPIO output states, returns current states */
UNS_32 gpio_set_gpo_state(UNS_32 set_pins,
                          UNS_32 clr_pins);

/* Sets and clears pin direction, returns current direction */
UNS_32 gpio_set_dir(UNS_32 out_pins,
                    UNS_32 in_pins);

/* Sets and clears SDRAM pin output states (when muxed as GPIOs),
   returns current states */
UNS_32 gpio_set_sdr_state(UNS_32 set_pins,
                          UNS_32 clr_pins);

/***********************************************************************
 * GPIO driver functions for P0 group
 **********************************************************************/

/* Returns current input states of the P0 pin input states */
#define gpio_get_p0pin_states() GPIO->p0_inp_state

/* Sets or clears output states for P0 pins, returns the current P0
   states */
UNS_32 gpio_set_p0_state(UNS_32 set_pins,
                         UNS_32 clr_pins);

/* Sets and clears P0 pin direction, returns current direction */
UNS_32 gpio_set_p0_dir(UNS_32 out_pins,
                       UNS_32 in_pins);

/***********************************************************************
 * GPIO driver functions for P1 group
 **********************************************************************/

/* Returns current input states of the P1 pin input states */
#define gpio_get_p1pin_states() GPIO->p1_inp_state

/* Sets or clears output states for P1 pins, returns the current P1
   states */
UNS_32 gpio_set_p1_state(UNS_32 set_pins,
                         UNS_32 clr_pins);

/* Sets and clears P1 pin direction, returns current direction */
UNS_32 gpio_set_p1_dir(UNS_32 out_pins,
                       UNS_32 in_pins);

/* setup the pin muxing for the I2S */
void i2s_pin_mux(UNS_32 i2s_num, UNS_32 en_dis);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_GPIO_DRIVER_H */
