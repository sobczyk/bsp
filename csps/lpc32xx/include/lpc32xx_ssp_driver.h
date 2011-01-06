/***********************************************************************
 * $Id:: lpc32xx_ssp_driver.h 1091 2008-08-18 22:17:28Z wellsk         $
 *
 * Project: LPC32XX SSP driver
 *
 * Description:
 *     This file contains driver support for the SSP module on the
 *     LPC32XX.
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

#ifndef LPC32XX_SSP_DRIVER_H
#define LPC32XX_SSP_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc32xx_ssp.h"

/***********************************************************************
 * SSP device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure for setting up SSP parameters */
typedef struct
{
  /* Number of data bits, must be between 4 and 16 */
  UNS_32 databits;
  /* Transfer mode, must be a value of SSP_CR0_FRF_MOT (Motorola SPI),
     SSP_CR0_FRF_TI (TI synchronous serial), or
     SSP_CR0_FRF_NS (National Microwire) */
  UNS_32 mode;
  /* Flag used to set clock polarity high between frames when in
     SPI mode */
  BOOL_32 highclk_spi_frames;
  /* Flag used to set clock out phase, use TRUE to capture serial
     data on the second clock transition of the frame, or FALSE to
     use the first clock transition of the frame */
  BOOL_32 usesecond_clk_spi;
  /* Serial clock rate */
  UNS_32 ssp_clk;
  /* Master/slave mode, use TRUE for master mode */
  BOOL_32 master_mode;
} SSP_CONFIG_T;

/* Callbacks for SSP */
typedef struct
{
  /* Transmit callback when more data is needed */
  PFV txcb;
  /* Receive callback (FIFO, FIFO timeout, and RX overtrun) */
  PFV rxcb;
} SSP_CBS_T;

/* SSP device commands (IOCTL commands) */
typedef enum
{
  /* Enable or disable the SSP, use arg = 0 to disable, arg = 1
     to enable */
  SSP_ENABLE,
  /* Setup the SSP controller, use arg as a pointer to type
     SSP_CONFIG_T */
  SSP_CONFIG,
  /* Enable or disable loopback mode, use arg as '1' to enable or
     '0' to disable */
  SSP_ENABLE_LOOPB,
  /* Slave output disable, use '1' to disable, or '0' for normal
     operation */
  SSP_SO_DISABLE,
  /* Setup callbacks, use arg as a pointer to type SSP_CBS_T */
  SSP_SET_CALLBACKS,
  /* Clear SSP interrupts, use arg as a OR'ed value of
     SSP_IIR_RORRIS or SSP_IIR_RTRIS */
  SSP_CLEAR_INTS,
  /* Get a SSP status, use an argument type of SSP_IOCTL_STS_T
     as the argument to return the correct status */
  SSP_GET_STATUS
} SSP_IOCTL_CMD_T;

/* SSP device arguments for SSP_GET_STATUS command (IOCTL arguments) */
typedef enum
{
  /* Returns SSP clock rate in Hz */
  SSP_CLOCK_ST,
  /* Returns masked pending interrupts (a combination of
     SSP_IIR_RORRIS, SSP_IIR_RTRIS, SSP_IIR_RXRIS, and
     SSP_IIR_TXRIS) */
  SSP_PENDING_INTS_ST,
  /* Returns raw pending interrupts (a combination of SSP_IIR_RORRIS,
     SSP_IIR_RTRIS, SSP_IIR_RXRIS, and SSP_IIR_TXRIS) */
  SSP_RAW_INTS_ST
} SSP_IOCTL_STS_T;

/***********************************************************************
 * SSP driver API functions
 **********************************************************************/

/* Open the SSP */
INT_32 ssp_open(void *ipbase,
                INT_32 arg);

/* Close the SSP */
STATUS ssp_close(INT_32 devid);

/* SSP configuration block */
STATUS ssp_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* SSP write function - the buffer must be aligned on a 16-bit
   boundary if the data size is 9 bits or more */
INT_32 ssp_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_fifo);

/* SSP read function - the buffer must be aligned on a 16-bit
   boundary if the data size is 9 bits or more */
INT_32 ssp_read(INT_32 devid,
                void *buffer,
                INT_32 max_fifo);

/* SSP0 interrupt handler */
void ssp0_int(void);

/* SSP1 interrupt handler */
void ssp1_int(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SSP_DRIVER_H */
