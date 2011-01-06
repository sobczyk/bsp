/***********************************************************************
 * $Id:: lpc32xx_timer_driver.h 962 2008-07-28 17:36:25Z wellsk        $
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

#ifndef LPC32XX_TIMER_DRIVER_H
#define LPC32XX_TIMER_DRIVER_H

#include "lpc32xx_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * Timer device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

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
} TMR_PSCALE_SETUP_T;

/* Structure for setting up the prescale increment control clock
   source */
typedef struct
{
  /* Set this flag to TRUE to use the PERIPH_CLK to drive the timer
     logic. If this is set to TRUE, the CAPn.x signals are not used.
     This will increment the prescaler counter or main counter on
     every positive edge of PERIPH_CLK */
  BOOL_32   use_pclk;
  /* Set this flag to TRUE to increment the perscaler counter or
     the main counter on every positive edge of the CAPn.x signal
     as selected by the count_in_sel value. This can be used with
     use_cap0_neg to generate signals on both edges. */
  BOOL_32   use_cap0_pos;
  /* Set this flag to TRUE to increment the perscaler counter or
     the main counter on every negative edge of the CAPn.x signal
     as selected by the count_in_sel value. This can be used with
     use_cap0_pos to generate signals on both edges. */
  BOOL_32   use_cap0_neg;
  /* Selected capture input (CAPn.cap_input) where cap_input is
     0 to 3 */
  UNS_32    cap_input;
} TMR_INPUT_CLK_T;

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
} TMR_MATCH_SETUP_T;

/* Structure for setting up capture control functions */
typedef struct
{
  /* Selected capture counter to use in this structure */
  UNS_32    timer_num;
  /* Capture interrupt enable flag, use TRUE to enable interrupts for
     when a capture occurs, or FALSE to disable */
  BOOL_32   use_capture_int;
  /* Capture enable for positive edge, use TRUE to enable a timer
     count capture on a positive edge of CAPn.0 (where n = 0 to 3),
     use FALSE to disable */
  BOOL_32   cap_on_rising;
  /* Capture enable for negative edge, use TRUE to enable a timer
     count capture on a negative edge of CAPn.0 (where n = 0 to 3),
     use FALSE to disable */
  BOOL_32   cap_on_falling;
} TMR_CAP_CLOCK_CTRL_T;

/* Structure for generating the external match signals */
typedef struct
{
  /* Selected timer match output function to use in this structure,
     use timer_num as 0 to 3 to select MATn.timer_num pin */
  UNS_32    timer_num;
  /* Forced state of MATn.timer_num pin, must be 0 or 1, used with
     the TIMER_CNTR_EMR_TOGGLE function to control output on
     MATn.timer_num */
  UNS_32    matn_state;
  /* State control for the MATn.timer_num pin when a match occurs,
     Selects what happens to the MATn.timer_num pin. Must be a value
     of TIMER_CNTR_EMR_NOTHING, TIMER_CNTR_EMR_LOW,
     TIMER_CNTR_EMR_HIGH, or TIMER_CNTR_EMR_TOGGLE */
  UNS_32    matn_state_ctrl;
} TMR_MATCH_GEN_T;

/* Structure for fetching the current count values */
typedef struct
{
  /* Returned current timer count value */
  UNS_32    count_val;
  /* Returned current timer prescale count value */
  UNS_32    ps_count_val;
  /* Returned timer capture count values */
  UNS_32    cap_count_val [4];
} TMR_COUNTS_T;

/* Timer device commands (IOCTL commands) */
typedef enum
{
  /* Enable or disable the timer, use arg with a value of '1' or
     '0' in arg to enable or disable */
  TMR_ENABLE,
  /* Resets the timer count and prescale values, arg value does not
     matter */
  TMR_RESET,
  /* Sets up a timer prescaler to generate the main timer counter
     increment rate, use arg as a pointer to type
     TMR_PSCALE_SETUP_T */
  TMR_SETUP_PSCALE,
  /* Setup the timer input clock source, use arg as a pointer to
     type TMR_INPUT_CLK_T */
  TMR_SETUP_CLKIN,
  /* Sets up a timer match function, use arg as a pointer to type
     TMR_MATCH_SETUP_T */
  TMR_SETUP_MATCH,
  /* Setup capture control functions, use arg as a pointer to type
     TMR_CAP_CLOCK_CTRL_T */
  TMR_SETUP_CAPTURE,
  /* Setup match output functions, use arg as a poniter to type
     TMR_MATCH_GEN_T */
  TMR_SETUP_MATCHOUT,
  /* Clear latched timer interrupts, use arg as a Or'ed value of
     TIMER_CNTR_MTCH_BIT(n) and TIMER_CNTR_MTCH_BIT(n), with n =
     0 to 3 for selected timer interrupts */
  TMR_CLEAR_INTS,
  /* Get current timer counts, use arg as a pointer to type
     TMR_COUNTS_T to be filled by the driver */
  TMR_GET_COUNTS,
  /* Get a timer status, use arg as value of type MST_IOCTL_STS_T */
  TMR_GET_STATUS
} TMR_IOCTL_CMD_T;

/* Timer device arguments for TMR_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  /* Returns the clock rate driving the standard timers */
  TMR_GET_CLOCK,
  /* Returns current timer count value */
  TMR_VALUE_ST,
  /* Returns pending interrupt status, mask with
     TIMER_CNTR_MTCH_BIT(n) or TIMER_CNTR_CAPT_BIT(n), where n = 0
     to 3 (timer number) to determine if a specific match or capture
     interrupt pending */
  TMR_INT_PEND
} TMR_IOCTL_STS_T;

/***********************************************************************
 * MSTIMER driver API functions
 **********************************************************************/

/* Open the timer */
INT_32 timer_open(void *ipbase,
                  INT_32 arg);

/* Close the timer */
STATUS timer_close(INT_32 devid);

/* Timer configuration block */
STATUS timer_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg);

/* Timer read function (stub only) */
INT_32 timer_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes);

/* Timer write function (stub only) */
INT_32 timer_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes);

/***********************************************************************
 * Other MSTIMER driver functions
 **********************************************************************/

/* Delay for msec milliseconds (minimum) */
void timer_wait_ms(TIMER_CNTR_REGS_T *pTimer, UNS_32 msec);

/* Delay for usec microseconds (minimum) */
void timer_wait_us(TIMER_CNTR_REGS_T *pTimer, UNS_32 usec);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_TIMER_DRIVER_H */
