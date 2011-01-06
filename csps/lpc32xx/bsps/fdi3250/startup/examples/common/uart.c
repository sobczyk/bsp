/***********************************************************************
 * $Id:: uart.c 3391 2010-05-06 16:03:54Z usb10132                    $
 *
 * Project: UART diagnostic output
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

#include "lpc32xx_uart_driver.h"
#include "lpc32xx_hsuart_driver.h"
#include "misc_config.h"

extern int strlen(const UNS_8 *aString);
/* Device handler */
static INT_32 uartdev;

/***********************************************************************
 *
 * Function: uart_output
 *
 * Purpose: Outputs some string data on the UART
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff : Null terminated string to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void uart_output(UNS_8 *buff)
{
  int stt, sent, tosend, bytes;

  sent = 0;
  tosend = bytes = strlen(buff);
  while (bytes > 0)
  {
      /* ((unsigned int)UARTOUTPUTDEV) */
#ifdef UART_IS_HIGHSPEED
       stt = hsuart_write(uartdev, &buff [sent], tosend);
#else
       stt = uart_write(uartdev, &buff [sent], tosend);
#endif
    sent += stt;
	tosend -= stt;
	bytes -= stt;
  }
}

/***********************************************************************
 *
 * Function: uart_output_init
 *
 * Purpose: Initialize UART output
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void uart_output_init(void)
{
#ifdef UART_IS_HIGHSPEED
    HSUART_CONTROL_T ucntl;

     /* Open the UART for message output */
    /* Setup UART for 8 data bits, no parity, 1 stop bit */
    ucntl.baud_rate = UARTOUTPUTRATE;
    ucntl.cts_en = FALSE;
    ucntl.rts_en = FALSE;
    uartdev = hsuart_open((void *) UARTOUTPUTDEV, (INT_32) &ucntl);
#else
  UART_CONTROL_T ucntl;
  
   /* Open the UART for message output */
  /* Setup UART for 8 data bits, no parity, 1 stop bit */
  ucntl.baud_rate = UARTOUTPUTRATE;
  ucntl.parity = UART_PAR_NONE;
  ucntl.databits = 8;
  ucntl.stopbits = 1;
  uartdev = uart_open((void *) UARTOUTPUTDEV, (INT_32) &ucntl);
#endif
}

/***********************************************************************
 *
 * Function: uart_input
 *
 * Purpose: Read data from the UART
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_input(void *buffer, INT_32 max_bytes)
{
#ifdef UART_IS_HIGHSPEED
    return hsuart_read(uartdev, buffer, max_bytes);
#else
	return uart_read(uartdev, buffer, max_bytes);
#endif
	}
