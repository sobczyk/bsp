/***********************************************************************
 * $Id:: lpc32xx_rtc_driver.c 1021 2008-08-06 22:23:18Z wellsk         $
 *
 * Project: LPC32xx RTC driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx RTC.
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

#include "lpc32xx_rtc_driver.h"

/***********************************************************************
 * RTC driver package data
***********************************************************************/

/* RTC device configuration structure type */
typedef struct
{
  RTC_REGS_T *regptr;
  BOOL_32 init;
} RTC_CFG_T;

/* RTC driver data */
static RTC_CFG_T rtcdat;

/* Clock speed for RTC block */
#define RTC_CLK_RATE CLOCK_OSC_FREQ

/***********************************************************************
 * RTC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: rtc_open
 *
 * Purpose: Open the RTC
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a RTC peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a RTC config structure or NULL
 *
 * Notes: No settings for the timer are changed with this call.
 *
 **********************************************************************/
INT_32 rtc_open(void *ipbase,
                INT_32 arg)
{
  INT_32 rtcptr = (INT_32) NULL;

  /* Has the RTC been previously initialized? */
  if (rtcdat.init == FALSE)
  {
    /* RTC is available */
    rtcdat.init = TRUE;
    rtcdat.regptr = (RTC_REGS_T *) ipbase;

    /* Return pointer to specific RTC structure */
    rtcptr = (INT_32) & rtcdat;
  }

  return rtcptr;
}

/***********************************************************************
 *
 * Function: rtc_close
 *
 * Purpose: Close the RTC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE
 *     and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to RTC config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS rtc_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((rtcdat.init == TRUE) && (devid == (INT_32) &rtcdat))
  {
    rtcdat.init = FALSE;
    status = _NO_ERROR;
  }

  return status;
}

/***********************************************************************
 *
 * Function: rtc_ioctl
 *
 * Purpose: RTC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate RTC parameter.
 *
 * Parameters:
 *     devid: Pointer to RTC config structure
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
STATUS rtc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  RTC_CFG_T *pTimerCfg = (RTC_CFG_T *) devid;
  RTC_RAM_SG_T *pRAM;
  RTC_MATCH_SETUP_T *pmrtc;
  UNS_32 mi, onsw;
  INT_32 status = _ERROR;

  if ((rtcdat.init == TRUE) && (pTimerCfg == &rtcdat))
  {
    status = _NO_ERROR;

    switch (cmd)
    {
      case RTC_RESET:
        if (arg != 0)
        {
          rtcdat.regptr->ctrl |= RTC_SW_RESET;
        }
        else
        {
          rtcdat.regptr->ctrl &= ~RTC_SW_RESET;
        }
        break;

      case RTC_ENABLE:
        if (arg != 0)
        {
          /* Enable the RTC */
          rtcdat.regptr->ctrl &= ~RTC_CNTR_DIS;
        }
        else
        {
          /* Disable the RTC */
          rtcdat.regptr->ctrl |= RTC_CNTR_DIS;
        }
        break;

      case RTC_SET_COUNT:
        rtcdat.regptr->ucount = (UNS_32) arg;
        rtcdat.regptr->dcount = 0xFFFFFFFF - ((UNS_32) arg);
        break;

      case RTC_CLEAR_INTS:
        rtcdat.regptr->intstat = (UNS_32) arg;
        break;

      case RTC_SETUP_MATCH:
        mi = onsw = 0;
        pmrtc = (RTC_MATCH_SETUP_T *) arg;
        if (pmrtc->match_num == 0)
        {
          mi = RTC_MATCH0_EN;
          onsw = RTC_ONSW_MATCH0_EN;
          rtcdat.regptr->match0 = pmrtc->match_tick_val;
        }
        else if (pmrtc->match_num == 1)
        {
          mi = RTC_MATCH1_EN;
          onsw = RTC_ONSW_MATCH1_EN;
          rtcdat.regptr->match1 = pmrtc->match_tick_val;
        }
        if (mi != 0)
        {
          if (pmrtc->use_match_int != 0)
          {
            rtcdat.regptr->ctrl |= mi;
          }
          else
          {
            rtcdat.regptr->ctrl &= ~mi;
          }
          if (pmrtc->enable_onsw != 0)
          {
            rtcdat.regptr->ctrl |= onsw;
          }
          else
          {
            rtcdat.regptr->ctrl &= ~onsw;
          }
        }
        else
        {
          status = _ERROR;
        }
        break;

      case RTC_SETCLR_KEY:
        if (arg != 0)
        {
          rtcdat.regptr->key = RTC_KEY_ONSW_LOADVAL;
        }
        else
        {
          rtcdat.regptr->key = ~RTC_KEY_ONSW_LOADVAL;
        }
        break;

      case RTC_SET_RAM:
        if (arg != 0)
        {
          pRAM = (RTC_RAM_SG_T *) arg;
          if (pRAM->ram_index >= 32)
          {
            status = LPC_BAD_PARAMS;
          }
          else
          {
            rtcdat.regptr->sram [pRAM->ram_index] = pRAM->ram_val;
          }
        }
        break;

      case RTC_GET_RAM:
        if (arg != 0)
        {
          pRAM = (RTC_RAM_SG_T *) arg;
          if (pRAM->ram_index >= 32)
          {
            status = LPC_BAD_PARAMS;
          }
          else
          {
            pRAM->ram_val = rtcdat.regptr->sram [pRAM->ram_index];
          }
        }
        break;

      case RTC_GET_STATUS:
        /* Return a RTC status */
        switch (arg)
        {
          case RTC_GET_CLOCK:
            status = RTC_CLK_RATE;
            break;

          case RTC_UP_VALUE_ST:
            status = rtcdat.regptr->ucount;
            break;

          case RTC_DOWN_VALUE_ST:
            status = rtcdat.regptr->dcount;
            break;

          case RTC_INT_PEND:
            status = rtcdat.regptr->intstat;
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
 * Function: rtc_read
 *
 * Purpose: RTC read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to RTC descriptor
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
INT_32 rtc_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: rtc_write
 *
 * Purpose: Timer RTC function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to RTC descriptor
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
INT_32 rtc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}
