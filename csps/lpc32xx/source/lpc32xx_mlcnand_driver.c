/***********************************************************************
 * $Id:: lpc32xx_mlcnand_driver.c 1024 2008-08-06 22:24:29Z wellsk     $
 *
 * Project: LPC3xxx MLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC3xxx MLC NAND
 *     controller.
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
 *********************************************************************/

#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc_nandflash_params.h"
#include "lpc32xx_mlcnand_driver.h"

/***********************************************************************
 * MLC NAND controller driver package data
***********************************************************************/

/* MLC NAND controller device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  MLCNAND_REGS_T *regptr; /* Pointer to MLC NAND registers */
  NANDFLASH_PARAM_T *paramptr; /* Pointer to NAND flash parameters */
} MLCNAND_CFG_T;

/* MLC NAND controller driver data */
static MLCNAND_CFG_T mlcdat;

/* device handle */
static INT_32 dmach;

/* DMA list */
static DMAC_LL_T *dmalist;

/* Pointer to DMA registers */
static DMAC_REGS_T *pdmaregs;

/* completion of DMA */
static BOOL_32 dma_completion;

/* Number of DMA channels */
#define DMA_MAX_CHANNELS 8

/***********************************************************************
 * SDcard controller driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mlcnand_dma_interrupt
 *
 * Purpose: DMA controller interrupt handler
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void mlcnand_dma_interrupt(void)
{
  INT_32 ch;

  pdmaregs = dma_get_base();

  /* Check all pending interrupt statuses */
  ch = 0;
  while (ch < DMA_MAX_CHANNELS)
  {
    /* Check channel interrupt status */
    if ((pdmaregs->int_stat & _BIT(ch)) != 0)
    {
      /* Check channel terminal count interrupt status */
      if ((pdmaregs->int_tc_stat & _BIT(ch)) != 0)
      {
        pdmaregs->int_tc_clear = _BIT(ch);

        /* Select MLC NAND controller, 
                    disable DMA_REQ signal on NAND_RnB,
                    disable DMA_REQ signal on NAND_INT */
        clkpwr_setup_nand_ctrlr(0, 0, 0);

        dma_completion = TRUE;
      }

      /* Check channel error interrupt status */
      if ((pdmaregs->int_err_stat & _BIT(ch)) != 0)
      {
        pdmaregs->int_err_clear = _BIT(ch);
      }
    }
    /* Next channel */
    ch++;
  }
}

/***********************************************************************
 *
 * Function: mlcnand_interrupt
 *
 * Purpose: MLC NAND controller interrupt handler
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void mlcnand_interrupt(void)
{
  /* Clear Interrupt Status */
  while ((mlcdat.regptr->mlc_irq_sr & (MLC_INT_DEV_RDY |
                                       MLC_INT_CNTRLLR_RDY | MLC_INT_DECODE_FAIL |
                                       MLC_INT_DECODE_ERR | MLC_INT_ENCDEC_RDY |
                                       MLC_INT_SWWP_FAULT)));
}

