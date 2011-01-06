/***********************************************************************
 * $Id:: lpc3xxx_spwm_driver.c 4978 2010-09-20 22:32:52Z usb10132      $
 *
 * Project: LPC3xxx simple PWM driver
 *
 * Description:
 *     This file contains driver support for the LPC3xxx simple PWM.
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

#include "lpc3xxx_spwm_driver.h"

/***********************************************************************
 * SPWM driver package data
***********************************************************************/

/* SPWM device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  BOOL_32 onoff;          /* Device status flag */
  SPWM_REGS_T *regptr;    /* Pointer to SPWM registers */
} SPWM_CFG_T;

/* SPWM driver data for PWM1 and PWM2*/
static SPWM_CFG_T spwmdat[2];

/***********************************************************************
 * SPWM driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: spwm_percent_to_regv
 *
 * Purpose: convert duty cycle [%] into a register value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *    arg : user specified duty cycle in %
 *
 * Outputs: None
 *
 * Returns: a value to be written into the control register
 *
 * Notes: None
 *
 **********************************************************************/

UNS_32 spwm_percent_to_ctrl(UNS_32 ctrlreg, UNS_32 percent)
{
  if (percent == 0)
  {
    return (ctrlreg & ((SPWM_RELOAD(0xFF)) | (SPWM_DUTY(0xFF))));
  }
  else
  {
    if (percent > 99)
    {
      return (
               (ctrlreg & ((SPWM_RELOAD(0xFF)) | (SPWM_DUTY(0xFF)))) |
               SPWM_PIN_LEVEL);
    }
    else
    {
      return ((ctrlreg & (SPWM_EN | 
         SPWM_PIN_LEVEL | SPWM_RELOAD(0xFF))) |
         SPWM_DUTY((UNS_32)((((100 - percent) << 8) - percent) / 100)));
    }
  }
}

/***********************************************************************
 * SPWM driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: spwm_open
 *
 * Purpose: Open the simple PWM
 *
 * Processing:
 *     If the passed register base is valid and the simple PWM
 *     driver isn't already initialized, then setup the simple PWM
 *     into a default initialized state that is disabled. Return
 *     a pointer to the driver's data structure or NULL if driver
 *     initialization failed.
 *
 * Parameters:
 *     ipbase: Pointer to a simple PWM peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a simple PWM config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 spwm_open(void *ipbase, INT_32 arg)
{
  UNS_32 pwm_id = 9999;
  INT_32 tptr = (INT_32) NULL;

  if ((SPWM_REGS_T *) ipbase == PWM1) pwm_id = 0;
  if ((SPWM_REGS_T *) ipbase == PWM2) pwm_id = 1;
  if (pwm_id != 9999)
  {
    /* has the selected PWM been previously initialized? */
    if (spwmdat[pwm_id].init == FALSE)
    {
      /* Device not initialized and it is usable, so set it to
         used */
      spwmdat[pwm_id].init = TRUE;
      spwmdat[pwm_id].onoff = FALSE;

      /* Save address of register block */
      if (pwm_id == 0) spwmdat[pwm_id].regptr = PWM1;
      if (pwm_id == 1) spwmdat[pwm_id].regptr = PWM2;

      /* Disable selected PWM timer */
      spwmdat[pwm_id].regptr->pwm_ctrl &= ~SPWM_EN;

      /* Set PWM output */
      if (arg == 0)
        spwmdat[pwm_id].regptr->pwm_ctrl &= ~SPWM_PIN_LEVEL;
      else
        spwmdat[pwm_id].regptr->pwm_ctrl |= SPWM_PIN_LEVEL;

      tptr = (INT_32) & spwmdat[pwm_id];
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: spwm_close
 *
 * Purpose: Close the simple PWM
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the simple
 *     PWM, set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to simple PWM config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS spwm_close(INT_32 devid)
{
  UNS_32 pwm_id = 9999;
  STATUS status = _ERROR;

  if ((SPWM_CFG_T *) devid == &spwmdat[0]) pwm_id = 0;
  if ((SPWM_CFG_T *) devid == &spwmdat[1]) pwm_id = 1;
  {
    if (spwmdat[pwm_id].init == TRUE)
    {
      /* Disable PWM */
      spwmdat[pwm_id].regptr->pwm_ctrl &= ~SPWM_EN;

      /* Set timer as uninitialized */
      spwmdat[pwm_id].init = FALSE;
      spwmdat[pwm_id].onoff = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: spwm_ioctl
 *
 * Purpose: Simple PWM configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to simple PWM config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS spwm_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
  volatile UNS_32 tmp;
  UNS_32 pwm_id = 9999;
  SPWM_SETUP_T *pspwmct = (SPWM_SETUP_T *) arg;
  INT_32 status = _ERROR;

  if ((SPWM_CFG_T *) devid == &spwmdat[0]) pwm_id = 0;
  if ((SPWM_CFG_T *) devid == &spwmdat[1]) pwm_id = 1;

  if (pwm_id != 9999)
  {
    if (spwmdat[pwm_id].init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case SPWM_ON:
          spwmdat[pwm_id].regptr->pwm_ctrl |= SPWM_EN;
          spwmdat[pwm_id].onoff = TRUE;
          break;

        case SPWM_OFF:
          spwmdat[pwm_id].onoff = FALSE;
          if (arg == 0)
            tmp = 0;
          else
            tmp = 100;
          spwmdat[pwm_id].regptr->pwm_ctrl =
            spwm_percent_to_ctrl(
              spwmdat[pwm_id].regptr->pwm_ctrl, tmp);
          break;

        case SPWM_RELOAD_REG:
          spwmdat[pwm_id].regptr->pwm_ctrl =
            ((spwmdat[pwm_id].regptr->pwm_ctrl) &
             (~SPWM_RELOAD(0xFF))) | SPWM_RELOAD(arg);
          break;

        case SPWM_DUTY_REG:
          if (arg != 0)
          {
            spwmdat[pwm_id].regptr->pwm_ctrl =
              ((spwmdat[pwm_id].regptr->pwm_ctrl) &
               (~SPWM_DUTY(0xFF))) | SPWM_DUTY(arg);
            if (spwmdat[pwm_id].onoff == TRUE)
            {
              spwmdat[pwm_id].regptr->pwm_ctrl |= SPWM_EN;
            }
          }
          break;

        case SPWM_DUTY_PER:
          spwmdat[pwm_id].regptr->pwm_ctrl =
            spwm_percent_to_ctrl(
              spwmdat[pwm_id].regptr->pwm_ctrl, arg);
          if ((arg > 0) && (arg < 100) && 
              (spwmdat[pwm_id].onoff == TRUE))
          {
            spwmdat[pwm_id].regptr->pwm_ctrl |= SPWM_EN;
          }
          break;

        case SPWM_SETUP:
          tmp = SPWM_RELOAD(pspwmct->reload) | SPWM_EN;
          tmp = spwm_percent_to_ctrl(tmp, pspwmct->duty_per);
          spwmdat[pwm_id].regptr->pwm_ctrl = tmp;
          spwmdat[pwm_id].regptr->pwm_ctrl |= SPWM_EN;
          spwmdat[pwm_id].onoff = TRUE;
          break;

        default:
          /* Unsupported parameter */
          status = SMA_BAD_PARAMS;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: spwm_read
 *
 * Purpose: Simple PWM read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to millisecond timer descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 spwm_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: spwm_write
 *
 * Purpose: Simple PWM write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to millisecond timer descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 spwm_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes)
{
  return 0;
}
