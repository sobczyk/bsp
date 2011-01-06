/***********************************************************************
 * $Id:: lpc32xx_hsuart_driver.c 4978 2010-09-20 22:32:52Z usb10132    $
 *
 * Project: LPC3xxx High Speed UART driver
 *
 * Description:
 *     This file contains driver support for the LPC3xxx HS UART
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

#include "lpc32xx_uart.h"
#include "lpc32xx_hsuart_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * HSUART driver package data
***********************************************************************/

/* HSUART driver data */
static HSUART_CFG_T hsuartdat [3];

/***********************************************************************
 * HSUART driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: hsuart_gen_int_handler
 *
 * Purpose: General HS UART interrupt handler and router
 *
 * Processing:
 *     Handles transmit, receive, and status interrupts for the HS UART.
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
void hsuart_gen_int_handler(HSUART_CFG_T *phsuartcfg)
{
  volatile UNS_32 tmp;

  /* Determine the interrupt source */
  tmp = phsuartcfg->regptr->iir;

  if ((tmp & (HSU_RX_TRIG_INT | HSU_RX_TIMEOUT_INT)) != 0)
  {
    /* RX interrupt, needs servicing */
    if (phsuartcfg->cbs.rxcb != NULL)
    {
      phsuartcfg->cbs.rxcb(&phsuartcfg);
    }
    else
    {
      /* No callback, disable interrupt */
      phsuartcfg->regptr->ctrl &= ~HSU_RX_INT_EN;
    }
  }

  if ((tmp & HSU_TX_INT) != 0)
  {
    /* TX interrupt, needs servicing */
    if (phsuartcfg->cbs.txcb != NULL)
    {
      phsuartcfg->cbs.txcb(&phsuartcfg);
    }
    else
    {
      /* No callback, disable interrupt */
      phsuartcfg->regptr->ctrl &= ~HSU_TX_INT_EN;
    }
  }

  if ((tmp & (HSU_RX_OE_INT | HSU_BRK_INT | HSU_FE_INT)) != 0)
  {
    /* Error interrupt, needs servicing */
    if (phsuartcfg->cbs.rxerrcb != NULL)
    {
      phsuartcfg->cbs.rxerrcb(&phsuartcfg);
    }
    else
    {
      /* No callback, disable interrupt */
      phsuartcfg->regptr->ctrl &= ~HSU_ERR_INT_EN;
    }
  }

  /* Clear pending */
  phsuartcfg->regptr->iir = tmp;
}

/***********************************************************************
 *
 * Function: uart1_int_handler
 *
 * Purpose: UART1 interrupt handler and router
 *
 * Processing:
 *     Handles UART 1 interrupt by routing the to general handler with
 *     the UART 1 driver data.
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
void uart1_int_handler(void)
{
  hsuart_gen_int_handler(&hsuartdat[0]);
}

/***********************************************************************
 *
 * Function: uart2_int_handler
 *
 * Purpose: UART2 interrupt handler and router
 *
 * Processing:
 *     Handles UART 2 interrupt by routing the to general handler with
 *     the UART 2 driver data.
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
void uart2_int_handler(void)
{
  hsuart_gen_int_handler(&hsuartdat[1]);
}

/***********************************************************************
 *
 * Function: uart7_int_handler
 *
 * Purpose: UART7 interrupt handler and router
 *
 * Processing:
 *     Handles UART 7 interrupt by routing the to general handler with
 *     the UART 7 driver data.
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
void uart7_int_handler(void)
{
  hsuart_gen_int_handler(&hsuartdat[2]);
}

/***********************************************************************
 *
 * Function: hsuart_abs
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
INT_32 hsuart_abs(INT_32 v1, INT_32 v2)
{
  if (v1 > v2)
  {
    return v1 - v2;
  }

  return v2 - v1;
}

/***********************************************************************
 *
 * Function: hsuart_ptr_to_hsuart_num
 *
 * Purpose: Convert a HS UART register pointer to a HS UART number
 *
 * Processing:
 *     Based on the passed HS UART address, return the HS UART number.
 *
 * Parameters:
 *     puart : Pointer to a HS UART register set
 *
 * Outputs: None
 *
 * Returns: The HS UART number (0 to 2) or -1 if register pointer is bad
 *
 **********************************************************************/
