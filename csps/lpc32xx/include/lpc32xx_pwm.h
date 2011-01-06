/***********************************************************************
* $Id:: lpc32xx_pwm.h 1113 2008-08-21 20:58:14Z stefanovicz            $
*
* Project: LPC32XX PWM definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         PWM
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

#ifndef LPC32XX_PWM_H
#define LPC32XX_PWM_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* PWM register structures
**********************************************************************/

/* PWM module register structures */
typedef struct
{
  volatile UNS_32 pwm_ir;         /* PWM Interrupt Register */
  volatile UNS_32 pwm_tcr;        /* PWM Timer Control Register */
  volatile UNS_32 pwm_tc;         /* PWM Timer Counter */
  volatile UNS_32 pwm_pr;         /* PWM Prescale Register */
  volatile UNS_32 pwm_pc;         /* PWM Prescale Counter */
  volatile UNS_32 pwm_mcr;        /* PWM Match Control Register */
  volatile UNS_32 pwm_mr0;        /* PWM Match Register 0 */
  volatile UNS_32 pwm_mr1;        /* PWM Match Register 1 */
  volatile UNS_32 pwm_mr2;        /* PWM Match Register 2 */
  volatile UNS_32 pwm_mr3;        /* PWM Match Register 3 */
  volatile UNS_32 pwm_ccr;        /* PWM Capture Control Register */
  volatile UNS_32 pwm_cr0;        /* PWM Capture Register 0 */
  volatile UNS_32 pwm_cr1;        /* PWM Capture Register 1 */
  volatile UNS_32 reserved[3];
  volatile UNS_32 pwm_mr4;        /* PWM Match Register 4 */
  volatile UNS_32 pwm_mr5;        /* PWM Match Register 5 */
  volatile UNS_32 pwm_mr6;        /* PWM Match Register 6 */
  volatile UNS_32 pwm_pcr;        /* PWM Control Register */
  volatile UNS_32 pwm_ler;        /* PWM Load Enable Register */
  volatile UNS_32 reserved1[7];
  volatile UNS_32 pwm_ctcr;       /* PWM Count Control register */
} PWM_REGS_T;

/**********************************************************************
* pwm_ir register definitions
**********************************************************************/
/* Interrupt flag for PWM match channel 6 */
#define PWM_INT_MATCH(n)    ((n) < 4 ? _BIT(n) : _BIT(n+4))

/**********************************************************************
* pwm_tcr register definitions
**********************************************************************/
#define PWM_COUNTER_ENABLE      _BIT(0) /* PWM Counter Enable */
#define PWM_COUNTER_RESET       _BIT(1) /* PWM Counter Reset */
#define PWM_ENABLE              _BIT(3) /* PWM Enable */
#define PWM_MASTER_DISABLE      _BIT(4) /* PWM3 Master Disable */

/**********************************************************************
* pwm_ctcr register definitions
**********************************************************************/
/* PWM Counter/Timer Mode */
#define PWM_CT_MODE(n)          _SBF(0, ((n) & 0x03))
/* PWM Capture input select */
#define PWM_CIN_SELECT(n)       _BIT(2, ((n) & 0x03))

/**********************************************************************
* pwm_mcr register definitions
**********************************************************************/
/* generate a PWM interrupt when a MATCHn occurs */
#define PWM_INT_ON_MATCH(n)     _BIT((((n)&0x7) << 1) + ((n)&0x07))
/* reset the PWM when a MATCHn occurs */
#define PWM_RESET_ON_MATCH(n)   _BIT((((n)&0x7) << 1) + ((n)&0x7) + 1)
/* stop the PWM when a MATCHn occurs */
#define PWM_STOP_ON_MATCH(n)    _BIT((((n)&0x7) << 1) + ((n)&0x7) + 2)

/**********************************************************************
* pwm_ccr register definitions
**********************************************************************/
/* PCAPn is rising edge sensitive */
#define PWM_PCAP_RISING(n)  _BIT((((n) & 0x1) << 1) + ((n) &0x1))
/* PCAPn is falling edge sensitive */
#define PWM_PCAP_FALLING(n) _BIT((((n) & 0x1) << 1) + ((n) &0x1) + 1)
/* a PWM interrupt is generated on a PCAP event */
#define PWM_INT_ON_PCAP(n)  _BIT((((n) & 0x1) << 2) + ((n) &0x1) + 2)

/**********************************************************************
* pwm_pcr register definitions
**********************************************************************/
/* PWM output n is a single edge controlled output */
#define PWM_SINGLE_EDGE(n)  _SBF((n) & 0x7, 0)
/* PWM output n is a double edge controlled output */
#define PWM_DOUBLE_EDGE(n)  (((n) & 0x7) < 2 ? 0 : _SBF(((n) & 0x7), 1))
/* enable PWM output n */
#define PWM_ENABLE_OUT(n)   (((n)&0x7) < 1 ? 0 : _SBF(((n)&0x7) + 8, 1))

/**********************************************************************
* pwm_ler register definitions
**********************************************************************/
/* PWM MATCHn register update control */
#define PWM_EN_MATCHL(n)    ((n) < 7 ? _SBF(n, 1) : 0)

/* Macro pointing to PWM registers */
#define PWM3 ((PWM_REGS_T *)(PWM3_BASE))
#define PWM4 ((PWM_REGS_T *)(PWM4_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_PWM_H */
