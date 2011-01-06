/***********************************************************************
 * $Id:: nand_support_common.h 3408 2010-05-06 19:33:47Z usb10132      $
 *
 * Project: Common NAND support definitions
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

#ifndef NAND_SUPPORT_COMMON_H
#define NAND_SUPPORT_COMMON_H

#include "lpc_types.h"
#include "lpc_nandflash_params.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Structure that stores the device geometry */
extern NAND_GEOM_T nandgeom;

/* Offset for factory bad block marker in large and small block NAND */
#define NAND_LB_BADBLOCK_OFFS  2048
#define NAND_SB_BADBLOCK_OFFS  518 /* 512 on some devices */

/* Good block marker flag */
#define NAND_GOOD_BLOCK_MARKER 0xFF

/***********************************************************************
 * These functions are common among all NAND drivers
 **********************************************************************/

/* Translate a block and page address to a sector */
UNS_32 nand_bp_to_sector(UNS_32 block, UNS_32 page);

/* Translate a sector address to a block and page address */
void nand_sector_to_bp(UNS_32 sector, UNS_32 *block, UNS_32 *page);

/* Return a block address from a sector address */
UNS_32 nand_sector_to_block(UNS_32 sector);

#ifdef __cplusplus
}
#endif

#endif /* NAND_SUPPORT_COMMON_H */
