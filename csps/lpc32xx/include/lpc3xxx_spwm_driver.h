/***********************************************************************
 * $Id:: lpc3xxx_spwm_driver.h 1111 2008-08-21 20:56:55Z stefanovicz   $
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

#ifndef LPC3XXX_SPWM_DRIVER_H
#define LPC3XXX_SPWM_DRIVER_H

#include "lpc3xxx_spwm.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* SPWM device control structure type */
typedef struct
{
  UNS_32 reload;          /* reload for output frequency */
  UNS_32 duty_per;        /* output duty cycle in percents*/
} SPWM_SETUP_T;


/***********************************************************************
 * SPWM device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* SPWM device commands (IOCTL commands) */
typedef enum
{
  /* Enable the simple PWM with existing setup, arg does not matter */
  SPWM_ON,
  /* Disable the simple PWM, use arg to set pin output */
  SPWM_OFF,
  /* Update reload value, use arg to pass new value dir to the reg */
  SPWM_RELOAD_REG,
  /* Update duty value, use arg to pass new value dir to the reg */
  SPWM_DUTY_REG,
  /* Update duty value, use arg to pass percents (integer)*/
  SPWM_DUTY_PER,
  /* Set all simple PWM parameters, use arg to pass a pointer
     to SMPLPWM_CONTROL_T */
  SPWM_SETUP
} SPWM_IOCTL_CMD_T;

/***********************************************************************
 * SPWM driver API functions
 **********************************************************************/

/* Open the simple PWM */
INT_32 spwm_open(void *ipbase, INT_32 arg);

/* Close the simple PWM */
STATUS spwm_close(INT_32 devid);

/* Simple PWM configuration block */
STATUS spwm_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg);

/* Simple PWM read function (stub only) */
INT_32 spwm_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes);

/* Simple PWM write function (stub only) */
INT_32 spwm_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes);

/***********************************************************************
 * Other SPWM driver functions
 **********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LPC3XXX_SPWM_DRIVER_H */
