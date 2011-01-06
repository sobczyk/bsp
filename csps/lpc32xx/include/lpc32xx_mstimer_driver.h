/***********************************************************************
 * $Id:: lpc32xx_mstimer_driver.h 953 2008-07-28 17:07:53Z wellsk      $
 *
 * Project: LPC32xx millisecond timer driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx millisecond
 *     timer.
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

#ifndef LPC32XX_MSTIMER_DRIVER_H
#define LPC32XX_MSTIMER_DRIVER_H

#include "lpc32xx_mstimer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * MSTIMER device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure for setting up a millisecond timer match interrupt */
typedef struct
{
  /* Match timer to setup, must be 0 or 1 */
  UNS_32    timer_num;
  /* Interrupt enable flag, use TRUE to enable match interrupts for
     the selected timer, or FALSE to disable */
  BOOL_32   use_int;
  /* Match timer stop control, use TRUE to stop the timer from
     counting when the match value is reached, or FALSE if the timer
     should stay running. */
  BOOL_32   stop_on_match;
  /* Match timer reset control, use TRUE to reset the timer count on
     a timer match, or FALSE to not reset timer count on a match */
  BOOL_32   reset_on_match;
  /* Match timer compare value. This number is used to set the match
     value to compare the main timer value against. Use 0 to use the
     value of ms_val instead to autocompute a match time. */
  UNS_32    tick_val;
  /* If tick_val is 0, then this time (in milliseconds) is used to
     automatically compute a match compare value for the selected
     match counter */
  UNS_32    ms_val;
} MST_MATCH_SETUP_T;

/* MSTIMER device commands (IOCTL commands) */
typedef enum
{
  /* Enable or disable the timer, use arg with a value of '1' or
     '0' in arg to enable or disable */
  MST_TMR_ENABLE,
  /* Enable or disable a timer debug mode, use '1' or '0' in arg to
     enable or disable */
  MST_PAUSE_EN,
  /* Resets the timer count value, arg value does not matter */
  MST_TMR_RESET,
  /* Sets a raw value in the timer count register, use arg as a
     value to directly set */
  MST_SET_VALUE,
  /* Setup match compare logic, use arg as a pointer to a structure
     of type MST_MATCH_SETUP_T */
  MST_TMR_SETUP_MATCH,
  /* Clear a pending match interrupt, use arg as the match timer
     number (0 or 1) to clear */
  MST_CLEAR_INT,
  /* Get a timer status, use arg as value of type MST_IOCTL_STS_T */
  MST_GET_STATUS
} MST_IOCTL_CMD_T;

/* Timer device arguments for MST_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  /* Returns the clock rate driving the MSTIMER */
  MST_GET_CLOCK,
  /* Returns current timer count value */
  MST_VALUE_ST,
  /* Returns current match 0 compare value */
  MST_M0_VALUE_ST,
  /* Returns current match 1 compare value */
  MST_M1_VALUE_ST,
  /* Returns pending interrupt status, mask with MSTIM_INT_MATCH0_INT
     or MSTIM_INT_MATCH1_INT to determine if the match 0 and/or
     match 1 interrupt is pending */
  MST_INT_PEND
} MST_IOCTL_STS_T;

/* MSTIMER base clock rate */
#define MSTIMER_CLOCK (CLOCK_OSC_FREQ)

/***********************************************************************
 * MSTIMER driver API functions
 **********************************************************************/

/* Open the millisecond timer */
INT_32 mstimer_open(void *ipbase, INT_32 arg);

/* Close the millisecond timer */
STATUS mstimer_close(INT_32 devid);

/* Millisecond timer configuration block */
STATUS mstimer_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg);

/* Millisecond timer read function (stub only) */
INT_32 mstimer_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes);

/* Millisecond timer write function (stub only) */
INT_32 mstimer_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes);

/***********************************************************************
 * Other MSTIMER driver functions
 **********************************************************************/

/* Delay for msec milliseconds (minimum) */
void mstimer_wait_ms(UNS_32 msec);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_MSTIMER_DRIVER_H */
