/***********************************************************************
* $Id:: lpc32xx_hstimer.h 799 2008-06-10 23:52:05Z sscaglia            $
*
* Project: LPC32XX High speed timer definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         High speed timer
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

#ifndef LPC32XX_HSTIMER_H
#define LPC32XX_HSTIMER_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* High speed timer register structures
**********************************************************************/

/* High speed timer module register structures */
typedef struct
{
  volatile UNS_32 hstim_int;     /* HSTIMER interrupt status reg */
  volatile UNS_32 hstim_ctrl;    /* HSTIMER control register */
  volatile UNS_32 hstim_counter; /* HSTIMER counter value reg */
  volatile UNS_32 hstim_pmatch;  /* HSTIMER prescale count match */
  volatile UNS_32 hstim_pcount;  /* HSTIMER prescale count value */
  volatile UNS_32 hstim_mctrl;   /* HSTIMER match control reg */
  volatile UNS_32 hstim_match[3];  /* HSTIMER match registers */
  volatile UNS_32 reserved;
  volatile UNS_32 hstim_ccr;     /* HSTIMER capture counter ctrl */
  volatile UNS_32 hstim_cap0;    /* HSTIMER capture 0 reg */
  volatile UNS_32 hstim_cap1;    /* HSTIMER capture 1 reg */
} HSTIMER_REGS_T;

/**********************************************************************
* hstim_int register definitions
* Write a '1' to clear interrupt, reading a '1' indicates active int
**********************************************************************/
#define HSTIM_RTC_TICK_INT      _BIT(5) /* HSTIMER RTC tick int bit */
#define HSTIM_GPI_06_INT        _BIT(4) /* HSTIMER GPI 06 int bit */
#define HSTIM_MATCH2_INT        _BIT(2) /* HSTIMER match 2 int bit */
#define HSTIM_MATCH1_INT        _BIT(1) /* HSTIMER match 1 int bit */
#define HSTIM_MATCH0_INT        _BIT(0) /* HSTIMER match 0 int bit */

/**********************************************************************
* hstim_ctrl register definitions
**********************************************************************/
#define HSTIM_CTRL_PAUSE_EN     _BIT(2) /* Timer pauses in dbg mode */
#define HSTIM_CTRL_RESET_COUNT  _BIT(1) /* Timer count is reset */
#define HSTIM_CTRL_COUNT_ENAB   _BIT(0) /* Timer counter is enabled */


/**********************************************************************
* hstim_mctrl register definitions
**********************************************************************/
/* Bit location for interrupt on MRx match, n = 0 to 2 */
#define HSTIM_CNTR_MCR_MTCH(n)     (0x1 << ((n) * 3))

/* Bit location for reset on MRx match, n = 0 to 2 */
#define HSTIM_CNTR_MCR_RESET(n)    (0x1 << (((n) * 3) + 1))

/* Bit location for stop on MRx match, n = 0 to 2*/
#define HSTIM_CNTR_MCR_STOP(n)     (0x1 << (((n) * 3) + 2))


/**********************************************************************
* ccr register definitions
**********************************************************************/
/* Bit location for CAP.n on CRx rising edge, n = 0 to 3 */
#define HSTIM_CNTR_CCR_CAPNRE(n)   (0x1 << ((n) * 3))

/* Bit location for CAP.n on CRx falling edge, n = 0 to 3 */
#define HSTIM_CNTR_CCR_CAPNFE(n)   (0x1 << (((n) * 3) + 1))

/* Bit location for CAP.n on CRx interrupt enable, n = 0 to 3 */
#define HSTIM_CNTR_CCR_CAPNI(n)    (0x1 << (((n) * 3) + 2))


/* Macro pointing to high speed timer registers */
#define HSTIMER ((HSTIMER_REGS_T *)(HSTIM_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC3XXX_HSTIMER_H */
