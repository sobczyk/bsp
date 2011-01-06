/***********************************************************************
 * $Id:: write_protect_disable.c 3391 2010-05-06 16:03:54Z usb10132    $
 *
 * Project: Burner support functions
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

/* Sample test board disable functions:
   Phytec 3250 NAND           : gpio_set_gpo_state(P3_STATE_GPO(19), 0)
   Phytec 3250 NOR            : Not software controllable (jumper JP16)
   Phytec 3250 SPI            : Not software controllable (jumper J31)
*/

/* User defined functions for disabling write protect (if needed) for
   NAND/SPI/NOR FLASH */
void nand_flash_wp_disable(void)
{
	gpio_set_gpo_state(P3_STATE_GPO(19), 0);
}

void nor_flash_wp_disable(void)
{
	;
}

void spi_flash_wp_disable(void)
{
	;
}
