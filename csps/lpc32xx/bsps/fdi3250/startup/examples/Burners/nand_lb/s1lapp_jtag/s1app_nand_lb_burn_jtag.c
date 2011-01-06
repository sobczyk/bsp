/***********************************************************************
 * $Id:: s1app_nand_lb_burn.c 3449 2010-05-10 17:40:36Z usb10132      $
 *
 * Project: SLC burner for stage 1 application
 *
 * Description:
 *     This version programs the kickstart or stage 1 application using
 *     the SLC with large block support.
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
#include "board_slc_nand_lb_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

/* NAND read and write buffers */
static UNS_32 rdbuff[2112 / 4];
static UNS_8 *rdbuff8;

/* NAND Spare Area Data */
static UNS_8 spare[64];

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
 * Returns: -1 on pass, or failed idx
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 isblksame(UNS_8 *buff1, UNS_8 *buff2)
{
	INT_32 idx;

    for (idx = 0; idx < 2048; idx++) 
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
 * Returns: -1 on pass, or failed idx
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 compdata(UNS_8 *buff1, UNS_8 data)
{
	INT_32 idx;

    for (idx = 0; idx < 2048; idx++) 
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
		bytes = nand_lb_slc_write_sector(sector, wrbuffer,spare);
		if(bytes < 0)
			return 0;
		wrbuffer = wrbuffer + 2048;
		size = size - 2048;
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
 * Returns: -1 on pass, or failed idx
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
			bytes = nand_lb_slc_read_sector(sector, rdbuffer, NULL);
			if(bytes < 0)
				return 0;

			idx = isblksame(rdbuffer, wrbuffer);
			if (idx >= 0)
  			{
				uart_output((UNS_8 *)"Verify error.\r\n");
   				return idx;
   			}
 			wrbuffer = wrbuffer + 2048;
   			size = size - 2048;
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
	UNS_32 noBlocks, sector, progblk, blkcnt;
//	UNS_32 *a1;
	UNS_32 loadsize;
	UNS_8 *p8;
//	UNS_8 tmp[16];
	UNS_32 good_blks[64];
	int idx = 0;
	UNS_32 blksize = 0, size = 0, offset = 0, ret;
	UNS_32 temp;
	  
	memset(spare, 0xFF, sizeof(spare));

    uart_output_init();

    // Fill loading area with 0xCC's.
    memset(STAGE1_LOAD_ADDR, 0xCC, STAGE1_LOAD_SIZE);

	/* Now to download the image */
    uart_output((UNS_8 *)"Load S1L into memory using JTAG.\r\n");

    // Wait for the end of the stage1 load area to be written to
    while (1) {
        temp = *((volatile UNS_32 *)(STAGE1_LOAD_ADDR));
        if (temp != 0xCCCCCCCC)
            break;
    }

    uart_output((UNS_8 *)"Calculating S1L size ...\r\n");

    // Look for a word that is not 0xCCCCCCCC
    for (idx=STAGE1_LOAD_SIZE-8; idx>=0; idx-=4) {
        temp = *((volatile UNS_32 *)(STAGE1_LOAD_ADDR+idx));
        if (temp != 0xCCCCCCCC)
            break;
    }

    /* Get size of secondary file */
	loadsize = idx;

    uart_output((UNS_8 *)"Burning S1L ... \r\n");
    p8 = (UNS_8 *) STAGE1_LOAD_ADDR;

    /* Init NAND controller */
	if (nand_lb_slc_init() == 0) 
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
		ret = nand_lb_slc_is_block_bad(progblk);
		if(ret == 0)
		{
			/* Get sector address for first page of block */
			sector = nand_bp_to_sector(progblk, 0);
			if (nand_lb_slc_erase_block(progblk) == 0)
			{
				/* Erase failure */
				uart_output((UNS_8 *)"Error: Erase failure... (failed SLC)\r\n");
				while(1);
			}

			/* Really, Block is erased? */
			nand_lb_slc_read_sector(sector, rdbuff8, NULL);
			idx = compdata(rdbuff8,0xFF);
			if(idx >= 0)
			{
				uart_output((UNS_8 *)"Error: Erase failure... (not 0xFF's)\r\n");
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
