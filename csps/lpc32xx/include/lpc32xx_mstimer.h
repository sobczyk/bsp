/***********************************************************************
* $Id:: lpc32xx_mstimer.h 953 2008-07-28 17:07:53Z wellsk             $
*
* Project: LPC32XX Millisecond timer definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         Millisecond timer
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

#ifndef LPC32XX_MSTIMER_H
#define LPC32XX_MSTIMER_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Millisecond timer register structures
**********************************************************************/

/* Millisecond timer module register structures */
typedef struct
{
  volatile UNS_32 mstim_int;     /* MSTIMER interrupt status reg */
  volatile UNS_32 mstim_ctrl;    /* MSTIMER control register */
  volatile UNS_32 mstim_counter; /* MSTIMER counter value reg */
  volatile UNS_32 reserved [2];
  volatile UNS_32 mstim_mctrl;   /* MSTIMER match control reg */
  volatile UNS_32 mstim_match0;  /* MSTIMER match 0 register */
  volatile UNS_32 mstim_match1;  /* MSTIMER match 1 register */
} MSTIMER_REGS_T;

/**********************************************************************
* mstim_int register definitions
* Write a '1' to clear interrupt, reading a '1' indicates active int
**********************************************************************/
#define MSTIM_INT_MATCH1_INT    _BIT(1) /* MSTIMER Match 1 int bit */
#define MSTIM_INT_MATCH0_INT    _BIT(0) /* MSTIMER Match 0 int bit */

/**********************************************************************
* mstim_ctrl register definitions
**********************************************************************/
#define MSTIM_CTRL_PAUSE_EN     _BIT(2) /* Timer pauses in dbg mode */
#define MSTIM_CTRL_RESET_COUNT  _BIT(1) /* Timer count is reset */
#define MSTIM_CTRL_COUNT_ENAB   _BIT(0) /* Timer counter is enabled */

/**********************************************************************
* mstim_mctrl register definitions
**********************************************************************/
#define MSTIM_MCTRL_STOP_COUNT1 _BIT(5) /* Stops counter on match 1 */
#define MSTIM_MCTRL_RST_COUNT1  _BIT(4) /* Reset counter on match 1 */
#define MSTIM_MCTRL_MR1_INT     _BIT(3) /* Match 1 interrupt enable */
#define MSTIM_MCTRL_STOP_COUNT0 _BIT(2) /* Stops counter on match 0 */
#define MSTIM_MCTRL_RST_COUNT0  _BIT(1) /* Reset counter on match 0 */
#define MSTIM_MCTRL_MR0_INT     _BIT(0) /* Match 0 interrupt enable */

/* Macro pointing to Millisecond timer registers */
#define MSTIMER ((MSTIMER_REGS_T *)(MSTIM_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_MSTIMER_H */
