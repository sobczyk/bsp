/***********************************************************************
 * $Id:: lpc32xx_slcnand_driver.c 727 2008-05-08 18:23:58Z wellsk      $
 *
 * Project: LPC32xx SLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx SLC NAND
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
#include "lpc32xx_slcnand_driver.h"

/***********************************************************************
 * SLC NAND controller driver package data
***********************************************************************/

/* SLC NAND controller device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  SLCNAND_REGS_T *regptr; /* Pointer to SLC NAND registers */
  NANDFLASH_PARAM_T *paramptr; /* Pointer to NAND flash parameters */
} SLCNAND_CFG_T;

/* SLC NAND controller driver data */
static SLCNAND_CFG_T slcdat;

/* ECC */
static UNS_32 *ECC;

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
 * Function: slcnand_dma_interrupt
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
void slcnand_dma_interrupt(void)
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
 * Function: slcnand_interrupt
 *
 * Purpose: SLC NAND controller interrupt handler
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
void slcnand_interrupt(void)
{
  if (slcdat.regptr->slc_int_stat & SLCSTAT_INT_RDY_EN)
  {
    slcdat.regptr->slc_icr = SLCSTAT_INT_RDY_EN;
  }

  if (slcdat.regptr->slc_int_stat & SLCSTAT_INT_TC)
  {
    slcdat.regptr->slc_icr = SLCSTAT_INT_TC;
    dma_completion = TRUE;
  }
}

