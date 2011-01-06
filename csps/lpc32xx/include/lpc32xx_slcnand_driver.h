/***********************************************************************
 * $Id:: lpc32xx_slcnand_driver.h 728 2008-05-08 18:24:57Z wellsk      $
 *
 * Project: LPC32xx SLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx SLC NAND
 *     controller.
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
 *********************************************************************/

#ifndef LPC32XX_SLCNAND_DRIVER_H
#define LPC32XX_SLCNAND_DRIVER_H

#include "lpc32xx_slcnand.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * SLC NAND controller device configuration commands (IOCTL commands
 * and arguments)
 **********************************************************************/

/* SLC NAND controller device commands (IOCTL commands) */
typedef enum
{
  SLC_RESET,       /* Reset the SLC NAND controller, use arg = 0 */
  SLC_ENABLE_ECC,  /* Enable of disable ECC accumulation, use arg as
                      '1' or '0' to enable or disable */
  SLC_CLEAR_ECC,   /* Reset and clear ECC accumulator, arg = 0 */
  SLC_READ_ECC,    /* Read ECC accumulator, arg = 0 */
  SLC_SET_TIMING,  /* Set SLC NAND signal timing, use arg as a
                      pointer to a structure of type SLC_TAC_T */
  SLC_ENAB_INTS,   /* Enable a SLC NAND controller interrupt, use
                      arg as the interrupt mask to enable */
  SLC_DISAB_INTS,  /* Disable a SLC NAND controller interrupt, use
                      arg as the interrupt mask to disable */
  SLC_SET_INTS,    /* Set a SLC NAND controller interrupt, use
                      arg as the interrupt mask to set */
  SLC_CLEAR_INTS,  /* Clear a SLC NAND controller interrupt, use
                      arg as the interrupt mask to clear */
  SLC_SEND_CMD,    /* Issue a series of commands to the NAND FLASH
                      device, use arg as a pointer to a structure
                      of type SLC_CMDADR_T */
  SLC_SEND_ADDR,   /* Issue a series of address bytes to the NAND
                      FLASH device, use arg as a pointer to a
                      structure of type SLC_CMDADR_T */
  SLC_STOP,        /* Stop current SLC NAND controller sequence, use
                      arg = 0 */
  SLC_GET_STATUS,  /* Get a SLC NAND controller status, use and arg
                      value of SLC_IOCTL_STS_T */
  SLC_READ_ID,     /* Read SLC NAND flash id
                      use arg = 0 */
  SLC_ERASE_BLOCK, /* Erase SLC NAND flash block,
                      use arg as block number */
  SLC_READ_SPARE,  /* Read SLC NAND flash spare, use arg as a
                      pointer to a structure of type SLC_BLOCK_PAGE_T */
  SLC_READ_PAGE,   /* Read SLC NAND flash page, use arg as a
                      pointer to a structure of type SLC_BLOCK_PAGE_T */
  SLC_WRITE_PAGE   /* Write SLC NAND flash page, use arg as a
                      pointer to a structure of type SLC_BLOCK_PAGE_T */
} SLC_IOCTL_CMD_T;

/* Command and address FIFO population structure */
typedef struct
{
  int num_items;      /* Number of command or address bytes */
  UNS_8 items[16];    /* Commands or addresses, 16 max */
} SLC_CMDADR_T;

/* SLC NAND timing control structure - used to set the number of clocks
   for each NAND signal timing component */
typedef struct
{
  UNS_32 w_rdy;   /* The time before the signal RDY is tested in terms
                     of 2 * clock cycles. After these 2*W_RDY[2:0]
                     clocks, RDY is sampled by the interface.
                     If RDY = 0, the bus sequencer stops. RDY is
                     sampled on each clock until it equals 1, then
                     the bus sequencer continues. */
  UNS_32 w_width; /* Write pulse width in clock cycles.
                     Programmable from 1 to 16 clocks. */
  UNS_32 w_hold;  /* Write hold time of ALE, CLE, CEn, and Data in
                     clock cycles. Programmable from 1 to 16
                     clocks. */
  UNS_32 w_setup; /* Write setup time of ALE, CLE, CEn, and Data in
                     clock cycles. Programmable from 1 to 16
                     clocks. */
  UNS_32 r_rdy;   /* Time before the signal RDY is tested in terms
                     of 2 * clock cycles. After these 2*R_RDY[2:0]
                     cycles, RDY is sampled by the interface.
                     If RDY = 0, the bus sequencer stops. RDY is
                     sampled on each clock until it equals 1, then
                     the bus sequencer continues. */
  UNS_32 r_width; /* Read pulse in clock cycles. Programmable from
                     1 to 16 clocks. */
  UNS_32 r_hold;  /* Read hold time of ALE, CLE, and CEn in clock
                     cycles. Programmable from 1 to 16 clocks. */
  UNS_32 r_setup; /* Read setup time of ALE, CLE, and CEn in clock
                     cycles. Programmable from 1 to 16 clocks. */
} SLC_TAC_T;

/* SLC NAND controller arguments for SLC_GET_STATUS command (IOCTL
   arguments), see argument TBD */
typedef enum
{
  DMA_FIFO_ST,     /* When used as arg, will return a '0' if SLC DMA
                      FIFO is empty, otherwise nonzero */
  SLC_FIFO_ST,     /* When used as arg, will return a '0' if SLC data
                      FIFO is empty, otherwise nonzero */
  SLC_READY_ST,    /* When used as arg, will return a '0' if SLC NAND
                      ready signal is low (busy), otherwise nonzero */
  SLC_INT_ST       /* When used as arg, will return the interrupt
                      states */
} SLC_IOCTL_STS_T;

/* SLC NAND flash block/page structure */
typedef struct
{
  DMAC_LL_T *dma;     /* DMA link list pointer
                         if 0 then dma disabled
                         else dma enabled */
  UNS_32 *ecc;        /* ECC buffer pointer
                         if 0 then ecc disabled
                         else ecc enabled */
  UNS_32 block_num;   /* Block number */
  UNS_32 page_num;    /* Page number */
  UNS_8 *buffer;      /* buffer pointer */
} SLC_BLOCKPAGE_T;

/***********************************************************************
 * SLC NAND driver API functions
 **********************************************************************/

/* Open the SLC NAND controller */
INT_32 slcnand_open(void *ipbase, INT_32 arg);

/* Close the SLC NAND controller */
STATUS slcnand_close(INT_32 devid);

/* SLC NAND controller configuration block */
STATUS slcnand_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg);

/* SLC NAND controller read function (non-DMA only) */
INT_32 slcnand_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes);

/* SLC NAND controller write function (non-DMA only) */
INT_32 slcnand_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SLCNAND_DRIVER_H */
