/***********************************************************************
 * $Id:: board_slc_nand_lb_driver.h 3388 2010-05-06 00:17:50Z usb10132 $
 *
 * Project: NAND device functions using the SLC (large block)
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

#ifndef BOARD_SLC_NAND_LB_DRIVER_H
#define BOARD_SLC_NAND_LB_DRIVER_H

#include "lpc_types.h"
#include "nand_support_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * NAND support functions (large block)
 **********************************************************************/

/* Initialize NAND and get NAND gemoetry */
INT_32 nand_lb_slc_init(void);

/* Erase a NAND block */
INT_32 nand_lb_slc_erase_block(UNS_32 block);

/* Mark a block as bad */
void nand_lb_slc_mark_bad_block(UNS_32 block);

/* Check is a passed block number is bad */
INT_32 nand_lb_slc_is_block_bad(UNS_32 block);

/* Read a NAND sector using hardware ECC, returns -1 on failure or
   >0 on pass */
INT_32 nand_lb_slc_read_sector(UNS_32 sector, UNS_8 *readbuff,
                               UNS_8 *spare);

/* Write a NAND sector using hardware ECC, returns -1 on failure or
   >0 on pass */
INT_32 nand_lb_slc_write_sector(UNS_32 sector, UNS_8 *writebuff,
                                UNS_8 *spare);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_SLC_NAND_LB_DRIVER_H */
