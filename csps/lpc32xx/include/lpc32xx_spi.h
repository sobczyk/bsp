/***********************************************************************
* $Id:: lpc32xx_spi.h 1701 2009-03-10 14:08:33Z kendwyer              $
*
* Project: LPC3xxx SPI1/2 definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC3xxx chip family component:
*         SPI1/2 definitions
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

#ifndef LPC32XX_SPI_H
#define LPC32XX_SPI_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* SPI register structures
**********************************************************************/

/* SPI register structures */
typedef struct
{
  volatile UNS_32 global;         /* SPI Global Control Register */
  volatile UNS_32 con;            /* SPI Control Register */
  volatile UNS_32 frm;            /* SPI Frame Count Register */
  volatile UNS_32 ier;            /* SPI Interrupt Enable Register */
  volatile UNS_32 stat;           /* SPI Status Register */
  volatile UNS_32 dat;            /* SPI Data Buffer Register */
  UNS_32 reserved[250];
  volatile UNS_32 tim_ctrl;       /* SPI Timer Control Register */
  volatile UNS_32 tim_count;      /* SPI Timer Counter Register */
  volatile UNS_32 tim_stat;       /* SPI Timer Status Register */
} SPI_REGS_T;

/**********************************************************************
* SPI global control register definitions
**********************************************************************/
#define SPI_GLOB_RST        _BIT(1)     /* SPI interfase sw reset */
#define SPI_GLOB_ENABLE     _BIT(0)     /* SPI interface enable */

/**********************************************************************
* SPI control register definitions
**********************************************************************/
#define SPI_CON_UNIDIR      _BIT(23)    /* DATIO pin dir control */
#define SPI_CON_BHALT       _BIT(22)    /* Busy halt control */
#define SPI_CON_BPOL        _BIT(21)    /* Busy line polarity */
#define SPI_CON_MSB         _BIT(19)    /* MSB/LSB control */
#define SPI_CON_CPOL        _BIT(17)    /* CPOL control*/
#define SPI_CON_CPHA        _BIT(16)    /* CPHA control*/
#define SPI_CON_MODE00      0           /* mode = 00 */
#define SPI_CON_MODE01      _BIT(16)    /* mode = 01 */
#define SPI_CON_MODE10      _BIT(17)    /* mode = 10 */
#define SPI_CON_MODE11      _SBF(16,0x3)/* mode = 11 */
#define SPI_CON_RXTX        _BIT(15)    /* Tx/Rx control */
#define SPI_CON_THR         _BIT(14)    /* FIFO threshold control */
#define SPI_CON_SHIFT_OFF   _BIT(13)    /* SPI clock control */
#define SPI_CON_BITNUM(n)   _SBF(9,((n-1)&0xF)) /* number of bits ctr */
#define SPI_CON_MS          _BIT(7)     /* Master mode control */
#define SPI_CON_RATE(n)     (n & 0x7F)  /* Transfer rate control */


/**********************************************************************
* SPI interrupt enable register definitions
**********************************************************************/
#define SPI_IER_INTEOT      _BIT(1)     /* End of transfer int en */
#define SPI_IER_INTTHR      _BIT(0)     /* FIFO threshold int en */

/**********************************************************************
* SPI status register definitions
**********************************************************************/
#define SPI_STAT_INTCLR     _BIT(8)     /* SPI interrupt clear */
#define SPI_STAT_EOT        _BIT(7)     /* SPI End of Transfer flag */
#define SPI_STAT_BUSYLEV    _BIT(6)     /* SPI BUSY level */
#define SPI_STAT_SHIFTACT   _BIT(3)     /* Shift active flag */
#define SPI_STAT_BF         _BIT(2)     /* FIFO full int flag */
#define SPI_STAT_THR        _BIT(1)     /* FIFO threshold int flag */
#define SPI_STAT_BE         _BIT(0)     /* FIFO empty int flag */

/**********************************************************************
* SPI timer control register definitions
**********************************************************************/
#define SPI_TIRQE           _BIT(2)     /* Timed interrupt enable */
#define SPI_PIRQE           _BIT(1)     /* Peripheral int enable */
#define SPI_MODE            _BIT(0)     /* Timer use control */

/**********************************************************************
* SPI timer status register definitions
**********************************************************************/
#define SPI_TIRQSTAT        _BIT(15)    /* Timed int status flag */

/* Macros pointing to SPI registers */
#define SPI1 ((SPI_REGS_T *)(SPI1_BASE))
#define SPI2 ((SPI_REGS_T *)(SPI2_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SPI_H */
