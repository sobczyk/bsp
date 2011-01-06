/***********************************************************************
 * $Id:: lpc32xx_kscan_driver.c 931 2008-07-24 18:06:41Z wellsk        $
 *
 * Project: LPC32xx Keyboard scanner driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx Keyboard
 *     scanner controller.
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

#include "lpc32xx_kscan_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * Keyboard scanner driver package data
***********************************************************************/

/* Keyboard scanner device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  KSCAN_REGS_T *regptr;   /* Pointer to Keyboard scanner registers */
} KSCAN_CFG_T;

/* Keyboard scanner driver data */
static KSCAN_CFG_T kscandat;

/***********************************************************************
 * Keyboard scanner driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: kscan_open
 *
 * Purpose: Open the Keyboard scanner
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a Keyboard scanner peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a Keyboard scanner config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 kscan_open(void *ipbase,
                  INT_32 arg)
{
  INT_32 tptr = (INT_32) NULL;

  if ((KSCAN_REGS_T *) ipbase == KSCAN)
  {
    /* Has SLC NAND driver been previously initialized? */
    if (kscandat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
         used */
      kscandat.init = TRUE;

      /* Save address of register block */
      kscandat.regptr = KSCAN;

      /* Enable clock to key scanner block */
      clkpwr_clk_en_dis(CLKPWR_KEYSCAN_CLK, 1);

      /* Keypad scanner starts with the 32KHz clock selected,
         normal scan mode, maximum debounce cycles, maximum
         clocks between each state, and 1x1 matrix, clear
         interrupt */
      kscandat.regptr->ks_deb = KSCAN_DEB_NUM_DEB_PASS(0xFF);
      kscandat.regptr->ks_scan_ctl = KSCAN_SCTRL_SCAN_DELAY(0xFF);
      kscandat.regptr->ks_fast_tst = KSCAN_FTST_USE32K_CLK;
      kscandat.regptr->ks_matrix_dim = KSCAN_MSEL_SELECT(1);
      kscandat.regptr->ks_irq = KSCAN_IRQ_PENDING_CLR;

      /* Use the RTC clock as the default keypad scanner clock
         source */
      kscandat.regptr->ks_fast_tst |= KSCAN_FTST_USE32K_CLK;

      tptr = (INT_32) & kscandat;
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: kscan_close
 *
 * Purpose: Close the Keypad scanner
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to Keypad scanner config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS kscan_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((KSCAN_CFG_T *) devid == &kscandat)
  {
    if (kscandat.init == TRUE)
    {
      /* Clear pendnig IRQ */
      kscandat.regptr->ks_irq = KSCAN_IRQ_PENDING_CLR;

      /* Disable keypad scanner clock to block */
      kscandat.regptr->ks_fast_tst &= ~KSCAN_FTST_USE32K_CLK;
      clkpwr_clk_en_dis(CLKPWR_KEYSCAN_CLK, 0);

      /* Set device as uninitialized */
      kscandat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: kscan_ioctl
 *
 * Purpose: Keyboard scanner configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to Keyboard scanner config structure
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
STATUS kscan_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg)
{
  KSCAN_SETUP_T *pksctp;
  INT_32 status = _ERROR;

  if ((KSCAN_CFG_T *) devid == &kscandat)
  {
    if (kscandat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case KSCAN_SETUP:
          pksctp = (KSCAN_SETUP_T *) arg;
          if (pksctp->matrix_size <= 8)
          {
            if (pksctp->deb_clks < 2)
            {
              pksctp->deb_clks = 2;
            }
            kscandat.regptr->ks_deb =
              KSCAN_DEB_NUM_DEB_PASS(pksctp->deb_clks);
            kscandat.regptr->ks_scan_ctl =
              KSCAN_SCTRL_SCAN_DELAY(
                pksctp->scan_delay_clks);
            kscandat.regptr->ks_matrix_dim =
              KSCAN_MSEL_SELECT(pksctp->matrix_size);
            if (pksctp->pclk_sel == 0)
            {
              kscandat.regptr->ks_fast_tst &=
                ~KSCAN_FTST_USE32K_CLK;
            }
            else
            {
              kscandat.regptr->ks_fast_tst |=
                KSCAN_FTST_USE32K_CLK;
            }
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case KSCAN_CLEAR_INT:
          kscandat.regptr->ks_irq = KSCAN_IRQ_PENDING_CLR;
          break;

        case KSCAN_SCAN_ONCE:
          if (arg != 0)
          {
            kscandat.regptr->ks_fast_tst |=
              KSCAN_FTST_FORCESCANONCE;
          }
          else
          {
            kscandat.regptr->ks_fast_tst &=
              ~KSCAN_FTST_FORCESCANONCE;
          }
          break;

        case KSCAN_GET_STATUS:
          /* Return a KSCAN status */
          switch (arg)
          {
            case KSCAN_GET_CLOCK:
              if ((kscandat.regptr->ks_fast_tst &
                   KSCAN_FTST_USE32K_CLK) == 0)
              {
                /* Using peripheral clock */
                status = clkpwr_get_base_clock_rate(
                           CLKPWR_PERIPH_CLK);
              }
              else
              {
                status = CLOCK_OSC_FREQ;
              }
              break;

            case KSCAN_IRQ_PENDING:
              if ((kscandat.regptr->ks_irq &
                   KSCAN_IRQ_PENDING_CLR) != 0)
              {
                status = 1;
              }
              else
              {
                status = 0;
              }
              break;

            case KSCAN_GET_STATE:
              status = (INT_32)
                       kscandat.regptr->ks_state_cond;
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
  }

  return status;
}

/***********************************************************************
 *
 * Function: kscan_read
 *
 * Purpose: Keyboard scanner read function
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     devid:     Pointer to Keyboard scanner descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes read, or _NO_ERROR on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 kscan_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes)
{
  INT_32 status = _NO_ERROR;
  UNS_8 *buff8 = (UNS_8 *) buffer;
  INT_32 idx;

  if ((KSCAN_CFG_T *) devid == &kscandat)
  {
    if (kscandat.init == TRUE)
    {
      status = _NO_ERROR;

      /* Limit size to 8 entries, the size of the keypad data */
      if (max_bytes > 8)
      {
        max_bytes = 8;
      }

      /* Read data */
      for (idx = 0; idx < max_bytes; idx++)
      {
        *buff8 = (UNS_8) kscandat.regptr->ks_data[idx];
        buff8++;
      }

      status = max_bytes;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: kscan_write
 *
 * Purpose: Keyboard scanner write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to Keyboard scanner descriptor
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
INT_32 kscan_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes)
{
  return 0;
}
