/***********************************************************************
 * $Id:: kickstart_spi.c 3406 2010-05-06 18:33:29Z usb10132            $
 *
 * Project: Kickstart loader from SPI FLASH
 *
 * Description:
 *     This file contains a sample kickstart loader that can be used
 *     to load a larger stage 1 application from SPI FLASH.
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
#include "board_spi_flash_driver.h"
#include "startup.h"
#include "misc_config.h"

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
	INT_32 toread, idx;
	PFV execa = (PFV) STAGE1_LOAD_ADDR;

  /* Force SSP configuration, use GPIO_05 (SSP0_CS) in software
     control mode */
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p_mux_set = P_SPI1CLK_SCK0 | P_SPI1DATAIN_SSP0_MISO |
	  P_SPI1DATAIO_SSP0_MOSI;

	board_spi_config();

	/* Read data into memory */
	toread = STAGE1_LOAD_SIZE;
	p8 = (UNS_8 *) STAGE1_LOAD_ADDR;
	idx = SPI_S1APP_OFFSET;

	while (toread > 0)
	{
		*p8 = board_spi_read(idx);
		p8++;
		idx++;
		toread--;
	}

#ifdef USE_MMU
	dcache_flush();
	dcache_inval();
	icache_inval();
#endif

	execa();
}
