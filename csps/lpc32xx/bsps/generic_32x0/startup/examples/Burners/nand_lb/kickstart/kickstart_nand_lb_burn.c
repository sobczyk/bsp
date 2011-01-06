/***********************************************************************
 * $Id:: kickstart_nand_lb_burn.c 3464 2010-05-13 23:21:20Z usb10132  $
 *
 * Project: MLC burner for kickstart or stage 1
 *
 * Description:
 *     This version programs the kickstart or stage 1 application using
 *     the MLC with large block support.
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
#include "board_mlc_nand_lb_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

/* NAND read and write buffers */
static UNS_32 rdbuff[2112 / 4], wrbuff[2112 / 4];
static UNS_16 *wrbuff16;
static UNS_8 *rdbuff8, *wrbuff8;

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
 * Returns: -1 on pass, or failed index
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
 * Purpose: Write data to a large block device
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
void nand_write_sectors(int sector, UNS_8 *wrbuffer, int size)
{
		while (size > 0) 
		{
			nand_lb_mlc_write_sector(sector, wrbuffer);
			wrbuffer = wrbuffer + 2048;
			size = size - 2048;
			sector++;
		}
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
		while (size > 0) 
		{
			nand_lb_mlc_read_sector(sector, rdbuffer);
			idx = isblksame(rdbuffer, wrbuffer);
			if (idx >= 0)
  			{
				uart_output((UNS_8 *) "Verify error.\r\n");
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
  UNS_32 ptw, *a1, sector, progblk;
  UNS_32 loadsize, loadcount;
  UNS_8 tmp16, tmp16i;
  UNS_8 *p8, tmp [16];
  int idx = 0;
  
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
  uart_output((UNS_8 *) "o");

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
  uart_output((UNS_8 *) "t");

  if (loadsize > (54 * 1024))
  {
   	uart_output((UNS_8 *) "Image too large for kickstart, 54K max!\r\n");
	while (1);
  }

  /* Init NAND controller */
  if (nand_lb_mlc_init() == 0) 
  {
   	uart_output((UNS_8 *) "Cannot initialize NAND device\r\n");
	while (1);
  }

  /* Disable write protect */
  nand_flash_wp_disable();

  wrbuff16 = (UNS_16 *) wrbuff;
  rdbuff8 = (UNS_8 *) rdbuff;
  wrbuff8 = (UNS_8 *) wrbuff;

  /* Setup a write block with a bad block marker in case it is
     needed */
  memset(wrbuff, 0xFF, 2112);
  wrbuff8[NAND_LB_BADBLOCK_OFFS] = (UNS_8) ~NAND_GOOD_BLOCK_MARKER;

  uart_output((UNS_8 *) "Formatting blocks...\r\n");
  progblk = 0;
  sector = nand_bp_to_sector(progblk, 0);

  /* Get sector address for first page of block */
  if (nand_lb_mlc_erase_block(progblk) == 0)
  {
	/* Erase failure, mark the block as bad */
	uart_output((UNS_8 *) "Erase failure\r\n");	
   	nand_lb_mlc_write_sector(sector, wrbuff8);
  }

  /* Really, Block is reased? */
  nand_lb_mlc_read_sector(sector, rdbuff8);
  idx = compdata(rdbuff8, 0xFF);
  if(idx >= 0)
  {
		uart_output((UNS_8 *) "Erase failure\r\n");	
		nand_lb_mlc_write_sector(sector, wrbuff8);
		while (1);
  }

  /* NAND is setup and ready */
  uart_output((UNS_8 *) "Format complete\r\n");

  /* Setup the write buffer for page #0 ICR data */
  memset(wrbuff, 0xFF, 2112);

  /* Setup FLASH config for large block, 5 address */
  tmp16 = 0x96;
  tmp16i = 0xFF - tmp16;
  wrbuff16[0] = tmp16;
  wrbuff16[2] = tmp16i;
  wrbuff16[4] = tmp16;
  wrbuff16[6] = tmp16i;

  ptw = loadsize / nandgeom.data_bytes_per_page;
  if ((ptw * nandgeom.data_bytes_per_page) < loadsize)
  {
	ptw++;
  }
  ptw++; /* Include non-used sector */

  tmp16 = (UNS_16) ptw;
  tmp16i = 0x00FF - tmp16;
  wrbuff16[8] = tmp16;
  wrbuff16[10] = tmp16i;
  wrbuff16[12] = tmp16;
  wrbuff16[14] = tmp16i;
  wrbuff16[16] = tmp16;
  wrbuff16[18] = tmp16i;
  wrbuff16[20] = tmp16;
  wrbuff16[22] = tmp16i;
  wrbuff16[24] = 0x00AA; /* Good block marker for page #0 ICR only */

  /* Get location where to write ICR */
  progblk = 0;
  sector = nand_bp_to_sector(progblk, 0);

  /* Write ICR data to first usable block/page #0 */
  nand_lb_mlc_write_sector(sector, wrbuff8);

  /* Verify page #0 */
  nand_lb_mlc_read_sector(sector, rdbuff8);
  idx = isblksame(rdbuff8, wrbuff8);
  if (idx >= 0)
  {
	uart_output((UNS_8 *) "Error writing ICR data in page #0\r\n");
	while (1);
  }

    progblk = 0;
	sector = nand_bp_to_sector(progblk, 1);
	uart_output((UNS_8 *) "\nWritting kickstart into flash...\r\n");

  nand_write_sectors(sector, p8, loadsize);
  uart_output((UNS_8 *) "Verifing data...");
  idx = verify_data(sector, p8, rdbuff8, loadsize);
  if(idx >= 0)
	uart_output((UNS_8 *) "...Failed\r\n");
  else
	uart_output((UNS_8 *) "...Successfully\r\n");
  
  uart_output((UNS_8 *) "NAND flash is programmed Successfully\r\n");		

  /* Loop forever */
  while (1);
}
