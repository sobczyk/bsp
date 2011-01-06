/***********************************************************************
 * $Id:: s1app_nand_sb_burn.c 3412 2010-05-06 19:50:21Z usb10132      $
 *
 * Project: SLC burner for stage 1 application
 *
 * Description:
 *     This version programs the kickstart or stage 1 application using
 *     the SLC with small block support.
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
#include "board_slc_nand_sb_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

/* NAND read and write buffers */
static UNS_32 rdbuff[528 / 4];
static UNS_8 *rdbuff8;

/* NAND Spare Area Data */
static UNS_8 spare[16];

/***********************************************************************
 *
 * Function: isblksame
 *
 * Purpose: Verify 2 blocks are the same
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff1 : Pointer to buffer 1 to verify
 *     buff2 : Pointer to buffer 2 to verify
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 isblksame(UNS_8 *buff1, UNS_8 *buff2)
{
	INT_32 idx;

    for (idx = 0; idx < 512; idx++) 
    {
    	if (buff1[idx] != buff2[idx]) 
    	{
			return idx;
    	}
    }

    return -1;
}

/***********************************************************************
 *
 * Function: compdata
 *
 * Purpose: Verify an entire buffer has the same value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff1 : Pointer to buffer 1 to verify
 *     data : value to verify against
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 compdata(UNS_8 *buff1, UNS_8 data)
{
	INT_32 idx;

    for (idx = 0; idx < 512; idx++) 
    {
    	if (buff1[idx] != data) 
    	{
	   		return idx;
    	}
    }

    return -1;
}

/***********************************************************************
 *
 * Function: nand_write_sectors
 *
 * Purpose: Write data to a Small block device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector to write
 *     wrbuffer : Pointer to buffer to write
 *     size : Total number of bytes
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 nand_write_sectors(int sector, UNS_8 *wrbuffer, int size)
{
	INT_32 bytes = 0;
	
	while (size > 0) 
	{
		bytes = nand_sb_slc_write_sector(sector, wrbuffer, spare);
		if(bytes < 0)
			return 0;
		wrbuffer = wrbuffer + 512;
		size = size - 512;
		sector++;
	}

	return 1;
}

/***********************************************************************
 *
 * Function: verify_data
 *
 * Purpose: Verify data written to a device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector : Sector to verify
 *     rdbuffer : Pointer to buffer to verify
 *     size : Total number of bytes
 *
 * Outputs: None
 *
 * Returns: -1 on pass, or failed index
 *
 * Notes: None
 *
 **********************************************************************/
int verify_data(int sector, UNS_8 *wrbuffer, UNS_8 *rdbuffer, int size)
{
	int idx = 0;
	INT_32 bytes = 0;
		while (size > 0) 
		{
			bytes = nand_sb_slc_read_sector(sector, rdbuffer, NULL);
			if(bytes < 0)
				return 0;

			idx = isblksame(rdbuffer, wrbuffer);
			if (idx >= 0)
  			{
				uart_output((UNS_8 *)"Verify error.\r\n");
   				return idx;
   			}
 			wrbuffer = wrbuffer + 512;
   			size = size - 512;
   			sector++;
		}

		return -1;
}

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
	UNS_32 noBlocks, *a1, sector, progblk, blkcnt;
	UNS_32 loadsize, loadcount;
	UNS_8 *p8, tmp [16];
	UNS_32 good_blks[64];
	int idx = 0;
	UNS_32 blksize = 0, size = 0, offset = 0, ret;

	memset(spare, 0xFF, sizeof(spare));

    uart_output_init();

	/* Now to download the image */
	uart_output((UNS_8 *)"X");

	/* Wait for 'p' */
	while (!uart_input(&tmp[0], 1));

	/* Wait for 8 bytes from other side */
	idx = 0;
	while (idx < 8)
	{
		idx += uart_input(&tmp[idx], (8 - idx));
	}
	uart_output((UNS_8 *)"o");

	a1 = (UNS_32 *) tmp;

	/* Get size of secondary file */
	loadsize = a1[1];

	/* Receive complete file using UART */
	loadcount = 0;
	p8 = (UNS_8 *) BURNER_LOAD_ADDR;
	while (loadsize > loadcount)
	{
		loadcount += uart_input(p8 + loadcount, (loadsize - loadcount));
	}
	uart_output((UNS_8 *)"t");

	/* Init NAND controller */
	if (nand_sb_slc_init() == 0) 
	{
   		uart_output((UNS_8 *)"Cannot initialize SLC NAND device\r\n");
		while (1);
	}

    /* Disable write protect */
    nand_flash_wp_disable();

	rdbuff8 = (UNS_8 *) rdbuff;
  
	/* Calculate Required Blocks for flashing stage1 application */ 
	blksize = nandgeom.data_bytes_per_page * nandgeom.pages_per_block;
	noBlocks = loadsize /blksize;
	if ((noBlocks * blksize) < loadsize)
	{
		noBlocks++;
	}
  
	/* Erase blocks starting from 1 & leave bad blocks */
	progblk = 1;
	uart_output((UNS_8 *)"Formatting blocks...\r\n");
	blkcnt = 0;

	do 
	{
		/* Check if block is bad or not */
		ret = nand_sb_slc_is_block_bad(progblk);
		if(ret == 0)
		{
			/* Get sector address for first page of block */
			sector = nand_bp_to_sector(progblk, 0);
			if (nand_sb_slc_erase_block(progblk) == 0)
			{
				/* Erase failure */
				uart_output((UNS_8 *)"Error: Erase failure\r\n");
				while(1);
			}

			/* Really, Block is reased? */
			nand_sb_slc_read_sector(sector, rdbuff8, NULL);
			idx = compdata(rdbuff8, 0xFF);
			if(idx >= 0)
			{
				uart_output((UNS_8 *)"Error: Erase failure...\r\n");	
				while (1);
			}

			good_blks[blkcnt] = progblk;
			blkcnt++;
		}
		progblk++;
	}while(blkcnt < noBlocks);

	uart_output((UNS_8 *)"Format complete\r\n");

	uart_output((UNS_8 *)"Writting S1 image into flash...\r\n");
	offset = 0;
  
	for (blkcnt = 0; blkcnt < noBlocks; blkcnt++)
	{
		progblk = good_blks[blkcnt];
		sector = nand_bp_to_sector(progblk, 0);

		if(loadsize <= blksize)
			size = loadsize;
		else
			size = blksize;

		/* Write 1 block size or less data into Flash */
		if (nand_write_sectors(sector, p8 + offset, size) <= 0)
		{
			uart_output((UNS_8 *)"Error: Failed to write data...\r\n");
			while(1);
		}

		idx = verify_data(sector, p8 + offset, rdbuff8, size);
		if(idx >= 0)
		{
			uart_output((UNS_8 *)"...Failed\r\n");
			while(1);
		}

		loadsize -= blksize;
		offset += blksize;
		progblk++;
	}
	uart_output((UNS_8 *)"NAND flash is programmed Successfully\r\n");		

	/* Loop forever */
	while (1);
}
