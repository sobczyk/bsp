/***********************************************************************
 * $Id:: board_mlc_nand_sb_driver.h 3388 2010-05-06 00:17:50Z usb10132 $
 *
 * Project: NAND device functions using the MLC (small block)
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

#ifndef BOARD_MLC_NAND_SB_DRIVER_H
#define BOARD_MLC_NAND_SB_DRIVER_H

#include "lpc_types.h"
#include "nand_support_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * MLC NAND support functions (small block)
 **********************************************************************/

/* Initialize NAND and get NAND gemoetry, returns 0 on failure */
INT_32 nand_sb_mlc_init(void);

/* Erase a NAND block, returns 0 on failure */
INT_32 nand_sb_mlc_erase_block(UNS_32 block);

/* Read a NAND sector, always returns 512 */
UNS_32 nand_sb_mlc_read_sector(UNS_32 sector, UNS_8 *readbuff);

/* Write a NAND sector, returns 0 on failure, or 512 on pass */
UNS_32 nand_sb_mlc_write_sector(UNS_32 sector, UNS_8 *writebuff);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_MLC_NAND_SB_DRIVER_H */
