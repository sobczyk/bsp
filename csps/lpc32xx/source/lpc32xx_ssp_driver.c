/***********************************************************************
 * $Id:: lpc32xx_ssp_driver.c 1092 2008-08-18 22:18:00Z wellsk         $
 *
 * Project: LPC32XX SSP driver
 *
 * Description:
 *     This file contains driver support for the SSP module on the
 *     LPC32XX
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

#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_ssp_driver.h"

/***********************************************************************
 * SSP driver private data and types
 **********************************************************************/

/* SSP device configuration structure type */
typedef struct
{
  BOOL_32 init;          /* Device initialized flag */
  SSP_REGS_T *regptr;    /* Pointer to SSP registers */
  SSP_CBS_T cbs;         /* Interrupt callbacks */
  INT_32 thisdev;        /* 0 or 1, SSP number */
  INT_32 dsize;          /* Size of data (in bytes) */
} SSP_DRVDAT_T;

/* SSP device configuration structure */
static SSP_DRVDAT_T sspdrv [2];

/* Clocks for each SSP */
static const CLKPWR_CLK_T sspclks [2] =
  {CLKPWR_SSP0_CLK, CLKPWR_SSP1_CLK};

/***********************************************************************
 * SSP driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ssp_set_clock
 *
 * Purpose: Sets or resets the serial clock rate of the SSP interface
 *          (in Hz)
 *
 * Processing:
 *     Determine the best dividers to generate the closest possible
 *     target clock rate for the SSP.
 *
 * Parameters:
 *     psspdrvdat   : Pointer to driver data
 *     target_clock : The value in Hz for the new SSP serial clock
 *
 * Outputs: None
 *
 * Returns: _ERROR if the configuration setup failed, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS ssp_set_clock(SSP_DRVDAT_T *psspdrvdat,
                            UNS_32 target_clock)
{
  UNS_32 control, prescale, cr0_div, cmp_clk, ssp_clk;

  /* The SSP clock is derived from the (main system oscillator / 2),
     so compute the best divider from that clock */
  ssp_clk = clkpwr_get_clock_rate(sspclks [psspdrvdat->thisdev]);

  /* Find closest divider to get at or under the target frequency.
     Use smallest prescaler possible and rely on the divider to get
     the closest target frequency */
  cr0_div = 0;
  cmp_clk = 0xFFFFFFFF;
  prescale = 2;
  while (cmp_clk > target_clock)
  {
    cmp_clk = ssp_clk / ((cr0_div + 1) * prescale);
    if (cmp_clk > target_clock)
    {
      cr0_div++;
      if (cr0_div > 0xFF)
      {
        cr0_div = 0;
        prescale += 2;
      }
    }
  }

  /* Write computed prescaler and divider back to register */
  control = psspdrvdat->regptr->cr0 &= ~(SSP_CR0_SCR(0xFF));
  psspdrvdat->regptr->cr0 = control | SSP_CR0_SCR(cr0_div - 1);
  psspdrvdat->regptr->cpsr = prescale;

  return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: ssp_configure
 *
 * Purpose: Configure SSP interface
 *
 * Processing:
 *     Setup the general capabilities of the SSP controller.
 *
 * Parameters:
 *     psspcfg : Pointer to an SSP_CONFIG_T structure
 *     psspdrvdat: Pointer to driver data
 *
 * Outputs: None
 *
 * Returns:
 *     _ERROR if the configuration setup failed, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS ssp_configure(SSP_CONFIG_T *psspcfg,
                            SSP_DRVDAT_T *psspdrvdat)
{
  UNS_32 tmp0, tmp1;
  STATUS setup = _NO_ERROR;
  SSP_REGS_T *psspregs = psspdrvdat->regptr;

  /* Setup CR0 word first */
  tmp0 = 0;
  if ((psspcfg->databits >= 4) && (psspcfg->databits <= 16))
  {
    tmp0 = SSP_CR0_DSS(psspcfg->databits);
  }
  else
  {
    setup = _ERROR;
  }
  if (psspcfg->databits <= 8)
  {
    psspdrvdat->dsize = 1;
  }
  else
  {
    psspdrvdat->dsize = 2;
  }

  /* Mode */
  if (((psspcfg->mode & ~SSP_CR0_PRT_MSK) != 0) ||
	  (psspcfg->mode == (0x3 << 4)))
  {
    setup = _ERROR;
  }
  tmp0 |= psspcfg->mode;

  /* SPI clock control */
  if (psspcfg->highclk_spi_frames == TRUE)
  {
    tmp0 |= SSP_CR0_CPOL(1);
  }
  if (psspcfg->usesecond_clk_spi == TRUE)
  {
    tmp0 |= SSP_CR0_CPHA(1);
  }

  /* Master/slave mode control */
  tmp1 = 0;
  if (psspcfg->master_mode == FALSE)
  {
    tmp1 = SSP_CR1_MS;
  }

  /* Setup clock */
  if (setup == _NO_ERROR)
  {
    psspregs->cr0 = tmp0;
    psspregs->cr1 = tmp1;
    setup = ssp_set_clock(psspdrvdat, psspcfg->ssp_clk);
  }

  return setup;
}

