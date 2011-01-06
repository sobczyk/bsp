/***********************************************************************
 * $Id:: board_mlc_nand_lb_driver.c 3380 2010-05-05 23:54:23Z usb10132 $
 *
 * Project: Large block NAND device functions using the MLC
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

#include "board_mlc_nand_lb_driver.h"
#include "nand_mlc_common.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_mlcnand.h"
#include "board_config.h"

/***********************************************************************
 *
 * Function: mlc_lb_write_address
 *
 * Purpose: Write a 5 bytes address to the MLC NAND
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to write
 *     page  : Page to write
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void mlc_lb_write_address(UNS_32 block, UNS_32 page)
{
    /* Write block and page address */
	mlc_addr((UNS_8) ((0 >> 0) & 0x00FF));
	mlc_addr((UNS_8) ((0 >> 8) & 0xFF00));
	mlc_addr((UNS_8) (((page >> 0) & 0x003F)) |
	    ((block << 6) & 0x00C0));
	mlc_addr((UNS_8) ((block >> 2) & 0x00FF));
	mlc_addr((UNS_8) ((block >> 10) & 0x0001));
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: nand_lb_mlc_init
 *
 * Purpose: Initialize NAND and get NAND gemoetry on the MLC interface
 *
 * Processing:
 *     Does nothing
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: '1' if a device was found, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_lb_mlc_init(void)
{
	UNS_8 id[4];
	volatile UNS_32 tmp;

	/* Enable MLC controller clock and setup MLC mode */
	clkpwr_setup_nand_ctrlr(0, 0, 0);
	clkpwr_clk_en_dis(CLKPWR_NAND_MLC_CLK, 1);

    /* Configure controller for no software write protection, x8 bus
       width, large block device, and 4 address words */
    MLCNAND->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;
    MLCNAND->mlc_icr = (MLC_LARGE_BLK_ENABLE | MLC_ADDR4_ENABLE);

    /* Setup MLC timing to a very conservative (slowest) timing */
    MLCNAND->mlc_lock_pr = MLC_UNLOCK_REG_VALUE;
    MLCNAND->mlc_time = (MLC_LOAD_TCEA(MLC_TCEA_TIME) |
		MLC_LOAD_TWBTRB(MLC_TWBTRB_TIME) |
        MLC_LOAD_TRHZ(MLC_TRHZ_TIME) |
		MLC_LOAD_TREH(MLC_TREH_TIME) |
        MLC_LOAD_TRP(MLC_TRP_TIME) |
		MLC_LOAD_TWH(MLC_TWH_TIME) |
		MLC_LOAD_TWP(MLC_TWP_TIME));

    /* Make sure MLC interrupts are disabled */
    MLCNAND->mlc_irq_mr = 0;

    /* Normal chip enable operation */
    MLCNAND->mlc_ceh = MLC_NORMAL_NCE;

	/* Reset NAND device and wait for ready */
	mlc_cmd(NAND_CMD_RESET);
	mlc_wait_ready();

    /* Reset buffer pointer */
    MLCNAND->mlc_rubp = 0x1;

	/* Read the device ID */
	mlc_cmd(LPCNAND_CMD_READ_ID);
	mlc_addr(0);
	tmp = MLCNAND->mlc_data [0];
	id [0] = (UNS_8) ((tmp >> 0) & 0xFF);
	id [1] = (UNS_8) ((tmp >> 8) & 0xFF);
	id [2] = (UNS_8) ((tmp >> 16) & 0xFF);
	id [3] = (UNS_8) ((tmp >> 24) & 0xFF);

	/* Verify Micron ID */
	if (id [0] == LPCNAND_VENDOR_MICRON)
	{
		if (id [3] == 0x15)
		{
		    nandgeom.num_blocks      = 2048;
		    nandgeom.pages_per_block = 64;
		    nandgeom.data_bytes_per_page = 2048;
		    nandgeom.spare_bytes_per_page = 64;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

/***********************************************************************
 *
 * Function: nand_lb_mlc_erase_block
 *
 * Purpose: Erase a NAND block
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to erase
 *
 * Outputs: None
 *
 * Returns: '1' if the block was erased, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_lb_mlc_erase_block(UNS_32 block) 
{
    INT_32 erased = 0;
    UNS_8 status;

    /* Issue block erase1 command */
    mlc_cmd(LPCNAND_CMD_ERASE1);

    /* Write block and page address */
	mlc_addr((UNS_8) ((block << 6) & 0x00C0));
	mlc_addr((UNS_8) ((block >> 2) & 0x00FF));
	mlc_addr((UNS_8) ((block >> 10) & 0x0001));

    /* Issue page erase2 command */
    mlc_cmd(LPCNAND_CMD_ERASE2);

	/* Wait for ready */
	mlc_wait_ready();

    /* Read status */
    status = mlc_get_status();
    if ((status & 0x1) == 0)
    {
    	/* Erase was good */
    	erased = 1;
    }

    return erased;
}

/***********************************************************************
 *
 * Function: nand_lb_mlc_write_sector
 *
 * Purpose: Write a NAND sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector   : Sector to write
 *     writebuff : Pointer to write buffer
 *
 * Outputs: None
 *
 * Returns: Returns 2112, or 0 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 nand_lb_mlc_write_sector(UNS_32 sector, UNS_8 *writebuff)
{
    UNS_32 bytes = 0, idx, towrite, block, page;
    UNS_8 status;
	volatile UNS_32 tmp;

    /* Translate to page/block address */
    nand_sector_to_bp(sector, &block, &page);

    /* Force nCE for the entire cycle */
    MLCNAND->mlc_ceh = MLC_NORMAL_NCE;
        
    /* Issue page write1 command */
    mlc_cmd(LPCNAND_CMD_PAGE_WRITE1);

    /* Write block and page address */
	mlc_lb_write_address(block, page);

    /* 4 sub-pages of 518 bytes each = 2072 bytes */
    for (idx = 0; idx < 4; idx++) 
    {
        /* Start encode */
        MLCNAND->mlc_enc_ecc = 1;

        /* Write 512 bytes of data */
	    towrite = 512;
	    while (towrite > 0)
    	{
			tmp = *writebuff;
			writebuff++;
			tmp |= (*writebuff << 8);
			writebuff++;
			tmp |= (*writebuff << 16);
			writebuff++;
			tmp |= (*writebuff << 24);
			writebuff++;
    		MLCNAND->mlc_data [0] = tmp;
    		towrite = towrite - 4;
	    }
   		bytes = bytes + 512;

	    /* Write 6 dummy bytes */
	    MLCNAND->mlc_data [0] = 0;
	    * (volatile UNS_16 *) &MLCNAND->mlc_data [0] = 0;
	    
	    /* Write NAND parity */
	    MLCNAND->mlc_wpr = 1;

		/* Wait for controller ready */
		while ((MLCNAND->mlc_isr & MLC_CNTRLLR_RDY_STS) == 0);
    }

    /* Issue page write2 command */
    mlc_cmd(LPCNAND_CMD_PAGE_WRITE2);

    /* Deassert nCE */
    MLCNAND->mlc_ceh = 0;

	/* Wait for device ready */
	mlc_wait_ready();

    /* Read status */
    status = mlc_get_status();
    if ((status & 0x1) != 0)
    {
    	/* Program was not good */
    	bytes = 0;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: nand_lb_mlc_read_sector
 *
 * Purpose: Read a NAND sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector   : Sector to read
 *     readbuff : Pointer to read buffer >= 2048 bytes
 *
 * Outputs: None
 *
 * Returns: Always returns 2112
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 nand_lb_mlc_read_sector(UNS_32 sector, UNS_8 *readbuff)
{
    UNS_32 bytes = 0, idx, toread, block, page;
	volatile UNS_32 tmp;
	volatile UNS_16 tmp16;

    /* Translate to page/block address */
    nand_sector_to_bp(sector, &block, &page);

    /* Force nCE for the entire cycle */
    MLCNAND->mlc_ceh = MLC_NORMAL_NCE;
        
    /* Issue page read1 command */
    mlc_cmd(LPCNAND_CMD_PAGE_READ1);

    /* Write block and page address */
	mlc_lb_write_address(block, page);
	mlc_addr((UNS_8) ((0 >> 0) & 0x00FF));

    /* Issue page read2 command */
    mlc_cmd(LPCNAND_CMD_PAGE_READ2);

	/* Wait for ready */
	mlc_wait_ready();

    /* 4 sub-blocks of 512 bytes each */
    for (idx = 0; idx < 4; idx++) 
    {
	    /* Start decode */
	    MLCNAND->mlc_dec_ecc = 1;

	    /* Read data */
    	toread = 512;
	    while (toread > 0)
    	{
			tmp = MLCNAND->mlc_data [0];
	    	*readbuff = (UNS_8) ((tmp >> 0) & 0xFF);
    		readbuff++;
    		*readbuff = (UNS_8) ((tmp >> 8) & 0xFF);
	    	readbuff++;
    		*readbuff = (UNS_8) ((tmp >> 16) & 0xFF);
    		readbuff++;
	    	*readbuff = (UNS_8) ((tmp >> 24) & 0xFF);
    		readbuff++;
    		toread = toread - 4;
	    }
	    bytes = bytes + 512;

	    /* Read 6 dummy bytes */
	    tmp = MLCNAND->mlc_data [0];
	    tmp16 = * (volatile UNS_16 *) &MLCNAND->mlc_data [0];

        /* Write read parity register */
	    MLCNAND->mlc_rpr = 1;

		/* Wait for ECC ready */
		while ((MLCNAND->mlc_isr & MLC_ECC_RDY_STS) == 0);
    }

    /* Deassert nCE */
    MLCNAND->mlc_ceh = 0;

	/* Wait for device ready */
	mlc_wait_ready();

    return bytes;
}
