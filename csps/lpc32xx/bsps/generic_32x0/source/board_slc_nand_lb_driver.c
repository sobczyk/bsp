/***********************************************************************
 * $Id:: board_slc_nand_lb_driver.c 4928 2010-09-15 14:26:41Z ing03005 $
 *
 * Project: Small block NAND device functions using the SLC
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

#include "board_slc_nand_lb_driver.h"
#include "nand_slc_common.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_slcnand.h"
#include "board_config.h"

static UNS_32 ecc_data[8];

/* Contains ECC Start Offset */
static UNS_8 ecc_layout[8] = {8,12,24,28,40,44,56,60};

#define NUM_OF_DMA_DESC 0x11
static DMAC_LL_T dma_desc[NUM_OF_DMA_DESC];

/***********************************************************************
 *
 * Function: slc_lb_write_address
 *
 * Purpose: Write a 4 bytes address to the SLC NAND
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     page  : Page to write
 *     block : Block to write
 *	   spare : Perform Spare read/write Operation
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void slc_lb_write_address(UNS_32 block, UNS_32 page, UNS_32 spare)
{
    /* Write block and page address */
	slc_addr((UNS_8) ((0 >> 0) & 0x00FF));
	
    if(spare)
		slc_addr((UNS_8) (0x08 & 0x00FF));
	else
		slc_addr((UNS_8) ((0 >> 8) & 0xFF00));

	slc_addr((UNS_8) (((page >> 0) & 0x003F)) |
	    ((block << 6) & 0x00C0));
	slc_addr((UNS_8) ((block >> 2) & 0x00FF));
	slc_addr((UNS_8) ((block >> 10) & 0x0003));
}

/***********************************************************************
 *
 * Function: nand_lb_write_blk_addr
 *
 * Purpose: Write block address to NAND device
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to use for address write
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void nand_lb_write_blk_addr(UNS_32 block)
{
	/* Generate block address */
	slc_addr((UNS_8) ((block << 6) & 0x00C0));
	slc_addr((UNS_8) ((block >> 2) & 0x00FF));
	slc_addr((UNS_8) ((block >> 10) & 0x0003));
}

/***********************************************************************
 *
 * Function: slc_lb_dma_write
 *
 * Purpose: To setup DMA for NAND write operation
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     dmasrc: DMA source address (NAND data to write)
 *     spare: Pointer to spare area to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes read
 *
 * Notes: 0 - on Success
 *
 **********************************************************************/
