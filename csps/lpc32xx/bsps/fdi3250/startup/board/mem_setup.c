/***********************************************************************
 * $Id:: mem_setup.c 3376 2010-05-05 22:28:09Z usb10132                $
 *
 * Project: Memory setup code
 *
 * Description:
 *     Memory interface setup for the board
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

#include "startup.h"
#include "dram_configs.h"
#include "lpc32xx_emc.h"
#include "lpc32xx_clkpwr_driver.h"
#include "board_config.h"

/***********************************************************************
 *
 * Function: mem_setup
 *
 * Purpose: Setup memory
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
void mem_setup(void)
{
	UNS_32 dramclk;

    /* Mirror IRAM at address 0x0 */
    CLKPWR->clkpwr_bootmap = CLKPWR_BOOTMAP_SEL_BIT;

	/* Enable HCLK and SDRAM bus clocks before EMC accesses */
	CLKPWR->clkpwr_sdramclk_ctrl = 0;

	/* Enable EMC interface */
	EMC->emccontrol = EMC_DYN_SDRAM_CTRL_EN;
	EMC->emcconfig = 0;

	/* Set a very long dynamic refresh time so the controller isn't
	   always busy refreshing */
	EMC->emcdynamicrefresh = 0x7ff;

	/* Intialized Low Power DDR SDRAM */
	dramclk = clkpwr_get_clock_rate(CLKPWR_SDRAMDDR_CLK);
	ddr_sdram_lp_setup(dramclk);

		/* Enable buffers in AHB ports */
	EMC->emcahn_regs [0].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
	EMC->emcahn_regs [2].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
	EMC->emcahn_regs [3].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
	EMC->emcahn_regs [4].emcahbcontrol = EMC_AHB_PORTBUFF_EN;

	/* Enable port timeouts */
	EMC->emcahn_regs [0].emcahbtimeout = EMC_AHB_SET_TIMEOUT(32);
	EMC->emcahn_regs [2].emcahbtimeout = EMC_AHB_SET_TIMEOUT(32);
	EMC->emcahn_regs [3].emcahbtimeout = EMC_AHB_SET_TIMEOUT(32);
	EMC->emcahn_regs [4].emcahbtimeout = EMC_AHB_SET_TIMEOUT(32);
}
