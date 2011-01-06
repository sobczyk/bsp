/***********************************************************************
 * $Id:: kickstart_lb_nand.c 3418 2010-05-06 19:59:34Z usb10132        $
 *
 * Project: Kickstart loader from large block NAND FLASH
 *
 * Description:
 *     This file contains a sample kickstart loader that can be used
 *     to load a larger stage 1 application from large block NAND
 *     FLASH.
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

#include "board_slc_nand_lb_driver.h"
#include "startup.h"
#include "misc_config.h"

static UNS_8 tmpbuff [2048+64];

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
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
void c_entry(void) {
	UNS_8 *p8, ret;
	INT_32 toread, idx, blk, page, sector;
	PFV execa = (PFV) STAGE1_LOAD_ADDR;

	/* Initialize NAND FLASH */
	if (nand_lb_slc_init() != 1) 
	{
		while (1);
	}

	/* Read data into memory */
	toread = STAGE1_LOAD_SIZE;
	blk = 1;
	page = 0;
	p8 = (UNS_8 *) STAGE1_LOAD_ADDR;
	while (toread > 0) 
	{
		ret = nand_lb_slc_is_block_bad(blk);
		if (ret == 0)
		{
			while(page < nandgeom.pages_per_block)
			{
				sector = nand_bp_to_sector(blk, page);
				nand_lb_slc_read_sector(sector, tmpbuff,NULL);
				for (idx = 0; idx < 2048; idx++) 
				{
					*p8 = tmpbuff [idx];
					p8++;
				}
				page++;
				toread = toread - 2048;
			}
			blk++;
			page = 0;
		}
		else
		{
			blk++;	
		}
	}
	
#ifdef USE_MMU
	dcache_flush();
	dcache_inval();
	icache_inval();
#endif

	execa();
}
