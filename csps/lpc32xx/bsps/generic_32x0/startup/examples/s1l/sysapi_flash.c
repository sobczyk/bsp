/***********************************************************************
 * $Id:: sysapi_flash.c 4365 2010-08-20 16:57:27Z usb10132             $
 *
 * Project: FLASH functions
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         flash_init
 *         flash_deinit
 *         flash_read_sector
 *         flash_write_sector
 *         flash_erase_block
 *         flash_is_bad_block
 *
 * Note:
 *     These functions need to be developed for the board they will be
 *     executed on. Examples of how these functions work can be seen
 *     in the Phytec and Embedded Artists versions fo S1L. For this
 *     generic package, these functions are stubs.
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

#include "sys.h"
#include "s1l_sys.h"
#include "s1l_sys_inf.h"
#include "board_slc_nand_sb_driver.h"
#include "board_slc_nand_lb_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

/***********************************************************************
 *
 * Function: flash_init
 *
 * Purpose: Initialize NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *     Returns pointer to an initialized NAND structure if good, NULL
 *     if bad
 *
 * Notes: None
 *
 **********************************************************************/
NAND_GEOM_T *flash_init(void)
{
#ifdef S1L_SUPPORT_NAND
	nand_flash_wp_disable();

#ifdef USE_SMALL_BLOCK
	if (nand_sb_slc_init())

#else
	if (nand_lb_slc_init())
#endif
	{
		return &nandgeom;
	}
#endif

	return NULL;
}

/***********************************************************************
 *
 * Function: flash_deinit
 *
 * Purpose: De-initialize FLASH
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void flash_deinit(void)
{
}

/***********************************************************************
 *
 * Function: flash_read_sector
 *
 * Purpose: Read a NAND sector, will skip bad blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *     Returns >0 on success, -1 on fail, or -2 if the block
 *     associated with the sector is bad
 *
 * Notes:
 *     The block is read regardless of whether the block is bad or not.
 *
 **********************************************************************/
int flash_read_sector(UNS_32 sector, void *buff, void *extra)
{
#ifdef S1L_SUPPORT_NAND
	UNS_32 page, block;
	static UNS_32 lastblock = 0xffffffff;
	static int lastblockgood = 0;
	int ret;

	nand_sector_to_bp(sector, &block, &page);

	if (lastblock != block)
	{
#ifdef USE_SMALL_BLOCK
		if (nand_sb_slc_is_block_bad(block))

#else
		if (nand_lb_slc_is_block_bad(block))
#endif
		{
			lastblockgood = 0;
		}
		else
		{
			lastblockgood = 1;
		}

		lastblock = block;
	}

	/* Flush and invalidate cache as NAND uses DMA */
	dcache_flush();
	dcache_inval();

#ifdef USE_SMALL_BLOCK
	ret = nand_sb_slc_read_sector(sector, (UNS_8 *) buff,
		(UNS_8 *) extra);

#else
	ret = nand_lb_slc_read_sector(sector, (UNS_8 *) buff,
		(UNS_8 *) extra);
#endif
	if (lastblockgood == 0)
	{
		return -2;
	}

	return ret;

#else
	return -1;
#endif
}

/***********************************************************************
 *
 * Function: flash_write_sector
 *
 * Purpose: Writes a NAND sector, will skip bad blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *     Returns >0 on success, -1 on fail, or -2 if the block
 *     associated with the sector is bad
 *
 * Notes: The block will not be written if it is bad.
 *
 **********************************************************************/
int flash_write_sector(UNS_32 sector, void *buff, void *extra)
{
#ifdef S1L_SUPPORT_NAND
	UNS_32 page, block;
	static UNS_32 lastblock = 0xffffffff;
	static int lastblockgood = 0;

	nand_sector_to_bp(sector, &block, &page);

	if (lastblock != block)
	{
#ifdef USE_SMALL_BLOCK
		if (nand_sb_slc_is_block_bad(block))

#else
		if (nand_lb_slc_is_block_bad(block))
#endif
		{
			lastblockgood = 0;
		}
		else
		{
			lastblockgood = 1;
		}

		lastblock = block;
	}

	if (lastblockgood == 0)
	{
		return -2;
	}

	/* Flush cache as NAND uses DMA */
	dcache_flush();

#ifdef USE_SMALL_BLOCK
	return nand_sb_slc_write_sector(sector, (UNS_8 *) buff,
		(UNS_8 *) extra);

#else
	return nand_lb_slc_write_sector(sector, (UNS_8 *) buff,
		(UNS_8 *) extra);
#endif

#else
	return -1;
#endif
}

/***********************************************************************
 *
 * Function: flash_erase_block
 *
 * Purpose: Erase a block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block: Block number
 *
 * Outputs: None
 *
 * Returns: TRUE on good erase, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 flash_erase_block(UNS_32 block)
{
#ifdef S1L_SUPPORT_NAND
	/* Don't allow erasure of bad blocks in S1L */
#ifdef USE_SMALL_BLOCK
	if (!nand_sb_slc_is_block_bad(block))
	{
		if (nand_sb_slc_erase_block(block))
		{
			return TRUE;
		}
	}

#else
	if (!nand_lb_slc_is_block_bad(block))
	{
		if (nand_lb_slc_erase_block(block))
		{
			return TRUE;
		}
	}
#endif
#endif

	return FALSE;
}

/***********************************************************************
 *
 * Function: flash_is_bad_block
 *
 * Purpose: Erase a block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block: Block number
 *
 * Outputs: None
 *
 * Returns: TRUE if the block is bad, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 flash_is_bad_block(UNS_32 block)
{
#ifdef S1L_SUPPORT_NAND
#ifdef USE_SMALL_BLOCK
	if (nand_sb_slc_is_block_bad(block))

#else
		if (nand_lb_slc_is_block_bad(block))
#endif
	{
		return TRUE;
	}
#endif

	return FALSE;
}
