/***********************************************************************
 * $Id:: lpc32xx_gpio_driver.c 1017 2008-08-06 18:36:07Z wellsk        $
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

#include "lpc32xx_gpio_driver.h"

/***********************************************************************
 * GPIO driver package data
***********************************************************************/

/***********************************************************************
 * GPIO driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: gpio_set_gpo_state
 *
 * Purpose:
 *     Sets and clears GPO or GPIO output states, returns current states
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     set_pins : Pin states to set (high)
 *     clr_pins : Pin states to clear (low)
 *
 * Outputs: None
 *
 * Returns: Current state of all drive pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_gpo_state(UNS_32 set_pins,
                          UNS_32 clr_pins)
{
  if (set_pins != 0)
  {
    GPIO->p3_outp_set = set_pins;
  }

  if (clr_pins != 0)
  {
    GPIO->p3_outp_clr = clr_pins;
  }

  return GPIO->p3_outp_state;
}

/***********************************************************************
 *
 * Function: gpio_set_dir
 *
 * Purpose:
 *     Sets and clears pin input and output states, returns current
 *     states
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     out_pins : Pin states to set to outputs
 *     in_pins  : Pin states to set to inputs
 *
 * Outputs: None
 *
 * Returns: Current direction state of all pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_dir(UNS_32 out_pins,
                    UNS_32 in_pins)
{
  if (out_pins != 0)
  {
    GPIO->p2_dir_set = out_pins;
  }

  if (in_pins != 0)
  {
    GPIO->p2_dir_clr = in_pins;
  }

  return GPIO->p2_dir_state;
}

/***********************************************************************
 *
 * Function: gpio_set_sdr_state
 *
 * Purpose:
 *     Sets and clears SDRAM pin output states (when muxed as GPIOs),
 *     returns current states
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     set_pins : Pin states to set (high)
 *     clr_pins : Pin states to clear (low)
 *
 * Outputs: None
 *
 * Returns: Current state of all SDRAM pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_sdr_state(UNS_32 set_pins,
                          UNS_32 clr_pins)
{
  if (set_pins != 0)
  {
    GPIO->p2_outp_set = set_pins;
  }

  if (clr_pins != 0)
  {
    GPIO->p2_outp_clr = clr_pins;
  }

  return GPIO->p2_inp_state;
}

/***********************************************************************
 *
 * Function: gpio_set_p0_state
 *
 * Purpose:
 *     Sets or clears output states for P0 pins, returns the current P0
 *     states.
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     set_pins : Pin states to set (high)
 *     clr_pins : Pin states to clear (low)
 *
 * Outputs: None
 *
 * Returns: Current state of all P0 pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_p0_state(UNS_32 set_pins,
                         UNS_32 clr_pins)
{
  if (set_pins != 0)
  {
    GPIO->p0_outp_set = set_pins;
  }

  if (clr_pins != 0)
  {
    GPIO->p0_outp_clr = clr_pins;
  }

  return GPIO->p0_outp_state;
}

/***********************************************************************
 *
 * Function: gpio_set_p0_dir
 *
 * Purpose: Sets and clears P0 pin direction, returns current direction
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     out_pins : Pin states to set to outputs
 *     in_pins  : Pin states to set to inputs
 *
 * Outputs: None
 *
 * Returns: Current direction state of all pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_p0_dir(UNS_32 out_pins,
                       UNS_32 in_pins)
{
  if (out_pins != 0)
  {
    GPIO->p0_dir_set = out_pins;
  }

  if (in_pins != 0)
  {
    GPIO->p0_dir_clr = in_pins;
  }

  return GPIO->p0_dir_state;
}

/***********************************************************************
 *
 * Function: gpio_set_p1_state
 *
 * Purpose:
 *     Sets or clears output states for P1 pins, returns the current P1
 *     states.
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     set_pins : Pin states to set (high)
 *     clr_pins : Pin states to clear (low)
 *
 * Outputs: None
 *
 * Returns: Current state of all P1 pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_p1_state(UNS_32 set_pins,
                         UNS_32 clr_pins)
{
  if (set_pins != 0)
  {
    GPIO->p1_outp_set = set_pins;
  }

  if (clr_pins != 0)
  {
    GPIO->p1_outp_clr = clr_pins;
  }

  return GPIO->p1_outp_state;
}

/***********************************************************************
 *
 * Function: gpio_set_p1_dir
 *
 * Purpose: Sets and clears P1 pin direction, returns current direction
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     out_pins : Pin states to set to outputs
 *     in_pins  : Pin states to set to inputs
 *
 * Outputs: None
 *
 * Returns: Current direction state of all pins
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 gpio_set_p1_dir(UNS_32 out_pins,
                       UNS_32 in_pins)
{
  if (out_pins != 0)
  {
    GPIO->p1_dir_set = out_pins;
  }

  if (in_pins != 0)
  {
    GPIO->p1_dir_clr = in_pins;
  }

  return GPIO->p1_dir_state;
}
/***********************************************************************
 *
 * Function: i2s_pin_mux
 *
 * Purpose: configures the pin muxing for the supplied I2S device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     i2s_num : I2S block to set, can be 0 or 1
 *     en_dis :  I2S muxing enable (1), or disable (0)
 *
 * Outputs: None
 *
 *
 * Notes: None
 *
 **********************************************************************/
void i2s_pin_mux(UNS_32 i2s_num, UNS_32 en_dis)
{

  /* I2S0 */
  if (i2s_num == 0)
  {
    if (en_dis == 1)
    {
      GPIO->p0_mux_set   = (P0_GPOP2_I2SRXSDA0 | P0_GPOP3_I2SRXCLK0 |
        P0_GPOP4_I2SRXWS0 | P0_GPOP5_I2STXSDA0 | P0_GPOP6_I2STXCLK0 |
        P0_GPOP7_I2STXWS0) ;
    }
    else
    {
      GPIO->p0_mux_clr   = (P0_GPOP2_I2SRXSDA0 | P0_GPOP3_I2SRXCLK0 |
        P0_GPOP4_I2SRXWS0 | P0_GPOP5_I2STXSDA0 | P0_GPOP6_I2STXCLK0 |
        P0_GPOP7_I2STXWS0) ;
    }
  }

  /* I2S1 */
  if (i2s_num == 1)
  {
    if (en_dis == 1)
    {
      GPIO->p_mux_clr = (P_I2STXSDA1_MAT31 | P_I2STXCLK1_MAT30
                            | P_I2STXWS1_CAP30);
      GPIO->p0_mux_set = (P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1);
      /* Note that I2S1RX_SDA does not need to be muxed as it is
         connected to the block */
    }
    else
    {
      GPIO->p_mux_set = (P_I2STXSDA1_MAT31 | P_I2STXCLK1_MAT30
                            | P_I2STXWS1_CAP30);
      GPIO->p0_mux_clr = (P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1);
    }
  }
}
