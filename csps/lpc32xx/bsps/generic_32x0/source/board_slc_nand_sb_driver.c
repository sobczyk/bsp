/***********************************************************************
 * $Id:: board_slc_nand_sb_driver.c 4917 2010-09-14 16:16:37Z ing03005 $
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

#include "board_slc_nand_sb_driver.h"
#include "nand_slc_common.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_slcnand.h"
#include "board_config.h"
#include <string.h>
#include "lpc_lbecc.h"

static UNS_32 ecc_data[2];
#define NUM_OF_DMA_DESC 5
static DMAC_LL_T dma_desc[NUM_OF_DMA_DESC];

/***********************************************************************
 *
 * Function: slc_start_dma
 *
 * Purpose: Enables DMA controler and starts transfer using channel 0
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     desc   : Pointer to first DMA descriptor
 *     config : DMA Configuration
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void slc_start_dma(DMAC_LL_T * desc, UNS_32 config)
{
  DMAC_REGS_T * pdmaregs;

  /* We use DMA Channel 0 */
  pdmaregs = (DMAC_REGS_T *) DMA_BASE;
  pdmaregs->int_tc_clear = _BIT(0);
  pdmaregs->int_err_clear = _BIT(0);
  pdmaregs->config = DMAC_CTRL_ENABLE;
  pdmaregs->dma_chan[0].src_addr = desc->dma_src;
  pdmaregs->dma_chan[0].dest_addr = desc->dma_dest;
  pdmaregs->dma_chan[0].lli = (UNS_32)desc->next_lli;
  pdmaregs->dma_chan[0].control = desc->next_ctrl;
  pdmaregs->dma_chan[0].config_ch = config;
}

/***********************************************************************
 *
 * Function: slc_sb_write_address
 *
 * Purpose: Write a 4 bytes address to the SLC NAND
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     page  : Page to write
 *     block : Block to write
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void slc_sb_write_address(UNS_32 block, UNS_32 page)
{
  UNS_32 nandaddr;

  /* Block  Page  Index */
  /* 31..13 12..8 7..0  */

  nandaddr = (page & 0x1F) << 8;
  nandaddr = nandaddr | ((block & 0xFFF) << 13);

  /* Write block and page address */
  slc_addr((UNS_8)(nandaddr >> 0)  & 0xFF);
  slc_addr((UNS_8)(nandaddr >> 8)  & 0xFF);
  slc_addr((UNS_8)(nandaddr >> 16) & 0xFF);
  if (nandgeom.address_cycles == 4)
  {
    slc_addr((UNS_8)(nandaddr >> 24) & 0xFF);
  }
}

/***********************************************************************
 *
 * Function: nand_sb_write_blk_addr
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
static void nand_sb_write_blk_addr(UNS_32 block)
{
	/* Generate block address */
	slc_addr((UNS_8) ((block << 5) & 0x00E0));
	slc_addr((UNS_8) ((block >> 3) & 0x00FF));
	if (nandgeom.address_cycles == 4)
	{
		slc_addr((UNS_8) ((block >> 11) & 0x0003));
	}
}

