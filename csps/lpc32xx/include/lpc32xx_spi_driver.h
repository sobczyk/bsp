/***********************************************************************
 * $Id:: lpc32xx_spi_driver.h 1116 2008-08-21 20:59:49Z stefanovicz    $
 *
 * Project: LPC3xxx SPI driver
 *
 * Description:
 *     This file contains driver support for the SPI module on the
 *     LPC3250
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

#ifndef LPC32XX_SPI_DRIVER_H
#define LPC32XX_SPI_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc32xx_spi.h"

/***********************************************************************
 * SPI device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure for setting up SPI parameters */
typedef struct
{
  /* Number of data bits, must be between 4 and 16 */
  UNS_32 databits;
  /* Flag used to set clock polarity high between frames when in
     SPI mode */
  BOOL_32 highclk_spi_frames;
  /* Flag used to set clock out phase, use TRUE to capture serial
     data on the second clock transition of the frame, or FALSE to
     use the first clock transition of the frame */
  BOOL_32 usesecond_clk_spi;
  /* Serial clock rate */
  UNS_32 spi_clk;
  /* Flag used to select MSB option */
  BOOL_32 msb;
  /* Flag used to select transmit/receive functionality */
  BOOL_32 transmitter;
  /* Flag used to select busy control */
  BOOL_32 busy_halt;
  /* Flag used to select busy signal polarity if used */
  BOOL_32 busy_polarity;
  /* Flag used to select usage of SPI_DATIO/DATIN pins */
  BOOL_32 unidirectional;
} SPI_CONFIG_T;

/* Callbacks for SPI */
typedef struct
{
  /* Transmit callback when more data is needed */
  PFV txcb;
  /* Receive callback */
  PFV rxcb;
} SPI_CBS_T;

/* SPI device commands (IOCTL commands) */
typedef enum
{
  /* Enable or disable the SPI, use arg = 0 to disable, arg = 1
     to enable */
  SPI_ENABLE,
  /* Setup the SPI controller, use arg as a pointer to type
     SPI_CONFIG_T */
  SPI_CONFIG,
  /* Quick Tx/Rx update */
  SPI_TXRX,
  /* Setup callbacks, use arg as a pointer to type SPI_CB_T */
  SPI_SET_CALLBACKS,
  /* Clear SPI interrupts */
  SPI_CLEAR_INTS,
  /* Clear SPI Rx buffer */
  SPI_CLEAR_RX_BUFFER,
  /* Get a SPI status, use an argument type of SPI_IOCTL_STS_T
     as the argument to return the correct status */
  SPI_GET_STATUS,
  /* Drive SSEL line, use an argument to set the level */
  SPI_DRIVE_SSEL,
  /* Make a delay after the last elemnt is sent in case the
     Frame Counter is not used */
  SPI_DELAY
} SPI_IOCTL_CMD_T;

/* SPI device arguments for SPI_GET_STATUS command (IOCTL arguments) */
typedef enum
{
  /* Returns SPI clock rate in Hz */
  SPI_CLOCK_ST,
  /* Returns masked pending interrupts */
  SPI_PENDING_INTS_ST,
  /* Returns raw pending interrupts */
  SPI_RAW_INTS_ST
} SPI_IOCTL_STS_T;

/***********************************************************************
 * SPI driver API functions
 **********************************************************************/

/* Open the SPI */
INT_32 spi_open(void *ipbase,
                INT_32 arg);

/* Close the SPI */
STATUS spi_close(INT_32 devid);

/* SPI configuration block */
STATUS spi_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* SPI write function - the buffer must be aligned on a 16-bit
   boundary if the data size is 9 bits or more */
INT_32 spi_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_fifo);

/* SPI read function - the buffer must be aligned on a 16-bit
   boundary if the data size is 9 bits or more */
INT_32 spi_read(INT_32 devid,
                void *buffer,
                INT_32 max_fifo);

/* SPI1 interrupt handler */
void spi1_int(void);

/* SPI2 interrupt handler */
void spi2_int(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SPI_DRIVER_H */