static INT_32 slc_lb_dma_write(UNS_8 *dmasrc, UNS_8 *spare)
{
	DMAC_REGS_T *pdmaregs;
	UNS_32 idx = 0;

	for (idx = 0; idx < 4; idx++)
	{
		/* Write first 256 Bytes of data from Memory to Flash */
		dma_desc[idx*4].dma_src = (UNS_32)(dmasrc + ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
										  (idx * 2)));
		dma_desc[idx*4].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
		dma_desc[idx*4].next_lli = (UNS_32) &dma_desc[(idx*4)+1];
		dma_desc[idx*4].next_ctrl = (((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
									DMAC_CHAN_SRC_BURST_4 |
									DMAC_CHAN_DEST_BURST_4 |
									DMAC_CHAN_SRC_WIDTH_32 |
									DMAC_CHAN_DEST_WIDTH_32 |
									DMAC_CHAN_DEST_AHB1 |
									DMAC_CHAN_SRC_AHB1 | 
									DMAC_CHAN_SRC_AUTOINC);
		
	   /*
	    * Copy ECC data from register to spare area memory
        * Copy Multiple times to to sync DMA with Flash Controller
		* ECC Layout: 
		*				1st 256 --> Bytes: 8,9,10,11
		*				3st 256 --> Bytes: 24,25,26,27
		*				5st 256 --> Bytes: 40,41,42,43
		*				7st 256 --> Bytes: 56,57,58,59
        */
		dma_desc[(idx*4)+1].dma_src = (UNS_32)&SLCNAND->slc_ecc;
		dma_desc[(idx*4)+1].dma_dest = (UNS_32)(&spare[ecc_layout[idx *2]]);
		dma_desc[(idx*4)+1].next_lli = (UNS_32) & dma_desc[(idx*4)+2];
		dma_desc[(idx*4)+1].next_ctrl = (0x5 |
										DMAC_CHAN_SRC_BURST_1 |
										DMAC_CHAN_DEST_BURST_1 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1 |
										DMAC_CHAN_SRC_AHB1);

		/* Write second 256 Bytes of data from Memory to Flash */	
		dma_desc[(idx*4)+2].dma_src = (UNS_32)(dmasrc +
									  ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
									  ((idx * 2) + 1)));
		dma_desc[(idx*4)+2].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
		dma_desc[(idx*4)+2].next_lli = (UNS_32) & dma_desc[(idx*4)+3];
		dma_desc[(idx*4)+2].next_ctrl = (((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
										DMAC_CHAN_SRC_BURST_4 |
										DMAC_CHAN_DEST_BURST_4 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1 |
										DMAC_CHAN_SRC_AHB1 |
										DMAC_CHAN_SRC_AUTOINC);

	   /*
	    * Copy ECC data from register to spare area memory
        * Copy Multiple times to to sync DMA with Flash Controller
		* ECC Layout: 
		*				2nd 256 --> Bytes: 12,13,14,15
		*				4th 256 --> Bytes: 28,29,30,31
		*				6th 256 --> Bytes: 44,45,46,47
		*				8tht 256 --> Bytes: 60,61,62,63
        */
		dma_desc[(idx*4)+3].dma_src = (UNS_32)&SLCNAND->slc_ecc;
		dma_desc[(idx*4)+3].dma_dest = (UNS_32)(&spare[ecc_layout[(idx*2) + 1]]);
		dma_desc[(idx*4)+3].next_lli = (UNS_32) & dma_desc[(idx*4)+4];
		dma_desc[(idx*4)+3].next_ctrl = (0x5 |
										DMAC_CHAN_SRC_BURST_1 |
										DMAC_CHAN_DEST_BURST_1 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1 |
										DMAC_CHAN_SRC_AHB1);
	}
	/* Write Spare Area with ECC info from Memory to Flash */
	dma_desc[idx*4].dma_src = (UNS_32)spare;
	dma_desc[idx*4].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
	dma_desc[idx*4].next_lli = 0;
	dma_desc[idx*4].next_ctrl = ((LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
								DMAC_CHAN_SRC_BURST_4 |
								DMAC_CHAN_DEST_BURST_4 |
								DMAC_CHAN_SRC_WIDTH_32 |
								DMAC_CHAN_DEST_WIDTH_32 |
								DMAC_CHAN_DEST_AHB1 |
								DMAC_CHAN_SRC_AHB1 |
								DMAC_CHAN_SRC_AUTOINC |
								DMAC_CHAN_INT_TC_EN);

	/* We use DMA Channel 0 */
	pdmaregs = (DMAC_REGS_T *) DMA_BASE;
	pdmaregs->int_tc_clear = _BIT(0);
	pdmaregs->int_err_clear = _BIT(0);
	pdmaregs->config = DMAC_CTRL_ENABLE;
	pdmaregs->dma_chan[0].src_addr = dma_desc[0].dma_src;
	pdmaregs->dma_chan[0].dest_addr = dma_desc[0].dma_dest;
	pdmaregs->dma_chan[0].lli = (UNS_32)dma_desc[0].next_lli;
	pdmaregs->dma_chan[0].control = dma_desc[0].next_ctrl;
	pdmaregs->dma_chan[0].config_ch = (DMAC_CHAN_FLOW_D_M2P |
								DMAC_DEST_PERIP(DMA_PERID_NAND1) |
								DMAC_SRC_PERIP(0) |
								DMAC_CHAN_ENABLE);
  return 0;
}

/***********************************************************************
 *
 * Function: slc_lb_dma_read
 *
 * Purpose: To setup DMA for NAND read operation
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *        dmadest: DMA destination address (NAND data to fill)
 *        spare: Pointer to spare area to fill
 *
 * Outputs: None
 *
 * Returns: Number of bytes read
 *
 * Notes: 0 - on Success
 *
 **********************************************************************/
static INT_32 slc_lb_dma_read(UNS_8 *dmadest, UNS_8 *spare)
{
	DMAC_REGS_T *pdmaregs;
	UNS_32 idx = 0;

	for (idx = 0; idx < 4; idx++)
	{
		/* Read first 256 Bytes of data from Flash to Memory */
		dma_desc[idx*4].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
		dma_desc[idx*4].dma_dest = (UNS_32) (dmadest + ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
											(idx * 2)));
		dma_desc[idx*4].next_lli = (UNS_32) &dma_desc[(idx*4)+1];
		dma_desc[idx*4].next_ctrl = (((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
										DMAC_CHAN_SRC_BURST_4 |
										DMAC_CHAN_DEST_BURST_4 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1 |
										DMAC_CHAN_DEST_AUTOINC);

		/* Read ECC data from Register to Memory */
		dma_desc[(idx*4)+1].dma_src = (UNS_32) &SLCNAND->slc_ecc;
		dma_desc[(idx*4)+1].dma_dest = (UNS_32) &ecc_data[idx*2];
		dma_desc[(idx*4)+1].next_lli = (UNS_32) &dma_desc[(idx*4)+2];
		dma_desc[(idx*4)+1].next_ctrl = (0x5 |
										DMAC_CHAN_SRC_BURST_1 |
										DMAC_CHAN_DEST_BURST_1 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1);

		/* Read second 256 Bytes of data from Flash to Memory */
		dma_desc[(idx*4)+2].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
		dma_desc[(idx*4)+2].dma_dest = (UNS_32)(dmadest +
										((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
									   ((idx * 2) + 1)));
		dma_desc[(idx*4)+2].next_lli = (UNS_32) &dma_desc[(idx*4)+3];
		dma_desc[(idx*4)+2].next_ctrl = (((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
										DMAC_CHAN_SRC_BURST_4 |
										DMAC_CHAN_DEST_BURST_4 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1 |
										DMAC_CHAN_DEST_AUTOINC);

		/* Read ECC data from Register to Memory */
		dma_desc[(idx*4)+3].dma_src = (UNS_32) &SLCNAND->slc_ecc;
		dma_desc[(idx*4)+3].dma_dest = (UNS_32) &ecc_data[(idx*2)+1];
		dma_desc[(idx*4)+3].next_lli = (UNS_32) &dma_desc[(idx*4)+4];
		dma_desc[(idx*4)+3].next_ctrl = (0x5 |
										DMAC_CHAN_SRC_BURST_1 |
										DMAC_CHAN_DEST_BURST_1 |
										DMAC_CHAN_SRC_WIDTH_32 |
										DMAC_CHAN_DEST_WIDTH_32 |
										DMAC_CHAN_DEST_AHB1);
	}
	
	/* Read Spare Area Data from Flash to Memory */
	dma_desc[idx*4].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
	dma_desc[idx*4].dma_dest = (UNS_32) &spare[0];
	dma_desc[idx*4].next_lli = 0;
	dma_desc[idx*4].next_ctrl = ((LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
								DMAC_CHAN_SRC_BURST_4 |
								DMAC_CHAN_DEST_BURST_4 |
								DMAC_CHAN_SRC_WIDTH_32 |
								DMAC_CHAN_DEST_WIDTH_32 |
								DMAC_CHAN_DEST_AHB1 |
								DMAC_CHAN_DEST_AUTOINC |
								DMAC_CHAN_INT_TC_EN);


	/* We use DMA Channel 0 */
	pdmaregs = (DMAC_REGS_T *) DMA_BASE;
	pdmaregs->int_tc_clear = _BIT(0);
	pdmaregs->int_err_clear = _BIT(0);
	pdmaregs->config = DMAC_CTRL_ENABLE;
	pdmaregs->dma_chan[0].src_addr = dma_desc[0].dma_src;
	pdmaregs->dma_chan[0].dest_addr = dma_desc[0].dma_dest;
	pdmaregs->dma_chan[0].lli = (UNS_32)dma_desc[0].next_lli;
	pdmaregs->dma_chan[0].control = dma_desc[0].next_ctrl;
	pdmaregs->dma_chan[0].config_ch = (DMAC_CHAN_FLOW_D_P2M |
								DMAC_DEST_PERIP(0) |
								DMAC_SRC_PERIP(DMA_PERID_NAND1) |
								DMAC_CHAN_ENABLE);
  return 0;
}

/***********************************************************************
 *
 * Function: wait_dma
 *
 * Purpose: Function that will wait till DMA to be ready
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *          None
 *
 * Outputs: None
 *
 * Returns: Number of bytes read
 *
 * Notes: 1 - on Success, 2 - Error
 *
 **********************************************************************/
static INT_32 wait_dma(void)
{
  DMAC_REGS_T *pdmaregs = (DMAC_REGS_T *) DMA_BASE;

  while (!(pdmaregs->raw_tc_stat & 0x1) &&
         !(pdmaregs->raw_err_stat & 0x1));

  return (pdmaregs->raw_tc_stat & 0x1) == 0;
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: nand_lb_slc_init
 *
 * Purpose: Initialize NAND and get NAND gemoetry on the SLC interface
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
INT_32 nand_lb_slc_init(void)
{
	UNS_8 id[4];
		
	/* Setup SLC mode and enable SLC clock */
	CLKPWR->clkpwr_nand_clk_ctrl = (CLKPWR_NANDCLK_SEL_SLC |
		CLKPWR_NANDCLK_SLCCLK_EN);

	/* Reset SLC controller and setup for 8-bit mode, disable and
	   clear interrupts */
	SLCNAND->slc_ctrl = SLCCTRL_SW_RESET | SLCCTRL_ECC_CLEAR;
	SLCNAND->slc_cfg = 0;
	SLCNAND->slc_ien = SLCSTAT_INT_RDY_EN;

	/* Setup SLC timing */
    SLCNAND->slc_tac = (
		SLCTAC_WDR(SLC_NAND_W_RDY) |
		SLCTAC_WWIDTH(SLC_NAND_W_WIDTH) |
		SLCTAC_WHOLD(SLC_NAND_W_HOLD) |
		SLCTAC_WSETUP(SLC_NAND_W_SETUP) |
		SLCTAC_RDR(SLC_NAND_R_RDY) |
		SLCTAC_RWIDTH(SLC_NAND_R_WIDTH) |
		SLCTAC_RHOLD(SLC_NAND_R_HOLD) |
		SLCTAC_RSETUP(SLC_NAND_R_SETUP));

	/* Reset NAND device and wait for ready */
	slc_cmd(NAND_CMD_RESET);
	slc_wait_ready();

	/* Read the device ID */
	slc_cmd(LPCNAND_CMD_READ_ID);
	slc_addr(0);
	id [0] = (UNS_8) SLCNAND->slc_data;
	id [1] = (UNS_8) SLCNAND->slc_data;
	id [2] = (UNS_8) SLCNAND->slc_data;
	id [3] = (UNS_8) SLCNAND->slc_data;

	/* Verify Micron ID */
	if (id [0] == LPCNAND_VENDOR_MICRON)
	{
		switch (id [1])
		{
			case 0xDA:
				/* MT29F2G08 */
			    nandgeom.num_blocks      = 2048;
			    nandgeom.pages_per_block = 64;
				nandgeom.data_bytes_per_page = 2048;
			    nandgeom.spare_bytes_per_page = 64;
				nandgeom.address_cycles = 5;
				break;

			default:
				return 0;
				break;
		}
    }

	/* Enable DMA Controller Clock */
    clkpwr_clk_en_dis(CLKPWR_DMA_CLK, 1);

	return 1;
}

/***********************************************************************
 *
 * Function: nand_lb_slc_erase_block
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
INT_32 nand_lb_slc_erase_block(UNS_32 block) 
{
	UNS_8 status;

	/* Lock access and chip select */
    slc_sb_set_cs(TRUE);
    
    /* Issue erase command with block address and wait */
    slc_cmd(LPCNAND_CMD_ERASE1);

	/* Write block address */
	nand_lb_write_blk_addr(block);
    slc_cmd(LPCNAND_CMD_ERASE2);

	slc_wait_ready();

    /* Get NAND operation status */
    status = slc_get_status();
    
    /* Unlock access and chip select */
    slc_sb_set_cs(FALSE);
    
    if ((status & 0x1) == 0)
	{
        /* Passed */
        return 1;
    }

	/* Mark block as bad */
	nand_lb_slc_mark_bad_block(block);

	return 0;
}

/***********************************************************************
 *
 * Function: nand_lb_slc_mark_bad_block
 *
 * Purpose: Mark a block as bad
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to mark as bad
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void nand_lb_slc_mark_bad_block(UNS_32 block)
{
	UNS_8 i = 0;
	UNS_8 spare[LARGE_BLOCK_PAGE_SPARE_AREA_SIZE];

	/* Writing Bad Block Marker */
	memset(spare, 0xFF, sizeof(spare));
	spare[NAND_LB_BADBLOCK_OFFS - 2048] = 0xFE;

	/* Lock chip select */
    slc_sb_set_cs(TRUE);

	slc_cmd(LPCNAND_CMD_PAGE_WRITE1);

	/* Write block address */
	slc_lb_write_address(block, 0, 1);

	/* Write to mark factory bad block marker */
	for(i =0; i < LARGE_BLOCK_PAGE_SPARE_AREA_SIZE; i++)
	{
		SLCNAND->slc_data = spare[i];
	}

	slc_cmd(LPCNAND_CMD_PAGE_WRITE2);
	slc_wait_ready();

    /* Unlock access and chip select */
    slc_sb_set_cs(FALSE);
}

/***********************************************************************
 *
 * Function: nand_lb_slc_is_block_bad
 *
 * Purpose: Check is a passed block number is bad
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     block : Block to mark as bad
 *
 * Outputs: None
 *
 * Returns: 0 if Block is good
 *			1 if Block is bad
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_lb_slc_is_block_bad(UNS_32 block)
{
	INT_32 badblock;
	UNS_8 i, spare[LARGE_BLOCK_PAGE_SPARE_AREA_SIZE];

	/* Lock chip select */
    slc_sb_set_cs(TRUE);

	slc_cmd(LPCNAND_CMD_PAGE_READ1);
	
	/* Write block address */
	slc_lb_write_address(block, 0, 1);

	slc_cmd(LPCNAND_CMD_PAGE_READ2);

	/* Write to mark factory bad block marker */
	for(i = 0; i < LARGE_BLOCK_PAGE_SPARE_AREA_SIZE; i++)
		spare[i] = (UNS_8)(SLCNAND->slc_data & _BITMASK(8));
	
	if(spare[NAND_LB_BADBLOCK_OFFS - 2048] == NAND_GOOD_BLOCK_MARKER)
	{
		badblock = 0;
	}
	else
	{
		badblock = 1;
	}

    /* Unlock access and chip select */
    slc_sb_set_cs(FALSE);

	return badblock;
}

/***********************************************************************
 *
 * Function: nand_lb_slc_read_sector
 *
 * Purpose: Read a NAND sector
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector   : Sector to read
 *     readbuff : Pointer to read buffer
 *     spare : Pointer to spare area to fill
 *
 * Outputs: None
 *
 * Returns: Returns 2048 or -1 if Failure
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_lb_slc_read_sector(UNS_32 sector, UNS_8 *readbuff,
							   UNS_8 *spare)
{
    UNS_32 block, page, offset;
	UNS_8 *tmpspare, *pspare;
	INT_32 ret, i, bytes = LARGE_BLOCK_PAGE_MAIN_AREA_SIZE;
	UNS_8 tmp[LARGE_BLOCK_PAGE_SPARE_AREA_SIZE];

    /* Translate to page/block address */
    nand_sector_to_bp(sector, &block, &page);

	/* Set Spare Area With 0xFF if no spare area passed */
	if (spare)
	{
		tmpspare = spare;
	}
	else
	{
		tmpspare = tmp;
	}

	/* Lock access and chip select */
    slc_sb_set_cs(TRUE);

	/* Enable hardware ECC */
	SLCNAND->slc_cfg |= (SLCCFG_DMA_DIR |
						SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
						SLCCFG_DMA_ECC);

	/* Configure DMA Channel0 for NAND Read */
	slc_lb_dma_read(readbuff, tmpspare);

	/* Wait for ready */
	slc_wait_ready();

	/* Issue page read1 command */
	slc_cmd(LPCNAND_CMD_PAGE_READ1);

	/* Write address */
	slc_lb_write_address(block, page, 0);

    /* Issue page read2 command */
    slc_cmd(LPCNAND_CMD_PAGE_READ2);

	/* Wait for ready */
	slc_wait_ready();

	/* Set transfer count */
	SLCNAND->slc_tc = LARGE_BLOCK_PAGE_SIZE;

	/* Clear ECC */
	SLCNAND->slc_ctrl |= SLCCTRL_ECC_CLEAR;

	/* Start DMA */
	SLCNAND->slc_ctrl |= SLCCTRL_DMA_START;
		
	/* Wait for DMA to Complete */
	wait_dma();
	
	/* Wait for ready */
	slc_wait_ready();

	/* Stop DMA & HW ECC */
	SLCNAND->slc_ctrl &= ~SLCCTRL_DMA_START;
	SLCNAND->slc_cfg &= ~(SLCCFG_DMA_DIR |
            SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
            SLCCFG_DMA_ECC);

	/* Assign ECC Data Pointer from Spare Area */
	pspare = tmpspare;

	/* Detect and Correct Errors if any */
	for (i = 0; i < 8; i++)
	{
		offset = i * 256;
		ret = nand_slc_correct_ecc((UNS_32 *)&ecc_data[i],
			(UNS_32 *)&pspare[ecc_layout[i]], readbuff + offset);
		if(ret < 0)
		{
			bytes = -1;
		}
	}

	/* Unlock access and chip select */
	slc_sb_set_cs(FALSE);

	return bytes;
}

/***********************************************************************
 *
 * Function: nand_lb_slc_write_sector
 *
 * Purpose: Write a NAND sector using hardware ECC 
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sector    : Sector to write
 *     writebuff : Pointer to write buffer
 *     spare     : Pointer to spare data to write
 *
 * Outputs: None
 *
 * Returns: Returns 2048, or -1 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_lb_slc_write_sector(UNS_32 sector, UNS_8 *writebuff,
								UNS_8 *spare)
{
    UNS_32 block, page;
	INT_32 bytes = LARGE_BLOCK_PAGE_MAIN_AREA_SIZE;
	UNS_8 status, tmp[LARGE_BLOCK_PAGE_SPARE_AREA_SIZE], *tmpspare;

		/* Translate to page/block address */
    nand_sector_to_bp(sector, &block, &page);

	/* Set Spare Area With 0xFF if no spare area passed */
	if (spare)
	{
		tmpspare = spare;
	}
	else
	{
		memset(tmp, 0xFF, sizeof(tmp));
		tmpspare = tmp;
	}

	/* Lock access and chip select */
    slc_sb_set_cs(TRUE);

    /* Enable hardware ECC */
    SLCNAND->slc_cfg |= (SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
						SLCCFG_DMA_ECC);

	/* DMA wtite to slc */
    SLCNAND->slc_cfg &= ~SLCCFG_DMA_DIR;

	/* Configure DMA Channel0 for NAND Write */
	slc_lb_dma_write(writebuff, tmpspare);
	
	/* Wait for Device to ready */
	slc_wait_ready();

	/* Issue Serial Input Command */
	slc_cmd(LPCNAND_CMD_PAGE_WRITE1);

	/* Write Block & Page address */
	slc_lb_write_address(block, page, 0);

	/* Set transfer count */
	SLCNAND->slc_tc = LARGE_BLOCK_PAGE_SIZE;

	/* Clear ECC*/
	SLCNAND->slc_ctrl |= SLCCTRL_ECC_CLEAR;

	/* Start DMA */
	SLCNAND->slc_ctrl |= SLCCTRL_DMA_START;

	/* Wait for DMA to Complete Transfer */
	wait_dma();

	/* Issue Page Program Command to Write Data */
	slc_cmd(LPCNAND_CMD_PAGE_WRITE2);

	/* Wait for Device to ready */
	slc_wait_ready();

	/* Stop DMA & Disable HW ECC */
	SLCNAND->slc_ctrl &= ~SLCCTRL_DMA_START;
	SLCNAND->slc_cfg &= ~( SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
							SLCCFG_DMA_ECC);

	/* Get NAND operation status */
	status = slc_get_status();
	if ((status & 0x1) != 0)
	{
		/* Program was not good */
		bytes = -1;
	}

	/* Unlock access and chip select */
	slc_sb_set_cs(FALSE);
	return bytes;
}
