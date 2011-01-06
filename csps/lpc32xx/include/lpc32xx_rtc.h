/***********************************************************************
* $Id:: lpc32xx_rtc.h 936 2008-07-24 18:31:56Z wellsk                 $
*
* Project: LPC32XX Real time clock definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         Real time clock
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

#ifndef LPC32XX_RTC_H
#define LPC32XX_RTC_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Real time clock register structures
**********************************************************************/

/* Real time clock module register structures */
typedef struct
{
  volatile UNS_32 ucount;
  volatile UNS_32 dcount;
  volatile UNS_32 match0;
  volatile UNS_32 match1;
  volatile UNS_32 ctrl;
  volatile UNS_32 intstat;
  volatile UNS_32 key;
  volatile UNS_32 reserved [25];
  volatile UNS_32 sram [32];
} RTC_REGS_T;

/**********************************************************************
* ctrl register definitions
**********************************************************************/
/* Bit for enabling RTC match 0 interrupt */
#define RTC_MATCH0_EN              _BIT(0)
/* Bit for enabling RTC match 1 interrupt */
#define RTC_MATCH1_EN              _BIT(1)
/* Bit for enabling ONSW signal on match 0 */
#define RTC_ONSW_MATCH0_EN         _BIT(2)
/* Bit for enabling ONSW signal on match 1 */
#define RTC_ONSW_MATCH1_EN         _BIT(3)
/* Bit for performing an RC software reset, must be cleared after set */
#define RTC_SW_RESET               _BIT(4)
/* Bit for disabling 1Hz up/down counters */
#define RTC_CNTR_DIS               _BIT(6)
/* Bit for forcing ONSW high (1) or default state */
#define RTC_ONSW_FORCE_HIGH        _BIT(7)

/**********************************************************************
* intstat register definitions
**********************************************************************/
/* Bit for match 0 interrupt status or clearing latched state */
#define RTC_MATCH0_INT_STS         _BIT(0)
/* Bit for match 1 interrupt status or clearing latched state */
#define RTC_MATCH1_INT_STS         _BIT(1)
/* Bit for ONSW interrupt status or clearing ONSW high state */
#define RTC_ONSW_INT_STS           _BIT(2)

/**********************************************************************
* key register definitions
**********************************************************************/
/* RTC key register enable value (must be loaded for ONSW to work) */
#define RTC_KEY_ONSW_LOADVAL       0xB5C13F27

/* Macro pointing to Real time clock registers */
#define RTC ((RTC_REGS_T *)(RTC_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_RTC_H */
