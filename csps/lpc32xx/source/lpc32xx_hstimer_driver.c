/***********************************************************************
 * $Id:: lpc32xx_hstimer_driver.c 1022 2008-08-06 22:23:42Z wellsk     $
 *
 * Project: LPC32xx high speed timer driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx high speed
 *     timer.
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

#include "lpc32xx_hstimer_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * HSTIMER driver package data
***********************************************************************/

/* HSTIMER device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  HSTIMER_REGS_T *regptr; /* Pointer to HSTIMER registers */
} HSTIMER_CFG_T;

/* HSTIMER driver data */
static HSTIMER_CFG_T hstdat;

/***********************************************************************
 * HSTIMER driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: timer_usec_to_val
 *
 * Purpose: Convert a time to a timer count value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clknum : Timer clock ID
 *     usec   : Time in microseconds
 *
 * Outputs: None
 *
 * Returns: The number of required clock ticks to give the time delay
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 timer_usec_to_value(CLKPWR_CLK_T clknum,
                           UNS_32 usec)
{
  UNS_64 clkdlycnt;
  UNS_64 longcnt;

  /* Determine the value to exceed before the count reaches the
     desired delay time */
  longcnt = (UNS_64) clkpwr_get_clock_rate(clknum);
  clkdlycnt = (longcnt / 1000);
  clkdlycnt = (clkdlycnt * (UNS_64) usec) / 1000;

  return (UNS_32) clkdlycnt;
}

