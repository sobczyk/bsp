/***********************************************************************
* $Id:: lpc3xxx_spwm.h 1112 2008-08-21 20:57:46Z stefanovicz           $
*
* Project: LPC3XXX simple PWM definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC3xxx chip family component:
*         Simple PWM
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

#ifndef LPC3XXX_SPWM_H
#define LPC3XXX_SPWM_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Simple PWM register structures
**********************************************************************/

/* Simple PWM module register structures */
typedef struct
{
  volatile UNS_32 pwm_ctrl;      /* control register*/
} SPWM_REGS_T;

/**********************************************************************
* pwm_ctrl register definitions
**********************************************************************/
/* clock gate and the external output enable */
#define SPWM_EN                 _BIT(31)
/* output pin level control when simple PWM is not used */
#define SPWM_PIN_LEVEL          _BIT(30)
/* reload value for simple PWM output frequency */
#define SPWM_RELOAD(n)          _SBF(8, ((n) & 0xFF))
/* output duty cycle adjustement */
#define SPWM_DUTY(n)            _SBF(0, ((n) & 0xFF))

/* Macro pointing to simple PWM registers */
#define PWM1 ((SPWM_REGS_T *)(PWM1_BASE))
#define PWM2 ((SPWM_REGS_T *)(PWM2_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC3XXX_SPWM_H */
