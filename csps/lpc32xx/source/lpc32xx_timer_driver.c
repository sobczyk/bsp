/***********************************************************************
 * $Id:: lpc32xx_timer_driver.c 998 2008-08-06 00:22:05Z wellsk        $
 *
 * Project: LPC32XX timer driver
 *
 * Description:
 *     This file contains driver support for the LPC32XX timer.
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

#include "lpc32xx_timer_driver.h"
#include "lpc32xx_clkpwr_driver.h"

/***********************************************************************
 * Timer driver package data
***********************************************************************/

/* Maximum number of timers */
#define MAX_TIMERS 4

/* Timer device configuration structure type */
typedef struct
{
  TIMER_CNTR_REGS_T *regptr;
} TIMER_CFG_T;

/* Initialized flags */
static BOOL_32 tmr_init [MAX_TIMERS];

/* Timer driver data */
static TIMER_CFG_T tmrdat [MAX_TIMERS];

/* Array to find a clock ID from it's timer number */
static const CLKPWR_CLK_T timer_num_to_clk_enum [MAX_TIMERS] =
{
  CLKPWR_TIMER0_CLK,
  CLKPWR_TIMER1_CLK,
  CLKPWR_TIMER2_CLK,
  CLKPWR_TIMER3_CLK
};

/***********************************************************************
 * Timer driver private functions
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
UNS_32 timer_usec_to_val(CLKPWR_CLK_T clknum,
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
 *
 * Function: timer_ptr_to_timer_num
 *
 * Purpose: Convert a timer register pointer to a timer number
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer : Pointer to a timer register set
 *
 * Outputs: None
 *
 * Returns: The timer number (0 to 3) or -1 if register pointer is bad
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_ptr_to_timer_num(TIMER_CNTR_REGS_T *pTimer)
{
  INT_32 tnum = -1;

  if (pTimer == TIMER_CNTR0)
  {
    tnum = 0;
  }
  else if (pTimer == TIMER_CNTR1)
  {
    tnum = 1;
  }
  else if (pTimer == TIMER_CNTR2)
  {
    tnum = 2;
  }
  else if (pTimer == TIMER_CNTR3)
  {
    tnum = 3;
  }

  return tnum;
}

/***********************************************************************
 *
 * Function: timer_cfg_to_timer_num
 *
 * Purpose: Convert a timer driver config pointer to a timer number
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimerCfg : Pointer to a timer config structure
 *
 * Outputs: None
 *
 * Returns: The timer number (0 to 3) or -1 if pointer is bad
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_cfg_to_timer_num(TIMER_CFG_T *pTimerCfg)
{
  INT_32 tnum = -1;

  if (pTimerCfg == &tmrdat[0])
  {
    tnum = 0;
  }
  else if (pTimerCfg == &tmrdat[1])
  {
    tnum = 1;
  }
  else if (pTimerCfg == &tmrdat[2])
  {
    tnum = 2;
  }
  else if (pTimerCfg == &tmrdat[3])
  {
    tnum = 3;
  }

  return tnum;
}

/***********************************************************************
 *
 * Function: timer_delay_cmn
 *
 * Purpose: Delay for a period of microseconds
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer: Pointer to timer register set to use
 *     usec  :  the delay time in microseconds
 *
 * Outputs: None
 *
 * Returns: The clock rate of the timer in Hz, or 0 if invalid
 *
 * Notes: None
 *
 **********************************************************************/
void timer_delay_cmn(TIMER_CNTR_REGS_T *pTimer, UNS_32 usec)
{
  INT_32 timernum;

  /* Get clock number */
  timernum = timer_ptr_to_timer_num(pTimer);
  if (timernum < 0)
  {
    return;
  }

  /* Enable timer system clock */
  clkpwr_clk_en_dis(timer_num_to_clk_enum[timernum], 1);

  /* Reset timer */
  pTimer->tcr = TIMER_CNTR_TCR_RESET;
  pTimer->tcr = 0;

  /* Clear and enable match function */
  pTimer->ir = TIMER_CNTR_MTCH_BIT(0);

  /* Count mode is PCLK edge */
  pTimer->ctcr = TIMER_CNTR_SET_MODE(TIMER_CNTR_CTCR_TIMER_MODE);

  /* Set prescale counter value for a 1uS tick */
  pTimer->pr = (UNS_32) timer_usec_to_val(
                 timer_num_to_clk_enum[timernum], 1);

  /* Set match for number of usecs */
  pTimer->mr[0] = usec;

  /* Interrupt on match 0 */
  pTimer->mcr = TIMER_CNTR_MCR_MTCH(0);

  /* Enable the timer */
  pTimer->tcr = TIMER_CNTR_TCR_EN;

  /* Loop until match occurs */
  while ((pTimer->ir & TIMER_CNTR_MTCH_BIT(0)) == 0);

  /* Stop timer */
  pTimer->tcr = 0;

  /* Disable timer system clock */
  clkpwr_clk_en_dis(timer_num_to_clk_enum[timernum], 0);
}

