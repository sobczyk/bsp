/***********************************************************************
 * $Id:: sysapi_termio.c 3394 2010-05-06 17:56:27Z usb10132            $
 *
 * Project: Terminal (serial) support functions
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         term_dat_out
 *         term_dat_out_crlf
 *         term_dat_out_len
 *         term_dat_in
 *         term_dat_in_ready
 *         term_dat_flush
 *         term_init
 *         term_deinit
 *         term_setbaud
 *         term_break
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

#include "s1l_sys_inf.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_hsuart_driver.h"
#include "lpc_string.h"

/* TX an RX ring buffers */
static UNS_8 txbuff [512], rxbuff [512];
volatile static int txsize, rxsize;
static int txfill, rxget, txget, rxfill;
static INT_32 uartdev;
static volatile BOOL_32 uartbrk;
static UNS_8 crlf[] = "\r\n";

#define TERMIO_IRQ      IRQ_UART_IIR1

/***********************************************************************
 *
 * Function: term_dat_send_cb
 *
 * Purpose: UART transmit data callback
 *
 * Processing:
 *     Stuff more data into the UART FIFO.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: This function is called in interrupt context.
 *
 **********************************************************************/
void term_dat_send_cb(void) {
  INT_32 bwrite, tosend = 512 - txget;

  if (tosend > txsize)
  {
    tosend = txsize;
  }

  /* Write data */
  bwrite = hsuart_write(uartdev, &txbuff[txget], tosend);
  txsize = txsize - bwrite;
  txget = txget + bwrite;
  if (txget >= 512)
  {
    txget = 0;
  }
}

/***********************************************************************
 *
 * Function: term_dat_recv_cb
 *
 * Purpose: UART receive data callback
 *
 * Processing:
 *     Read data from the driver into the RX ring buffer.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Ring buffer overflow is not accounted for in this application.
 *     This function is called in interrupt context.
 *
 **********************************************************************/
void term_dat_recv_cb(void) {
  INT_32 bread, toreadmax = 512 - rxfill;

  /* Read data */
  bread = hsuart_read(uartdev, &rxbuff[rxfill], toreadmax);
  rxsize = rxsize + bread;
  rxfill = rxfill + bread;
  if (rxfill >= 512)
  {
    rxfill = 0;
  }
}

/***********************************************************************
 *
 * Function: term_status_cb
 *
 * Purpose: UART misc callback
 *
 * Processing:
 *     Tests for the break condition.
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
void term_status_cb(void) 
{
	UNS_32 tmp;
	
	tmp = hsuart_ioctl(uartdev, HSUART_GET_STATUS, HSUART_GET_LINE_STATUS);
	if ((tmp & HSU_BRK_INT) != 0)
	{
		uartbrk = TRUE;
	}
}

/***********************************************************************
 *
 * Function: term_dat_out_len
 *
 * Purpose: Send a number of characters on the terminal interface
 *
 * Processing:
 *     Move data into the UART ring buffer.
 *
 * Parameters:
 *     dat   : Data to send
 *     chars : Number of bytes to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Will block until all bytes are sent.
 *
 **********************************************************************/
void term_dat_out_len(UNS_8 *dat,
				      int chars) {
  while (chars > 0)
  {
    while ((chars > 0) && (txsize < 512))
    {
      txbuff[txfill] = *dat;
      txfill++;
      if (txfill >= 512)
      {
        txfill = 0;
      }
      dat++;
      chars--;
      int_disable(TERMIO_IRQ);
      txsize++;
      int_enable(TERMIO_IRQ);
    }

    int_disable(TERMIO_IRQ);
    term_dat_send_cb();
    int_enable(TERMIO_IRQ);
  }
}

/***********************************************************************
 *
 * Function: term_dat_out
 *
 * Purpose:
 *     Send some data on the terminal interface up to NULL character
 *
 * Processing:
 *     Move data into the UART ring buffer.
 *
 * Parameters:
 *     dat : Data to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Will block until all bytes are sent.
 *
 **********************************************************************/
void term_dat_out(UNS_8 *dat)
{
	term_dat_out_len(dat, str_size(dat));
}

/***********************************************************************
 *
 * Function: term_dat_out_crlf
 *
 * Purpose:
 *     Send some data on the terminal interface up to NULL character
 *     with a linefeed
 *
 * Processing:
 *     Move data into the UART ring buffer.
 *
 * Parameters:
 *     dat : Data to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Will block until all bytes are sent.
 *
 **********************************************************************/