/***********************************************************************
 *
 * Function: ssp_standard_interrupt
 *
 * Purpose: SSP standard interrupt function
 *
 * Processing:
 *     Handle the SSP interrupt from SSP0 or SSP1. Route to the
 *     necessary callback function as needed. Disable interrupt if a
 *     callback is not associated with it.
 *
 * Parameters:
 *     psspdrvdat : Pointer to an SSP driver data
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void ssp_standard_interrupt(SSP_DRVDAT_T *psspdrvdat)
{
  SSP_REGS_T *psspregs = psspdrvdat->regptr;

  /* Interrupt was due to a receive data FIFO service request */
  if ((psspregs->mis &
       (SSP_RIS_RTRIS | SSP_MIS_RXMIS | SSP_MIS_RORMIS)) != 0)
  {
    if (psspdrvdat->cbs.rxcb == NULL)
    {
      /* Disable interrupt, no support for it */
      psspregs->imsc &= ~(SSP_IMSC_RTIM | SSP_IMSC_RXIM);
    }
    else
    {
      /* Handle callback */
      psspdrvdat->cbs.rxcb();
      psspregs->icr = (SSP_ICR_RORIC | SSP_ICR_RTIC);
    }
  }

  /* Interrupt was due to a transmit data FIFO service request */
  if ((psspregs->mis & SSP_MIS_TXMIS) != 0)
  {
    if (psspdrvdat->cbs.txcb == NULL)
    {
      /* Disable interrupt, no support for it */
      psspregs->imsc &= ~SSP_IMSC_TXIM;
    }
    else
    {
      /* Handle callback */
      psspdrvdat->cbs.txcb();
    }
  }
}

