/***********************************************************************
* $Id:: lpc32xx_timer.h 962 2008-07-28 17:36:25Z wellsk               $
*
* Project: LPC32XX Timer/counter definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         Timer/counter
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

#ifndef LPC32XX_TIMER_CNTR_H
#define LPC32XX_TIMER_CNTR_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Timer/counter register structures
**********************************************************************/

/* Timer module register structures */
typedef struct
{
  volatile UNS_32 ir;          /* Timer interrupt status reg */
  volatile UNS_32 tcr;         /* Timer control register */
  volatile UNS_32 tc;          /* Timer counter value reg */
  volatile UNS_32 pr;          /* Timer prescale register */
  volatile UNS_32 pc;          /* Timer prescale counter reg */
  volatile UNS_32 mcr;         /* Timer Match control reg */
  volatile UNS_32 mr[4];       /* Timer Match registers */
  volatile UNS_32 ccr;         /* Timer Capture control reg */
  volatile UNS_32 cr[4];       /* Timer Capture registers */
  volatile UNS_32 emr;         /* Timer External match reg */
  volatile UNS_32 rsvd2[12];   /* Reserved */
  volatile UNS_32 ctcr;        /* Timer Count control reg */
} TIMER_CNTR_REGS_T;

/**********************************************************************
* ir register definitions
* Write a '1' to clear interrupt, reading a '1' indicates active int
**********************************************************************/
/* Macro for getting a timer match interrupt bit */
#define TIMER_CNTR_MTCH_BIT(n)     (1 << ((n) & 0x3))

/* Macro for getting a capture event interrupt bit */
#define TIMER_CNTR_CAPT_BIT(n)     (1 << (4 + ((n) & 0x3)))

/**********************************************************************
* tcr register definitions
**********************************************************************/
/* Timer/counter enable bit */
#define TIMER_CNTR_TCR_EN          0x1

/* Timer/counter reset bit */
#define TIMER_CNTR_TCR_RESET       0x2

/**********************************************************************
* mcr register definitions
**********************************************************************/
/* Bit location for interrupt on MRx match, n = 0 to 3 */
#define TIMER_CNTR_MCR_MTCH(n)     (0x1 << ((n) * 3))

/* Bit location for reset on MRx match, n = 0 to 3 */
#define TIMER_CNTR_MCR_RESET(n)    (0x1 << (((n) * 3) + 1))

/* Bit location for stop on MRx match, n = 0 to 3 */
#define TIMER_CNTR_MCR_STOP(n)     (0x1 << (((n) * 3) + 2))

/**********************************************************************
* ccr register definitions
**********************************************************************/
/* Bit location for CAP.n on CRx rising edge, n = 0 to 3 */
#define TIMER_CNTR_CCR_CAPNRE(n)   (0x1 << ((n) * 3))

/* Bit location for CAP.n on CRx falling edge, n = 0 to 3 */
#define TIMER_CNTR_CCR_CAPNFE(n)   (0x1 << (((n) * 3) + 1))

/* Bit location for CAP.n on CRx interrupt enable, n = 0 to 3 */
#define TIMER_CNTR_CCR_CAPNI(n)    (0x1 << (((n) * 3) + 2))

/**********************************************************************
* emr register definitions
**********************************************************************/
/* Bit location for output state change of MAT.n when external match
   happens, n = 0 to 3 */
#define TIMER_CNTR_EMR_DRIVE(n)    (1 << (n))

/* Macro for setting MAT.n soutput state */
#define TIMER_CNTR_EMR_DRIVE_SET(n, s) (((s) & 0x1) << (n))

/* Output state change of MAT.n when external match happens */
#define TIMER_CNTR_EMR_NOTHING     0x0
#define TIMER_CNTR_EMR_LOW         0x1
#define TIMER_CNTR_EMR_HIGH        0x2
#define TIMER_CNTR_EMR_TOGGLE      0x3

/* Macro for setting for the MAT.n change state bits */
#define TIMER_CNTR_EMR_EMC_SET(n, s) (((s) & 0x3) << (4 + ((n) * 2)))

/* Mask for the MAT.n change state bits */
#define TIMER_CNTR_EMR_EMC_MASK(n) (0x3 << (4 + ((n) * 2)))

/**********************************************************************
* ctcr register definitions
**********************************************************************/
/* Mask to get the Counter/timer mode bits */
#define TIMER_CNTR_CTCR_MODE_MASK  0x3

/* Mask to get the count input select bits */
#define TIMER_CNTR_CTCR_INPUT_MASK 0xC

/* Counter/timer modes */
#define TIMER_CNTR_CTCR_TIMER_MODE 0x0
#define TIMER_CNTR_CTCR_TCINC_MODE 0x1
#define TIMER_CNTR_CTCR_TCDEC_MODE 0x2
#define TIMER_CNTR_CTCR_TCBOTH_MODE 0x3

/* Count input selections */
#define TIMER_CNTR_CTCR_INPUT_CAP0 0x0
#define TIMER_CNTR_CTCR_INPUT_CAP1 0x1
#define TIMER_CNTR_CTCR_INPUT_CAP2 0x2
#define TIMER_CNTR_CTCR_INPUT_CAP3 0x3

/* Macro for setting the counter/timer mode */
#define TIMER_CNTR_SET_MODE(n)     ((n) & 0x3)

/* Macro for setting the count input select */
#define TIMER_CNTR_SET_INPUT(n)    (((n) & 0x3) << 2)

/* Macros pointing to timer registers */
#define TIMER_CNTR0 ((TIMER_CNTR_REGS_T *)(TIMER0_BASE))
#define TIMER_CNTR1 ((TIMER_CNTR_REGS_T *)(TIMER1_BASE))
#define TIMER_CNTR2 ((TIMER_CNTR_REGS_T *)(TIMER2_BASE))
#define TIMER_CNTR3 ((TIMER_CNTR_REGS_T *)(TIMER3_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_TIMER_CNTR_H */
