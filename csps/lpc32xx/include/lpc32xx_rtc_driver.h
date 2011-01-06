/***********************************************************************
 * $Id:: lpc32xx_rtc_driver.h 936 2008-07-24 18:31:56Z wellsk          $
 *
 * Project: LPC32xx RTC driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx RTC.
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

#ifndef LPC32XX_RTC_DRIVER_H
#define LPC32XX_RTC_DRIVER_H

#include "lpc32xx_rtc.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * RTC configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure for setting up RTC match functions */
typedef struct
{
  /* Selected match function to use in this structure, 0 or 1 */
  UNS_32    match_num;
  /* Match interrupt enable flag, use TRUE to enable match interrupts
     for the RTC, or FALSE to disable */
  BOOL_32   use_match_int;
  /* Match timer compare value. This number is used to set the match
     value to compare the main timer value against */
  UNS_32    match_tick_val;
  /* ONSW enable, use TRUE to enable the ONSW signal when a match
     occurs, or FALSE to disable the ONSW signal */
  BOOL_32   enable_onsw;
} RTC_MATCH_SETUP_T;

/* Sturcture for setting and getting RTC RAM values */
typedef struct
{
  /* Ram value to place or get */
  UNS_32 ram_val;
  /* Index into RAM (32-bit) for set or get */
  UNS_8 ram_index;
} RTC_RAM_SG_T;

/* RTC device commands (IOCTL commands) */
typedef enum
{
  /* Resets the RTC, arg value is '1' to reset, and '0' to clear
     reset */
  RTC_RESET,
  /* Enable or disable the RTC, use arg with a value of '1' or
     '0' in arg to enable or disable */
  RTC_ENABLE,
  /* Set RTC up count, use arg as count value to set (down count will
     automatically be set as 0xFFFFFFFF - this value). The RTC must
     be disabled prior to setting the count with RTC_ENABLE. */
  RTC_SET_COUNT,
  /* Clear latched RTC interrupts, use arg as a Or'ed value of
     RTC_MATCH0_INT_STS, RTC_MATCH1_INT_STS, or RTC_ONSW_INT_STS to
   clear specific interrupts */
  RTC_CLEAR_INTS,
  /* Sets up a timer match function, use arg as a pointer to type
     RTC_MATCH_SETUP_T */
  RTC_SETUP_MATCH,
  /* Sets or clears the RTC, use arg = '1' to set the key, or '0'
     to clear it */
  RTC_SETCLR_KEY,
  /* Set a RAM value, use arg as a pointer to a structure of type
     RTC_RAM_SG_T */
  RTC_SET_RAM,
  /* Get a RAM value, use arg as a pointer to a structure of type
     RTC_RAM_SG_T */
  RTC_GET_RAM,
  /* Get a RTC status, use arg as value of type RTC_IOCTL_STS_T */
  RTC_GET_STATUS
} RTC_IOCTL_CMD_T;

/* RTC device arguments for RTC_GET_STATUS command (IOCTL arguments) */
typedef enum
{
  /* Returns the clock rate driving the RTC block */
  RTC_GET_CLOCK,
  /* Returns current timer up count value */
  RTC_UP_VALUE_ST,
  /* Returns current timer down count value */
  RTC_DOWN_VALUE_ST,
  /* Returns pending interrupt status, mask with RTC_MATCH0_INT_STS,
   RTC_MATCH1_INT_STS, or RTC_ONSW_INT_STS to determine if a
   specific match or capture interrupt pending */
  RTC_INT_PEND
} RTC_IOCTL_STS_T;

/***********************************************************************
 * RTC driver API functions
 **********************************************************************/

/* Open and close functions will NOT alter RTC functions */

/* Open the RTC */
INT_32 rtc_open(void *ipbase,
                INT_32 arg);

/* Close the RTC */
STATUS rtc_close(INT_32 devid);

/* RTC configuration block */
STATUS rtc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* RTC read function (stub only) */
INT_32 rtc_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes);

/* RTC write function (stub only) */
INT_32 rtc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_RTC_DRIVER_H */
