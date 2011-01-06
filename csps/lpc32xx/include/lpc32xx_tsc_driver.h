/***********************************************************************
 * $Id:: lpc32xx_tsc_driver.h 717 2008-05-08 15:29:36Z kendwyer        $
 *
 * Project: LPC32xx TSC driver
 *
 * Description:
 *     This file contains driver support for the TSC module on the
 *     LPC32XX
 *
 * Notes:
 *     See the lpc32xx_tsc_driver.c file for more information on this
 *     driver.
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

#ifndef LPC32xx_TSC_DRIVER_H
#define LPC32xx_TSC_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc32xx_tsc.h"

/***********************************************************************
 * TSC device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* TSC device commands (IOCTL commands) */
typedef enum
{

  SET_FIFO_DEPTH,  /* Set the FIFO depth. Valid values are 1, 4, 8 and 16 */

  TSC_NUM_BITS,    /* Number of bits to convert, vaild values 
                      are 0==10bits, 3->10. This feature does not apply in
                      AUX mode(ADC_CON bit10 =1)
					  if arg is 0 then 10bit mode is setup */

  TSC_AUTO_EN,	  /* TSC in Auto mode, IOCTL arg = 0 => disable */

  TSC_AUTO_POS,	  /* TSC Auto position enable, Use only when TSC_AUTO_EN=1
					 This mode generates an interrupt when the X/Y 
					 corordinates satify the Min X/Y Max X/Y register 
					 settings. If the fifo is used then interrupt is 
					 generated only when the fifo has filled to the 
					 specified level with the filtered X/Y samples arg =1 
					 will setup the mode, arg =0 will disable this mode*/

  TSC_AUX_MODE,	 /* TSC AUX mode, measure the AUX input pin. Use only with
                    AUTO Mode enabled. Do not use with Postion Detect mode
	                Set the UTR, AUX_MIN and AUX_MAX that will produce the
	                AUX interrupt. arg =0 will setup the mode, 
	                arg =0 will disable this mode*/

  TSC_MIN_X,    /* Used in Auto position mode only (1001b), the Min X value
                   to accept from the TSC. The TSC interrupt will only 
                   trigger when this is satisfied */

  TSC_MIN_Y,	/* Used in Auto position mode only (1001b), the Min Y value
                   to accept from the TSC. The TSC interrupt will only
                   trigger when this is satisfied*/

  TSC_MAX_X,    /* Used in Auto position mode only (1001b), the Max X value
                   to accept from the TSC. The TSC interrupt will only 
                   trigger when this is satisfied */

  TSC_MAX_Y,    /* Used in Auto position mode only (1001b), the Max Y value
                   to accept from the TSC. The TSC interrupt will only 
                   trigger when this is satisfied */

  TSC_AUX_UTR,  /* AUX sampling frequency, in touch clocks increments. The
                   TSC in AUX mode will sample the AUX pin every time the 
                   UTR time elapses. If the value samples satifies 
                   the TSC_AUX_MIN/MAX criteria, then a AUX interrupt is 
                   generated */

  TSC_AUX_MIN,  /* Min AUX input that will cause an interrupt */

  TSC_AUX_MAX,  /* Max AUX input that will cause an interrupt */

  TSC_GET_STATUS,   /* Get an TSC status, use an argument type of 
                       TSC_IOCTL_STS_T as the argument to return the status
                       Status for FIFO Empty and Overrun */

} TSC_IOCTL_CMD_T;


/* TSC device arguments for TSC_GET_STATUS command (IOCTL arguments) */
typedef enum
{

  TSC_FIFO_EMPTY_ST, /*  Returns the FIFO empty status */
  TSC_FIFO_OVRUN_ST, /*  Returns the FIFO overrun status*/

} TSC_IOCTL_STS_T;


/* Possible TSC and driver modes - these modes defines how the TSC
   controller and driver will work and are used with the ADC_SET_STATE
   IOCTL command */
typedef enum
{
  /* The TSC controller is configured to it's default settings and
     the placed in a low power state. */
  TSC_MODE_RESET,

} TSC_SSTATE_T;



/***********************************************************************
 * TSC driver API functions
 **********************************************************************/

/* Open the TSC */
INT_32 tsc_open(void *ipbase,
                INT_32 arg);

/* Close the TSC */
STATUS tsc_close(INT_32 devid);

/* TSC configuration block */
STATUS tsc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* TSC read function (interrupt, ring buffer) */
INT_32 tsc_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes);

/* TSC write function (stub only) */
INT_32 tsc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

/***********************************************************************
 * TSC driver miscellaneous functions
 **********************************************************************/

/* TSC interrupt handlers */

/* this is used to read the tsc fifo */
void tsc_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_TSC_DRIVER_H */
