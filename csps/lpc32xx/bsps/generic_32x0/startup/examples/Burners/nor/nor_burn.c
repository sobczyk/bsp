/***********************************************************************
 * $Id:: nor_burn.c 3400 2010-05-06 18:09:59Z usb10132  $
 *
 * Project: NOR burner for the stage 1 Application.
 *
 * Description:
 *     This version programs the stage 1 application into NOR Flash.
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
#include "lpc32xx_gpio_driver.h"
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
  UNS_32 *a1;
  UNS_32 loadsize, loadcount;
  UNS_8 *p8, tmp [16], ret;
  UNS_32 idx = 0;

  uart_output_init();

  /* Now to download the image */
  uart_output((UNS_8 *) "X");

  /* Wait for 'p' */
  while (!uart_input(&tmp[0], 1));

  /* Wait for 8 bytes from other side */
  idx = 0;
  while (idx < 8)
  {
	  idx += uart_input(&tmp[idx], (8 - idx));
  }

  /* Send 'o' after receving 8 bytes */
  uart_output((UNS_8 *) "o");

  /* Get size of secondary file */
  a1 = (UNS_32 *) tmp;
  loadsize = a1[1];

  /* Receive complete file using UART */
  loadcount = 0;
  p8 = (UNS_8 *) BURNER_LOAD_ADDR;
  while (loadsize > loadcount)
  {
	  /* The extra 4 is for reservation of the NOR FLASH magic word */
	  loadcount += uart_input(p8 + 4 + loadcount, (loadsize - loadcount));
  }
  uart_output((UNS_8 *) "t");

  /* Initialize NOR */	
  board_nor_init(EMC_STC_MEMWIDTH_32);

  /* Disable write protect */
  nor_flash_wp_disable();

  /* Erase NOR Flash for s1l Image */
  uart_output((UNS_8 *) "Erasing chip.....\r\n");
  board_nor_erase_chip(); 	
  uart_output((UNS_8 *) "..Done\r\n");	

  /* Write Validation Word with Memory Width, note a 32-bit memory
     width is assumed here, but 8 and 16-bit sizes are also
	 possible. */
  p8[0] = 0xD0 | EMC_STC_MEMWIDTH_32;
  p8[1] = 0x9B;
  p8[2] = 0x57;
  p8[3] = 0x13;

  uart_output((UNS_8 *) "Writing S1 image into NOR flash\r\n");
  ret = board_nor_prg(BOARD_NOR_FLASH_BASE_ADDR, p8, loadsize);
  if(ret)
  {
	  uart_output((UNS_8 *) "FAILED: not able to progam flash.\r\n");
	while(1);
  }
  else
  {
	uart_output((UNS_8 *) "Verifing S1 image....\r\n");
	ret = board_nor_verify(BOARD_NOR_FLASH_BASE_ADDR, p8, loadsize);
	if(ret)
		uart_output((UNS_8 *) "S1 image verification fail.\r\n");
	else
		uart_output((UNS_8 *) "Successfully flash S1 image.\r\n");

	/* Loop forever */
	while (1);
  }
}
