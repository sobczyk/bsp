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
#include "s1l_sys.h"
#include "s1l_sys_inf.h"

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
}
