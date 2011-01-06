/***********************************************************************
 * $Id:: lpc32xx_mstimer_driver.c 1011 2008-08-06 18:18:03Z wellsk     $
 *
 * Project: LPC32xx millisecond timer driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx millisecond
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

#include "lpc32xx_mstimer_driver.h"

/***********************************************************************
 * MSTIMER driver package data
***********************************************************************/

/* MSTIMER device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  MSTIMER_REGS_T *regptr; /* Pointer to MSTIMER registers */
} MSTIMER_CFG_T;

/* MSTIMER driver data */
static MSTIMER_CFG_T mstdat;

/***********************************************************************
 * MSTIMER driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mstimer_msecs_to_ticks
 *
 * Purpose: Convert milliseconds to MSTIMER clock ticks
 *
 * Processing:
 *     Converts a millisecond integer to the closest possible value
 *     to generate the equivalent time in milliseconds in the
 *     millisecond timer clock rate.
 *
 * Parameters:
 *     msecs : Number of msecs to convert
 *
 * Outputs: None
 *
 * Returns: The number of clock ticks in msecs
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 mstimer_msecs_to_ticks(UNS_32 msecs)
{
  return ((MSTIMER_CLOCK * msecs) / 1000);
}

/***********************************************************************
 * MSTIMER driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mstimer_open
 *
 * Purpose: Open the millisecond timer
 *
 * Processing:
 *     If the passed register base is valid and the millisecond timer
 *     driver isn't already initialized, then setup the millisecond
 *     timer into a default initialized state that is disabled. Return
 *     a pointer to the driver's data structure or NULL if driver
 *     initialization failed.
 *
 * Parameters:
 *     ipbase: Pointer to a millisecond timer peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a millisecond timer config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mstimer_open(void *ipbase, INT_32 arg)
{
  INT_32 tptr = (INT_32) NULL;

  if ((MSTIMER_REGS_T *) ipbase == MSTIMER)
  {
    /* Valid timer selected, has it been previously initialized? */
    if (mstdat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
         used */
      mstdat.init = TRUE;

      /* Save address of register block */
      mstdat.regptr = MSTIMER;

      /* Disable millisecond timer and match timers */
      mstdat.regptr->mstim_ctrl = MSTIM_CTRL_RESET_COUNT;
      mstdat.regptr->mstim_mctrl = 0;
      mstdat.regptr->mstim_ctrl = 0;

      /* Clear pending interrupts */
      mstdat.regptr->mstim_int = (MSTIM_INT_MATCH0_INT |
                                  MSTIM_INT_MATCH1_INT);

      tptr = (INT_32) & mstdat;
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: mstimer_close
 *
 * Purpose: Close the millisecond timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to millisecond timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mstimer_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((MSTIMER_CFG_T *) devid == &mstdat)
  {
    if (mstdat.init == TRUE)
    {
      /* Disable timers */
      mstdat.regptr->mstim_ctrl = 0;
      mstdat.regptr->mstim_mctrl = 0;

      /* Set timer as uninitialized */
      mstdat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mstimer_ioctl
 *
 * Purpose: Millisecond timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to millisecond timer config structure
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
STATUS mstimer_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg)
{
  UNS_32 tmp, matchcnt, mtcmsk;
  MST_MATCH_SETUP_T *pmatstp = (MST_MATCH_SETUP_T *) arg;
  INT_32 status = _ERROR;

  if ((MSTIMER_CFG_T *) devid == &mstdat)
  {
    if (mstdat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case MST_TMR_ENABLE:
          if (arg != 0)
          {
            mstdat.regptr->mstim_ctrl |=
              MSTIM_CTRL_COUNT_ENAB;
          }
          else
          {
            mstdat.regptr->mstim_ctrl &=
              ~MSTIM_CTRL_COUNT_ENAB;
          }
          break;

        case MST_PAUSE_EN:
          if ((INT_32) arg != 0)
          {
            /* Enable debug mode (PAUSE_EN) */
            mstdat.regptr->mstim_ctrl |=
              MSTIM_CTRL_PAUSE_EN;
          }
          else
          {
            /* Disable debug mode (PAUSE_EN) */
            mstdat.regptr->mstim_ctrl &=
              ~MSTIM_CTRL_PAUSE_EN;
          }
          break;

        case MST_SET_VALUE:
          mstdat.regptr->mstim_counter = (UNS_32) arg;
          break;

        case MST_TMR_RESET:
          mstdat.regptr->mstim_ctrl |= MSTIM_CTRL_RESET_COUNT;
          while (mstdat.regptr->mstim_counter != 0);
          mstdat.regptr->mstim_ctrl &=
            ~MSTIM_CTRL_RESET_COUNT;
          break;

        case MST_TMR_SETUP_MATCH:
          /* Compute load value or match count */
          if (pmatstp->tick_val != 0)
          {
            matchcnt = pmatstp->tick_val;
          }
          else
          {
            /* Match count value needs to be computed */
            matchcnt =
              mstimer_msecs_to_ticks(pmatstp->ms_val);
          }

          /* Generate match setup bits */
          mtcmsk = 0;
          if (pmatstp->timer_num == 0)
          {
            /* Timer 0 match values only mask */
            mtcmsk = ~(MSTIM_MCTRL_STOP_COUNT0 |
                       MSTIM_MCTRL_RST_COUNT0 |
                       MSTIM_MCTRL_MR0_INT);

            /* Setup match control for timer 0 */
            tmp = 0;
            if (pmatstp->stop_on_match == TRUE)
            {
              tmp |= MSTIM_MCTRL_STOP_COUNT0;
            }
            if (pmatstp->reset_on_match == TRUE)
            {
              tmp |= MSTIM_MCTRL_RST_COUNT0;
            }
            if (pmatstp->use_int == TRUE)
            {
              tmp |= MSTIM_MCTRL_MR0_INT;
            }

            /* Update match control */
            mstdat.regptr->mstim_mctrl &= mtcmsk;
            mstdat.regptr->mstim_match0 = matchcnt;
            mstdat.regptr->mstim_mctrl |= tmp;
          }
          else if (pmatstp->timer_num == 1)
          {
            /* Timer 1 match values only mask */
            mtcmsk = ~(MSTIM_MCTRL_STOP_COUNT1 |
                       MSTIM_MCTRL_RST_COUNT1 |
                       MSTIM_MCTRL_MR1_INT);

            /* Setup match control for timer 1 */
            tmp = 0;
            if (pmatstp->stop_on_match == TRUE)
            {
              tmp |= MSTIM_MCTRL_STOP_COUNT1;
            }
            if (pmatstp->reset_on_match == TRUE)
            {
              tmp |= MSTIM_MCTRL_RST_COUNT1;
            }
            if (pmatstp->use_int == TRUE)
            {
              tmp |= MSTIM_MCTRL_MR1_INT;
            }

            /* Update match control */
            mstdat.regptr->mstim_mctrl &= mtcmsk;
            mstdat.regptr->mstim_match1 = matchcnt;
            mstdat.regptr->mstim_mctrl |= tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case MST_CLEAR_INT:
          if (arg == 0)
          {
            mstdat.regptr->mstim_int = MSTIM_INT_MATCH0_INT;
          }
          else if (arg == 1)
          {
            mstdat.regptr->mstim_int = MSTIM_INT_MATCH1_INT;
          }
		  else
		  {
			  status = LPC_BAD_PARAMS;
		  }
          break;

        case MST_GET_STATUS:
          /* Return a MSTIMER status */
          switch (arg)
          {
            case MST_GET_CLOCK:
              /* Returns the clock rate dricing the MSTIMER */
              status = MSTIMER_CLOCK;
              break;

            case MST_VALUE_ST:
              status = (INT_32) mstdat.regptr->mstim_counter;
              break;

            case MST_M0_VALUE_ST:
              status = (INT_32) mstdat.regptr->mstim_match0;
              break;

            case MST_M1_VALUE_ST:
              status = (INT_32) mstdat.regptr->mstim_match1;
              break;

            case MST_INT_PEND:
              status = (INT_32) mstdat.regptr->mstim_int;
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
 * Function: mstimer_read
 *
 * Purpose: Millisecond timer read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to millisecond timer descriptor
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
INT_32 mstimer_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: mstimer_write
 *
 * Purpose: Millisecond timer write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to millisecond timer descriptor
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
INT_32 mstimer_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: mstimer_wait_ms
 *
 * Purpose: Delay for msec milliseconds (minimum)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     msec:  the delay time in milliseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Use of this function will destroy any previous millisecond timer
 *     settings and should not be used if the millisecond timer is
 *     simultaneously being used for something else.
 *
 **********************************************************************/
void mstimer_wait_ms(UNS_32 msec)
{
  UNS_32 tmp = mstimer_msecs_to_ticks(msec);

  /* Stop timers */
  mstdat.regptr->mstim_ctrl = 0;
  mstdat.regptr->mstim_mctrl = 0;

  /* Set timer count value */
  mstdat.regptr->mstim_counter = tmp;

  /* Reset timer count value */
  mstdat.regptr->mstim_ctrl |= MSTIM_CTRL_RESET_COUNT;
  mstdat.regptr->mstim_ctrl &= ~MSTIM_CTRL_RESET_COUNT;

  /* Clear and enable match 0 interrupt for determing when to stop */
  mstdat.regptr->mstim_int = MSTIM_INT_MATCH0_INT;
  mstdat.regptr->mstim_mctrl |= MSTIM_MCTRL_MR0_INT;

  /* Start timer */
  mstdat.regptr->mstim_ctrl |= MSTIM_CTRL_COUNT_ENAB;

  /* Wait for match 0 interrupt */
  while ((mstdat.regptr->mstim_int & MSTIM_INT_MATCH0_INT) == 0);

  /* Disable timer */
  mstdat.regptr->mstim_ctrl &= ~MSTIM_CTRL_COUNT_ENAB;
}
