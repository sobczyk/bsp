/***********************************************************************
 * $Id:: lpc32xx_hsuart_driver.h 1037 2008-08-06 23:10:44Z wellsk      $
 *
 * Project: LPC3\2xx High Speed UART driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx HS UART.
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

#ifndef LPC32XX_HSUART_DRIVER_H
#define LPC32XX_HSUART_DRIVER_H

#include "lpc32xx_hsuart.h"
#include "lpc_params.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* HSUART control */
typedef struct
{
  UNS_32 	baud_rate;     	/* Device baud rate */
  BOOL_32	cts_en;			/* CTS control		*/
  BOOL_32	cts_inv;		/* CTS inverted ctr	*/
  BOOL_32	rts_en;			/* RTS control		*/
  BOOL_32	rts_inv;		/* RTS inverted ctr	*/
} HSUART_CONTROL_T;

/***********************************************************************
 * HS UART device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure containing callback functions for data FIFO management
   and error management */
typedef struct
{
  /* Pointer to RX FIFO callback, called when their is data to
     read in the RX FIFO */
  PFV rxcb;
  /* Pointer to TX FIFO callback, called when transmit data is
     needed in the FIFO */
  PFV txcb;
  /* Pointer to error/break handler callback, called when their is
     a frame, parity, RX FIFO overrun error, or a break */
  PFV rxerrcb;
} HSUART_CBS_T;

/* HSUART device configuration structure type */
typedef struct
{
  HSUART_REGS_T *regptr;
  HSUART_CBS_T cbs;
  INT_32 hsuartnum; /* Used for array indicing, 0 = UART1 */
  UNS_32 baudrate;
  INT_32 divider;
  BOOL_32 hsuart_init;
} HSUART_CFG_T;

/* HS UART device commands (IOCTL commands) */
typedef enum
{
  /* Clear RX and/or TX UART FIFOs, use arg as a OR'ed combination
     of UART_FCR_TXFIFO_FLUSH and/or UART_FCR_RXFIFO_FLUSH to
     select which FIFOs to clear */
  HSUART_CLEAR_FIFOS,
  /* Setup general UART parameters, use arg as a pointer to a
     structure of type UART_CONTROL_T */
  HSUART_SETUP_TRANSFER,
  /* Enable or disable loopback mode for the UART, use arg as '0'
     to disable, or any other value to enable */
  HSUART_LOOPBACK_EN,
  /* Force a break condition on the TX line, use arg as '0' to clear
     the break, or any other value to set a break */
  HSUART_FORCE_BREAK,
  /* Install UART data handler callback functions, use arg as a
     pointer to type UART_CBS_T */
  HSUART_INSTALL_CBS,
  /* Get a UART status, use arg as value of type UART_IOCTL_STS_T */
  HSUART_GET_STATUS
} HSUART_IOCTL_CMD_T;

/* HS UART device arguments for HSUART_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  /* Returns the clock rate driving the standard UARTS */
  HSUART_GET_IP_CLOCK,
  /* Returns the actual clock rate the UARTs will transfer data at,
     after division */
  HSUART_GET_DERIVED_CLOCK,
  /* Returns the current line status register value, this is useful
     for popping the FIFO one byte at a time and getting the status
     for each byte, or using polling operation */
  HSUART_GET_LINE_STATUS
} HSUART_IOCTL_STS_T;

/***********************************************************************
 * HS UART driver API functions
 **********************************************************************/

/* Open the HS UART */
INT_32 hsuart_open(void *ipbase,
                   INT_32 arg);

/* Close the HS UART */
STATUS hsuart_close(INT_32 devid);

/* HS UART configuration block */
STATUS hsuart_ioctl(INT_32 devid,
                    INT_32 cmd,
                    INT_32 arg);

/* HS UART read function */
INT_32 hsuart_read(INT_32 devid,
                   void *buffer,
                   INT_32 max_bytes);

/* HS UART write function */
INT_32 hsuart_write(INT_32 devid,
                    void *buffer,
                    INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_HSUART_DRIVER_H */
