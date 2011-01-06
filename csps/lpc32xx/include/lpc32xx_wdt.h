/***********************************************************************
* $Id:: lpc32xx_wdt.h 1110 2008-08-21 20:56:22Z stefanovicz            $
*
* Project: LPC32XX WDT definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         WDT
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

#ifndef LPC32XX_WDT_H
#define LPC32XX_WDT_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* WDT register structures
**********************************************************************/

/* WDT module register structures */
typedef struct
{
  volatile UNS_32 wdtim_int;      /* WDT interrupt status register */
  volatile UNS_32 wdtim_ctrl;     /* WDT control register */
  volatile UNS_32 wdtim_counter;  /* WDT counter value register */
  volatile UNS_32 wdtim_mctrl;    /* WDT match control register */
  volatile UNS_32 wdtim_match0;   /* WDT match 0 register */
  volatile UNS_32 wdtim_emr;      /* WDT external match control reg */
  volatile UNS_32 wdtim_pulse;    /* WDT reset pulse length register */
  volatile UNS_32 wdtim_res;      /* WDT reset source register */
} WDT_REGS_T;

/**********************************************************************
* wdtim_int register definitions
**********************************************************************/
/* Interrupt flag for MATCH 0 interrupt */
#define WDT_MATCH_INT           _BIT(0)

/**********************************************************************
* wdtim_ctrl register definitions
**********************************************************************/
#define WDT_COUNT_ENAB          _BIT(0) /* Timer Counter enable */
#define WDT_RESET_COUNT         _BIT(1) /* Timer Counter reset */
#define WDT_PAUSE_EN            _BIT(2) /* Timer Cntr stopped in debug*/

/**********************************************************************
* wdtim_mctrl register definitions
**********************************************************************/
#define WDT_MR0_INT             _BIT(0) /* Enable WDT int on MR0 */
#define WDT_RESET_COUNT0        _BIT(1) /* Enable WDT reset on MR0 */
#define WDT_STOP_COUNT0         _BIT(2) /* Enable WDT stop on MR0 */
#define WDT_M_RES1              _BIT(3) /* M_RES1 control */
#define WDT_M_RES2              _BIT(4) /* M_RES2 control */
#define WDT_RESFRC1             _BIT(5) /* RESFRC1 control */
#define WDT_RESFRC2             _BIT(6) /* RESFRC2 control */

/**********************************************************************
* wdtim_emr register definitions
**********************************************************************/
#define WDT_EXT_MATCH0          _BIT(0) /* current match output */
#define WDT_MATCH_CTRL(n)       _SBF(4,(n) & 0x3)  /* match out ctrl */

/* Macro pointing to WDT registers */
#define WDT ((WDT_REGS_T *)(WDTIM_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_WDT_H */
