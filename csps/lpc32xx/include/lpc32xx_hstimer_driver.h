/***********************************************************************
 * $Id:: lpc32xx_hstimer_driver.h 801 2008-06-10 23:53:16Z sscaglia    $
 *
 * Project: LPC32xx high speed timer driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx high speed
 *     timer and counters.
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

#ifndef LPC32XX_HSTIMER_DRIVER_H
#define LPC32XX_HSTIMER_DRIVER_H

#include "lpc32xx_hstimer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * HSTIMER device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* HSTIMER device commands (IOCTL commands) */
typedef enum
{
  HST_ENABLE,      /* Enable or disable the timer, use arg with a
 					    value of '1' or '0' in arg to enable or
					    disable */
  HST_RESET,       /* Resets the timer count and prescale values, arg
					    value does not matter */

  HST_PAUSE,		/* Enable or disable the PAUSE_EN bit (Timer Counter is stopped
						when the core is in debug mode), use arg with a
 					    value of '1' or '0' in arg to enable or
					    disable */

  /* Clear a pending interrupt, use arg as an OR'ed value of
     TIMER_CNTR_MTCH_BIT(n), where n = 0 to 3, and
     TIMER_CNTR_CAPT_BIT(n), where n = 0 to 3 to clear */
  HST_CLEAR_INTS,
  /* Sets up a timer prescaler to generate the main timer counter
     increment rate, use arg as a pointer to type
     HST_PSCALE_SETUP_T */
  HST_SETUP_PSCALE,
  /* Sets up a timer match function, use arg as a pointer to type
     HST_MATCH_SETUP_T */
  HST_SETUP_MATCH,
  /* Setup capture control functions, use arg as a pointer to type
     HST_CAP_CLOCK_CTRL_T */
  HST_SETUP_CAPTURE,
  HST_GET_STATUS  /* Get a timer status, use a pointer to the
					   HST_ARG_T structure with the timer enumeration
					   and a arg value of HST_IOCTL_STS_T */
} HST_IOCTL_CMD_T;

/* Structure used for some IOCTLS */
typedef struct
{
  INT_32 arg1;
  UNS_32 arg2;
} HST_ARG_T;


/* Structure for setting up the timer prescale count control */
typedef struct
{
  /* Prescaler compare value. This number is used to set the prescale
     value to generate a divider for the timer counter. Use 0 to use
     the value of ps_us_val instead to autocompute a prescale time. */
  UNS_32    ps_tick_val;
  /* If ps_tick_val is 0, then this time (in milliseconds) is used to
     automatically compute a match compare value for the selected
     match counter */
  UNS_32    ps_us_val;
} HST_PSCALE_SETUP_T;

/* Structure for setting up timer match functions */
typedef struct
{
  /* Selected timer match function to use in this structure */
  UNS_32    timer_num;
  /* Match interrupt enable flag, use TRUE to enable match interrupts
     for the timer, or FALSE to disable */
  BOOL_32   use_match_int;
  /* Match timer stop control, use TRUE to stop the timer from
     counting when the match value is reached, or FALSE if the timer
     should stay running. */
  BOOL_32   stop_on_match;
  /* Match timer reset control, use TRUE to reset the timer count on
     a timer match, or FALSE to not reset timer count on a match */
  BOOL_32   reset_on_match;
  /* Match timer compare value. This number is used to set the match
     value to compare the main timer value against */
  UNS_32    match_tick_val;
} HST_MATCH_SETUP_T;


/* Structure for setting up capture control functions */
typedef struct
{
  /* Selected capture counter to use in this structure */
  UNS_32    cap_num;
  /* Capture interrupt enable flag, use TRUE to enable interrupts for
     when a capture occurs, or FALSE to disable */
  BOOL_32   use_capture_int;
  /* Capture enable for positive edge, use TRUE to enable a timer
     count capture on a positive edge of CAPn (where n = 0 or 1),
     use FALSE to disable */
  BOOL_32   cap_on_rising;
  /* Capture enable for negative edge, use TRUE to enable a timer
     count capture on a negative edge of CAPn (where n = 0 or 1),
     use FALSE to disable */
  BOOL_32   cap_on_falling;
} HST_CAP_CLOCK_CTRL_T;


/* HSTIMER device arguments for HST_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  HST_GET_COUNT,      /* Returns the current timer count */
  HST_GET_PS_COUNT,   /* Returns the current prescaler count */
  HST_GET_MATCH0_REG, /* Returns MACTH0 register content */
  HST_GET_MATCH1_REG, /* Returns MACTH1 register content */
  HST_GET_MATCH2_REG, /* Returns MACTH2 register content */
  HST_GET_CR0_REG,    /* Returns CAPTURE 0 register content */
  HST_GET_CR1_REG,    /* Returns CAPTURE1 register content */
  HST_GET_CLOCK,      /* Returns the clock rate (Hz) driving the clock */
  HST_INT_PEND        /* Returns interrupt pending status*/
} HST_IOCTL_STS_T;

/***********************************************************************
 * HSTIMER driver API functions
 **********************************************************************/

/* Open the timer */
INT_32 hstimer_open(void *ipbase,
                    INT_32 arg);

/* Close the timer */
STATUS hstimer_close(INT_32 devid);

/* Timer configuration block */
STATUS hstimer_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg);

/* Timer read function (stub only) */
INT_32 hstimer_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes);

/* Timer write function (stub only) */
INT_32 hstimer_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_HSTIMER_DRIVER_H */
