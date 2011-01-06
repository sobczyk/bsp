/***********************************************************************
 * $Id:: nand_support_common.c 3380 2010-05-05 23:54:23Z usb10132      $
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

#include "nand_support_common.h"

NAND_GEOM_T nandgeom;

/***********************************************************************
 *
 * Function: nand_bp_to_sector
 *
 * Purpose: Translate a block and page address to a sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block number
 *     page  : Page number
 *
 * Outputs: None
 *
 * Returns: The mapped sector number
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 nand_bp_to_sector(UNS_32 block, UNS_32 page) 
{
    return (page + (block * nandgeom.pages_per_block));
}

/***********************************************************************
 *
 * Function: nand_sector_to_bp
 *
 * Purpose: Translate a sector address to a block and page address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector number
 *     block  : Pointer to block number to fill
 *     page   : Pointer to page number
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void nand_sector_to_bp(UNS_32 sector, UNS_32 *block, UNS_32 *page)
{
    *block = sector / nandgeom.pages_per_block;
    *page = sector - (*block * nandgeom.pages_per_block);
}

/***********************************************************************
 *
 * Function: nand_sector_to_block
 *
 * Purpose: Return a block address from a sector address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector number
 *
 * Outputs: None
 *
 * Returns: The block for the opassed sector
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 nand_sector_to_block(UNS_32 sector)
{
    return (sector / nandgeom.pages_per_block);
}
