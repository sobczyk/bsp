/***********************************************************************
 * $Id:: nor_erase.c 3452 2010-05-10 18:33:56Z usb10132 $
 *
 * Project: Simple NOR erase program
 *
 * Description:
 *     Erases the start of NOR to prevent NOR boot.
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
#include "board_nor_flash_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

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
 * Returns: Always returns 1, or <0 on an error
 *
 * Notes: None
 *
 **********************************************************************/
void c_entry(void)
{
  uart_output_init();

  /* Initialize NOR */	
  board_nor_init(EMC_STC_MEMWIDTH_32);

  /* Disable write protect */
  nor_flash_wp_disable();

  /* Erase NOR Flash for s1l Image */
  uart_output((UNS_8 *) "Erasing chip.....\r\n");
  board_nor_erase_chip(); 	
  uart_output((UNS_8 *) "..Done\r\n");	

	/* Loop forever */
	while (1);
}
