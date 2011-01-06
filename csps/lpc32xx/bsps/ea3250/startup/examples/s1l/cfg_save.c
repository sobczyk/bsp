/***********************************************************************
 * $Id:: cfg_save.c 3395 2010-05-06 17:57:16Z usb10132                 $
 *
 * Project: S1L config save/restore functions (common)
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         cfg_save
 *         cfg_load
 *         cfg_override
 *         cfg_user_reset
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

#include "lpc_string.h"
#include "sys.h"
#include "s1l_cfg.h"
#include "s1l_sys.h"
#include "s1l_sys_inf.h"
#include "startup.h"

#define CHECK_KEY 0x1352EADB

S1L_BOARD_CFG_T s1l_board_cfg;

static BOOL_32 cfg_size_check(void)
{
	BOOL_32 sts = TRUE;

	if (sizeof(secdat) < (sizeof(S1L_CFG_T) + sizeof(S1L_BOARD_CFG_T) +
		sizeof(UNS_32)))
	{
		sts = FALSE;
	}
	
	return sts;
}

/***********************************************************************
 *
 * Function: cfg_save
 *
 * Purpose: Save a S1L configuration
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to save
 *
 * Outputs: None
 *
 * Returns: Always returns FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_save(S1L_CFG_T *pCfg)
{
	UNS_8 *fptr;
	UNS_32 sector, blk = BL_NUM_BLOCKS - 1;
	const UNS_32 cfg_check_data = CHECK_KEY;

	if (cfg_size_check() == FALSE)
		return FALSE;

	for (blk = BL_NUM_BLOCKS - 2; blk < BL_NUM_BLOCKS; blk++)
	{
		/* This won't erase bad blocks */
		if (flash_erase_block(blk) == TRUE)
		{
			/* Format the data in the temporary sector buffer */
			fptr = (UNS_8 *) secdat;
			memcpy(fptr, pCfg, sizeof(*pCfg));
			fptr += sizeof(*pCfg);
			memcpy(fptr, &s1l_board_cfg, sizeof(s1l_board_cfg));
			fptr += sizeof(s1l_board_cfg);
			memcpy(fptr, &cfg_check_data, sizeof(cfg_check_data));

			/* Write data */
			sector = conv_to_sector(blk, 0);
			if (flash_write_sector(sector, secdat, NULL) >= 0)
				return TRUE;
		}
	}

	return FALSE;
}

/***********************************************************************
 *
 * Function: cfg_load
 *
 * Purpose: Load an S1L configuariton
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to populate
 *
 * Outputs: None
 *
 * Returns: FALSE if the structure wasn't loaded, otherwise TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_load(S1L_CFG_T *pCfg)
{
	UNS_32 cfg_check_val;
	UNS_32 sector, blk = BL_NUM_BLOCKS - 1;
	UNS_8 *fptr;

	if (cfg_size_check() == FALSE)
		return FALSE;

	for (blk = BL_NUM_BLOCKS - 2; blk < BL_NUM_BLOCKS; blk++)
	{
		/* Read data */
		sector = conv_to_sector(blk, 0);
		if (flash_read_sector(sector, secdat, NULL) >= 0)
		{
			/* Re-format data */
			fptr = (UNS_8 *) secdat;
			memcpy(pCfg, fptr, sizeof(*pCfg));
			fptr += sizeof(*pCfg);
			memcpy(&s1l_board_cfg, fptr, sizeof(s1l_board_cfg));
			fptr += sizeof(s1l_board_cfg);
			memcpy(&cfg_check_val, fptr, sizeof(cfg_check_val));

			if (cfg_check_val == CHECK_KEY)
			{
#ifdef ENABLE_CKLSWITCHING
				clock_adjust();
#endif
				return TRUE;
			}
		}
	}

	return FALSE;
}

/***********************************************************************
 *
 * Function: cfg_override
 *
 * Purpose: Return override state for saved config
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_override(void)
{
	return FALSE;
}

/***********************************************************************
 *
 * Function: cfg_user_reset
 *
 * Purpose: Reset user configuration data
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
void cfg_user_reset(void)
{
	s1l_board_cfg.armclk = CPU_CLOCK_RATE;
	s1l_board_cfg.hclkdiv = HCLK_DIVIDER;
	s1l_board_cfg.pclkdiv = PCLK_DIVIDER;

#ifdef ENABLE_CKLSWITCHING
	clock_adjust();
#endif
}