/***********************************************************************
 * SSP driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ssp_open
 *
 * Purpose: Open the SSP
 *
 * Processing:
 *     Initializes the SSP clocks and default state.
 *
 * Parameters:
 *     ipbase: SSP descriptor device address
 *     arg   : Pointer to config structure, or NULL if not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a SSP config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_open(void *ipbase,
                INT_32 arg)
{
  SSP_CONFIG_T ssp_cfg, *psspcfg;
  volatile UNS_32 tmp;
  SSP_REGS_T *psspregs = (SSP_REGS_T *) ipbase;
  SSP_DRVDAT_T *psspdrvdat = NULL;
  INT_32 status = 0;

  /* Map SSP registers to data index */
  if (psspregs == SSP0)
  {
    psspdrvdat = (SSP_DRVDAT_T *) & sspdrv [0];
    psspdrvdat->thisdev = 0;
  }
  else if (psspregs == SSP1)
  {
    psspdrvdat = (SSP_DRVDAT_T *) & sspdrv [1];
    psspdrvdat->thisdev = 1;
  }

  if (psspdrvdat != NULL)
  {
    if (psspdrvdat->init == FALSE)
    {
      /* Save and return address of peripheral block */
      psspdrvdat->regptr = (SSP_REGS_T *) ipbase;

      /* Enable SSP clock */
      clkpwr_clk_en_dis(sspclks[psspdrvdat->thisdev], 1);

      /* No initial callbacks */
      psspdrvdat->cbs.txcb = NULL;
      psspdrvdat->cbs.rxcb = NULL;

      /* Initialize device */
      if (arg == 0)
      {
        /* Create and use defaults */
        ssp_cfg.databits = 8;
        ssp_cfg.mode = SSP_CR0_FRF_SPI;
        ssp_cfg.highclk_spi_frames = TRUE;
        ssp_cfg.usesecond_clk_spi = FALSE;
        ssp_cfg.ssp_clk = 1000000;
        ssp_cfg.master_mode = TRUE;
        psspcfg = &ssp_cfg;
      }
      else
      {
        psspcfg = (SSP_CONFIG_T *) arg;
      }
      if (ssp_configure(psspcfg, psspdrvdat) != _ERROR)
      {
        /* Device is valid */
        psspdrvdat->init = TRUE;
        status = (INT_32) psspdrvdat;
      }
      else
      {
        clkpwr_clk_en_dis(sspclks[psspdrvdat->thisdev], 0);
      }

      /* Empty FIFO */
      while ((psspdrvdat->regptr->sr & SSP_SR_RNE) != 0)
      {
        tmp = psspdrvdat->regptr->data;
      }

      /* Clear latched interrupts */
      psspdrvdat->regptr->icr = (SSP_ICR_RORIC | SSP_ICR_RTIC);

      /* Enable interrupts */
      psspdrvdat->regptr->imsc = (SSP_IMSC_RORIM |
                                  SSP_IMSC_RTIM | SSP_IMSC_RXIM);
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: ssp_close
 *
 * Purpose: Close the SSP
 *
 * Processing:
 *     Disable the SSP clock and device.
 *
 * Parameters:
 *     devid: Pointer to SSP config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS ssp_close(INT_32 devid)
{
  SSP_DRVDAT_T *sspdrvdat = (SSP_DRVDAT_T *) devid;
  STATUS status = _ERROR;

  if (sspdrvdat->init == TRUE)
  {
    /* 'Uninitialize' device */
    sspdrvdat->init = FALSE;
    status = _NO_ERROR;

    /* Disable device */
    sspdrvdat->regptr->cr1 &= ~SSP_CR1_SSP_ENABLE;

    /* Disable clock */
    clkpwr_clk_en_dis(sspclks[sspdrvdat->thisdev], 0);
  }

  return status;
}

/***********************************************************************
 *
 * Function: ssp_ioctl
 *
 * Purpose: SSP configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate SSP parameter.
 *
 * Parameters:
 *     devid: Pointer to SSP config structure
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
STATUS ssp_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  SSP_REGS_T *sspregs;
  SSP_CBS_T *psspcb;
  UNS_32 sspclk, tmp, tmp2;
  SSP_DRVDAT_T *sspdrvdat = (SSP_DRVDAT_T *) devid;
  STATUS status = _ERROR;

  if (sspdrvdat->init == TRUE)
  {
    status = _NO_ERROR;
    sspregs = sspdrvdat->regptr;

    switch (cmd)
    {
      case SSP_ENABLE:
        if (arg == 1)
        {
          /* Enable SSP */
          sspregs->cr1 |= SSP_CR1_SSP_ENABLE;
        }
        else
        {
          /* Disable SSP */
          sspregs->cr1 &= ~SSP_CR1_SSP_ENABLE;
        }
        break;

      case SSP_CONFIG:
        status = ssp_configure((SSP_CONFIG_T *) arg,
                               sspdrvdat);
        break;

      case SSP_ENABLE_LOOPB:
        /* Enable or disable loopback mode */
        if (arg == 1)
        {
          /* Enable SSP loopback mode */
          sspregs->cr1 |= SSP_CR1_LBM;
        }
        else
        {
          /* Disable SSP loopback mode */
          sspregs->cr1 &= ~SSP_CR1_LBM;
        }
        break;

      case SSP_SO_DISABLE:
        /* Slave output disable */
        if (arg != 0)
        {
          sspregs->cr1 |= SSP_CR1_SOD;
        }
        else
        {
          sspregs->cr1 &= ~SSP_CR1_SOD;
        }
        break;

      case SSP_SET_CALLBACKS:
        psspcb = (SSP_CBS_T *) arg;
        sspdrvdat->cbs.txcb = psspcb->txcb;
        sspdrvdat->cbs.rxcb = psspcb->rxcb;
        break;

      case SSP_CLEAR_INTS:
        sspregs->icr = ((UNS_32) arg) &
                       (SSP_ICR_RORIC | SSP_ICR_RTIC);
        break;

      case SSP_GET_STATUS:
        /* Return an SSP status */
        switch (arg)
        {
          case SSP_CLOCK_ST:
            /* Return clock speed of SSP interface */
            tmp = (sspregs->cr0 & SSP_CR0_SCR(0xFF)) >> 8;
            tmp2 = sspregs->cpsr;
            if (tmp2 < 1)
            {
              /* Not a valid value, so use a divider of 1 */
              tmp2 = 1;
            }

            /* Compute SSP bit clock rate */
            sspclk = clkpwr_get_clock_rate(
                       sspclks [sspdrvdat->thisdev]);
            status = sspclk / (tmp2 * (tmp + 1));
            break;

          case SSP_PENDING_INTS_ST:
            status = sspregs->mis;
            break;

          case SSP_RAW_INTS_ST:
            status = sspregs->ris;
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
 * Function: ssp_read
 *
 * Purpose: SSP read function
 *
 * Processing:
 *     Reads data from the SSP FIFO.
 *
 * Parameters:
 *     devid:     Pointer to SSP config structure
 *     buffer:    Pointer to data buffer to copy to (2 byte aligned)
 *     max_fifo:  Number of items (of programmed data width) to read
 *
 * Outputs: None
 *
 * Returns: Number of items read from the SSP FIFO
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_read(INT_32 devid,
                void *buffer,
                INT_32 max_fifo)
{
  volatile UNS_32 tmp1;
  INT_32 count = 0;
  SSP_DRVDAT_T *sspcfgptr = (SSP_DRVDAT_T *) devid;
  UNS_16 *data16 = (UNS_16 *) buffer;
  UNS_8 *data8 = (UNS_8 *) buffer;

  if (sspcfgptr->init == TRUE)
  {
    while ((max_fifo > 0) &&
           ((sspcfgptr->regptr->sr & SSP_SR_RNE) != 0))
    {
      tmp1 = sspcfgptr->regptr->data;
      if (sspcfgptr->dsize == 1)
      {
        *data8 = (UNS_8) tmp1;
        data8++;
      }
      else
      {
        *data16 = (UNS_16) tmp1;
        data16++;
      }

      /* Increment data count and decrement buffer size count */
      count++;
      max_fifo--;
    }
  }

  return count;
}

/***********************************************************************
 *
 * Function: ssp_write
 *
 * Purpose: SSP write function
 *
 * Processing:
 *     Write data to the SSP FIFO.
 *
 * Parameters:
 *     devid:   Pointer to SSP config structure
 *     buffer:  Pointer to data buffer to copy from (2 byte aligned)
 *     n_fifo:  Number of times to write data to the transmit fifo
 *
 * Outputs: None
 *
 * Returns: Number of items written to the transmit fifo
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_fifo)
{
  INT_32 count = 0;
  SSP_DRVDAT_T *sspcfgptr = (SSP_DRVDAT_T *) devid;
  UNS_16 *data16 = (UNS_16 *) buffer;
  UNS_8 *data8 = (UNS_8 *) buffer;

  if (sspcfgptr->init == TRUE)
  {
    /* Loop until transmit ring buffer is full or until n_bytes
       expires */
    while ((n_fifo > 0) &&
           ((sspcfgptr->regptr->sr & SSP_SR_TNF) != 0))
    {
      if (sspcfgptr->dsize == 1)
      {
        sspcfgptr->regptr->data = (UNS_32) * data8;
        data8++;
      }
      else
      {
        sspcfgptr->regptr->data = (UNS_32) * data16;
        data16++;
      }

      /* Increment data count and decrement buffer size count */
      count++;
      n_fifo--;
    }

    /* Enable transmit interrupt */
    if (count > 0)
    {
      sspcfgptr->regptr->imsc |= SSP_IMSC_TXIM;
    }
  }

  return count;
}

/***********************************************************************
 *
 * Function: ssp0_int
 *
 * Purpose: SSP0 interrupt handler
 *
 * Processing:
 *     Handle the SSP0 interrupt.
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
void ssp0_int(void)
{
  ssp_standard_interrupt(&sspdrv [0]);
}

/***********************************************************************
 *
 * Function: ssp1_int
 *
 * Purpose: SSP1 interrupt handler
 *
 * Processing:
 *     Handle the SSP1 interrupt.
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
void ssp1_int(void)
{
  ssp_standard_interrupt(&sspdrv [1]);
}
