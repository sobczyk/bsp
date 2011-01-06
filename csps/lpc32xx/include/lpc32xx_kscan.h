/***********************************************************************
* $Id:: lpc32xx_kscan.h 930 2008-07-24 18:06:26Z wellsk               $
*
* Project: LPC32XX Keyboard scanner controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         Keyboard scanner
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

#ifndef LPC32XX_KSCAN_H
#define LPC32XX_KSCAN_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* Keyboard scanner control register structures
**********************************************************************/

/* Keyboard scanner module register structures */
typedef struct
{
  volatile UNS_32 ks_deb;
  volatile UNS_32 ks_state_cond;
  volatile UNS_32 ks_irq;
  volatile UNS_32 ks_scan_ctl;
  volatile UNS_32 ks_fast_tst;
  volatile UNS_32 ks_matrix_dim;
  volatile UNS_32 reserved [10];
  volatile UNS_32 ks_data [8];
} KSCAN_REGS_T;

/**********************************************************************
* ks_deb register definitions
**********************************************************************/
/* Keypad debouncing register, number of equal matrix values read,
   n = 0 to 255 passes */
#define KSCAN_DEB_NUM_DEB_PASS(n)  ((n) & 0xFF)

/**********************************************************************
* ks_state_cond register definitions
**********************************************************************/
/* Keypad in the idle state */
#define KSCAN_SCOND_IN_IDLE        0x0
/* Keypad in the scan-once state */
#define KSCAN_SCOND_IN_SCANONCE    0x1
/* Keypad in the IRQ generation state */
#define KSCAN_SCOND_IN_IRQGEN      0x2
/* Keypad in the IRQ scan-matrix state */
#define KSCAN_SCOND_IN_SCAN_MATRIX 0x3

/**********************************************************************
* ks_irq register definitions
**********************************************************************/
/* Interrupt pending flag, write to clear */
#define KSCAN_IRQ_PENDING_CLR      0x1

/**********************************************************************
* ks_scan_ctl register definitions
**********************************************************************/
/* Time in clocks between each scan state in matrix mode, n = 1 to
   255, use 0 for scan always */
#define KSCAN_SCTRL_SCAN_DELAY(n)  ((n) & 0xFF)

/**********************************************************************
* ks_fast_tst register definitions
**********************************************************************/
/* Force scan-once state */
#define KSCAN_FTST_FORCESCANONCE   0x1
/* Select keypad scanner clock, 0 = PERIPH_CLK, 1 = 32K clock */
#define KSCAN_FTST_USE32K_CLK      0x2

/**********************************************************************
* ks_matrix_dim register definitions
**********************************************************************/
/* Matrix dimension selection clock, n = 1 to 8 for a 1x1 to 8x8
   matrix */
#define KSCAN_MSEL_SELECT(n)       ((n) & 0xF)

/* Macro pointing to Keyboard scanner control registers */
#define KSCAN ((KSCAN_REGS_T *)(KSCAN_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_KSCAN_H */
