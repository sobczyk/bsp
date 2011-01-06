/***********************************************************************
 * $Id:: lpc32xx_uart_driver.c 3563 2010-05-21 17:08:34Z usb10132      $
 *
 * Project: LPC32xx standard UART driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx standard UART
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

#include "lpc32xx_uart_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * UART driver package data
***********************************************************************/

/* Structure for computing high/low dividers for clock rate */
typedef struct
{
  UNS_32 divx;
  UNS_32 divy; /* For x/y */
} UART_CLKDIV_T;

/* UART device configuration structure type */
typedef struct
{
  UART_REGS_T *regptr;
  UART_CBS_T cbs;
  INT_32 uartnum; /* Used for array indicing, 0 = UART1 */
  UNS_32 baudrate;
  UART_CLKDIV_T divs;
  BOOL_32 uart_init;
} UART_CFG_T;

/* UART driver data */
static UART_CFG_T uartdat [4];

/* Array to find a clock ID from it's UART number */
static const CLKPWR_CLK_T uart_num_to_clk_enum [4] =
{
  CLKPWR_UART3_CLK,
  CLKPWR_UART4_CLK,
  CLKPWR_UART5_CLK,
  CLKPWR_UART6_CLK
};

/***********************************************************************
 * UART driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: uart_gen_int_handler
 *
 * Purpose: General UART interrupt handler and router
 *
 * Processing:
 *     Handles transmit, receive, and status interrupts for the UART.
 *     Based on the interrupt status, routes the interrupt to the
 *     respective callback to be handled by the user application using
 *     this driver.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: If a callback doesn't exist, the interrupt will be disabled.
 *
 **********************************************************************/
void uart_gen_int_handler(UART_CFG_T *puartcfg)
{
  volatile UNS_32 tmp;

  /* Determine the interrupt source */
  tmp = puartcfg->regptr->iir_fcr & UART_IIR_INTSRC_MASK;
  switch (tmp)
  {
    case UART_IIR_INTSRC_RXLINE:
    default:
      /* RX line status interrupt, needs servicing */
      if (puartcfg->cbs.rxerrcb != NULL)
      {
        puartcfg->cbs.rxerrcb(puartcfg->uartnum + 3);
      }
      else
      {
        /* No callback, disable interrupt */
        puartcfg->regptr->dlm_ier &= ~(UART_IER_RXLINE_STS |
          UART_IER_MODEM_STS);
      }
      break;

    case UART_IIR_INTSRC_THRE:
      /* Disable interrupt, write will re-enable it */
      if (puartcfg->cbs.txcb != NULL)
      {
        puartcfg->cbs.txcb(puartcfg->uartnum + 3);
      }
      break;

    case UART_IIR_INTSRC_RDA:
    case UART_IIR_INTSRC_CTI:
      /* Receive interrupt, needs servicing */
      if (puartcfg->cbs.rxcb != NULL)
      {
        puartcfg->cbs.rxcb(puartcfg->uartnum + 3);
      }
      else
      {
        /* No callback, disable interrupt */
        puartcfg->regptr->dlm_ier &= ~UART_IER_RDA;
      }
      break;
  }
}