/***********************************************************************
 *
 * Function: slcnand_dma_read
 *
 * Purpose: Setup DMA channel for dma read from slc
 *
 * Processing:
 *     Transfer data from slc to memory and save ECC generated
 *     by hardware.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     dmasrc: DMA src address
 *     dmadest: DMA dest address
 *     ecc: ECC ECC enable(TRUE)/disable(FALSE)
 *
 * Outputs: None
 *
 * Returns: The status of DMA channel setup for dma read from slc
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_dma_read(INT_32 devid, UNS_32 dmasrc, UNS_32 dmadest,
                        DMAC_LL_T *dma, UNS_32 *ecc)
{
  STATUS status = _NO_ERROR;
  INT_32 idx;

  dma_completion = FALSE;

  dmalist = dma;

  ECC = ecc;

  if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
  {
    if (ecc != 0)
    {
      dmalist[0].dma_src = dmasrc;
      dmalist[0].dma_dest = (UNS_32)dmadest;
      dmalist[0].next_lli = (UNS_32) & dmalist[1];
      dmalist[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[1].dma_src = (UNS_32) & slcdat.regptr->slc_ecc;
      dmalist[1].dma_dest = (UNS_32) & ECC[0];
      dmalist[1].next_lli = (UNS_32) & dmalist[2];
      dmalist[1].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[2].dma_src = dmasrc;
      dmalist[2].dma_dest = (UNS_32)(dmadest +
                    (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2));
      dmalist[2].next_lli = (UNS_32) & dmalist[3];
      dmalist[2].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[3].dma_src = (UNS_32) & slcdat.regptr->slc_ecc;
      dmalist[3].dma_dest = (UNS_32) & ECC[1];
      dmalist[3].next_lli = (UNS_32) & dmalist[4];
      dmalist[3].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[4].dma_src = dmasrc;
      dmalist[4].dma_dest = (UNS_32)(dmadest +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[4].next_lli = 0;
      dmalist[4].next_ctrl = (
                    (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
    else
    {
      dmalist[0].dma_src = dmasrc;
      dmalist[0].dma_dest = (UNS_32)dmadest;
      dmalist[0].next_lli = (UNS_32) & dmalist[1];
      dmalist[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[1].dma_src = dmasrc;
      dmalist[1].dma_dest = (UNS_32)(dmadest +
                    (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2));
      dmalist[1].next_lli = (UNS_32) & dmalist[2];
      dmalist[1].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[2].dma_src = dmasrc;
      dmalist[2].dma_dest = (UNS_32)(dmadest +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[2].next_lli = 0;
      dmalist[2].next_ctrl = (
                    (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
  }
  else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
  {
    if (ecc != 0)
    {
      for (idx = 0; idx < 4; idx++)
      {
        dmalist[idx*4].dma_src = dmasrc;
        dmalist[idx*4].dma_dest = (UNS_32)(dmadest +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) * (idx * 2)));
        dmalist[idx*4].next_lli = (UNS_32) & dmalist[(idx*4)+1];
        dmalist[idx*4].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+1].dma_src = (UNS_32)
                    & slcdat.regptr->slc_ecc;
        dmalist[(idx*4)+1].dma_dest = (UNS_32) & ECC[idx*2];
        dmalist[(idx*4)+1].next_lli =
                    (UNS_32) & dmalist[(idx*4)+2];
        dmalist[(idx*4)+1].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+2].dma_src = dmasrc;
        dmalist[(idx*4)+2].dma_dest = (UNS_32)(dmadest +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
                    ((idx * 2) + 1)));
        dmalist[(idx*4)+2].next_lli =
                    (UNS_32) & dmalist[(idx*4)+3];
        dmalist[(idx*4)+2].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+3].dma_src = (UNS_32)
                    & slcdat.regptr->slc_ecc;
        dmalist[(idx*4)+3].dma_dest = (UNS_32) & ECC[(idx*2)+1];
        dmalist[(idx*4)+3].next_lli =
                    (UNS_32) & dmalist[(idx*4)+4];
        dmalist[(idx*4)+3].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);
      }
      dmalist[idx*4].dma_src = dmasrc;
      dmalist[idx*4].dma_dest = (UNS_32)(dmadest +
                    LARGE_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[idx*4].next_lli = 0;
      dmalist[idx*4].next_ctrl = (
                    (LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
    else
    {
      for (idx = 0; idx < 4; idx++)
      {
        dmalist[idx*2].dma_src = dmasrc;
        dmalist[idx*2].dma_dest = (UNS_32)(dmadest +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) * (idx * 2)));
        dmalist[idx*2].next_lli = (UNS_32) & dmalist[(idx*2)+1];
        dmalist[idx*2].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*2)+1].dma_src = dmasrc;
        dmalist[(idx*2)+1].dma_dest = (UNS_32)(dmadest +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
                    ((idx * 2) + 1)));
        dmalist[(idx*2)+1].next_lli =
                    (UNS_32) & dmalist[(idx*2)+2];
        dmalist[(idx*2)+1].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
      }
      dmalist[idx*2].dma_src = dmasrc;
      dmalist[idx*2].dma_dest = (UNS_32)(dmadest +
                    LARGE_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[idx*2].next_lli = 0;
      dmalist[idx*2].next_ctrl = (
                    (LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_DEST_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
  }

  /* Get a DMA channel */
  dmach = dma_alloc_channel(-1, (PFV) slcnand_dma_interrupt);
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
                    DMAC_SRC_PERIP(DMA_PERID_NAND1) |
                    DMAC_CHAN_ENABLE;

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_dma_write
 *
 * Purpose: Setup DMA channel for dma write to slc
 *
 * Processing:
 *     Transfer data from memory to slc and transfer ECC generated
 *     by hardware to memory.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     dmasrc: DMA src address
 *     dmadest: DMA dest address
 *     ecc: ECC ECC enable(TRUE)/disable(FALSE)
 *
 * Outputs: None
 *
 * Returns: The status of DMA channel setup for dma write to slc
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_dma_write(INT_32 devid, UNS_32 dmasrc, UNS_32 dmadest,
                         DMAC_LL_T *dma, UNS_32 *ecc)
{
  STATUS status = _NO_ERROR;
  INT_32 idx;

  dma_completion = FALSE;

  dmalist = dma;

  if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
  {
    if (ecc != 0)
    {
      dmalist[0].dma_src = (UNS_32)dmasrc;
      dmalist[0].dma_dest = dmadest;
      dmalist[0].next_lli = (UNS_32) & dmalist[1];
      dmalist[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[1].dma_src = (UNS_32)(dmasrc +
                    (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2));
      dmalist[1].dma_dest = dmadest;
      dmalist[1].next_lli = (UNS_32) & dmalist[2];
      dmalist[1].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[2].dma_src = (UNS_32) & slcdat.regptr->slc_ecc;
      dmalist[2].dma_dest = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_1ST_ECC);
      dmalist[2].next_lli = (UNS_32) & dmalist[3];
      dmalist[2].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[3].dma_src = (UNS_32) & slcdat.regptr->slc_ecc;
      dmalist[3].dma_dest = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_2ND_ECC);
      dmalist[3].next_lli = (UNS_32) & dmalist[4];
      dmalist[3].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[4].dma_src = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[4].dma_dest = dmadest;
      dmalist[4].next_lli = 0;
      dmalist[4].next_ctrl = (
                    (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
    else
    {
      dmalist[0].dma_src = (UNS_32)dmasrc;
      dmalist[0].dma_dest = dmadest;
      dmalist[0].next_lli = (UNS_32) & dmalist[1];
      dmalist[0].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[1].dma_src = (UNS_32)(dmasrc +
                    (SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2));
      dmalist[1].dma_dest = dmadest;
      dmalist[1].next_lli = (UNS_32) & dmalist[2];
      dmalist[1].next_ctrl = (
                    ((SMALL_BLOCK_PAGE_MAIN_AREA_SIZE / 2) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

      dmalist[2].dma_src = (UNS_32)(dmasrc +
                    SMALL_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[2].dma_dest = dmadest;
      dmalist[2].next_lli = 0;
      dmalist[2].next_ctrl = (
                    (SMALL_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
  }
  else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
  {
    if (ecc != 0)
    {
      for (idx = 0; idx < 4; idx++)
      {
        dmalist[idx*4].dma_src = (UNS_32)(dmasrc +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) * (idx * 2)));
        dmalist[idx*4].dma_dest = dmadest;
        dmalist[idx*4].next_lli = (UNS_32) & dmalist[(idx*4)+1];
        dmalist[idx*4].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+1].dma_src = (UNS_32)(dmasrc +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
                    ((idx * 2) + 1)));
        dmalist[(idx*4)+1].dma_dest = dmadest;
        dmalist[(idx*4)+1].next_lli =
                    (UNS_32) & dmalist[(idx*4)+2];
        dmalist[(idx*4)+1].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+2].dma_src = (UNS_32)
                    & slcdat.regptr->slc_ecc;
        dmalist[(idx*4)+2].dma_dest = (UNS_32)(dmasrc +
                    LARGE_BLOCK_PAGE_1ST_ECC(idx));
        dmalist[(idx*4)+2].next_lli =
                    (UNS_32) & dmalist[(idx*4)+3];
        dmalist[(idx*4)+2].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*4)+3].dma_src = (UNS_32)
                    & slcdat.regptr->slc_ecc;
        dmalist[(idx*4)+3].dma_dest = (UNS_32)(dmasrc +
                    LARGE_BLOCK_PAGE_2ND_ECC(idx));
        dmalist[(idx*4)+3].next_lli =
                    (UNS_32) & dmalist[(idx*4)+4];
        dmalist[(idx*4)+3].next_ctrl = (0x1 |
                    DMAC_CHAN_SRC_BURST_1 |
                    DMAC_CHAN_DEST_BURST_1 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_INT_TC_EN);
      }
      dmalist[idx*4].dma_src = (UNS_32)(dmasrc +
                    LARGE_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[idx*4].dma_dest = dmadest;
      dmalist[idx*4].next_lli = 0;
      dmalist[idx*4].next_ctrl = (
                    (LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
    else
    {
      for (idx = 0; idx < 4; idx++)
      {
        dmalist[idx*2].dma_src = (UNS_32)(dmasrc +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) * (idx * 2)));
        dmalist[idx*2].dma_dest = dmadest;
        dmalist[idx*2].next_lli = (UNS_32) & dmalist[(idx*2)+1];
        dmalist[idx*2].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);

        dmalist[(idx*2)+1].dma_src = (UNS_32)(dmasrc +
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) *
                    ((idx * 2) + 1)));
        dmalist[(idx*2)+1].dma_dest = dmadest;
        dmalist[(idx*2)+1].next_lli =
                    (UNS_32) & dmalist[(idx*2)+2];
        dmalist[(idx*2)+1].next_ctrl = (
                    ((LARGE_BLOCK_PAGE_MAIN_AREA_SIZE / 8) / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
      }
      dmalist[idx*2].dma_src = (UNS_32)(dmasrc +
                    LARGE_BLOCK_PAGE_MAIN_AREA_SIZE);
      dmalist[idx*2].dma_dest = dmadest;
      dmalist[idx*2].next_lli = 0;
      dmalist[idx*2].next_ctrl = (
                    (LARGE_BLOCK_PAGE_SPARE_AREA_SIZE / 4) |
                    DMAC_CHAN_SRC_BURST_4 |
                    DMAC_CHAN_DEST_BURST_4 |
                    DMAC_CHAN_SRC_WIDTH_32 |
                    DMAC_CHAN_DEST_WIDTH_32 |
                    DMAC_CHAN_DEST_AHB1 |
                    DMAC_CHAN_SRC_AUTOINC |
                    DMAC_CHAN_INT_TC_EN);
    }
  }

  /* Get a DMA channel */
  dmach = dma_alloc_channel(-1, (PFV) slcnand_dma_interrupt);
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
                    DMAC_CHAN_FLOW_D_M2P |
                    DMAC_DEST_PERIP(DMA_PERID_NAND1) |
                    DMAC_SRC_PERIP(0) |
                    DMAC_CHAN_ENABLE;

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_read_id
 *
 * Purpose: Read SLC NAND flash id
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, read SLC NAND flash id.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     buf:   Pointer to data buffer to copy to
 *
 * Outputs: None
 *
 * Returns: The status of read id operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_read_id(INT_32 devid, void *buf)
{
  STATUS status = _ERROR;
  UNS_8 *bptr = buf;
  int idx;

  if (slcdat.init == TRUE)
  {
    status = _NO_ERROR;

    /* ID read (1) command */
    slcdat.regptr->slc_cmd = NAND_CMD_READID;

    /* Column Address */
    slcdat.regptr->slc_addr = 0;

    /* Wait for SLC NAND ready */
    while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

    /* Read ID_SIZE bytes from SLC NAND flash data register */
    for (idx = 0; idx < ID_SIZE; idx++)
    {
      *bptr++ = (UNS_8)(slcdat.regptr->slc_data & _BITMASK(8));
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_erase_block
 *
 * Purpose: Erase SLC NAND flash block
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, erase SLC NAND flash block.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     block_num: Block to be erased
 *
 * Outputs: None
 *
 * Returns: The status of block erase operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_erase_block(INT_32 devid, UNS_32 block_num)
{
  STATUS status = _ERROR;

  if (slcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Auto block erase 1-st command */
      slcdat.regptr->slc_cmd = NAND_CMD_ERASE1ST;

      /* Page Address 1st */
      slcdat.regptr->slc_addr = ((block_num << 5) & 0xE0);

      /* Page Address 2nd */
      slcdat.regptr->slc_addr = ((block_num << 5) >> 8) &
                    _BITMASK(8);

      if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
      {
        /* Page Address 3rd */
        slcdat.regptr->slc_addr = ((block_num << 5) >> 16) &
                    _BITMASK(8);
      }

      /* Auto block erase 2-nd command */
      slcdat.regptr->slc_cmd = NAND_CMD_ERASE2ND;

      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Status read (1) command */
      slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

      /* Operation status */
      if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
      {
        status = _ERROR;
      }
    }
    else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Auto block erase 1-st command */
      slcdat.regptr->slc_cmd = NAND_CMD_ERASE1ST;

      /* Page Address 1st */
      slcdat.regptr->slc_addr = ((block_num << 6) & 0xC0);

      /* Page Address 2nd */
      slcdat.regptr->slc_addr = ((block_num << 6) >> 8) &
                    _BITMASK(8);

      if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
      {
        /* Page Address 3rd */
        slcdat.regptr->slc_addr = ((block_num << 6) >> 16) &
                    _BITMASK(8);
      }

      /* Auto block erase 2-nd command */
      slcdat.regptr->slc_cmd = NAND_CMD_ERASE2ND;

      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Status read (1) command */
      slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

      /* Operation status */
      if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
      {
        status = _ERROR;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_read_spare
 *
 * Purpose: Read SLC NAND flash page spare area
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, read SLC NAND flash page spare
 *     area.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     blockpage: Pointer to SLC_BLOCKPAGE_T structure
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
 *     |   2048 bytes main   |   64 bytes spare   |
 *      ------------------------------------------
 *
 **********************************************************************/
STATUS slcnand_read_spare(INT_32 devid, SLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_8 *bptr = blockpage->buffer;
  int idx;

  if (slcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Read mode (3) command */
      slcdat.regptr->slc_cmd = NAND_CMD_READ3;

      /* Column Address */
      slcdat.regptr->slc_addr = 0;

      /* Page Address 1st */
      slcdat.regptr->slc_addr = (((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

      /* Page Address 2nd */
      slcdat.regptr->slc_addr = (((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

      if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
      {
        /* Page Address 3rd */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
      }

      /* Read SMALL_BLOCK_PAGE_SPARE_AREA_SIZE bytes from
      				SLC NAND flash data register */
      for (idx = 0; idx < SMALL_BLOCK_PAGE_SPARE_AREA_SIZE;
           idx++)
      {
        *bptr++ = (UNS_8)(slcdat.regptr->slc_data & _BITMASK(8));
      }
    }
    else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      /* Wait for SLC NAND ready */
      while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

      /* Read mode (1) command */
      slcdat.regptr->slc_cmd = NAND_CMD_READ1;

      /* Column Address 1st */
      slcdat.regptr->slc_addr = 0;

      /* Column Address 2nd */
      slcdat.regptr->slc_addr = 8;

      /* Page Address 1st */
      slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

      /* Page Address 2nd */
      slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

      if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
      {
        /* Page Address 3rd */
        slcdat.regptr->slc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
      }

      /* Read start command */
      slcdat.regptr->slc_cmd = NAND_CMD_READSTART;

      /* Read LARGE_BLOCK_PAGE_SPARE_AREA_SIZE bytes from
      				SLC NAND flash data register */
      for (idx = 0; idx < LARGE_BLOCK_PAGE_SPARE_AREA_SIZE; idx++)
      {
        *bptr++ = (UNS_8)(slcdat.regptr->slc_data &
                    _BITMASK(8));
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_read_page
 *
 * Purpose: Read SLC NAND flash page
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, read SLC NAND flash page.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     blockpage: Pointer to SLC_BLOCKPAGE_T structure
 *
 * Outputs: None
 *
 * Returns: The status of read page operation
 *
 * Notes:
 *     Read both SLC NAND flash page main area and spare area.
 *     Small page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *     Large page -
 *      ------------------------------------------
 *     |   2048 bytes main   |   64 bytes spare   |
 *      ------------------------------------------
 *     If DMA & ECC enabled, then the ECC generated for the 1st 256-byte
 *     data is compared with the 3rd word of the spare area. The ECC
 *     generated for the 2nd 256-byte data is compared with the 4th word
 *     of the spare area. The ECC generated for the 3rd 256-byte data is
 *     compared with the 7th word of the spare area. The ECC generated
 *     for the 4th 256-byte data is compared with the 8th word of the
 *     spare area and so on.
 *
 **********************************************************************/
STATUS slcnand_read_page(INT_32 devid, SLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_8 *bptr = blockpage->buffer;
  UNS_32 *dwptr = (UNS_32 *)blockpage->buffer;
  int idx;

  if (slcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      if (blockpage->dma != 0)
      {
        /* Enable TC interrupt */
        slcdat.regptr->slc_ien |= SLCSTAT_INT_TC;

        if (blockpage->ecc != 0)
        {
          /* Enable DMA ECC channel, ECC, burst,
          		DMA read from slc */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                    SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst, DMA read from slc */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST);
        }

        slcnand_dma_read(devid,
                    (UNS_32)&slcdat.regptr->slc_dma_data,
                    (UNS_32)dwptr, blockpage->dma,
                    blockpage->ecc);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Read mode (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_READ1;

        /* Column Address */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* Set transfer count */
        slcdat.regptr->slc_tc = SMALL_BLOCK_PAGE_SIZE;

        /* Clear ECC, start DMA */
        slcdat.regptr->slc_ctrl |= (SLCCTRL_DMA_START |
                    SLCCTRL_ECC_CLEAR);

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        slcdat.regptr->slc_ctrl &= ~SLCCTRL_DMA_START;

        if (blockpage->ecc != 0)
        {
          /* Disable DMA ECC channel, ECC, burst,
          		DMA read from slc */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                    SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst, DMA read from slc */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST);
        }

        /* release the DMA Channel */
        dma_free_channel(dmach);

        if (blockpage->ecc != 0)
        {
          dwptr = (UNS_32 *)(blockpage->buffer +
                    SMALL_BLOCK_PAGE_1ST_ECC);
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[0] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    SMALL_BLOCK_PAGE_2ND_ECC);
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[1] & _BITMASK(24)))
          {
            return _ERROR;
          }
        }
      }
      else
      {
        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Read mode (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_READ1;

        /* Column Address */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* Read SMALL_BLOCK_PAGE_SIZE bytes from
        			SLC NAND flash data register */
        for (idx = 0; idx < SMALL_BLOCK_PAGE_SIZE; idx++)
        {
          *bptr++ = (UNS_8)(slcdat.regptr->slc_data &
                    _BITMASK(8));
        }
      }
    }
    else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      if (blockpage->dma != 0)
      {
        /* Enable TC interrupt */
        slcdat.regptr->slc_ien |= SLCSTAT_INT_TC;

        if (blockpage->ecc != 0)
        {
          /* Enable DMA ECC channel, ECC, burst,
          		DMA read from slc */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                    SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst, DMA read from slc */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST);
        }

        slcnand_dma_read(devid,
                    (UNS_32)&slcdat.regptr->slc_dma_data,
                    (UNS_32)dwptr, blockpage->dma,
                    blockpage->ecc);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Read mode (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_READ1;

        /* Column Address 1st */
        slcdat.regptr->slc_addr = 0;

        /* Column Address 2nd */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Read start command */
        slcdat.regptr->slc_cmd = NAND_CMD_READSTART;

        /* Set transfer count */
        slcdat.regptr->slc_tc = LARGE_BLOCK_PAGE_SIZE;

        /* Clear ECC, start DMA */
        slcdat.regptr->slc_ctrl |= (SLCCTRL_DMA_START |
                    SLCCTRL_ECC_CLEAR);

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        slcdat.regptr->slc_ctrl &= ~SLCCTRL_DMA_START;

        if (blockpage->ecc != 0)
        {
          /* Disable DMA ECC channel, ECC, burst,
          		DMA read from slc */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST | SLCCFG_ECC_EN |
                    SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst, DMA read from slc */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_DIR |
                    SLCCFG_DMA_BURST);
        }

        /* release the DMA Channel */
        dma_free_channel(dmach);

        if (blockpage->ecc != 0)
        {
          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_1ST_ECC(0));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[0] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_2ND_ECC(0));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[1] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_1ST_ECC(1));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[2] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_2ND_ECC(1));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[3] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_1ST_ECC(2));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[4] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_2ND_ECC(2));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[5] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_1ST_ECC(3));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[6] & _BITMASK(24)))
          {
            return _ERROR;
          }

          dwptr = (UNS_32 *)(blockpage->buffer +
                    LARGE_BLOCK_PAGE_2ND_ECC(3));
          if ((*dwptr & _BITMASK(24)) !=
                    (ECC[7] & _BITMASK(24)))
          {
            return _ERROR;
          }
        }
      }
      else
      {
        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Read mode (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_READ1;

        /* Column Address 1st */
        slcdat.regptr->slc_addr = 0;

        /* Column Address 2nd */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Read start command */
        slcdat.regptr->slc_cmd = NAND_CMD_READSTART;

        /* Read LARGE_BLOCK_PAGE_SIZE bytes from
        			SLC NAND flash data register */
        for (idx = 0; idx < LARGE_BLOCK_PAGE_SIZE; idx++)
        {
          *bptr++ = (UNS_8)(slcdat.regptr->slc_data &
                    _BITMASK(8));
        }
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_write_page
 *
 * Purpose: Write SLC NAND flash page
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR as slc was not
 *     previously opened. Otherwise, write SLC NAND flash page.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *     blockpage: Pointer to SLC_BLOCKPAGE_T structure
 *
 * Outputs: None
 *
 * Returns: The status of write page operation
 *
 * Notes:
 *     Write both SLC NAND flash page main area and spare area.
 *     Small page -
 *      ------------------------------------------
 *     |    512 bytes main   |   16 bytes spare   |
 *      ------------------------------------------
 *     Large page -
 *      ------------------------------------------
 *     |   2048 bytes main   |   64 bytes spare   |
 *      ------------------------------------------
 *     If DMA & ECC enabled, then the ECC generated for the 1st 256-byte
 *     data is written to the 3rd word of the spare area. The ECC
 *     generated for the 2nd 256-byte data is written to the 4th word
 *     of the spare area. The ECC generated for the 3rd 256-byte data is
 *     written to the 7th word of the spare area. The ECC generated
 *     for the 4th 256-byte data is written to the 8th word of the
 *     spare area and so on.
 *
 **********************************************************************/
STATUS slcnand_write_page(INT_32 devid, SLC_BLOCKPAGE_T *blockpage)
{
  STATUS status = _ERROR;
  UNS_8 *bptr = blockpage->buffer;
  UNS_32 *dwptr = (UNS_32 *)blockpage->buffer;
  int idx;

  if (slcdat.init == TRUE)
  {
    status = _NO_ERROR;

    if (slcdat.paramptr->block_page == BLOCK_PAGE_SMALL)
    {
      if (blockpage->dma != 0)
      {
        /* Enable TC interrupt */
        slcdat.regptr->slc_ien |= SLCSTAT_INT_TC;

        if (blockpage->ecc != 0)
        {
          /* Enable DMA ECC channel, ECC, burst */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_BURST |
                    SLCCFG_ECC_EN | SLCCFG_DMA_ECC);
          /* DMA wtite to slc */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_DIR;
        }
        else
        {
          /* Enable burst */
          slcdat.regptr->slc_cfg |= SLCCFG_DMA_BURST;
          /* DMA wtite to slc */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_DIR;
        }

        slcnand_dma_write(devid, (UNS_32)dwptr,
                    (UNS_32)&slcdat.regptr->slc_dma_data,
                    blockpage->dma, blockpage->ecc);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Serial data input command */
        slcdat.regptr->slc_cmd = NAND_CMD_SDIN;

        /* Column Address */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* Set transfer count */
        slcdat.regptr->slc_tc = SMALL_BLOCK_PAGE_SIZE;

        /* Clear ECC, start DMA */
        slcdat.regptr->slc_ctrl |= (SLCCTRL_DMA_START |
                    SLCCTRL_ECC_CLEAR);

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* Auto program command */
        slcdat.regptr->slc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Status read (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

        /* Operation status */
        if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
        {
          status = _ERROR;
        }

        slcdat.regptr->slc_ctrl &= ~SLCCTRL_DMA_START;

        if (blockpage->ecc != 0)
        {
          /* Disable DMA ECC channel, ECC, burst */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_BURST |
                    SLCCFG_ECC_EN | SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_BURST;
        }

        /* release the DMA Channel */
        dma_free_channel(dmach);
      }
      else
      {
        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Serial data input command */
        slcdat.regptr->slc_cmd = NAND_CMD_SDIN;

        /* Column Address */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) &
                    0xE0) | (blockpage->page_num & 0x1F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5) >>
                    8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_4)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (
                    ((blockpage->block_num << 5)
                    >> 16) & _BITMASK(8));
        }

        /* Write SMALL_BLOCK_PAGE_SIZE bytes to
        			SLC NAND flash data register */
        for (idx = 0; idx < SMALL_BLOCK_PAGE_SIZE; idx++)
        {
          slcdat.regptr->slc_data = (UNS_32)(*bptr++);
        }

        /* Auto program command */
        slcdat.regptr->slc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Status read (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

        /* Operation status */
        if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
        {
          status = _ERROR;
        }
      }
    }
    else if (slcdat.paramptr->block_page == BLOCK_PAGE_LARGE)
    {
      if (blockpage->dma != 0)
      {
        /* Enable TC interrupt */
        slcdat.regptr->slc_ien |= SLCSTAT_INT_TC;

        if (blockpage->ecc != 0)
        {
          /* Enable DMA ECC channel, ECC, burst */
          slcdat.regptr->slc_cfg |= (SLCCFG_DMA_BURST |
                    SLCCFG_ECC_EN | SLCCFG_DMA_ECC);
          /* DMA wtite to slc */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_DIR;
        }
        else
        {
          /* Enable burst */
          slcdat.regptr->slc_cfg |= SLCCFG_DMA_BURST;
          /* DMA wtite to slc */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_DIR;
        }

        slcnand_dma_write(devid, (UNS_32)dwptr,
                    (UNS_32)&slcdat.regptr->slc_dma_data,
                    blockpage->dma, blockpage->ecc);

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Serial data input command */
        slcdat.regptr->slc_cmd = NAND_CMD_SDIN;

        /* Column Address 1st */
        slcdat.regptr->slc_addr = 0;

        /* Column Address 2nd */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Set transfer count */
        slcdat.regptr->slc_tc = LARGE_BLOCK_PAGE_SIZE;

        /* Clear ECC, start DMA */
        slcdat.regptr->slc_ctrl |= (SLCCTRL_DMA_START |
                    SLCCTRL_ECC_CLEAR);

        /* Wait for completion of DMA */
        while (dma_completion == FALSE);

        /* Auto program command */
        slcdat.regptr->slc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Status read (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

        /* Operation status */
        if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
        {
          status = _ERROR;
        }

        slcdat.regptr->slc_ctrl &= ~SLCCTRL_DMA_START;

        if (blockpage->ecc != 0)
        {
          /* Disable DMA ECC channel, ECC, burst */
          slcdat.regptr->slc_cfg &= ~(SLCCFG_DMA_BURST |
                    SLCCFG_ECC_EN | SLCCFG_DMA_ECC);
        }
        else
        {
          /* Enable burst */
          slcdat.regptr->slc_cfg &= ~SLCCFG_DMA_BURST;
        }

        /* release the DMA Channel */
        dma_free_channel(dmach);
      }
      else
      {
        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Serial data input command */
        slcdat.regptr->slc_cmd = NAND_CMD_SDIN;

        /* Column Address 1st */
        slcdat.regptr->slc_addr = 0;

        /* Column Address 2nd */
        slcdat.regptr->slc_addr = 0;

        /* Page Address 1st */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    & 0xC0) | (blockpage->page_num & 0x3F));

        /* Page Address 2nd */
        slcdat.regptr->slc_addr = (((blockpage->block_num << 6)
                    >> 8) & _BITMASK(8));

        if (slcdat.paramptr->addr_cycles == ADDR_CYCLES_5)
        {
          /* Page Address 3rd */
          slcdat.regptr->slc_addr = (((blockpage->block_num <<
                    6) >> 16) & _BITMASK(8));
        }

        /* Write LARGE_BLOCK_PAGE_SIZE bytes to
        			SLC NAND flash data register */
        for (idx = 0; idx < LARGE_BLOCK_PAGE_SIZE; idx++)
        {
          slcdat.regptr->slc_data = (UNS_32)(*bptr++);
        }

        /* Auto program command */
        slcdat.regptr->slc_cmd = NAND_CMD_PAGEPROG;

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Status read (1) command */
        slcdat.regptr->slc_cmd = NAND_CMD_STATUS;

        /* Operation status */
        if (slcdat.regptr->slc_data & NAND_FLASH_FAILED)
        {
          status = _ERROR;
        }
      }
    }
  }

  return status;
}

/***********************************************************************
 * SLC NAND controller driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: slcnand_open
 *
 * Purpose: Open the SLC NAND controller
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a SLC NAND controller peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a SLC NAND controller config structure or
 *          NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slcnand_open(void *ipbase, INT_32 arg)
{
  INT_32 status = 0;

  if ((SLCNAND_REGS_T *) ipbase == SLCNAND)
  {
    /* Has SLC NAND driver been previously initialized? */
    if (slcdat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
      used */
      slcdat.init = TRUE;

      /* Save address of register block */
      slcdat.regptr = SLCNAND;

      /* Save address of NAND flash parameters */
      slcdat.paramptr = (NANDFLASH_PARAM_T *) arg;

      if (slcdat.paramptr->bus_width == BUS_WIDTH_8)
      {
        /* Select SLC NAND controller
                    disable DMA_REQ signal on NAND_RnB,
                    disable DMA_REQ signal on NAND_INT */
        clkpwr_setup_nand_ctrlr(1, 0, 0);

        /* Enable SLC NAND clock */
        clkpwr_clk_en_dis(CLKPWR_NAND_SLC_CLK, 1);

        /* Reset SLC NAND controller & clear ECC */
        slcdat.regptr->slc_ctrl = (SLCCTRL_SW_RESET |
                                   SLCCTRL_ECC_CLEAR);

        /* 8-bit bus, no DMA, CE normal */
        slcdat.regptr->slc_cfg = 0;

        /* Interrupts disabled and cleared */
        slcdat.regptr->slc_ien = 0;
        slcdat.regptr->slc_icr = (SLCSTAT_INT_TC |
                                  SLCSTAT_INT_RDY_EN);

        /* Start with slowest timings possible */
        slcdat.regptr->slc_tac = (SLCTAC_WDR(15) |
                    SLCTAC_WWIDTH(15) |
                    SLCTAC_WHOLD(15) |
                    SLCTAC_WSETUP(15) |
                    SLCTAC_RDR(15) |
                    SLCTAC_RWIDTH(15) |
                    SLCTAC_RHOLD(15) |
                    SLCTAC_RSETUP(15));

        /* Reset SLC NAND Flash */
        slcdat.regptr->slc_cmd = NAND_CMD_RESET;

        /* Wait for SLC NAND ready */
        while (!(slcdat.regptr->slc_stat & SLCSTAT_NAND_READY));

        /* Install SLC NAND controller interrupt handler as a
        			IRQ interrupts */
        int_install_irq_handler(IRQ_FLASH,
                    (PFV) slcnand_interrupt);

        /* Enable SLC NAND interrupt in the interrupt
        			controller */
        int_enable(IRQ_FLASH);

        /* Return pointer to SLC NAND controller
        			config structure */
        status = (INT_32) & slcdat;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_close
 *
 * Purpose: Close the SLC NAND controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS slcnand_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((SLCNAND_CFG_T *) devid == &slcdat)
  {
    if (slcdat.init == TRUE)
    {
      /* Set default SLC NAND functions to disabled */
      slcdat.regptr->slc_ctrl = 0;

      /* 8-bit bus, no DMA, CE normal */
      slcdat.regptr->slc_cfg = 0;

      /* Disable SLC NAND clock */
      clkpwr_clk_en_dis(CLKPWR_NAND_SLC_CLK, 0);

      /* Set device as uninitialized */
      slcdat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: slcnand_ioctl
 *
 * Purpose: SLC NAND controller configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to SLC NAND controller config structure
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
STATUS slcnand_ioctl(INT_32 devid,
                     INT_32 cmd,
                     INT_32 arg)
{
  int idx;
  INT_32 status = _ERROR;

  if ((SLCNAND_CFG_T *) devid == &slcdat)
  {
    if (slcdat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case SLC_RESET:
          /* Reset SLC NAND controller */
          slcdat.regptr->slc_ctrl |= SLCCTRL_SW_RESET;
          break;

        case SLC_ENABLE_ECC:
          /* Enable or disable ECC */
          if (arg == 1)
          {
            /* Enable */
            slcdat.regptr->slc_cfg |= SLCCFG_ECC_EN;
          }
          else
          {
            /* Disable */
            slcdat.regptr->slc_cfg &= ~SLCCFG_ECC_EN;
          }
          break;

        case SLC_CLEAR_ECC:
          /* Clear ECC */
          slcdat.regptr->slc_ctrl = SLCCTRL_ECC_CLEAR;
          break;

        case SLC_READ_ECC:
          /* Read ECC */
          status = slcdat.regptr->slc_ecc;
          break;

        case SLC_SET_TIMING:
          /* Set timing */
          slcdat.regptr->slc_tac = (
                    SLCTAC_WDR(
                    ((SLC_TAC_T *) arg)->w_rdy) |
                    SLCTAC_WWIDTH(
                    ((SLC_TAC_T *) arg)->w_width) |
                    SLCTAC_WHOLD(
                    ((SLC_TAC_T *) arg)->w_hold) |
                    SLCTAC_WSETUP(
                    ((SLC_TAC_T *) arg)->w_setup) |
                    SLCTAC_RDR(
                    ((SLC_TAC_T *) arg)->r_rdy) |
                    SLCTAC_RWIDTH(
                    ((SLC_TAC_T *) arg)->r_width) |
                    SLCTAC_RHOLD(
                    ((SLC_TAC_T *) arg)->r_hold) |
                    SLCTAC_RSETUP(
                    ((SLC_TAC_T *) arg)->r_setup));
          break;

        case SLC_ENAB_INTS:
          /* Enable interrupt */
          slcdat.regptr->slc_ien |= arg;
          break;

        case SLC_DISAB_INTS:
          /* Disable interrupt */
          slcdat.regptr->slc_ien &= ~arg;
          break;

        case SLC_SET_INTS:
          /* Set interrupt */
          slcdat.regptr->slc_isr = arg;
          break;

        case SLC_CLEAR_INTS:
          /* Clear interrupt */
          slcdat.regptr->slc_icr = arg;
          break;

        case SLC_SEND_CMD:
          /* Issue command to SLC NAND Flash */
          for (idx = 0; idx <
                    ((SLC_CMDADR_T *) arg)->num_items; idx++)
          {
            slcdat.regptr->slc_cmd =
                    ((SLC_CMDADR_T *) arg)->items[idx];
          }
          break;

        case SLC_SEND_ADDR:
          /* Issue address to SLC NAND Flash */
          for (idx = 0; idx <
                    ((SLC_CMDADR_T *) arg)->num_items; idx++)
          {
            slcdat.regptr->slc_addr =
                    ((SLC_CMDADR_T *) arg)->items[idx];
          }
          break;

        case SLC_STOP:
          /* Stop current SLC NAND controller sequence */
          slcdat.regptr->slc_stop = arg;
          break;

        case SLC_GET_STATUS:
          /* Get SLC NAND controller status */
          switch (arg)
          {
            case DMA_FIFO_ST:
              /* Return SLC DMA FIFO status */
              if ((slcdat.regptr->slc_stat &
                    SLCSTAT_DMA_FIFO) != 0)
              {
                status = 1;
              }
              else
              {
                status = 0;
              }
              break;

            case SLC_FIFO_ST:
              /* Return SLC data FIFO status */
              if ((slcdat.regptr->slc_stat &
                    SLCSTAT_SLC_FIFO) != 0)
              {
                status = 1;
              }
              else
              {
                status = 0;
              }
              break;

            case SLC_READY_ST:
              /* Return SLC NAND ready signal */
              if ((slcdat.regptr->slc_stat &
                    SLCSTAT_NAND_READY) != 0)
              {
                status = 1;
              }
              else
              {
                status = 0;
              }
              break;

            case SLC_INT_ST:
              /* Return SLC NAND interrupt states */
              status = slcdat.regptr->slc_int_stat;
              break;

            default:
              /* Unsupported parameter */
              status = LPC_BAD_PARAMS;
          }
          break;

        case SLC_READ_ID:
          /* Read SLC NAND Flash id */
          status = slcnand_read_id(devid, (void *) arg);
          break;

        case SLC_ERASE_BLOCK:
          /* Erase SLC NAND Flash block */
          status = slcnand_erase_block(devid, (UNS_32) arg);
          break;

        case SLC_READ_SPARE:
          /* Read SLC NAND flash spare */
          status = slcnand_read_spare(devid,
                    (SLC_BLOCKPAGE_T *) arg);
          break;

        case SLC_READ_PAGE:
          /* Read SLC NAND flash page */
          status = slcnand_read_page(devid,
                    (SLC_BLOCKPAGE_T *) arg);
          break;

        case SLC_WRITE_PAGE:
          /* Write SLC NAND flash page */
          status = slcnand_write_page(devid,
                    (SLC_BLOCKPAGE_T *) arg);
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
 * Function: slcnand_read
 *
 * Purpose: SLC NAND controller read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to SLC NAND controller descriptor
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
INT_32 slcnand_read(INT_32 devid,
                    void *buffer,
                    INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: slcnand_write
 *
 * Purpose: SLC NAND controller write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to SLC NAND controller descriptor
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
INT_32 slcnand_write(INT_32 devid,
                     void *buffer,
                     INT_32 n_bytes)
{
  return 0;
}