/***********************************************************************
 * Timer driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: timer_open
 *
 * Purpose: Open the timer
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a timer peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a timer config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_open(void *ipbase,
                  INT_32 arg)
{
  TIMER_CFG_T *pTimer;
  INT_32 tnum, tptr = (INT_32) NULL;

  /* Try to find a matching timer number based on the pass pointer */
  tnum = timer_ptr_to_timer_num((TIMER_CNTR_REGS_T *) ipbase);
  if (tnum >= 0)
  {
    /* Has the timer been previously initialized? */
    if (tmr_init[tnum] == FALSE)
    {
      /* Timer is free */
      tmr_init[tnum] = TRUE;
      pTimer = &tmrdat[tnum];
      pTimer->regptr = (TIMER_CNTR_REGS_T *) ipbase;

      /* Enable timer system clock */
      clkpwr_clk_en_dis(timer_num_to_clk_enum[tnum], 1);

      /* Setup default timer state as standard timer mode, timer
         disabled and all match and counters disabled */
      pTimer->regptr->tcr = 0;
      pTimer->regptr->ctcr =
        TIMER_CNTR_SET_MODE(TIMER_CNTR_CTCR_TIMER_MODE);
      pTimer->regptr->mcr = 0;
      pTimer->regptr->ccr = 0;
      pTimer->regptr->emr =
        TIMER_CNTR_EMR_EMC_SET(0, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(1, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(2, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(3, TIMER_CNTR_EMR_NOTHING);

      /* Return pointer to specific timer structure */
      tptr = (INT_32) & tmrdat[tnum];

      /* Clear pending interrupts and reset counts */
      pTimer->regptr->tc = 0;
      pTimer->regptr->pr = 0;
      pTimer->regptr->pc = 0;
      for (tnum = 0; tnum < MAX_TIMERS; tnum++)
      {
        pTimer->regptr->mr [tnum] = 0;
      }
      pTimer->regptr->ir = ((TIMER_CNTR_MTCH_BIT(0) |
        TIMER_CNTR_MTCH_BIT(1) | TIMER_CNTR_MTCH_BIT(2) |
        TIMER_CNTR_MTCH_BIT(3) | TIMER_CNTR_CAPT_BIT(0) |
        TIMER_CNTR_CAPT_BIT(1) | TIMER_CNTR_CAPT_BIT(2) |
        TIMER_CNTR_CAPT_BIT(3)));
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: timer_close
 *
 * Purpose: Close the timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS timer_close(INT_32 devid)
{
  INT_32 tnum;
  TIMER_CFG_T *pTimer;
  STATUS status = _ERROR;

  /* Get timer number from passed device structure */
  tnum = timer_cfg_to_timer_num((TIMER_CFG_T *) devid);
  if (tnum >= 0)
  {
    if (tmr_init[tnum] == TRUE)
    {
      pTimer = &tmrdat[tnum];
      tmr_init[tnum] = FALSE;

      /* Disable all timer fucntions */
      pTimer->regptr->tcr = 0;
      pTimer->regptr->ctcr =
        TIMER_CNTR_SET_MODE(TIMER_CNTR_CTCR_TIMER_MODE);
      pTimer->regptr->mcr = 0;
      pTimer->regptr->ccr = 0;
      pTimer->regptr->emr =
        TIMER_CNTR_EMR_EMC_SET(0, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(1, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(2, TIMER_CNTR_EMR_NOTHING) |
        TIMER_CNTR_EMR_EMC_SET(3, TIMER_CNTR_EMR_NOTHING);

      /* Disable timer system clock */
      clkpwr_clk_en_dis(timer_num_to_clk_enum[tnum], 0);
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: timer_ioctl
 *
 * Purpose: Timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
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
STATUS timer_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg)
{
  UNS_32 msk, tmp;
  INT_32 tnum;
  TIMER_CFG_T *pTimerCfg;
  TIMER_CNTR_REGS_T *pTimer;
  TMR_PSCALE_SETUP_T *ppstp;
  TMR_INPUT_CLK_T *pclkins;
  TMR_MATCH_SETUP_T *pmstp;
  TMR_CAP_CLOCK_CTRL_T *pstpcap;
  TMR_MATCH_GEN_T *pmgen;
  TMR_COUNTS_T *ptcnts;
  INT_32 status = _ERROR;

  /* Get timer number from passed device structure */
  tnum = timer_cfg_to_timer_num((TIMER_CFG_T *) devid);
  if (tnum >= 0)
  {
    if (tmr_init[tnum] == TRUE)
    {
      status = _NO_ERROR;
      pTimerCfg = &tmrdat[tnum];
      pTimer = pTimerCfg->regptr;

      switch (cmd)
      {
        case TMR_ENABLE:
          if (arg != 0)
          {
            /* Enable the timer */
            pTimer->tcr |= TIMER_CNTR_TCR_EN;
          }
          else
          {
            /* Disable the timer */
            pTimer->tcr &= ~TIMER_CNTR_TCR_EN;
          }
          break;

        case TMR_RESET:
          pTimer->tcr |= TIMER_CNTR_TCR_RESET;
          while (pTimer->tc != 0);
          pTimer->tcr &= ~TIMER_CNTR_TCR_RESET;
          break;

        case TMR_SETUP_PSCALE:
          ppstp = (TMR_PSCALE_SETUP_T *) arg;
          if (ppstp->ps_tick_val != 0)
          {
            pTimer->pr = ppstp->ps_tick_val;
          }
          else
          {
            pTimer->pr =
              timer_usec_to_val(timer_num_to_clk_enum[
                                  tnum], ppstp->ps_us_val);
          }
          break;

        case TMR_SETUP_CLKIN:
          pclkins = (TMR_INPUT_CLK_T *) arg;
          if (pclkins->use_pclk == TRUE)
          {
            pTimer->ctcr = TIMER_CNTR_SET_MODE(
                             TIMER_CNTR_CTCR_TIMER_MODE);
          }
          else
          {
            tmp = 0;
            if ((pclkins->use_cap0_pos == TRUE) &&
                (pclkins->use_cap0_neg == TRUE))
            {
              tmp = TIMER_CNTR_SET_MODE(
                      TIMER_CNTR_CTCR_TCBOTH_MODE);
            }
            else if (pclkins->use_cap0_pos == TRUE)
            {
              tmp = TIMER_CNTR_SET_MODE(
                      TIMER_CNTR_CTCR_TCINC_MODE);
            }
            else if (pclkins->use_cap0_neg == TRUE)
            {
              tmp = TIMER_CNTR_SET_MODE(
                      TIMER_CNTR_CTCR_TCDEC_MODE);
            }

            /* Select clock input CAPn.x */
            if (pclkins->cap_input <= 3)
            {
              tmp |= TIMER_CNTR_SET_INPUT(
                       pclkins->cap_input);
              pTimer->ctcr = tmp;
            }
            else
            {
              status = LPC_BAD_PARAMS;
            }
          }
          break;

        case TMR_SETUP_MATCH:
          pmstp = (TMR_MATCH_SETUP_T *) arg;

          if (pmstp->timer_num <= 3)
          {
            /* Generate mask for match bits */
            msk = ~((TIMER_CNTR_MCR_MTCH(pmstp->timer_num))
                    | (TIMER_CNTR_MCR_RESET(pmstp->timer_num))
                    | (TIMER_CNTR_MCR_STOP(pmstp->timer_num)));

            /* Save timer match value */
            pTimer->mr [pmstp->timer_num] =
              pmstp->match_tick_val;

            tmp = 0;
            if (pmstp->use_match_int == TRUE)
            {
              tmp |=
                TIMER_CNTR_MCR_MTCH(pmstp->timer_num);
            }
            if (pmstp->stop_on_match == TRUE)
            {
              tmp |=
                TIMER_CNTR_MCR_STOP(pmstp->timer_num);
            }
            if (pmstp->reset_on_match == TRUE)
            {
              tmp |=
                TIMER_CNTR_MCR_RESET(pmstp->timer_num);
            }
            pTimer->mcr = (pTimer->mcr & msk) | tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case TMR_SETUP_CAPTURE:
          pstpcap = (TMR_CAP_CLOCK_CTRL_T *) arg;
          if (pstpcap->timer_num <= 3)
          {
            /* Generate mask for capture control bits */
            msk = ~((TIMER_CNTR_CCR_CAPNI(
                       pstpcap->timer_num)) |
                    (TIMER_CNTR_CCR_CAPNFE(pstpcap->timer_num))
                    | (TIMER_CNTR_CCR_CAPNRE(
                         pstpcap->timer_num)));

            /* Setup capture control */
            tmp = 0;
            if (pstpcap->cap_on_rising == TRUE)
            {
              tmp |= TIMER_CNTR_CCR_CAPNRE(
                       pstpcap->timer_num);
            }
            if (pstpcap->cap_on_falling == TRUE)
            {
              tmp |= TIMER_CNTR_CCR_CAPNFE(
                       pstpcap->timer_num);
            }
            if (pstpcap->use_capture_int == TRUE)
            {
              tmp |= TIMER_CNTR_CCR_CAPNI(
                       pstpcap->timer_num);
            }
            pTimer->ccr = (pTimer->ccr & msk) | tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case TMR_SETUP_MATCHOUT:
          pmgen = (TMR_MATCH_GEN_T *) arg;
          if (pmgen->timer_num <= 3)
          {
            /* Generate mask for match output bits */
            msk = ~((TIMER_CNTR_EMR_DRIVE(pmgen->timer_num))
                    | TIMER_CNTR_EMR_EMC_MASK(
                      pmgen->timer_num));
            tmp = (TIMER_CNTR_EMR_DRIVE_SET(pmgen->timer_num,
                                            pmgen->matn_state) |
                   TIMER_CNTR_EMR_EMC_SET(pmgen->timer_num,
                                          pmgen->matn_state_ctrl));
            pTimer->emr = (pTimer->emr & msk) | tmp;
          }
          else
          {
            status = LPC_BAD_PARAMS;
          }
          break;

        case TMR_CLEAR_INTS:
          pTimer->ir = (UNS_32) arg;
          break;

        case TMR_GET_COUNTS:
          ptcnts = (TMR_COUNTS_T *) arg;
          for (tmp = 0; tmp <= 3; tmp++)
          {
            ptcnts->cap_count_val [tmp] =
              pTimer->cr [tmp];
          }
          ptcnts->ps_count_val = pTimer->pc;
          ptcnts->count_val = pTimer->tc;
          break;

        case TMR_GET_STATUS:
          /* Return a timer status */
          switch (arg)
          {
            case TMR_GET_CLOCK:
              status =
                clkpwr_get_clock_rate(timer_num_to_clk_enum[tnum]);
              break;

            case TMR_VALUE_ST:
              status = pTimer->tc;
              break;

            case TMR_INT_PEND:
              status = pTimer->ir;
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
 * Function: timer_read
 *
 * Purpose: Timer read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to timer descriptor
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
INT_32 timer_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: timer_write
 *
 * Purpose: Timer write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to timer descriptor
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
INT_32 timer_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: timer_wait_ms
 *
 * Purpose: Delay for msec milliseconds (minimum)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer: Pointer to timer register set to use
 *     msec  :  the delay time in milliseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Use of this function will destroy any previous timer settings
 *     (for the specific timer used) and should not be used if that
 *     timer is simultaneously being used for something else.
 *
 **********************************************************************/
void timer_wait_ms(TIMER_CNTR_REGS_T *pTimer, UNS_32 msec)
{
  timer_delay_cmn(pTimer, (msec * 1000));
}

/***********************************************************************
 *
 * Function: timer_wait_us
 *
 * Purpose: Delay for usec microseconds (minimum)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer: Pointer to timer register set to use
 *     usec  :  the delay time in microseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Use of this function will destroy any previous timer settings
 *     (for the specific timer used) and should not be used if that
 *     timer is simultaneously being used for something else.
 *
 **********************************************************************/
void timer_wait_us(TIMER_CNTR_REGS_T *pTimer, UNS_32 usec)
{
  timer_delay_cmn(pTimer, usec);
}