/***********************************************************************
 *
 * Function: slc_dma_write
 *
 * Purpose: Set up DMA for a NAND write operation with ECC support
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *        dmasrc: DMA source address (NAND data to write)
 *        spare: Pointer to spare area to write
 *
 * Outputs: None
 *
 * Returns: Always returns 0
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 slc_dma_write(UNS_32 dmasrc, UNS_8 *spare)
{
  UNS_32 *spare_ecc1 = &ecc_data[0];
  UNS_32 *spare_ecc2 = &ecc_data[1];

  /* Write first 256 Bytes of data from Memory to Flash */
  dma_desc[0].dma_src = (UNS_32) dmasrc;
  dma_desc[0].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[0].next_lli = ((UNS_32) &dma_desc[1]);
  dma_desc[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
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
   */
  dma_desc[1].dma_src = (UNS_32) &SLCNAND->slc_ecc;
  dma_desc[1].dma_dest = (UNS_32) spare_ecc1;
  dma_desc[1].next_lli = ((UNS_32) &dma_desc[2]);
  dma_desc[1].next_ctrl = (0x5 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
					DMAC_CHAN_SRC_AHB1);

  /* Write second 256 Bytes of data from Memory to Flash */
  dma_desc[2].dma_src = (UNS_32) dmasrc + (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2);
  dma_desc[2].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[2].next_lli = ((UNS_32) &dma_desc[3]);
  dma_desc[2].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
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
   */
  dma_desc[3].dma_src = (UNS_32) & SLCNAND->slc_ecc;
  dma_desc[3].dma_dest = (UNS_32) spare_ecc2;
  dma_desc[3].next_lli = 0;
  dma_desc[3].next_ctrl = (0x5 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
					DMAC_CHAN_SRC_AHB1 |
					DMAC_CHAN_INT_TC_EN);

  /* Write Spare Area with ECC info from Memory to Flash */
  dma_desc[4].dma_src = (UNS_32 )spare;
  dma_desc[4].dma_dest = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[4].next_lli = 0;
  dma_desc[4].next_ctrl = (
	  (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
      DMAC_CHAN_SRC_BURST_4 |
      DMAC_CHAN_DEST_BURST_4 |
      DMAC_CHAN_SRC_WIDTH_32 |
      DMAC_CHAN_DEST_WIDTH_32 |
      DMAC_CHAN_DEST_AHB1 |
	  DMAC_CHAN_SRC_AHB1 |
      DMAC_CHAN_SRC_AUTOINC |
	  DMAC_CHAN_INT_TC_EN);

  /* We use DMA Channel 0 */
  slc_start_dma(dma_desc,
                    DMAC_CHAN_FLOW_D_M2P |
                    DMAC_DEST_PERIP(DMA_PERID_NAND1) |
                    DMAC_SRC_PERIP(0) |
                    DMAC_CHAN_ENABLE);
  return 0;
}

/***********************************************************************
 *
 * Function: slc_dma_read
 *
 * Purpose: Set up DMA for a NAND read operation with ECC support
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
 * Returns: Always returns 0
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 slc_dma_read(UNS_8 *dmadest, UNS_8 *spare)
{
  DMAC_REGS_T *pdmaregs;

  /* Read first 256 Bytes of data from Flash to Memory */
  dma_desc[0].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[0].dma_dest = (UNS_32) dmadest;
  dma_desc[0].next_lli = (UNS_32) &dma_desc[1];
  dma_desc[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC);

  /* Read ECC data from Register to Memory */
  dma_desc[1].dma_src = (UNS_32) &SLCNAND->slc_ecc;
  dma_desc[1].dma_dest = (UNS_32) &ecc_data[0];
  dma_desc[1].next_lli = (UNS_32) &dma_desc[2];
  dma_desc[1].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1);

  /* Read second 256 Bytes of data from Flash to Memory */
  dma_desc[2].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[2].dma_dest = (UNS_32)(dmadest  + (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2));
  dma_desc[2].next_lli = (UNS_32) &dma_desc[3];
  dma_desc[2].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC);

  /* Read ECC data from Register to Memory */
  dma_desc[3].dma_src = (UNS_32) &SLCNAND->slc_ecc;
  dma_desc[3].dma_dest = (UNS_32) &ecc_data[1];
  dma_desc[3].next_lli = (UNS_32) &dma_desc[4];
  dma_desc[3].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1);

   /* Read Spare Area Data from Flash to Memory */
  dma_desc[4].dma_src = (UNS_32) &SLCNAND->slc_dma_data;
  dma_desc[4].dma_dest = (UNS_32) &spare[0];
  dma_desc[4].next_lli = 0;
  dma_desc[4].next_ctrl = (
                    (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
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
  pdmaregs->dma_chan[0].config_ch =
                    DMAC_CHAN_FLOW_D_P2M |
                    DMAC_DEST_PERIP(0) |
                    DMAC_SRC_PERIP(DMA_PERID_NAND1) |
                    DMAC_CHAN_ENABLE;
  return 0;
}

/***********************************************************************
 *
 * Function: wait_dma
 *
 * Purpose: Function that will wait until DMA transfer is complete
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
 * Function: nand_sb_slc_init
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
INT_32 nand_sb_slc_init(void)
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
	if (id [0] == LPCNAND_VENDOR_STMICRO)
	{
		nandgeom.pages_per_block = 32;
	    nandgeom.data_bytes_per_page = 512;
	    nandgeom.spare_bytes_per_page = 16;
		nandgeom.address_cycles = 3;

		switch (id [1])
		{
			case 0x73:
			/* NAND128-A */
			nandgeom.num_blocks = 1024;
			break;

	      case 0x35:
		  case 0x75:
			/* NAND256-A */
		    nandgeom.num_blocks = 2048;
	        break;

	      case 0x36:
		  case 0x76:
			/* NAND512-A */
			nandgeom.num_blocks = 4096;
	        nandgeom.address_cycles = 4;
		    break;

	      case 0x39:
			case 0x79:
			/* NAND01G-A */
	        nandgeom.num_blocks = 8192;
		    nandgeom.address_cycles = 4;
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
 * Function: nand_sb_slc_erase_block
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
INT_32 nand_sb_slc_erase_block(UNS_32 block) 
{
	UNS_8 status;

	/* Lock access and chip select */
    slc_sb_set_cs(TRUE);
    
    /* Issue erase command with block address and wait */
    slc_cmd(LPCNAND_CMD_ERASE1);

	/* Write block address */
	nand_sb_write_blk_addr(block);
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
	nand_sb_slc_mark_bad_block(block);

	return 0;
}

/***********************************************************************
 *
 * Function: nand_sb_slc_mark_bad_block
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
void nand_sb_slc_mark_bad_block(UNS_32 block)
{
	UNS_8 i = 0;
	UNS_8 spare[16];

	/* Writing Bad Block Marker */
	memset(spare, 0xFF, sizeof(spare));
	spare[NAND_SB_BADBLOCK_OFFS - 512] = 0xFE;

	/* Lock chip select */
    slc_sb_set_cs(TRUE);

	slc_cmd(NAND_CMD_READ3);
	slc_cmd(LPCNAND_CMD_PAGE_WRITE1);

	/* Write block address */
	slc_sb_write_address(block, 0);

	/* Write to mark factory bad block marker */
	for(i = 0; i < 16; i++)
		SLCNAND->slc_data = spare[i];

	slc_cmd(LPCNAND_CMD_PAGE_WRITE2);
	slc_wait_ready();

    /* Unlock access and chip select */
    slc_sb_set_cs(FALSE);
}