/***********************************************************************
 *
 * Function: uart3_int_handler
 *
 * Purpose: UART3 interrupt handler and router
 *
 * Processing:
 *     Handles UART 3 interrupt by routing the to general handler with
 *     the UART 3 driver data.
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
void uart3_int_handler(void)
{
  uart_gen_int_handler(&uartdat[0]);
}

/***********************************************************************
 *
 * Function: uart4_int_handler
 *
 * Purpose: UART4 interrupt handler and router
 *
 * Processing:
 *     Handles UART 4 interrupt by routing the to general handler with
 *     the UART 4 driver data.
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
void uart4_int_handler(void)
{
  uart_gen_int_handler(&uartdat[1]);
}

/***********************************************************************
 *
 * Function: uart5_int_handler
 *
 * Purpose: UART5 interrupt handler and router
 *
 * Processing:
 *     Handles UART 5 interrupt by routing the to general handler with
 *     the UART 5 driver data.
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
void uart5_int_handler(void)
{
  uart_gen_int_handler(&uartdat[2]);
}

/***********************************************************************
 *
 * Function: uart6_int_handler
 *
 * Purpose: UART6 interrupt handler and router
 *
 * Processing:
 *     Handles UART 6 interrupt by routing the to general handler with
 *     the UART 6 driver data.
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
void uart6_int_handler(void)
{
  uart_gen_int_handler(&uartdat[3]);
}

/***********************************************************************
 *
 * Function: uart_abs
 *
 * Purpose: ABS difference function
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     v1 : Value 1 for ABS
 *     v2 : Value 2 for ABS
 *
 * Outputs: None
 *
 * Returns: Absolute difference between the 2 values
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_abs(INT_32 v1, INT_32 v2)
{
  if (v1 > v2)
  {
    return v1 - v2;
  }

  return v2 - v1;
}

/***********************************************************************
 *
 * Function: uart_ptr_to_uart_num
 *
 * Purpose: Convert a UART register pointer to a UART number
 *
 * Processing:
 *     Based on the passed UART address, return the UART number.
 *
 * Parameters:
 *     puart : Pointer to a UART register set
 *
 * Outputs: None
 *
 * Returns: The UART number (0 to 3) or -1 if register pointer is bad
 *
 * Notes: Returned numbers are adjusted to offset 0 (UART3 is 0)
 *
 **********************************************************************/
INT_32 uart_ptr_to_uart_num(UART_REGS_T *puart)
{
  INT_32 uartnum = -1;

  if (puart == UART3)
  {
    uartnum = 0; /* UART 3 */
  }
  else if (puart == UART4)
  {
    uartnum = 1; /* UART 4 */
  }
  else if (puart == UART5)
  {
    uartnum = 2; /* UART 5 */
  }
  else if (puart == UART6)
  {
    uartnum = 3; /* UART 6 */
  }

  return uartnum;
}

/***********************************************************************
 *
 * Function: uart_flush_fifos
 *
 * Purpose: Flushes one or both of the UART FIFOs
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pregs     : Pointer to a UART register base
 *     flushword : Masked flush value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void uart_flush_fifos(UART_REGS_T *pregs,
                      UNS_32 flushword)
{
  volatile UNS_32 tmp;

  pregs->iir_fcr = (flushword | (UART_FCR_TXFIFO_FLUSH |
                                  UART_FCR_RXFIFO_FLUSH));

  /* An extra read of the RX FIFO (even if empty) will also clear any
     pending RX interrupts */
  tmp = pregs->dll_fifo;
}

/***********************************************************************
 *
 * Function: uart_find_clk
 *
 * Purpose: Determines best dividers to get a target clock rate
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     uartnum : UART number (0..3) for UARTS (3..6)
 *     freq    : Desired UART baud rate
 *     divs    : Structure to place dividers to get rate into
 *
 * Outputs: None
 *
 * Returns: Actual UART baud rate
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 uart_find_clk(UNS_32 freq,
                     UART_CLKDIV_T *divs)
{
  UNS_32 clkrate, savedclkrate, diff, basepclk;
  INT_32 idxx, idyy;
  UART_CLKDIV_T div;

  /* Get the clock rate for the UART block */
  basepclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK) >> 4;

  /* Find the best divider */
  div.divx = div.divy = 0;
  savedclkrate = 0;
  diff = 0xFFFFFFFF;
  for (idxx = 1; idxx < 0xFF; idxx++)
  {
    for (idyy = idxx; idyy < 0xFF; idyy++)
    {
      clkrate = (basepclk * idxx) / idyy;
      if (uart_abs(clkrate, freq) < diff)
      {
        diff = uart_abs(clkrate, freq);
        savedclkrate = clkrate;
        div.divx = idxx;
        div.divy = idyy;
      }
    }
  }

  /* Saved computed dividers */
  divs->divx = div.divx;
  divs->divy = div.divy;

  return savedclkrate;
}

