/***********************************************************************
 * $Id:: board_init.c 3376 2010-05-05 22:28:09Z usb10132               $
 *
 * Project: Board startup code
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
#include "lpc32xx_chip.h"

/***********************************************************************
 *
 * Function: board_init
 *
 * Purpose: Main startup code entry point, called from reset entry code
 *
 * Processing:
 *     Call the individual board init functions to setup the system.
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
void board_init(void)
{
	/* Setup GPIO and MUX states */
	gpio_setup();

	/* Setup system clocks and run mode */
	clock_setup(CPU_CLOCK_RATE, HCLK_DIVIDER, PCLK_DIVIDER);

	/* Setup memory */
	mem_setup();
}