/***********************************************************************
 *
 * Function: nand_sb_slc_is_block_bad
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
INT_32 nand_sb_slc_is_block_bad(UNS_32 block)
{
	INT_32 badblock;
	UNS_8 i, spare[16];

	/* Lock chip select */
    slc_sb_set_cs(TRUE);

	slc_cmd(NAND_CMD_READ3);
	
	/* Write block address */
	slc_sb_write_address(block, 0);

	/* Write to mark factory bad block marker */
	for(i = 0; i < 16; i++)
	{
		spare[i] = (UNS_8)(SLCNAND->slc_data & _BITMASK(8));
	}
	
	if(spare[NAND_SB_BADBLOCK_OFFS - 512] == NAND_GOOD_BLOCK_MARKER)
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
 * Function: nand_sb_slc_read_sector
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
 * Returns: Returns 512 or -1 if Failure
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_sb_slc_read_sector(UNS_32 sector, UNS_8 *readbuff,
							   UNS_8 *spare)
{
    UNS_32 block, page, pstrecc[2];
	UNS_8 *tmpspare;
	INT_32 ret, i;
	UNS_32 offset;
	INT_32 bytes = SMALL_BLOCK_PAGE_MAIN_AREA_SIZE;
	UNS_8 tmp[16];

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
	slc_dma_read(readbuff, tmpspare);

	/* Wait for ready */
	slc_wait_ready();

	/* Issue page read1 command */
	slc_cmd(LPCNAND_CMD_PAGE_READ1);

	/* Write address */
	slc_sb_write_address(block, page);

	/* Set transfer count */
	SLCNAND->slc_tc = SMALL_BLOCK_PAGE_SIZE;

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
	slc_ecc_copy_from_buffer(tmpspare, pstrecc, 2);

	/* Detect and Correct Errors if any */
	for (i = 0; i < 2; i++)
	{
		offset = i * 256;
		ret = nand_slc_correct_ecc((UNS_32 *) &ecc_data[i],
			&pstrecc[i], readbuff + offset);
		if (ret != LPC_ECC_NOERR && ret != LPC_ECC_CORRECTED)
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
 * Function: nand_sb_slc_write_sector
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
 * Returns: Returns 512, or -1 if a write error occurs
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_sb_slc_write_sector(UNS_32 sector, UNS_8 *writebuff,
								UNS_8 *spare)
{
    UNS_32 block, page;
    UNS_8 status, tmp[16], *tmpspare;
	INT_32 bytes = SMALL_BLOCK_PAGE_MAIN_AREA_SIZE;

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

	/* Configure DMA Channel 0 for NAND Write */
	slc_dma_write((UNS_32) writebuff, tmpspare);

	/* Issue Serial Input Command */
    slc_cmd(LPCNAND_CMD_PAGE_READ1);
	slc_cmd(LPCNAND_CMD_PAGE_WRITE1);

	/* Write Block & Page address */
	slc_sb_write_address(block, page);

	/* Set transfer count */
	SLCNAND->slc_tc = SMALL_BLOCK_PAGE_SIZE;

	/* Clear ECC*/
	SLCNAND->slc_ctrl |= SLCCTRL_ECC_CLEAR;

	/* Start DMA */
	SLCNAND->slc_ctrl |= SLCCTRL_DMA_START;

	/* Wait for DMA to Complete Transfer */
	wait_dma();

	slc_ecc_copy_to_buffer(tmpspare, ecc_data, 2);

	/* We use DMA Channel 0 */
	slc_start_dma(&dma_desc[4],
                    DMAC_CHAN_FLOW_D_M2P |
                    DMAC_DEST_PERIP(DMA_PERID_NAND1) |
                    DMAC_SRC_PERIP(0) |
                    DMAC_CHAN_ENABLE);

	/* Wait for completion of DMA */
	wait_dma();

	/* Issue Page Program Command to Write Data */
	slc_cmd(LPCNAND_CMD_PAGE_WRITE2);

	/* Wait for Device to ready */
	slc_wait_ready();

	/* Stop DMA & Disable HW ECC */
	SLCNAND->slc_ctrl &= ~SLCCTRL_DMA_START;
	SLCNAND->slc_cfg &= ~(SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
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
