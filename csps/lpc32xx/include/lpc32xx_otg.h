/***********************************************************************
* $Id:: lpc32xx_otg.h 974 2008-07-28 21:07:32Z wellsk                 $
*
* Project: LPC32XX OTG controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         OTG controller
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

#ifndef LPC32XX_OTG_H
#define LPC32XX_OTG_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"
#include "lpc32xx_i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* OTG I2C controller register structures
**********************************************************************/

/* OTG I2C controller module register structures */
typedef struct
{
  volatile UNS_32 otg_i2c_txrx;      /* OTG I2C Tx/Rx Data FIFO */
  volatile UNS_32 otg_i2c_stat;      /* OTG I2C Status Register */
  volatile UNS_32 otg_i2c_ctrl;      /* OTG I2C Control Register */
  volatile UNS_32 otg_i2c_clk_hi;    /* OTG I2C Clock Divider high */
  volatile UNS_32 otg_i2c_clk_lo;    /* OTG I2C Clock Divider low */
} OTGI2C_REGS_T;

/**********************************************************************
* OTG controller register structures
**********************************************************************/

/* OTG controller module register structures */
typedef struct
{
  volatile UNS_32 reserved1[64];
  volatile UNS_32 otg_int_sts;      /* OTG int status register */
  volatile UNS_32 otg_int_enab;     /* OTG int enable register */
  volatile UNS_32 otg_int_set;      /* OTG int set register */
  volatile UNS_32 otg_int_clr;      /* OTG int clear register */
  volatile UNS_32 otg_sts_ctrl;     /* OTG status/control register */
  volatile UNS_32 otg_timer;        /* OTG timer register */
  volatile UNS_32 reserved2[122];
  OTGI2C_REGS_T otg_i2c;
  volatile UNS_32 reserved3[824];
  volatile UNS_32 otg_clk_ctrl;      /* OTG clock control reg */
  volatile UNS_32 otg_clk_sts;       /* OTG clock status reg */
} OTG_REGS_T;

/**********************************************************************
* otg_int_sts, otg_int_enab, otg_int_set, and otg_int_clr register
* definitions
**********************************************************************/
#define OTG_INT_HNP_SUCC _BIT(3)       /* HNP success */
#define OTG_INT_HNP_FAIL _BIT(2)       /* HNP failure */
#define OTG_INT_REM_PLLUP _BIT(1)      /* Remove pullup interrupt */
#define OTG_INT_TIMER _BIT(0)          /* Timer interrupt */

/**********************************************************************
* otg_sts_ctrl register definitions
**********************************************************************/
#define OTG_PLLUP_REMD _BIT(10)        /* Pullup removed */
#define OTG_AB_HNP_TRK _BIT(9)         /* A to B HNP track */
#define OTG_BA_HNP_TRK _BIT(8)         /* B to A HNP track */
#define OTGTRAN_I2C_EN _BIT(7)         /* Enable transparent I2C */
#define OTG_TIMER_RST _BIT(6)          /* Reset the timer */
#define OTG_TIMER_EN _BIT(5)           /* Enable the timer */
#define OTG_TIMER_MODE_FREE _BIT(4)    /* Enable freerun timer mode */
#define OTG_TIMER_GRAN_10US 0x0        /* 10uS timer granularity */
#define OTG_TIMER_GRAN_100US 0x4       /* 100uS timer granularity */
#define OTG_TIMER_GRAN_1000US 0x8      /* 1000uS timer granularity */
#define OTG_HOST_EN _BIT(0)            /* Enable host mode */

/**********************************************************************
* OTG I2C register definitions
* The register definitions are exactly the same as "master only" base
* I2C register settings. See the I2C header file for the register
* bit descriptions for this peripheral.
**********************************************************************/

/**********************************************************************
* otg_clk_ctrl and otg_clk_sts register definitions
**********************************************************************/
#define OTG_CLK_AHB_EN _BIT(4)         /* Enable AHB clock */
#define OTG_CLK_OTG_EN _BIT(3)         /* Enable OTG clock */
#define OTG_CLK_I2C_EN _BIT(2)         /* Enable I2C clock */
#define OTG_CLK_DEV_EN _BIT(1)         /* Enable device clock */
#define OTG_CLK_HOST_EN _BIT(0)        /* Enable host clock */

/* Macro pointing to I2C controller registers */
#define OTG     ((OTG_REGS_T  *)(USB_OTG_BASE))
/* Macro pointing to I2C controller registers */
#define OTGI2C  ((OTGI2C_REGS_T  *)(OTG_I2C_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_OTG_H */