/***********************************************************************
 *
 * Function: mlcnand_dma_read
 *
 * Purpose: Setup DMA channel for dma read from mlc
 *
 * Processing:
 *     Transfer data from mlc to memory
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     dmasrc: DMA src address
 *     dmadest: DMA dest address
 *
 * Outputs: None
 *
 * Returns: The status of DMA channel setup for dma read from mlc
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_dma_read(INT_32 devid, UNS_32 dmasrc, UNS_32 dmadest,
                        DMAC_LL_T *dma)
{
  STATUS status = _NO_ERROR;

  dma_completion = FALSE;

  dmalist = dma;

  dmalist[0].dma_src = dmasrc;
  dmalist[0].dma_dest = (UNS_32)dmadest;
  dmalist[0].next_lli = 0;
  dmalist[0].next_ctrl = ((SMALL_BLOCK_PAGE_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

  /* Get a DMA channel */
  dmach = dma_alloc_channel(-1, (PFV) mlcnand_dma_interrupt);
  if (dmach < 0)
  {
    return _ERROR;
  }

  /* setup DMA channel */
  pdmaregs = dma_get_base();
  pdmaregs->int_tc_clear = _BIT(dmach);
  pdmaregs->int_err_clear = _BIT(dmach);
  pdmaregs->dma_chan[dmach].src_addr = dmalist[0].dma_src;
  pdmaregs->dma_chan[dmach].dest_addr = dmalist[0].dma_dest;
  pdmaregs->dma_chan[dmach].lli = (UNS_32)dmalist[0].next_lli;
  pdmaregs->dma_chan[dmach].control = dmalist[0].next_ctrl;
  pdmaregs->dma_chan[dmach].config_ch = DMAC_CHAN_ITC |
                    DMAC_CHAN_IE |
                    DMAC_CHAN_FLOW_D_P2M |
                    DMAC_DEST_PERIP(0) |
                    DMAC_SRC_PERIP(DMA_PERID_NAND2) |
                    DMAC_CHAN_ENABLE;

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_dma_write
 *
 * Purpose: Setup DMA channel for dma write to mlc
 *
 * Processing:
 *     Transfer data from memory to mlc
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     dmasrc: DMA src address
 *     dmadest: DMA dest address
 *
 * Outputs: None
 *
 * Returns: The status of DMA channel setup for dma write to mlc
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_dma_write(INT_32 devid, UNS_32 dmasrc, UNS_32 dmadest,
                         DMAC_LL_T *dma)
{
  STATUS status = _NO_ERROR;

  dma_completion = FALSE;

  dmalist = dma;

  dmalist[0].dma_src = (UNS_32)dmasrc;
  dmalist[0].dma_dest = dmadest;
  dmalist[0].next_lli = (UNS_32) & dmalist[1];
  dmalist[0].next_ctrl = ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

  dmalist[1].dma_src = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE);
  dmalist[1].dma_dest = dmadest;
  dmalist[1].next_lli = (UNS_32) & dmalist[2];
  dmalist[1].next_ctrl = (1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

  dmalist[2].dma_src = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE + 4);
  dmalist[2].dma_dest = dmadest;
  dmalist[2].next_lli = 0;
  dmalist[2].next_ctrl = (1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_16 |
                    DMAC_CHAN_DEST_WIDTH_16 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

  /* Get a DMA channel */
  dmach = dma_alloc_channel(-1, (PFV) mlcnand_dma_interrupt);
  if (dmach < 0)
  {
    return _ERROR;
  }

  /* setup DMA channel */
  pdmaregs = dma_get_base();
  pdmaregs->int_tc_clear = _BIT(dmach);
  pdmaregs->int_err_clear = _BIT(dmach);
  pdmaregs->dma_chan[dmach].src_addr = dmalist[0].dma_src;
  pdmaregs->dma_chan[dmach].dest_addr = dmalist[0].dma_dest;
  pdmaregs->dma_chan[dmach].lli = (UNS_32)dmalist[0].next_lli;
  pdmaregs->dma_chan[dmach].control = dmalist[0].next_ctrl;
  pdmaregs->dma_chan[dmach].config_ch = DMAC_CHAN_ITC |
                    DMAC_CHAN_IE |
                    DMAC_CHAN_FLOW_D_M2M |
                    DMAC_DEST_PERIP(0) |
                    DMAC_SRC_PERIP(0) |
                    DMAC_CHAN_ENABLE;

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_read_id
 *
 * Purpose: Read MLC NAND flash id
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as mlc was not
 *     previously opened. Otherwise, read MLC NAND flash id.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     buf:   Pointer to data buffer to copy to
 *
 * Outputs: None
 *
 * Returns: The status of read id operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_read_id(INT_32 devid, void *buf)
{
  STATUS status = _ERROR;
  UNS_32 *dwptr = buf;
  int idx;

  if (mlcdat.init == TRUE)
  {
    status = _NO_ERROR;

    /* ID read (1) command */
    mlcdat.regptr->mlc_cmd = NAND_CMD_READID;

    /* Column Address */
    mlcdat.regptr->mlc_addr = 0;

    /* Wait for MLC NAND ready */
    while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

    /* Read ID_SIZE/4 words from MLC NAND flash data register */
    for (idx = 0; idx < (ID_SIZE / 4); idx++)
    {
      *dwptr = (UNS_32)mlcdat.regptr->mlc_data[0];
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_erase_block
 *
 * Purpose: Erase MLC NAND flash block
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as mlc was not
 *     previously opened. Otherwise, erase MLC NAND flash block.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     block_num: Block to be erased
 *
 * Outputs: None
 *
 * Returns: The status of block erase operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_erase_block(INT_32 devid, UNS_32 block_num)
{
  STATUS status = _ERROR;

  if (mlcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (mlcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Auto block erase 1-st command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_ERASE1ST;

      /* Page Address 1st */
      mlcdat.regptr->mlc_addr = ((block_num << 5) & 0xE0);

      /* Page Address 2nd */
      mlcdat.regptr->mlc_addr = ((block_num << 5) >> 8) &
                    _BITMASK(8);

      if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
      {
        /* Page Address 3rd */
        mlcdat.regptr->mlc_addr = ((block_num << 5) >> 16) &
                    _BITMASK(8);
      }

      /* Auto block erase 2-nd command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_ERASE2ND;

      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Status read (1) command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_STATUS;

      /* Operation status */
      if (mlcdat.regptr->mlc_data[0] & NAND_FLASH_FAILED)
      {
        status = _ERROR;
      }
    }
    else if (mlcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Auto block erase 1-st command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_ERASE1ST;

      /* Page Address 1st */
      mlcdat.regptr->mlc_addr = ((block_num << 6) & 0xC0);

      /* Page Address 2nd */
      mlcdat.regptr->mlc_addr = ((block_num << 6) >> 8) &
                    _BITMASK(8);

      if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
      {
        /* Page Address 3rd */
        mlcdat.regptr->mlc_addr = ((block_num << 6) >> 16) &
                    _BITMASK(8);
      }

      /* Auto block erase 2-nd command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_ERASE2ND;

      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Status read (1) command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_STATUS;

      /* Operation status */
      if (mlcdat.regptr->mlc_data[0] & NAND_FLASH_FAILED)
      {
        status = _ERROR;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_read_spare
 *
 * Purpose: Read MLC NAND flash page spare area
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as mlc was not
 *     previously opened. Otherwise, read MLC NAND flash page spare
 *     area.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     blockpage: Pointer to MLC_BLOCKPAGE_T structure
 *
 * Outputs: None
 *
 * Returns: The status of read spare operation
 *
 * Notes:
 *     Small page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *     Large page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *
 **********************************************************************/
STATUS mlcnand_read_spare(INT_32 devid, MLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_32 *dwptr = (UNS_32 *)blockpage->buffer;
  int idx, idy;

  if (mlcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (mlcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Read mode (3) command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_READ3;

      /* Column Address */
      mlcdat.regptr->mlc_addr = 0;

      /* Page Address 1st */
      mlcdat.regptr->mlc_addr = (((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

      /* Page Address 2nd */
      mlcdat.regptr->mlc_addr = (((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

      if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
      {
        /* Page Address 3rd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
      }

      /* ECC Auto Decode */
      mlcdat.regptr->mlc_autodec_dec = 0;

      /* Wait for MLC NAND controller ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

      /* Wait for MLC NAND ECC ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

      if ((mlcdat.regptr->mlc_isr & MLC_DECODE_ERR_DETECT_STS) ||
                    (mlcdat.regptr->mlc_isr &
                    MLC_DECODE_FAIL_STS))
      {
        status = _ERROR;
      }

      /* Read SMALL_BLOCK_PAGE_SPARE_AREA_SIZE/4 words from
      				MLC NAND flash buffer register */
      for (idx = 0; idx < (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4);
           idx++)
      {
        *dwptr++ = (UNS_32)mlcdat.regptr->mlc_buff[0];
      }
    }
    else if (mlcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      /* Wait for MLC NAND ready */
      while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

      /* Read mode (3) command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_READ3;

      /* Column Address 1st */
      mlcdat.regptr->mlc_addr = 0;

      /* Column Address 2nd */
      mlcdat.regptr->mlc_addr = 0;

      /* Page Address 1st */
      mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

      /* Page Address 2nd */
      mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

      if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
      {
        /* Page Address 3rd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
      }

      /* Read start command */
      mlcdat.regptr->mlc_cmd = NAND_CMD_READSTART;

      for (idy = 0; idy < 4; idy++)
      {
        /* ECC Auto Decode */
        mlcdat.regptr->mlc_autodec_dec = 0;

        /* Wait for MLC NAND controller ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

        /* Wait for MLC NAND ECC ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

        if ((mlcdat.regptr->mlc_isr &
                    MLC_DECODE_ERR_DETECT_STS) ||
                    (mlcdat.regptr->mlc_isr &
                    MLC_DECODE_FAIL_STS))
        {
          status = _ERROR;
        }

        /* Read (LARGE_BLOCK_PAGE_SPARE_AREA_SIZE/4)/4 words
        			from MLC NAND flash buffer register */
        for (idx = 0; idx <
                    ((LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) / 4);
                    idx++)
        {
          *dwptr++ = (UNS_32)mlcdat.regptr->mlc_buff[0];
        }
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_read_page
 *
 * Purpose: Read MLC NAND flash page
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as mlc was not
 *     previously opened. Otherwise, read MLC NAND flash page.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     blockpage: Pointer to MLC_BLOCKPAGE_T structure
 *
 * Outputs: None
 *
 * Returns: The status of read page operation
 *
 * Notes:
 *     Read both MLC NAND flash page main area and spare area.
 *     Small page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *     Large page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *
 **********************************************************************/
STATUS mlcnand_read_page(INT_32 devid, MLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_32 *dwptr;
  int idx, idy;

  if (mlcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (mlcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      if (blockpage->dma != 0)
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Read mode (1) command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READ1;

        /* Column Address */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* Clear Interrupt Status */
        while ((mlcdat.regptr->mlc_irq_sr & (MLC_INT_DEV_RDY |
                    MLC_INT_CNTRLLR_RDY | MLC_INT_DECODE_FAIL |
                    MLC_INT_DECODE_ERR | MLC_INT_ENCDEC_RDY)));

        /* Mask Interrupt */
        mlcdat.regptr->mlc_irq_mr = MLC_INT_CNTRLLR_RDY;

        /* Select MLC NAND controller, 
                    disable DMA_REQ signal on NAND_RnB,
                    enable DMA_REQ signal on NAND_INT */
        clkpwr_setup_nand_ctrlr(0, 0, 1);

        dwptr = (UNS_32 *)blockpage->buffer;
        mlcnand_dma_read(devid,
                    (UNS_32)&mlcdat.regptr->mlc_buff[0],
                    (UNS_32)dwptr, blockpage->dma);

        /* ECC Auto Decode */
        mlcdat.regptr->mlc_autodec_dec = 0;

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* Wait for MLC NAND ECC ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

        if (mlcdat.regptr->mlc_isr &
                    (MLC_DECODE_ERR_DETECT_STS |
                    MLC_DECODE_FAIL_STS))
        {
          status = _ERROR;
        }

        /* release the DMA Channel */
        dma_free_channel(dmach);

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Mask Interrupt */
        mlcdat.regptr->mlc_irq_mr = 0;
      }
      else
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Read mode (1) command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READ1;

        /* Column Address */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* ECC Auto Decode */
        mlcdat.regptr->mlc_autodec_dec = 0;

        /* Wait for MLC NAND controller ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

        /* Wait for MLC NAND ECC ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

        if (mlcdat.regptr->mlc_isr &
                    (MLC_DECODE_ERR_DETECT_STS |
                    MLC_DECODE_FAIL_STS))
        {
          status = _ERROR;
        }

        /* Read SMALL_BLOCK_PAGE_SIZE/4 words from
        			MLC NAND flash buffer register */
        dwptr = (UNS_32 *)blockpage->buffer;
        for (idx = 0; idx < SMALL_BLOCK_PAGE_SIZE / 4; idx++)
        {
          *dwptr++ = (UNS_32)mlcdat.regptr->mlc_buff[0];
        }
      }
    }
    else if (mlcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      if (blockpage->dma != 0)
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Read mode (1) command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READ1;

        /* Column Address 1st */
        mlcdat.regptr->mlc_addr = 0;

        /* Column Address 2nd */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Read start command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READSTART;

        for (idy = 0; idy < 4; idy++)
        {
          /* Clear Interrupt Status */
          while ((mlcdat.regptr->mlc_irq_sr &
                    (MLC_INT_DEV_RDY |
                    MLC_INT_CNTRLLR_RDY | MLC_INT_DECODE_FAIL |
                    MLC_INT_DECODE_ERR | MLC_INT_ENCDEC_RDY)));

          /* Mask Interrupt */
          mlcdat.regptr->mlc_irq_mr = MLC_INT_CNTRLLR_RDY;

        /* Select MLC NAND controller, 
                    disable DMA_REQ signal on NAND_RnB,
                    enable DMA_REQ signal on NAND_INT */
          clkpwr_setup_nand_ctrlr(0, 0, 1);

          dwptr = (UNS_32 *)(blockpage->buffer +
                    (LARGE_BLOCK_PAGE_SIZE / 4) * idy);
          mlcnand_dma_read(devid,
                    (UNS_32)&mlcdat.regptr->mlc_buff[0],
                    (UNS_32)dwptr, blockpage->dma);

          /* ECC Auto Decode */
          mlcdat.regptr->mlc_autodec_dec = 0;

          /* Wait for completion of DMA */
          while (dma_completion == FALSE);

          /* Wait for MLC NAND ECC ready */
          while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

          if (mlcdat.regptr->mlc_isr &
                    (MLC_DECODE_ERR_DETECT_STS |
                    MLC_DECODE_FAIL_STS))
          {
            status = _ERROR;
          }

          /* release the DMA Channel */
          dma_free_channel(dmach);
        }

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Mask Interrupt */
        mlcdat.regptr->mlc_irq_mr = 0;
      }
      else
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Read mode (1) command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READ1;

        /* Column Address 1st */
        mlcdat.regptr->mlc_addr = 0;

        /* Column Address 2nd */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Read start command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_READSTART;

        for (idy = 0; idy < 4; idy++)
        {
          /* ECC Auto Decode */
          mlcdat.regptr->mlc_autodec_dec = 0;

          /* Wait for MLC NAND controller ready */
          while (!(mlcdat.regptr->mlc_isr &
                    MLC_CNTRLLR_RDY_STS));

          /* Wait for MLC NAND ECC ready */
          while (!(mlcdat.regptr->mlc_isr & MLC_ECC_RDY_STS));

          if (mlcdat.regptr->mlc_isr &
                    (MLC_DECODE_ERR_DETECT_STS |
                    MLC_DECODE_FAIL_STS))
          {
            status = _ERROR;
          }

          /* Read (LARGE_BLOCK_PAGE_SIZE/4)/4 words from
          		MLC NAND flash buffer register */
          dwptr = (UNS_32 *)(blockpage->buffer +
                    (LARGE_BLOCK_PAGE_SIZE / 4) * idy);
          for (idx = 0; idx < ((LARGE_BLOCK_PAGE_SIZE / 4) / 4);
                    idx++)
          {
            *dwptr++ = (UNS_32)mlcdat.regptr->mlc_buff[0];
          }
        }
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_write_page
 *
 * Purpose: Write MLC NAND flash page
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as mlc was not
 *     previously opened. Otherwise, write MLC NAND flash page.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     blockpage: Pointer to MLC_BLOCKPAGE_T structure
 *
 * Outputs: None
 *
 * Returns: The status of write page operation
 *
 * Notes:
 *     Write both MLC NAND flash page main area and spare area.
 *     Small page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *     Large page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *
 **********************************************************************/
STATUS mlcnand_write_page(INT_32 devid, MLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_32 *dwptr;
  int idx, idy;

  if (mlcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (mlcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      if (blockpage->dma != 0)
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Serial data input command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_SDIN;

        /* Column Address */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* ECC Encode */
        mlcdat.regptr->mlc_enc_ecc = 0;

        dwptr = (UNS_32 *)blockpage->buffer;
        mlcnand_dma_write(devid, (UNS_32)dwptr,
                    (UNS_32)&mlcdat.regptr->mlc_buff[0],
                    blockpage->dma);

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* ECC Auto Encode */
        mlcdat.regptr->mlc_autoenc_enc = 0;

        /* Wait for MLC NAND controller ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

        /* release the DMA Channel */
        dma_free_channel(dmach);

        /* Auto program command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));
      }
      else
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Serial data input command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_SDIN;

        /* Column Address */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* ECC Encode */
        mlcdat.regptr->mlc_enc_ecc = 0;

        /* Write SMALL_BLOCK_PAGE_MAIN_AREA_SIZE/4 words to
        			MLC NAND flash buffer register */
        dwptr = (UNS_32 *)blockpage->buffer;
        for (idx = 0; idx < (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 4);
                    idx++)
        {
          mlcdat.regptr->mlc_buff[0] = *dwptr++;
        }

        /* Write 1 word to MLC NAND flash buffer register */
        mlcdat.regptr->mlc_buff[0] = *dwptr++;

        /* Write 1 halfword to MLC NAND flash buffer register */
        *(volatile UNS_16 *)&mlcdat.regptr->mlc_buff[0] =
                    *(UNS_16 *)dwptr++;

        /* ECC Auto Encode */
        mlcdat.regptr->mlc_autoenc_enc = 0;

        /* Wait for MLC NAND controller ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

        /* Auto program command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));
      }
    }
    else if (mlcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      if (blockpage->dma != 0)
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Serial data input command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_SDIN;

        /* Column Address 1st */
        mlcdat.regptr->mlc_addr = 0;

        /* Column Address 2nd */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        for (idy = 0; idy < 4; idy++)
        {
          /* ECC Encode */
          mlcdat.regptr->mlc_enc_ecc = 0;

          dwptr = (UNS_32 *)(blockpage->buffer +
                    (LARGE_BLOCK_PAGE_SIZE / 4) * idy);
          mlcnand_dma_write(devid, (UNS_32)dwptr,
                    (UNS_32)&mlcdat.regptr->mlc_buff[0],
                    blockpage->dma);

          /* Wait for completion of DMA */
          while (dma_completion == FALSE);

          /* ECC Auto Encode */
          mlcdat.regptr->mlc_autoenc_enc = 0;

          /* Wait for MLC NAND controller ready */
          while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));

          /* release the DMA Channel */
          dma_free_channel(dmach);
        }

        /* Auto program command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));
      }
      else
      {
        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Serial data input command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_SDIN;

        /* Column Address 1st */
        mlcdat.regptr->mlc_addr = 0;

        /* Column Address 2nd */
        mlcdat.regptr->mlc_addr = 0;

        /* Page Address 1st */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        mlcdat.regptr->mlc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          mlcdat.regptr->mlc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        for (idy = 0; idy < 4; idy++)
        {
          /* ECC Encode */
          mlcdat.regptr->mlc_enc_ecc = 0;

          /* Write (LARGE_BLOCK_PAGE_MAIN_AREA_SIZE/4)/4 words
          		to MLC NAND flash buffer register */
          dwptr = (UNS_32 *)(blockpage->buffer +
                    (LARGE_BLOCK_PAGE_SIZE / 4) * idy);
          for (idx = 0; idx <
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 4) / 4);
                    idx++)
          {
            mlcdat.regptr->mlc_buff[0] = *dwptr++;
          }

          /* Write 1 word to MLC NAND flash buffer register */
          mlcdat.regptr->mlc_buff[0] = *dwptr++;

          /* Write 1 halfword to MLC NAND flash buffer register */
          *(volatile UNS_16 *)&mlcdat.regptr->mlc_buff[0] =
                    *(UNS_16 *)dwptr++;

          /* ECC Auto Encode */
          mlcdat.regptr->mlc_autoenc_enc = 0;

          /* Wait for MLC NAND controller ready */
          while (!(mlcdat.regptr->mlc_isr & MLC_CNTRLLR_RDY_STS));
        }

        /* Auto program command */
        mlcdat.regptr->mlc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));
      }
    }
  }

  return status;
}

/***********************************************************************
 * MLC NAND controller driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mlcnand_open
 *
 * Purpose: Open the MLC NAND controller
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a MLC NAND controller peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a MLC NAND controller config structure or
 *          NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlcnand_open(void *ipbase, INT_32 arg)
{
  INT_32 status = 0;

  if ((MLCNAND_REGS_T *) ipbase == MLCNAND)
  {
    /* Has MLC NAND driver been previously initialized? */
    if (mlcdat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
      used */
      mlcdat.init = TRUE;

      /* Save address of register block */
      mlcdat.regptr = MLCNAND;

      /* Save address of NAND flash parameters */
      mlcdat.paramptr = (NANDFLASH_PARAM_T *) arg;

      if (mlcdat.paramptr->bus_width == BUS_WIDTH_8)
      {
        /* Select MLC NAND controller, 
                    disable DMA_REQ signal on NAND_RnB,
                    disable DMA_REQ signal on NAND_INT */
        clkpwr_setup_nand_ctrlr(0, 0, 0);

        /* Enable MLC NAND clock */
        clkpwr_clk_en_dis(CLKPWR_NAND_MLC_CLK, 1);

        /* normal nCE */
        mlcdat.regptr->mlc_ceh = MLC_NORMAL_NCE;

        /* Mask Interrupts */
        mlcdat.regptr->mlc_irq_mr = 0;

        /* Unlock the access to mlc_icr */
        mlcdat.regptr->mlc_lock_pr = 0xA25E;

        /* MLC NAND controller configuration */
        if (mlcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
        {
          if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_3)
          {
            mlcdat.regptr->mlc_icr = 0;
          }
          else if (mlcdat.paramptr->addr_cycles ==
                    ADDR_CYCLES_4)
          {
            mlcdat.regptr->mlc_icr = MLC_ADDR4_ENABLE;
          }

        }
        else if (mlcdat.paramptr->block_page ==
                    BLOCK_PAGE_LARGE)
        {
          if (mlcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
          {
            mlcdat.regptr->mlc_icr = MLC_LARGE_BLK_ENABLE;
          }
          else if (mlcdat.paramptr->addr_cycles ==
                    ADDR_CYCLES_5)
          {

            mlcdat.regptr->mlc_icr = MLC_LARGE_BLK_ENABLE |
                                     MLC_ADDR4_ENABLE;
          }

        }

        /* Unlock the access to mlc_time */
        mlcdat.regptr->mlc_lock_pr = 0xA25E;

        /* Start with default timings */
        mlcdat.regptr->mlc_time = (MLC_LOAD_TWP(7) |
                    MLC_LOAD_TWH(0) |
                    MLC_LOAD_TRP(0) |
                    MLC_LOAD_TREH(0) |
                    MLC_LOAD_TRHZ(0) |
                    MLC_LOAD_TWBTRB(0) |
                    MLC_LOAD_TCEA(0));

        /* Reset MLC NAND Flash */
        mlcdat.regptr->mlc_cmd = NAND_CMD_RESET;

        /* Wait for MLC NAND ready */
        while (!(mlcdat.regptr->mlc_isr & MLC_DEV_RDY_STS));

        /* Install MLC NAND controller interrupt handler as a
        			IRQ interrupts */
        int_install_irq_handler(IRQ_FLASH,
                    (PFV) mlcnand_interrupt);

        /* Enable MLC NAND interrupt in the interrupt
        			controller */
        int_enable(IRQ_FLASH);

        /* Return pointer to MLC NAND controller
        			config structure */
        status = (INT_32) & mlcdat;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_close
 *
 * Purpose: Close the MLC NAND controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((MLCNAND_CFG_T *) devid == &mlcdat)
  {
    if (mlcdat.init == TRUE)
    {
      /* normal nCE */
      mlcdat.regptr->mlc_ceh = 0;

      /* Unlock the access to mlc_icr */
      mlcdat.regptr->mlc_lock_pr = 0xA25E;

      /* MLC NAND controller configuration */
      mlcdat.regptr->mlc_icr = 0;

      /* Disable MLC NAND clock */
      clkpwr_clk_en_dis(CLKPWR_NAND_MLC_CLK, 0);

      /* Set device as uninitialized */
      mlcdat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_ioctl
 *
 * Purpose: MLC NAND controller configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to MLC NAND controller config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS mlcnand_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg)
{
  int idx;
  INT_32 status = _ERROR;

  if ((MLCNAND_CFG_T *) devid == &mlcdat)
  {
    if (mlcdat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case MLC_SET_TIMING:
          /* Unlock the access to mlc_time */
          mlcdat.regptr->mlc_lock_pr = 0xA25E;

          /* Set timing */
          mlcdat.regptr->mlc_time = (MLC_LOAD_TWP(
                    ((MLC_TIMING_T *) arg)->wr_low) |
                    MLC_LOAD_TWH(
                    ((MLC_TIMING_T *) arg)->wr_high) |
                    MLC_LOAD_TRP(
                    ((MLC_TIMING_T *) arg)->r_low) |
                    MLC_LOAD_TREH(
                    ((MLC_TIMING_T *) arg)->r_high) |
                    MLC_LOAD_TRHZ(
                    ((MLC_TIMING_T *) arg)->nand_ta) |
                    MLC_LOAD_TWBTRB(
                    ((MLC_TIMING_T *) arg)->busy_delay) |
                    MLC_LOAD_TCEA(
                    ((MLC_TIMING_T *) arg)->tcea_delay));
          break;

        case MLC_MASK_INTS:
          /* Mask interrupt */
          mlcdat.regptr->mlc_irq_mr = arg;
          break;

        case MLC_SEND_CMD:
          /* Issue command to MLC NAND Flash */
          for (idx = 0; idx <
                    ((MLC_CMDADR_T *) arg)->num_items; idx++)
          {
            mlcdat.regptr->mlc_cmd =
                    ((MLC_CMDADR_T *) arg)->items[idx];
          }
          break;

        case MLC_SEND_ADDR:
          /* Issue address to MLC NAND Flash */
          for (idx = 0; idx <
                    ((MLC_CMDADR_T *) arg)->num_items; idx++)
          {
            mlcdat.regptr->mlc_addr =
                    ((MLC_CMDADR_T *) arg)->items[idx];
          }
          break;

        case MLC_GET_STATUS:
          /* Get MLC NAND controller status */
          switch (arg)
          {
            case MLC_ST:
              /* Return MLC NAND status */
              status = mlcdat.regptr->mlc_isr;
              break;

            case MLC_INT_ST:
              /* Return MLC NAND interrupt states */
              status = mlcdat.regptr->mlc_irq_sr;
              break;

            default:
              /* Unsupported parameter */
              status = LPC_BAD_PARAMS;
          }
          break;

        case MLC_READ_ID:
          /* Read MLC NAND Flash id */
          status = mlcnand_read_id(devid, (void *) arg);
          break;

        case MLC_ERASE_BLOCK:
          /* Erase MLC NAND Flash block */
          status = mlcnand_erase_block(devid, (UNS_32) arg);
          break;

        case MLC_READ_SPARE:
          /* Read MLC NAND flash spare */
          status = mlcnand_read_spare(devid,
                    (MLC_BLOCKPAGE_T *) arg);
          break;

        case MLC_READ_PAGE:
          /* Read MLC NAND flash page */
          status = mlcnand_read_page(devid,
                    (MLC_BLOCKPAGE_T *) arg);
          break;

        case MLC_WRITE_PAGE:
          /* Write MLC NAND flash page */
          status = mlcnand_write_page(devid,
                    (MLC_BLOCKPAGE_T *) arg);
          break;

        default:
          /* Unsupported parameter */
          status = LPC_BAD_PARAMS;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: mlcnand_read
 *
 * Purpose: MLC NAND controller read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to MLC NAND controller descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlcnand_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: mlcnand_write
 *
 * Purpose: MLC NAND controller write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to MLC NAND controller descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mlcnand_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes)
{
  return 0;
}
