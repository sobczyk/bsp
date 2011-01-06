/***********************************************************************
 * $Id:: lpc32xx_kscan_driver.h 930 2008-07-24 18:06:26Z wellsk        $
 *
 * Project: LPC32XX Keyboard scanner controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32XX Keyboard
 *     scanner controller.
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

#ifndef LPC32XX_KSCAN_DRIVER_H
#define LPC32XX_KSCAN_DRIVER_H

#include "lpc32xx_kscan.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * Keyboard scanner device configuration commands (IOCTL commands and
 * arguments)
 **********************************************************************/

/* Structure for initial setup of key scanner */
typedef struct
{
  /* Number of debounce clocks for a scan in the scan once state,
       use deb_clks as a value of 0 to 255. The input must remain in
       a steady state for this many clocks before proceeding to the
       scan matrix state. */
  UNS_32 deb_clks;
  /* Number of clocks in the scan matrix state, use scan_delay_clks
     as a value of 0 to 255, 0 = scan always, other values =
     (1 / clock) * 32 * rate. This rate controls the delay in the
     scan matrix state (the time between scan once states) */
  UNS_32 scan_delay_clks;
  /* Use 0 for PERIPH_CLK clock or 1 for the RTC. High PERIPH_CLK
     rates may not work */
  UNS_32 pclk_sel;
  /* Set key scanner dimension, use 0 to disable, or 1 to 8 for a
     1x1 to 8x8 matrix */
  UNS_32 matrix_size;
} KSCAN_SETUP_T;

/* Keypad scanner states */
typedef enum
{
  KSCAN_ST_IDLE     = KSCAN_SCOND_IN_IDLE,
  KSCAN_ST_SCANONCE = KSCAN_SCOND_IN_SCANONCE,
  KSCAN_ST_IRQ      = KSCAN_SCOND_IN_IRQGEN,
  KSCAN_ST_SCANMTRX = KSCAN_SCOND_IN_SCAN_MATRIX,
} KSCAN_STATE_T;

/* Keyboard scanner device commands (IOCTL commands) */
typedef enum
{
  /* Setup key scan controller, use arg as a pointer to type
     KSCAN_SETUP_T */
  KSCAN_SETUP,
  /* Clears a pending interrupt, use arg = 0 */
  KSCAN_CLEAR_INT,
  /* Force a single-scan, use arg = 1 for a scan, or arg = 0 to
     disable single scan mode */
  KSCAN_SCAN_ONCE,
  /* Get a keypad scanner status, use a arg of type
     KSCAN_IOCTL_STS_T */
  KSCAN_GET_STATUS
} KSCAN_IOCTL_CMD_T;

/* Keypad scanner arguments for KSCAN_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  /* Returns the current key scanner clock rate */
  KSCAN_GET_CLOCK,
  /* Returns the IRQ pending status, (0=no pending interrupts,
     not 0 = pending interrupts) status */
  KSCAN_IRQ_PENDING,
  /* Returns the current keypad scanner state, a value of type
     KSCAN_STATE_T */
  KSCAN_GET_STATE
} KSCAN_IOCTL_STS_T;

/* Keyboard scanner base clock rate for 32KHz rate */
#define KSCAN_32K_CLOCK (CLOCK_OSC_FREQ)

/***********************************************************************
 * Keyboard scanner driver API functions
 **********************************************************************/

/* Open the Keyboard scanner */
INT_32 kscan_open(void *ipbase,
                  INT_32 arg);

/* Close the Keyboard scanner */
STATUS kscan_close(INT_32 devid);

/* Keyboard scanner configuration block */
STATUS kscan_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg);

/* Keyboard scanner read function (reads 1 to 8 bytes) */
INT_32 kscan_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes);

/* Keyboard scanner write function (stub only) */
INT_32 kscan_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_KSCAN_DRIVER_H */
