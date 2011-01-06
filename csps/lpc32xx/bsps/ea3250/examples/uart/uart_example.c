/***********************************************************************
 * $Id:: uart_example.c 960 2008-07-28 17:32:53Z wellsk                $
 *
 * Project: UART driver example
 *
 * Description:
 *     A simple UART driver example.
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

#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc_arm922t_cp15_driver.h"
#include "lpc32xx_uart_driver.h"
#include "lpc32xx_intc_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* UART device handles */
static INT_32 uartdev;
static UNS_8 txbuff [512], rxbuff [512];
volatile static int txsize, rxsize;
static int txfill, rxget, txget, rxfill;
static UNS_8 sendstr[] =
  "Press any key to have it read in from the terminal and\n\r"
  "returned back to the terminal. Press ESC to exit.\n\r";

/***********************************************************************
 *
 * Function: term_dat_send_cb
 *
 * Purpose: UART transmit data callback
 *
 * Processing:
 *     Move data from the ring buffer to the driver.
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
void term_dat_send_cb(void)
{
  INT_32 bwrite, tosend = 512 - txget;

  if (tosend > txsize)
  {
    tosend = txsize;
  }

  /* Write data */
  bwrite = uart_write(uartdev, &txbuff[txget], tosend);
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
void term_dat_recv_cb(void)
{
  INT_32 bread, toreadmax = 512 - rxfill;

  /* Read data */
  bread = uart_read(uartdev, &rxbuff[rxfill], toreadmax);
  rxsize = rxsize + bread;
  rxfill = rxfill + bread;
  if (rxfill >= 512)
  {
    rxfill = 0;
  }
}

/***********************************************************************
 *
 * Function: term_dat_out
 *
 * Purpose: Send some data on the terminal interface
 *
 * Processing:
 *     Place data into the TX ring buffer and start UART transmission.
 *
 * Parameters:
 *     dat   : Data to send
 *     bytes : Number of bytes to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Will block until all bytes are sent.
 *
 **********************************************************************/
void term_dat_out(UNS_8 *dat, int bytes)
{
  while (bytes > 0)
  {
    while ((bytes > 0) && (txsize < 512))
    {
      txbuff[txfill] = *dat;
      txfill++;
      if (txfill >= 512)
      {
        txfill = 0;
      }
      dat++;
      bytes--;
      int_disable(IRQ_UART_IIR5);
      txsize++;
      int_enable(IRQ_UART_IIR5);
    }

    int_disable(IRQ_UART_IIR5);
    term_dat_send_cb();
    int_enable(IRQ_UART_IIR5);
  }
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
int term_dat_in(UNS_8 *buff, int bytes)
{
  int bread = 0;

  while ((bytes > 0) && (rxsize > 0))
  {
    *buff = rxbuff[rxget];
    buff++;
    rxget++;
    if (rxget >= 512)
    {
      rxget = 0;
    }
    bytes--;
    bread++;
    int_disable(IRQ_UART_IIR5);
    rxsize--;
    int_enable(IRQ_UART_IIR5);
  }

  return bread;
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/
int c_entry(void)
{
  UART_CBS_T cbs;
  UART_CONTROL_T ucntl;
  UNS_8 ch[16];

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Setup UART for 115.2K, 8 data bits, no parity, 1 stop bit */
  ucntl.baud_rate = 115200;
  ucntl.parity = UART_PAR_NONE;
  ucntl.databits = 8;
  ucntl.stopbits = 1;
  uartdev = uart_open((void *) UART5, (INT_32) & ucntl);
  if (uartdev != 0)
  {
    /* Setup RX and TX callbacks */
    cbs.rxcb = term_dat_recv_cb;
    cbs.txcb = term_dat_send_cb;
    cbs.rxerrcb = NULL;
    uart_ioctl(uartdev, UART_INSTALL_CBS, (INT_32) &cbs);
    int_enable(IRQ_UART_IIR5);
  }

  /* Initialize TX and RX ring buffers */
  txfill = txget = rxfill = rxget = txsize = rxsize = 0;

  /* Enable interrupts */
  int_enable(IRQ_UART_IIR5);
  enable_irq();

  /* Loop returning the data from the terminal */
  term_dat_out(sendstr, sizeof(sendstr));
  ch[0] = 0;
  enable_irq();
  while (ch[0] != 27)
  {
    if (term_dat_in(&ch[0], 16) > 0)
    {
      term_dat_out(&ch[0], 1);
    }
  }

  int_disable(IRQ_UART_IIR5);
  uart_close(uartdev);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  return 1;
}