INT_32 hsuart_ptr_to_hsuart_num(HSUART_REGS_T *phsuart)
{
  INT_32 hsuartnum = -1;

  if (phsuart == UART1)
  {
    hsuartnum = 0; /* UART 1 */
  }
  else if (phsuart == UART2)
  {
    hsuartnum = 1; /* UART 2 */
  }
  else if (phsuart == UART7)
  {
    hsuartnum = 2; /* UART 7 */
  }

  return hsuartnum;
}

/***********************************************************************
 *
 * Function: hsuart_flush_fifos
 *
 * Purpose: Flushes one or both of the HS UART FIFOs
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pregs     : Pointer to a HS UART register base
 *     flushword : Masked flush value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void hsuart_flush_fifos(HSUART_REGS_T *pregs,
                        UNS_32 flushword)
{
  UNS_8 dummy;

  (void) dummy;

  while ((pregs->level & 0xFF00) != 0);
  while ((pregs->level & 0xFF) != 0)
  {
    dummy =	pregs->txrx_fifo;
  }
}

/***********************************************************************
 *
 * Function: hsuart_find_clk
 *
 * Purpose: Determines best divider to get a target clock rate
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     hsuartnum : HSUART number (0..2) for HS UARTS (1, 2, 7)
 *     freq      : Desired HS UART baud rate
 *     div       : Structure to place dividers to get rate into
 *
 * Outputs: None
 *
 * Returns: Actual HS UART baud rate
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 hsuart_find_clk(UNS_32 freq,
                       HSUART_CFG_T *phsuartcfg)
{
  UNS_32 clkrate, savedclkrate, diff, basepclk;
  INT_32 idiv, divider;

  /* Get the clock rate for the UART block */
  basepclk = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);

  /* Find the best divider */
  divider = 0;
  savedclkrate = 0;
  diff = 0xFFFFFFFF;
  for (idiv = 0; idiv < 0x100; idiv++)
  {
    clkrate = basepclk / (14 * (idiv + 1));
    if (hsuart_abs(clkrate, freq) < diff)
    {
      diff = hsuart_abs(clkrate, freq);
      savedclkrate = clkrate;
      divider = idiv;
    }
  }

  /* Saved computed divider */
  phsuartcfg->divider = divider;

  return savedclkrate;
}

/***********************************************************************
 *
 * Function: hsuart_setup_trans_mode
 *
 * Purpose: Sets up a HS UART data transfer mode
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     puartcfg   : Pointer to HS UART configuration data
 *     puartsetup : Pointer to a HS UART transfer mode setup structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if setup was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS hsuart_setup_trans_mode(HSUART_CFG_T *phsuartcfg,
                               HSUART_CONTROL_T *phsuartsetup)
{
  STATUS err = _NO_ERROR;

  /* Find closest baud rate for desired clock frequency */
  phsuartcfg->baudrate = hsuart_find_clk(phsuartsetup->baud_rate, phsuartcfg);

  /* Set clock divider for the HS UART */
  switch (phsuartcfg->hsuartnum)
  {
    case 0:
      UART1->rate = phsuartcfg->divider;
      break;

    case 1:
      UART2->rate = phsuartcfg->divider;
      break;

    case 2:
      UART7->rate = phsuartcfg->divider;
      break;

    default:
      err = _ERROR;
      break;
  }

  if (err == _NO_ERROR)
  {
    /* Set CTS control */
    if (phsuartsetup->cts_en)
    {
      phsuartcfg->regptr->ctrl |=	HSU_HCTS_EN;
      /* CTS inversion control */
      if (phsuartsetup->cts_inv)
      {
        phsuartcfg->regptr->ctrl |=	HSU_HCTS_INV;
      }
      else
      {
        phsuartcfg->regptr->ctrl &=	~HSU_HCTS_INV;
      }
    }
    else
    {
      phsuartcfg->regptr->ctrl &=	~HSU_HCTS_EN;
    }

    /* Set RTS control */
    if (phsuartsetup->rts_en)
    {
      phsuartcfg->regptr->ctrl |=	HSU_HRTS_EN;
      /* RTS inversion control */
      if (phsuartsetup->rts_inv)
      {
        phsuartcfg->regptr->ctrl |=	HSU_HRTS_INV;
      }
      else
      {
        phsuartcfg->regptr->ctrl &=	~HSU_HRTS_INV;
      }
    }
    else
    {
      phsuartcfg->regptr->ctrl &=	~HSU_HRTS_EN;
    }
  }

  return err;
}

