/***********************************************************************
 * $Id:: lpc32xx_mlcnand_driver.h 728 2008-05-08 18:24:57Z wellsk      $
 *
 * Project: LPC32xx MLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx MLC NAND
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

#ifndef LPC32XX_MLCNAND_DRIVER_H
#define LPC32XX_MLCNAND_DRIVER_H

#include "lpc32xx_mlcnand.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * MLC NAND controller device configuration commands (IOCTL commands
 * and arguments)
 **********************************************************************/

/* MLC NAND controller device commands (IOCTL commands) */
typedef enum
{
  MLC_RESET,       /* Reset the MLC NAND controller, use arg = 0 */
  MLC_SET_TIMING,  /* Set MLC NAND signal timing, use arg as a
                      pointer to a structure of type MLC_TIMING_T */
  MLC_MASK_INTS,   /* Mask a MLC NAND controller interrupt, use
                      arg as the interrupt mask */
  MLC_SEND_CMD,    /* Issue a series of commands to the NAND FLASH
                      device, use arg as a pointer to a structure
                      of type MLC_CMDADR_T */
  MLC_SEND_ADDR,   /* Issue a series of address bytes to the NAND
                      FLASH device, use arg as a pointer to a
                      structure of type MLC_CMDADR_T */
  MLC_GET_STATUS,  /* Get a MLC NAND controller status, use and arg
                      value of MLC_IOCTL_STS_T */
  MLC_READ_ID,     /* Read MLC NAND flash id
                      use arg = 0 */
  MLC_ERASE_BLOCK, /* Erase MLC NAND flash block,
                      use arg as block number */
  MLC_READ_SPARE,  /* Read MLC NAND flash spare, use arg as a
                      pointer to a structure of type
                      MLC_BLOCK_PAGE_T */
  MLC_READ_PAGE,   /* Read MLC NAND flash page, use arg as a
                      pointer to a structure of type
                      MLC_BLOCK_PAGE_T */
  MLC_WRITE_PAGE   /* Write MLC NAND flash page, use arg as a
                      pointer to a structure of type
                      MLC_BLOCK_PAGE_T */
} MLC_IOCTL_CMD_T;

/* Command and address FIFO population structure */
typedef struct
{
  int num_items;      /* Number of command or address bytes */
  UNS_8 items[16];    /* Commands or addresses, 16 max */
} MLC_CMDADR_T;

/* MLC NAND timing control structure - used to set the number of clocks
   for each NAND signal timing component */
typedef struct
{
  UNS_32 tcea_delay; /* nCE low to dout valid (tCEA). */
  UNS_32 busy_delay; /* Read/Write high to busy (tWB/tRB). */
  UNS_32 nand_ta;    /* Read high to high impedance (tRHZ). */
  UNS_32 r_high;     /* Read high hold time (tREH) */
  UNS_32 r_low;      /* Read pulse width (tRP) */
  UNS_32 wr_high;    /* Write high hold time (tWH) */
  UNS_32 wr_low;     /* Write pulse width (tWP) */
} MLC_TIMING_T;

/* MLC NAND controller arguments for MLC_GET_STATUS command (IOCTL
   arguments), see argument TBD */
typedef enum
{
  MLC_ST,          /* When used as arg, will return the states */
  MLC_INT_ST       /* When used as arg, will return the interrupt
                      states */
} MLC_IOCTL_STS_T;

/* MLC NAND flash block/page structure */
typedef struct
{
  DMAC_LL_T *dma;     /* DMA link list pointer
                         if 0 then dma disabled
                         else dma enabled */
  UNS_32 block_num;   /* Block number */
  UNS_32 page_num;    /* Page number */
  UNS_8 *buffer;      /* buffer pointer */
} MLC_BLOCKPAGE_T;

/***********************************************************************
 * MLC NAND driver API functions
 **********************************************************************/

/* Open the MLC NAND controller */
INT_32 mlcnand_open(void *ipbase, INT_32 arg);

/* Close the MLC NAND controller */
STATUS mlcnand_close(INT_32 devid);

/* MLC NAND controller configuration block */
STATUS mlcnand_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg);

/* MLC NAND controller read function (non-DMA only) */
INT_32 mlcnand_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes);

/* MLC NAND controller write function (non-DMA only) */
INT_32 mlcnand_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_MLCNAND_DRIVER_H */
