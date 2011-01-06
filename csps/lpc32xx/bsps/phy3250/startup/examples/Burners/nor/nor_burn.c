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

/* Sector Offset Lay-out for S29AL008D Bottom Boot NOR Flash */
const UNS_32 nor_sect_offset[19] = {
	/* Sectors (0,1,2) Used for Kickstart */
	0x00000, 0x04000, 0x06000,
	/* Sectors (3) Used by U-boot for Environment Variables */
	0x08000,
	/* Sectors (4,5,6,7) Used for U-boot/S1L/E-boot Code */
	0x10000, 0x20000, 0x30000, 0x40000,
	/* Rest Sectors */
	0x50000, 0x60000, 0x70000, 0x80000,
	0x90000, 0xA0000, 0xB0000, 0xC0000,
	0xD0000, 0xE0000, 0xF0000
};

const UNS_32 nor_sect_size[19] = {
	0x04000, 0x02000, 0x02000, 0x08000, 0x10000, 0x10000, 0x10000,
	0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000, 0x10000,
	0x10000, 0x10000, 0x10000, 0x10000
};
	
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
	UNS_32 loadsize, loadcount, secaddr;
	UNS_8 *p8, tmp [16], ret;
	UNS_32 idx, offset, i, j, noOfSecs, sizettl;
	int tidx;

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
	p8 = (UNS_8 *) 0x80000000;

#ifdef FLASH_KICKSTART
	/* The extra 4 is for reservation of the NOR FLASH magic word in Sector 0*/
	offset = 4;
#else
	offset = 0;
#endif
	while (loadsize > loadcount)
	{
		loadcount += uart_input(p8 + offset + loadcount, (loadsize - loadcount));
	}
	uart_output((UNS_8 *) "t");

	/* Initialize NOR */	
	board_nor_init(EMC_STC_MEMWIDTH_32);

	/* Disable write protect */
	nor_flash_wp_disable();

#ifdef FLASH_KICKSTART
	idx = 0;
	uart_output((UNS_8 *) "Erasing kickstart sectors");
#else
	idx = 4;
	uart_output((UNS_8 *) "Erasing S1L sectors");
#endif

	/* Dynamically compute number of needed sectors */
	tidx = idx;
	sizettl = noOfSecs = 0;
	while (sizettl < loadsize)
	{
		sizettl += nor_sect_size[tidx];
		tidx++;
		noOfSecs++;
	}

	/* Erase NOR Flash Sectors for Image */
	for(i = 0, j = idx; i < noOfSecs; i++, j++)
	{
		secaddr = BOARD_NOR_FLASH_BASE_ADDR + nor_sect_offset[j];
		uart_output((UNS_8 *) ".");
		ret = board_nor_erase_sector(secaddr); 
		if(ret != 0)
			uart_output((UNS_8 *) "Error: Erase fail.....\r\n");
	}
	uart_output((UNS_8 *) "done\r\n");

#ifdef FLASH_KICKSTART
	/* 
	 * Write Validation Word with Memory Width, note a 32-bit memory
     * width is assumed here, but 8 and 16-bit sizes are also possible.
	 */
	p8[0] = 0xD0 | EMC_STC_MEMWIDTH_32;
	p8[1] = 0x9B;
	p8[2] = 0x57;
	p8[3] = 0x13;
	secaddr = BOARD_NOR_FLASH_BASE_ADDR + nor_sect_offset[idx];
#else
	secaddr = BOARD_NOR_FLASH_BASE_ADDR + nor_sect_offset[idx];
#endif

	uart_output((UNS_8 *) "Writing image into NOR flash\r\n");
	ret = board_nor_prg(secaddr, p8, loadsize);
	if(ret)
	{
		uart_output((UNS_8 *) "FAILED: not able to progam flash.\r\n");
	}
	else
	{
		uart_output((UNS_8 *) "Verifing image....\r\n");
		ret = board_nor_verify(secaddr, p8, loadsize); 
		if(ret)
			uart_output((UNS_8 *) "Error: image verification fail.\r\n");
		else
			uart_output((UNS_8 *) "Successfully flash image.\r\n");
	}
	while(1);
}
