/***********************************************************************
 * $Id:: nanderase.c 3452 2010-05-10 18:33:56Z usb10132               $
 *
 * Project: Simple small block NAND erase program
 *
 * Description:
 *     Erases the start of small block NAND to prevent NAND boot.
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
#include "board_mlc_nand_sb_driver.h"
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
  UNS_32 i;

  uart_output_init();

  /* Init NAND controller */
  if (nand_sb_mlc_init() == 0) 
  {
   	uart_output((UNS_8*)"Cannot initialize NAND device\r\n");
	while (1);
  }

  /* Disable write protect */
  nand_flash_wp_disable();

  uart_output((UNS_8*)"Formatting blocks 0\r\n");

  /* Erase 25 blocks */
  for (i = 0; i < 25; i++)
  {
    if (nand_sb_mlc_erase_block(i) == 0)
    {
	  /* Erase failure, mark the block as bad */
  	uart_output((UNS_8*)"Erase failure\r\n");
    }
    else
    {
      uart_output((UNS_8*)"Block erased\r\n");
    }
  }

  /* Loop forever */
  while (1);
}