/***********************************************************************
 *
 * Function: uart_setup_trans_mode
 *
 * Purpose: Sets up a UART data transfer mode
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     puartcfg   : Pointer to UART configuration data
 *     puartsetup : Pointer to a UART transfer mode setup structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if setup was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS uart_setup_trans_mode(UART_CFG_T *puartcfg,
                             UART_CONTROL_T *puartsetup)
{
  UNS_32 tmp32, tmp = 0;
  STATUS err = _NO_ERROR;

  /* Setup stop bits */
  switch (puartsetup->stopbits)
  {
    case 2:
      tmp |= UART_LCR_STOP2BITS;
    case 1:
      break;

    default:
      err = _ERROR;
      break;
  }

  /* Setup parity */
  switch (puartsetup->parity)
  {
    case UART_PAR_EVEN:
      tmp |= (UART_LCR_PARITY_EVEN | UART_LCR_PARITY_ENABLE);
      break;

    case UART_PAR_ODD:
      tmp |= (UART_LCR_PARITY_ODD | UART_LCR_PARITY_ENABLE);
      break;

    case UART_PAR_NONE:
      break;

    default:
      err = _ERROR;
      break;
  }

  /* Setup data bits */
  switch (puartsetup->databits)
  {
    case 5:
      tmp |= UART_LCR_WLEN_5BITS;
      break;

    case 6:
      tmp |= UART_LCR_WLEN_6BITS;
      break;

    case 7:
      tmp |= UART_LCR_WLEN_7BITS;
      break;

    case 8:
      tmp |= UART_LCR_WLEN_8BITS;
      break;

    default:
      err = _ERROR;
      break;
  }

  if (err == _NO_ERROR)
  {
    /* Find closest baud rate for desired clock frequency */
    puartcfg->baudrate = uart_find_clk(puartsetup->baud_rate,
                                       &puartcfg->divs);

    /* Set clock x/y divider for the UART */
    switch (puartcfg->uartnum)
    {
      case 0:
        CLKPWR->clkpwr_uart3_clk_ctrl =
          CLKPWR_UART_X_DIV(puartcfg->divs.divx)
          | CLKPWR_UART_Y_DIV(puartcfg->divs.divy);
        break;

      case 1:
        CLKPWR->clkpwr_uart4_clk_ctrl =
          CLKPWR_UART_X_DIV(puartcfg->divs.divx)
          | CLKPWR_UART_Y_DIV(puartcfg->divs.divy);
        break;

      case 2:
        CLKPWR->clkpwr_uart5_clk_ctrl =
          CLKPWR_UART_X_DIV(puartcfg->divs.divx)
          | CLKPWR_UART_Y_DIV(puartcfg->divs.divy);
        break;

      case 3:
      default:
        CLKPWR->clkpwr_uart6_clk_ctrl =
          CLKPWR_UART_X_DIV(puartcfg->divs.divx)
          | CLKPWR_UART_Y_DIV(puartcfg->divs.divy);
        break;
    }

    /* Use automatic clocking */
    tmp32 = UARTCNTL->clkmode &
            ~UART_CLKMODE_MASK(puartcfg->uartnum + 3);
    UARTCNTL->clkmode = tmp32 | UART_CLKMODE_LOAD(
                          UART_CLKMODE_AUTO, (puartcfg->uartnum + 3));

    /* Set new UART settings */
    puartcfg->regptr->lcr = tmp;
  }

  return err;
}

