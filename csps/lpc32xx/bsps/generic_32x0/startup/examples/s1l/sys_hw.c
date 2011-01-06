/***********************************************************************
 * $Id:: sys_hw.c 3395 2010-05-06 17:57:16Z usb10132                   $
 *
 * Project: NXP PHY3250 startup code for stage 1
 *
 * Description:
 *     This file provides initialization code for the S1L.
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

#include "lpc_types.h"
#include "s1l_sys_inf.h"
#include "s1l_sys.h"
#include "sys.h"

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
	BOOL_32 goto_prompt = TRUE;

	while (1) 
	{
		/* Go to boot manager */
		boot_manager(goto_prompt);
		goto_prompt = FALSE;
	}
}