/***********************************************************************
 * HSTIMER driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: hstimer_open
 *
 * Purpose: Open the high speed timer
 *
 * Processing:
 *     If the passed register base is valid and the high speed timer
 *     driver isn't already initialized, then setup the high speed
 *     timer into a default initialized state that is disabled. Return
 *     a pointer to the driver's data structure or NULL if driver
 *     initialization failed.
 *
 * Parameters:
 *     ipbase: Pointer to a high speed timer peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a high speed timer config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 hstimer_open(void *ipbase, INT_32 arg)
{
  INT_32 tptr = (INT_32) NULL;

  if ((HSTIMER_REGS_T *) ipbase == HSTIMER)
  {
    /* Valid timer selected, has it been previously initialized? */
    if (hstdat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
         used */
      hstdat.init = TRUE;

      /* Save address of register block */
      hstdat.regptr = HSTIMER;

      /* Enable timer system clock */
      clkpwr_clk_en_dis(CLKPWR_HSTIMER_CLK, 1);

      /* Disable high speed timer and match timers */
      hstdat.regptr->hstim_ctrl = HSTIM_CTRL_RESET_COUNT;
      hstdat.regptr->hstim_mctrl = 0;
      hstdat.regptr->hstim_ctrl = 0;

      /* Clear pending interrupts */
      hstdat.regptr->hstim_int = (HSTIM_MATCH0_INT |
                                  HSTIM_MATCH1_INT | HSTIM_MATCH2_INT |
                                  HSTIM_GPI_06_INT | HSTIM_RTC_TICK_INT);

      tptr = (INT_32) & hstdat;
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: hstimer_close
 *
 * Purpose: Close the high speed timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to high speed timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS hstimer_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((HSTIMER_CFG_T *) devid == &hstdat)
  {
    if (hstdat.init == TRUE)
    {
      /* Disable timers */
      hstdat.regptr->hstim_ctrl = 0;
      hstdat.regptr->hstim_mctrl = 0;

      /* Set timer as uninitialized */
      hstdat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: hstimer_ioctl
 *
 * Purpose: High speed timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to High speed timer config structure
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
STATUS hstimer_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg)
{
  UNS_32 msk, tmp;
  HST_PSCALE_SETUP_T *ppstp;
  HST_MATCH_SETUP_T *pmatstp;
  HST_CAP_CLOCK_CTRL_T *pstpcap;
  INT_32 status = _ERROR;


  if ((HSTIMER_CFG_T *) devid == &hstdat)
  {
    if (hstdat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case HST_ENABLE:
          if (arg != 0)
          {
            hstdat.regptr->hstim_ctrl |=
              HSTIM_CTRL_COUNT_ENAB;
          }
          else
          {
            hstdat.regptr->hstim_ctrl &=
              ~HSTIM_CTRL_COUNT_ENAB;
          }
          break;

        case HST_RESET:
          hstdat.regptr->hstim_ctrl |= HSTIM_CTRL_RESET_COUNT;
          while (hstdat.regptr->hstim_counter != 0);
          hstdat.regptr->hstim_ctrl &= ~HSTIM_CTRL_RESET_COUNT;
          break;

        case HST_PAUSE:
          if (arg != 0)
          {
            hstdat.regptr->hstim_ctrl |=
              HSTIM_CTRL_PAUSE_EN;
          }
          else
          {
            hstdat.regptr->hstim_ctrl &=
              ~HSTIM_CTRL_PAUSE_EN;
          }
          break;

        case HST_CLEAR_INTS:
          hstdat.regptr->hstim_int = (UNS_32) arg;
          break;

        case HST_SETUP_PSCALE:
          ppstp = (HST_PSCALE_SETUP_T *) arg;
          if (ppstp->ps_tick_val != 0)
          {
            hstdat.regptr->hstim_pmatch = ppstp->ps_tick_val;
          }
          else
          {
            hstdat.regptr->hstim_pmatch =
              timer_usec_to_value(CLKPWR_HSTIMER_CLK, ppstp->ps_us_val);
          }
          break;

        case HST_SETUP_MATCH:
          pmatstp = (HST_MATCH_SETUP_T *) arg;

          if (pmatstp->timer_num <= 2)
          {
            /* Generate mask for match bits */
            msk = ~((HSTIM_CNTR_MCR_MTCH(pmatstp->timer_num))
                    | (HSTIM_CNTR_MCR_RESET(pmatstp->timer_num))
                    | (HSTIM_CNTR_MCR_STOP(pmatstp->timer_num)));

            /* Save timer match value */
            hstdat.regptr->hstim_match [pmatstp->timer_num] =
              pmatstp->match_tick_val;

            tmp = 0;
            if (pmatstp->use_match_int == TRUE)
            {
              tmp |=
                HSTIM_CNTR_MCR_MTCH(pmatstp->timer_num);
            }
            if (pmatstp->stop_on_match == TRUE)
            {
              tmp |=
                HSTIM_CNTR_MCR_STOP(pmatstp->timer_num);
            }
            if (pmatstp->reset_on_match == TRUE)
            {
              tmp |=
                HSTIM_CNTR_MCR_RESET(pmatstp->timer_num);
            }
            hstdat.regptr->hstim_mctrl = (hstdat.regptr->hstim_mctrl & msk) | tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case HST_SETUP_CAPTURE:
          pstpcap = (HST_CAP_CLOCK_CTRL_T *) arg;
          if (pstpcap->cap_num <= 1)
          {
            /* Generate mask for capture control bits */
            msk = ~((HSTIM_CNTR_CCR_CAPNI(pstpcap->cap_num))
                    | (HSTIM_CNTR_CCR_CAPNFE(pstpcap->cap_num))
                    | (HSTIM_CNTR_CCR_CAPNRE(pstpcap->cap_num)));

            /* Setup capture control */
            tmp = 0;
            if (pstpcap->cap_on_rising == TRUE)
            {
              tmp |= HSTIM_CNTR_CCR_CAPNRE(pstpcap->cap_num);
            }
            if (pstpcap->cap_on_falling == TRUE)
            {
              tmp |= HSTIM_CNTR_CCR_CAPNFE(pstpcap->cap_num);
            }
            if (pstpcap->use_capture_int == TRUE)
            {
              tmp |= HSTIM_CNTR_CCR_CAPNI(pstpcap->cap_num);
            }
            hstdat.regptr->hstim_ccr = (hstdat.regptr->hstim_ccr & msk) | tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;
        case HST_GET_STATUS:
          /* Return a timer status */
          switch (arg)
          {
            case HST_GET_COUNT:
              status = hstdat.regptr->hstim_counter;
              break;
            case HST_GET_PS_COUNT:
              status = hstdat.regptr->hstim_pcount;
              break;
            case HST_GET_MATCH0_REG:
              status = hstdat.regptr->hstim_match[0];
              break;
            case HST_GET_MATCH1_REG:
              status = hstdat.regptr->hstim_match[1];
              break;
            case HST_GET_MATCH2_REG:
              status = hstdat.regptr->hstim_match[2];
              break;
            case HST_GET_CR0_REG:
              status = hstdat.regptr->hstim_cap0;
              break;
            case HST_GET_CR1_REG:
              status = hstdat.regptr->hstim_cap1;
              break;
            case HST_GET_CLOCK:
              status = clkpwr_get_clock_rate(CLKPWR_HSTIMER_CLK);
              break;
            case HST_INT_PEND:
              status = hstdat.regptr->hstim_int;
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
 * Function: hstimer_read
 *
 * Purpose: High speed timer read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to High speed timer descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 hstimer_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: hstimer_write
 *
 * Purpose: High speed timer write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to High speed timer descriptor
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
INT_32 hstimer_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes)
{
  return 0;
}