void term_dat_out_crlf(UNS_8 *dat)
{
	term_dat_out(dat);
	term_dat_out(crlf);
}

/***********************************************************************
 *
 * Function: term_dat_in
 *
 * Purpose: Read some data from the terminal interface
 *
 * Processing:
 *     Move data from the ring buffer to the passed buffer.
 *
 * Parameters:
 *     buff  : Where to place the data
 *     bytes : Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read
 *
 * Notes: None
 *
 **********************************************************************/
int term_dat_in(UNS_8 *buff,
				int bytes) {
	int bread = 0;

	while ((bytes > 0) && (rxsize > 0)) {
		*buff = rxbuff[rxget];
		buff++;
		rxget++;
		if (rxget >= 512) {
			rxget = 0;
		}
		bytes--;
		bread++;
		int_disable(TERMIO_IRQ);
		rxsize--;
		int_enable(TERMIO_IRQ);
	}

	return bread;
}

/***********************************************************************
 *
 * Function: term_dat_in_ready
 *
 * Purpose:
 *    Determine how many bytes are waiting on the terminal interface
 *
 * Processing:
 *     Return the RX ring buffer size.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Number of bytes waiting to be read
 *
 * Notes: None
 *
 **********************************************************************/
int term_dat_in_ready(void) {
	return rxsize;
}

/***********************************************************************
 *
 * Function: term_dat_flush
 *
 * Purpose: Flush data in the terminal input buffer
 *
 * Processing:
 *     Flush the UART FIFOs.
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
void term_dat_flush(void) {
	hsuart_ioctl(uartdev, HSUART_CLEAR_FIFOS, 0);
}

/***********************************************************************
 *
 * Function: term_init
 *
 * Purpose: Initialize terminal I/O interface
 *
 * Processing:
 *     Use the UART driver to open and initialize the serial port
 *     session.
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
void term_init(void) {
	HSUART_CBS_T cbs;
	HSUART_CONTROL_T ucntl;
	HSUART_REGS_T *puartregs = UART1;

	/* 115.2K, 8 data bits, no parity, 1 stop bit */
	ucntl.baud_rate = 115200;
    ucntl.cts_en = FALSE;
    ucntl.cts_inv = FALSE;
    ucntl.rts_en = FALSE;
    ucntl.rts_inv = FALSE;
	uartdev = hsuart_open((void *) puartregs, (INT_32) &ucntl);

	if (uartdev != 0) {
		/* Setup RX and TX callbacks */
		cbs.rxcb = term_dat_recv_cb;
		cbs.txcb = term_dat_send_cb;
		cbs.rxerrcb = term_status_cb;
		hsuart_ioctl(uartdev, HSUART_INSTALL_CBS, (INT_32) &cbs);
		int_enable(TERMIO_IRQ);
	}

	/* Initialize TX and RX ring buffers */
	txfill = txget = rxfill = rxget = txsize = rxsize = 0;
    uartbrk = FALSE;
}

/***********************************************************************
 *
 * Function: term_deinit
 *
 * Purpose: Shutdown terminal I/O
 *
 * Processing:
 *     Close the serial port through the UART driver.
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
void term_deinit(void) {
	int_disable(TERMIO_IRQ);
	hsuart_close(uartdev);
}

/***********************************************************************
 *
 * Function: term_setbaud
 *
 * Purpose: Reset terminal baud rate
 *
 * Processing:
 *     Use the serial port IOCTL to reset baud rate.
 *
 * Parameters:
 *     baud : New baud rate in Hz
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void term_setbaud(UNS_32 baud) 
{
	HSUART_CONTROL_T ucntl;

		/* 8 data bits, no parity, 1 stop bit, no hardware flow control */
		ucntl.baud_rate = baud;
		ucntl.cts_en = FALSE;
		ucntl.cts_inv = FALSE;
        ucntl.rts_en = FALSE;
        ucntl.rts_inv = FALSE;
		hsuart_ioctl(uartdev, HSUART_SETUP_TRANSFER, (INT_32) &ucntl);
}

/***********************************************************************
 *
 * Function: term_break
 *
 * Purpose: Check for latched terminal break condition
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if a break was detected, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 term_break(void) 
{
	BOOL_32 bhit = uartbrk;
	uartbrk = FALSE;
	return bhit;
}
