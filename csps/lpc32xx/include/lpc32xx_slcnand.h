/***********************************************************************
* $Id:: lpc32xx_slcnand.h 2397 2009-10-28 00:20:51Z wellsk            $
*
* Project: LPC32XX SLC NAND controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         SLC NAND controller
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

#ifndef LPC32XX_SLCNAND_H
#define LPC32XX_SLCNAND_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* SLC NAND controller register structures
**********************************************************************/

/* SLC NAND controller module register structures */
typedef struct
{
  volatile UNS_32 slc_data;      /* SLC NAND data reg */
  volatile UNS_32 slc_addr;      /* SLC NAND address register */
  volatile UNS_32 slc_cmd;       /* SLC NAND command reg */
  volatile UNS_32 slc_stop;      /* SLC NAND stop register */
  volatile UNS_32 slc_ctrl;      /* SLC NAND control reg */
  volatile UNS_32 slc_cfg;       /* SLC NAND config register */
  volatile UNS_32 slc_stat;      /* SLC NAND status register */
  volatile UNS_32 slc_int_stat;  /* SLC NAND int status register */
  volatile UNS_32 slc_ien;       /* SLC NAND int enable register */
  volatile UNS_32 slc_isr;       /* SLC NAND int set register */
  volatile UNS_32 slc_icr;       /* SLC NAND int clear register */
  volatile UNS_32 slc_tac;       /* SLC NAND timing register */
  volatile UNS_32 slc_tc;        /* SLC NAND transfer count reg */
  volatile UNS_32 slc_ecc;       /* SLC NAND parity register */
  volatile UNS_32 slc_dma_data;  /* SLC NAND DMA data register */
} SLCNAND_REGS_T;

/**********************************************************************
* slc_ctrl register definitions
**********************************************************************/
#define SLCCTRL_SW_RESET    _BIT(2) /* Reset the NAND controller bit */
#define SLCCTRL_ECC_CLEAR   _BIT(1) /* Reset ECC bit */
#define SLCCTRL_DMA_START   _BIT(0) /* Start DMA channel bit */

/**********************************************************************
* slc_cfg register definitions
**********************************************************************/
#define SLCCFG_CE_LOW       _BIT(5) /* Force CE low bit */
#define SLCCFG_DMA_ECC      _BIT(4) /* Enable DMA ECC bit */
#define SLCCFG_ECC_EN       _BIT(3) /* ECC enable bit */
#define SLCCFG_DMA_BURST    _BIT(2) /* DMA burst bit */
#define SLCCFG_DMA_DIR      _BIT(1) /* DMA write(0)/read(1) bit */
#define SLCCFG_WIDTH        _BIT(0) /* External device width, 0=8bit */

/**********************************************************************
* slc_stat register definitions
**********************************************************************/
#define SLCSTAT_DMA_FIFO    _BIT(2) /* DMA FIFO has data bit */
#define SLCSTAT_SLC_FIFO    _BIT(1) /* SLC FIFO has data bit */
#define SLCSTAT_NAND_READY  _BIT(0) /* NAND device is ready bit */

/**********************************************************************
* slc_int_stat, slc_ien, slc_isr, and slc_icr register definitions
**********************************************************************/
#define SLCSTAT_INT_TC      _BIT(1) /* Transfer count bit */
#define SLCSTAT_INT_RDY_EN  _BIT(0) /* Ready interrupt bit */

/**********************************************************************
* slc_tac register definitions
**********************************************************************/
/* Clock setting for RDY write sample wait time in 2*n clocks */
#define SLCTAC_WDR(n)       ((((UNS_32) (n)) & 0xF) << 28)
/* Write pulse width in clocks cycles, 1 to 16 clocks */
#define SLCTAC_WWIDTH(n)    (((n) & 0xF) << 24)
/* Write hold time of control and data signals, 1 to 16 clocks */
#define SLCTAC_WHOLD(n)     (((n) & 0xF) << 20)
/* Write setup time of control and data signals, 1 to 16 clocks */
#define SLCTAC_WSETUP(n)    (((n) & 0xF) << 16)
/* Clock setting for RDY read sample wait time in 2*n clocks */
#define SLCTAC_RDR(n)       (((n) & 0xF) << 12)
/* Read pulse width in clocks cycles, 1 to 16 clocks */
#define SLCTAC_RWIDTH(n)    (((n) & 0xF) << 8)
/* Read hold time of control and data signals, 1 to 16 clocks */
#define SLCTAC_RHOLD(n)     (((n) & 0xF) << 4)
/* Read setup time of control and data signals, 1 to 16 clocks */
#define SLCTAC_RSETUP(n)    (((n) & 0xF) << 0)

/**********************************************************************
* slc_ecc register definitions
**********************************************************************/
/* ECC line party fetch macro */
#define SLCECC_TO_LINEPAR(n) (((n) >> 6) & 0x7FFF)
#define SLCECC_TO_COLPAR(n)  ((n) & 0x3F)

/* Macro pointing to SLC NAND controller registers */
#define SLCNAND ((SLCNAND_REGS_T *)(SLC_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SLCNAND_H */