/***********************************************************************
 * UART driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: uart_open
 *
 * Purpose: Open the UART
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a UART peripheral block
 *     arg   : Pointer to UART setup structure or NULL
 *
 * Outputs: None
 *
 * Returns: The pointer to a UART config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_open(void *ipbase,
                 INT_32 arg)
{
  UART_CONTROL_T uarttran;
  volatile UNS_32 tmp;
  INT_32 uartnum, tptr = (INT_32) NULL;
  UART_CONTROL_T *pucfg = (UART_CONTROL_T *) arg;

  /* Try to find a matching UART number based on the passed pointer */
  uartnum = uart_ptr_to_uart_num((UART_REGS_T *) ipbase);
  if (uartnum >= 0)
  {
    /* Has the UART been previously initialized? */
    if (uartdat[uartnum].uart_init == FALSE)
    {
      /* UART is free */
      uartdat[uartnum].uart_init = TRUE;
      uartdat[uartnum].uartnum = uartnum;
      uartdat[uartnum].regptr = (UART_REGS_T *) ipbase;

      /* Enable UART system clock */
      clkpwr_clk_en_dis(uart_num_to_clk_enum[uartnum], 1);
      tmp = UARTCNTL->clkmode & ~UART_CLKMODE_MASK(uartnum + 3);
      UARTCNTL->clkmode = (tmp |
        UART_CLKMODE_LOAD(UART_CLKMODE_AUTO, (uartnum + 3)));

      /* No callbacks by default */
      uartdat[uartnum].cbs.rxcb = NULL;
      uartdat[uartnum].cbs.txcb = NULL;
      uartdat[uartnum].cbs.rxerrcb = NULL;

      /* Install general interrupt handler */
      switch (uartnum)
      {
        case 0:
          int_install_irq_handler(IRQ_UART_IIR3,
                                  uart3_int_handler);
          break;
        case 1:
          int_install_irq_handler(IRQ_UART_IIR4,
                                  uart4_int_handler);
          break;

        case 2:
          int_install_irq_handler(IRQ_UART_IIR5,
                                  uart5_int_handler);
          break;

        case 3:
          int_install_irq_handler(IRQ_UART_IIR6,
                                  uart6_int_handler);
          break;
      }

      /* UART baud rate generator isn't used, so just set it to divider
          by 1 */
      uartdat[uartnum].regptr->lcr |= UART_LCR_DIVLATCH_EN;
      uartdat[uartnum].regptr->dll_fifo = 1;
      uartdat[uartnum].regptr->dlm_ier = 0;
      uartdat[uartnum].regptr->lcr &= ~UART_LCR_DIVLATCH_EN;

      /* Setup default UART state for 9600N81 with FIFO mode */
      if (pucfg == NULL)
      {
        uarttran.baud_rate = 9600;
        uarttran.parity = UART_PAR_NONE;
        uarttran.stopbits = 1;
        uarttran.databits = 8;
        uart_setup_trans_mode(&uartdat[uartnum], &uarttran);
      }
      else
      {
        uart_setup_trans_mode(&uartdat[uartnum], pucfg);
      }

      /* Clear FIFOs, set FIFO level, and pending interrupts */
      uartdat[uartnum].regptr->iir_fcr = (UART_FCR_RXFIFO_TL16 |
        UART_FCR_TXFIFO_TL0 | UART_FCR_FIFO_CTRL |
        UART_FCR_FIFO_EN | UART_FCR_TXFIFO_FLUSH |
        UART_FCR_RXFIFO_FLUSH);
      tmp = uartdat[uartnum].regptr->dll_fifo;
      tmp = uartdat[uartnum].regptr->iir_fcr;
      tmp = uartdat[uartnum].regptr->lsr;

      /* Receive,  RX line status, and mode status interrupts enabled */
      uartdat[uartnum].regptr->dlm_ier = (UART_IER_MODEM_STS |
                                         UART_IER_RXLINE_STS |
                                         UART_IER_RDA | UART_IER_THRE);

      /* Return pointer to specific UART structure */
      tptr = (INT_32) &uartdat[uartnum];
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: uart_close
 *
 * Purpose: Close the UART
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the UART,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS uart_close(INT_32 devid)
{
  volatile UNS_32 tmp;
  UART_CFG_T *puart = (UART_CFG_T *) devid;
  STATUS status = _ERROR;

  /* Close and disable device if it was previously initialized */
  if (puart->uart_init == TRUE)
  {
    /* Disable interrupts */
    puart->regptr->dlm_ier = 0;

    /* Turn off clocking */
    tmp = UARTCNTL->clkmode &
          ~UART_CLKMODE_MASK(puart->uartnum + 3);
    UARTCNTL->clkmode = tmp | UART_CLKMODE_LOAD(
                          UART_CLKMODE_OFF, (puart->uartnum + 3));

    /* Free UART and Disable timer system clock */
    puart->uart_init = FALSE;
    tmp = UARTCNTL->clkmode & ~UART_CLKMODE_MASK(puart->uartnum + 3);
    UARTCNTL->clkmode = (tmp |
        UART_CLKMODE_LOAD(UART_CLKMODE_OFF, (puart->uartnum + 3)));
    clkpwr_clk_en_dis(uart_num_to_clk_enum[puart->uartnum], 0);

    status = _NO_ERROR;
  }

  return status;
}

/***********************************************************************
 *
 * Function: uart_ioctl
 *
 * Purpose: UART configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate UART parameter.
 *
 * Parameters:
 *     devid: Pointer to UART config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS uart_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
  UART_CBS_T *pcbs;
  UART_CFG_T *puart = (UART_CFG_T *) devid;
  STATUS status = _ERROR;

  /* Close and disable device if it was previously initialized */
  if (puart->uart_init == TRUE)
  {
    status = _NO_ERROR;

    switch (cmd)
    {
      case UART_CLEAR_FIFOS:
        uart_flush_fifos(puart->regptr, (UNS_32) arg);
        break;

      case UART_SETUP_TRANSFER:
        uart_setup_trans_mode(puart, (UART_CONTROL_T *) arg);
        break;

      case UART_LOOPBACK_EN:
        if (arg != 0)
        {
          UARTCNTL->loop |=
            UART_LPBACK_ENABLED(puart->uartnum + 1);
        }
        else
        {
          UARTCNTL->loop &=
            ~UART_LPBACK_ENABLED(puart->uartnum + 1);
        }
        break;

      case UART_FORCE_BREAK:
        if (arg != 0)
        {
          puart->regptr->lcr |= UART_LCR_BREAK_EN;
        }
        else
        {
          puart->regptr->lcr &= ~UART_LCR_BREAK_EN;
        }
        break;

      case UART_INSTALL_CBS:
        pcbs = (UART_CBS_T *) arg;
        puart->cbs.rxcb = pcbs->rxcb;
        puart->cbs.txcb = pcbs->txcb;
        puart->cbs.rxerrcb = pcbs->rxerrcb;
        break;

      case UART_GET_STATUS:
        /* Return a UART status */
        switch (arg)
        {
          case UART_GET_IP_CLOCK:
            status = clkpwr_get_base_clock_rate(
                       CLKPWR_PERIPH_CLK);
            break;

          case UART_GET_DERIVED_CLOCK:
            status = puart->baudrate;
            break;

          case UART_GET_LINE_STATUS:
            status = (STATUS) puart->regptr->lsr;
            break;

          case UART_GET_MODEM_STATUS:
            status = (STATUS) puart->regptr->modem_status;
            break;

		  default:
            /* Unsupported parameter */
            status = LPC_BAD_PARAMS;
            break;
        }
        break;

      default:
        /* Unsupported parameter */
        status = LPC_BAD_PARAMS;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: uart_read
 *
 * Purpose: UART read function (stub only)
 *
 * Processing:
 *     Read the passed number of bytes in the passed buffer, or the
 *     amount of data that is available, whichever is less.
 *
 * Parameters:
 *     devid:     Pointer to UART descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes)
{
  INT_32 bread = 0;
  UART_CFG_T *puart = (UART_CFG_T *) devid;
  UART_REGS_T *pregs = puart->regptr;
  UNS_8 *buff8 = (UNS_8 *) buffer;

  while ((max_bytes > 0) && ((pregs->lsr & UART_LSR_RDR) != 0))
  {
    *buff8 = (UNS_8) pregs->dll_fifo;
    buff8++;
    max_bytes--;
    bread++;
  }

  return bread;
}

/***********************************************************************
 *
 * Function: uart_write
 *
 * Purpose: Timer UART function
 *
 * Processing:
 *     Write the passed number of bytes in the passed buffer to the UART
 *     FIFO, or the amounf of data that the FIFO can handle.
 *
 * Parameters:
 *     devid:   Pointer to UART descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes)
{
  INT_32 bwrite = 0;
  UART_CFG_T *puart = (UART_CFG_T *) devid;
  UART_REGS_T *pregs = puart->regptr;
  UNS_8 *buff8 = (UNS_8 *) buffer;

  /* Only add data if the current FIFO level can be determined */
  if ((pregs->lsr & UART_LSR_THRE) != 0)
  {
    while ((n_bytes > 0) && ((pregs->lsr & UART_LSR_THRE) != 0))
    {
      pregs->dll_fifo = (UNS_32) * buff8;
      buff8++;
      n_bytes--;
      bwrite++;
    }

    /* Since the trip point is at 8 bytes, it's ok to add up to
       8 more bytes of data for transmit */
    if (n_bytes > 8)
    {
      n_bytes = 8;
    }
    while (n_bytes > 0)
    {
      pregs->dll_fifo = (UNS_32) * buff8;
      buff8++;
      n_bytes--;
      bwrite++;
    }
  }

  return bwrite;
}