/***********************************************************************
 * HS UART driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: hsuart_open
 *
 * Purpose: Open the HS UART
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a HS UART peripheral block
 *     arg   : Pointer to HS UART setup structure or NULL
 *
 * Outputs: None
 *
 * Returns: The pointer to a HS UART config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 hsuart_open(void *ipbase,
                   INT_32 arg)
{
  HSUART_CONTROL_T hsuarttran;
  INT_32 hsuartnum, tptr = (INT_32) NULL;
  HSUART_CONTROL_T *pucfg = (HSUART_CONTROL_T *) arg;

  /* Try to find a matching HSUART number based on the passed pointer */
  hsuartnum = hsuart_ptr_to_hsuart_num((HSUART_REGS_T *) ipbase);
  if (hsuartnum >= 0)
  {
    /* Has the HS UART been previously initialized? */
    if (hsuartdat[hsuartnum].hsuart_init == FALSE)
    {
      /* HS UART is free */
      hsuartdat[hsuartnum].hsuart_init = TRUE;
      hsuartdat[hsuartnum].hsuartnum = hsuartnum;
      hsuartdat[hsuartnum].regptr = (HSUART_REGS_T *) ipbase;

      /* No callbacks by default */
      hsuartdat[hsuartnum].cbs.rxcb = NULL;
      hsuartdat[hsuartnum].cbs.txcb = NULL;
      hsuartdat[hsuartnum].cbs.rxerrcb = NULL;

      /* Install general interrupt handler */
      switch (hsuartnum)
      {
        case 0:
          int_install_irq_handler(IRQ_UART_IIR1,
                                  uart1_int_handler);
          break;
        case 1:
          int_install_irq_handler(IRQ_UART_IIR2,
                                  uart2_int_handler);
          break;

        case 2:
          int_install_irq_handler(IRQ_UART_IIR7,
                                  uart7_int_handler);
          break;
      }

      /* Setup default UART state for 9600 */
      if (pucfg == NULL)
      {
        hsuarttran.baud_rate = 9600;
        hsuarttran.cts_en = FALSE;
        hsuarttran.rts_en = FALSE;
        hsuart_setup_trans_mode(&hsuartdat[hsuartnum], &hsuarttran);
      }
      else
      {
        hsuart_setup_trans_mode(&hsuartdat[hsuartnum], pucfg);
      }

      /* Set FIFO level, and clear pending interrupts */
      hsuartdat[hsuartnum].regptr->ctrl = (HSU_HRTS_TRIG_32B | HSU_TMO_INACT_16B |
                                           HSU_OFFSET(0x14) | HSU_RX_TL32B | HSU_TX_TL0B);

      /* Receive, transmit and status interrupts enabled */
      hsuartdat[hsuartnum].regptr->ctrl |= HSU_RX_INT_EN | HSU_TX_INT_EN | HSU_ERR_INT_EN;

      /* Return pointer to specific UART structure */
      tptr = (INT_32) & hsuartdat[hsuartnum];
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: hsuart_close
 *
 * Purpose: Close the HS UART
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the UART,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to HS UART config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS hsuart_close(INT_32 devid)
{
  HSUART_CFG_T *phsuart = (HSUART_CFG_T *) devid;
  STATUS status = _NO_ERROR;

  /* Close and disable device if it was previously initialized */
  if (phsuart->hsuart_init == TRUE)
  {
    /* Disable interrupts */
    phsuart->regptr->ctrl &= ~(HSU_ERR_INT_EN | HSU_RX_INT_EN | HSU_TX_INT_EN);

    /* Free UART and Disable timer system clock */
    phsuart->hsuart_init = FALSE;
  }
  else
  {
    status = _ERROR;
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
STATUS hsuart_ioctl(INT_32 devid,
                    INT_32 cmd,
                    INT_32 arg)
{
  HSUART_CBS_T *pcbs;
  HSUART_CFG_T *phsuart = (HSUART_CFG_T *) devid;
  STATUS status = _ERROR;
  UNS_8 dummy;

  (void) dummy;

  /* Close and disable device if it was previously initialized */
  if (phsuart->hsuart_init == TRUE)
  {
    status = _NO_ERROR;

    switch (cmd)
    {
      case HSUART_CLEAR_FIFOS:
        while ((phsuart->regptr->level & 0xFF00) != 0);
        while ((phsuart->regptr->level & 0xFF) != 0)
        {
          dummy =	phsuart->regptr->txrx_fifo;
        }
        break;

      case HSUART_SETUP_TRANSFER:
        hsuart_setup_trans_mode(phsuart, (HSUART_CONTROL_T *) arg);
        break;

      case HSUART_LOOPBACK_EN:
        if (arg != 0)
        {
          UARTCNTL->loop |=
            UART_LPBACK_ENABLED(phsuart->hsuartnum + 1);
        }
        else
        {
          UARTCNTL->loop &=
            ~UART_LPBACK_ENABLED(phsuart->hsuartnum + 1);
        }
        break;

      case HSUART_FORCE_BREAK:
        if (arg != 0)
        {
          phsuart->regptr->ctrl |= HSU_BREAK;
        }
        else
        {
          phsuart->regptr->ctrl &= ~HSU_BREAK;
        }
        break;

      case HSUART_INSTALL_CBS:
        pcbs = (HSUART_CBS_T *) arg;
        phsuart->cbs.rxcb = pcbs->rxcb;
        phsuart->cbs.txcb = pcbs->txcb;
        phsuart->cbs.rxerrcb = pcbs->rxerrcb;
        break;

      case HSUART_GET_STATUS:
        /* Return a UART status */
        switch (arg)
        {
          case HSUART_GET_IP_CLOCK:
            status = clkpwr_get_base_clock_rate(
                       CLKPWR_PERIPH_CLK);
            break;

          case HSUART_GET_DERIVED_CLOCK:
            status = phsuart->baudrate;
            break;

          case HSUART_GET_LINE_STATUS:
            status = (STATUS) phsuart->regptr->iir;
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
INT_32 hsuart_read(INT_32 devid,
                   void *buffer,
                   INT_32 max_bytes)
{
  INT_32 bread = 0;
  HSUART_CFG_T *phsuart = (HSUART_CFG_T *) devid;
  HSUART_REGS_T *pregs = phsuart->regptr;
  UNS_8 *buff8 = (UNS_8 *) buffer;

  while ((max_bytes > 0) && ((pregs->level & 0xFF) != 0))
  {
    *buff8 = (UNS_8) pregs->txrx_fifo;
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
INT_32 hsuart_write(INT_32 devid,
                    void *buffer,
                    INT_32 n_bytes)
{
  INT_32 bwrite = 0;
  HSUART_CFG_T *phsuart = (HSUART_CFG_T *) devid;
  HSUART_REGS_T *pregs = phsuart->regptr;
  UNS_8 *buff8 = (UNS_8 *) buffer;

  /* Only add data if the current FIFO level can be determined */
  if (((pregs->level) >> 8) == 0)
  {
    while ((n_bytes > 0) && (((pregs->level) >> 8) == 0))
    {
      pregs->txrx_fifo = (UNS_32) * buff8;
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
      pregs->txrx_fifo = (UNS_32) * buff8;
      buff8++;
      n_bytes--;
      bwrite++;
    }
  }

  return bwrite;
}
