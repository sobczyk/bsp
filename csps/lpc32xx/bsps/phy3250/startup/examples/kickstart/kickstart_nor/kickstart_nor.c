/***********************************************************************
 * $Id:: kickstart_nor.c 3406 2010-05-06 18:33:29Z usb10132            $
 *
 * Project: Kickstart loader from NOR FLASH
 *
 * Description:
 *     This file contains a sample kickstart loader that can be used
 *     to load a larger stage 1 application from NOR FLASH.
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

#include "lpc32xx_gpio_driver.h"
#include "startup.h"
#include "misc_config.h"

#define NOR_LOAD_ADDR 0xE0010000

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
	UNS_8 *p8;
	INT_32 toread;

	/* Holds Uboot Entry Point Address */
	PFV execa = (PFV) STAGE1_LOAD_ADDR;

	/* Holds Address of Sector4 of Flash where Uboot is stored */
	UNS_8 *nor_addr = (UNS_8 *) NOR_LOAD_ADDR;

	/* Maximum Size of Stage1 Application (Uboot) 256K */
	toread = STAGE1_LOAD_SIZE;

	/* Copy in External Memory as Uboot is linked to this address */
	p8 = (UNS_8 *) STAGE1_LOAD_ADDR;

	/* Read data from NOR flash to memory */
	while (toread > 0)
	{
		*p8 = *nor_addr;
		p8++;
		nor_addr++;
		toread--;
	}

#ifdef USE_MMU
	dcache_flush();
	dcache_inval();
	icache_inval();
#endif

	execa();
}
