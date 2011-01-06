/***********************************************************************
 * $Id:: lpc32xx_dma_driver.c 973 2008-07-28 21:06:15Z wellsk          $
 *
 * Project: LPC32XX DMA driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx DMA controller.
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
***********************************************************************/

#include "lpc32xx_dma_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_arm922t_cp15_driver.h"

/***********************************************************************
 * DMA driver private data
***********************************************************************/

/* Number of DMA channels */
#define DMA_MAX_CHANNELS 8

/* DMA driver control structure */
typedef struct
{
  BOOL_32 init;
  INT_32  alloc_ch [DMA_MAX_CHANNELS];
  PFV     cb [DMA_MAX_CHANNELS];
  INT_32  num_alloc_ch;    /* Number of allocated channels */
  DMAC_REGS_T *pdma;
} DMA_DRV_DATA_T;

/* DMAS driver data */
static DMA_DRV_DATA_T dmadrv_dat;

/***********************************************************************
 * DMA driver private functions
***********************************************************************/

/***********************************************************************
 *
 * Function: dma_interrupt
 *
 * Purpose: DMA controller interrupt handler
 *
 * Processing:
 *     This function is called when a DMA interrupt occurs. It looks at
 *     the DMA statuses and calls the user defined callback function
 *     for the active DMA channel if it exists. If a callback function
 *     doesn't exist, then interrupt support for the DMA channel is
 *     disabled.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void dma_interrupt(void)
{
  INT_32 ch;
  UNS_32 sts_int;

  /* Get DMA statuses */
  sts_int = dmadrv_dat.pdma->int_stat;

  /* Check all pending interrupt statuses */
  ch = 0;
  while (ch < DMA_MAX_CHANNELS)
  {
    /* Check channel interrupt status */
    if ((sts_int & _BIT(ch)) != 0)
    {
      /* Channel interrupt is pending */
      if (dmadrv_dat.cb [ch] != NULL)
      {
        /* Call user defined callback function */
        dmadrv_dat.cb [ch]();
      }
      else
      {
        /* Interrupt is pending, but no user callback function
           exists, so disable the interrupts for this channel
           to prevent the interrupt from continuously firing */
        dmadrv_dat.pdma->dma_chan [ch].control &=
          ~DMAC_CHAN_INT_TC_EN;
        dmadrv_dat.pdma->dma_chan [ch].config_ch &=
          ~(DMAC_CHAN_ITC | DMAC_CHAN_IE);
      }
    }

    /* Next channel */
    ch++;
  }
}

/***********************************************************************
 * DMA driver public functions
***********************************************************************/

/***********************************************************************
 *
 * Function: dma_init
 *
 * Purpose: Initial DMA controller and driver
 *
 * Processing:
 *     This function sets up the DMA controller as initially disabled.
 *     All DMA channels used by the driver are unallocated.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *     _ERROR if the device was already initialized, otherside _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dma_init(void)
{
  INT_32 idx;
  STATUS init = _ERROR;

  /* Only continue if driver has not been previously initialized */
  if (dmadrv_dat.init == FALSE)
  {
    dmadrv_dat.init = TRUE;
    dmadrv_dat.num_alloc_ch = 0;

    /* Save base address of DMA controller registers */
    dmadrv_dat.pdma = (DMAC_REGS_T *) DMA_BASE;

    /* Enable clock to DMA controller (for now) */
    clkpwr_clk_en_dis(CLKPWR_DMA_CLK, 1);

    /* Make sure DMA controller and all channels are disabled.
       Controller is in little-endian mode. Disable sync signals */
    dmadrv_dat.pdma->config = 0;
    dmadrv_dat.pdma->sync = 0;

    /* Clear interrupt and error statuses */
    dmadrv_dat.pdma->int_tc_clear = 0xFF;
    dmadrv_dat.pdma->raw_tc_stat = 0xFF;

    /* All DMA channels are initially disabled and unallocated */
    for (idx = 0; idx < DMA_MAX_CHANNELS; idx++)
    {
      /* Channel is currently unallocated */
      dmadrv_dat.alloc_ch [idx] = FALSE;
      dmadrv_dat.cb [idx] = NULL;

      /* Make sure channel is disabled */
      dmadrv_dat.pdma->dma_chan [idx].control = 0;
      dmadrv_dat.pdma->dma_chan [idx].config_ch = 0;
    }

    /* Disable clock to DMA controller. The clock will only be
       enabled when one or moer channels are active. */
    clkpwr_clk_en_dis(CLKPWR_DMA_CLK, 0);

    init = _NO_ERROR;
  }

  return init;
}

/***********************************************************************
 *
 * Function: dma_alloc_channel
 *
 * Purpose: Allocate a channel for DMA
 *
 * Processing:
 *     If the passed channel is (-1), then a search loop is used to
 *     find the first unallocated channel. The channel value is saved
 *     and then checked to make sure it is unallocated. If it is
 *     already allocated or not allocatable, then an error si return to
 *     the caller. If the channel is not allocated, the channel is
 *     marked as allocated and the channel ID is returned to the caller.
 *     If at leasxt one channel is active, the DMA clock is enabled.
 *
 * Parameters:
 *     ch : Must be 0 (highest priority) to 7, or -1 for auto-allocation
 *     cb : Pointer to user callback function when an interrupt occurs
 *
 * Outputs: None
 *
 * Returns: The channel index, or _ERROR if a channel wasn't allocated
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dma_alloc_channel(INT_32 ch,
                         PFV cb)
{
  INT_32 challoc = ch;

  /* If auto-allocate is used, find the first available channel
     starting with the highest priority first */
  if (ch == -1)
  {
    ch = 0;
    challoc = _ERROR;
    while (ch < DMA_MAX_CHANNELS)
    {
      if (dmadrv_dat.alloc_ch [ch] == FALSE)
      {
        /* Channel is free, use it */
        challoc = ch;
        ch = DMA_MAX_CHANNELS;
      }
      else
      {
        /* Try next channel */
        ch++;
      }
    }
  }

  /* Only continue if channel is ok */
  if (challoc != _ERROR)
  {
    /* If the current channel is allocated already, then return an
        error instead */
    if (dmadrv_dat.alloc_ch [challoc] == FALSE)
    {
      /* Channel is free, so use it */
      dmadrv_dat.alloc_ch [challoc] = TRUE;
      dmadrv_dat.cb [challoc] = cb;
      dmadrv_dat.num_alloc_ch++;

      /* Enable DMA clock if at least 1 DMA channel is used */
      if (dmadrv_dat.num_alloc_ch == 1)
      {
        clkpwr_clk_en_dis(CLKPWR_DMA_CLK, 1);

        /* Enable DMA controller */
        dmadrv_dat.pdma->config = DMAC_CTRL_ENABLE;

        /* Install DMA interrupt handler in interrupt controller
           and enable DMA interrupt */
        int_install_irq_handler(IRQ_DMA, (PFV) dma_interrupt);
        int_enable(IRQ_DMA);
      }
    }
    else
    {
      /* Selected channel is allocated, return an error */
      challoc = _ERROR;
    }
  }

  return challoc;
}

/***********************************************************************
 *
 * Function: dma_free_channel
 *
 * Purpose: Return (free) an allocated DMA channel
 *
 * Processing:
 *     If the channel has been previously allocated, then deallocate
 *     the channel and disable the channel in the DMA controller. If
 *     no other DMA channels are enabled, the disable the DMA controller
 *     along with the controller clock and DMA interrupts.
 *
 * Parameters:
 *     ch : Must be 0 to 7
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the channel was freed, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dma_free_channel(INT_32 ch)
{
  STATUS status = _ERROR;
  if (dmadrv_dat.alloc_ch [ch] == TRUE)
  {
    /* Deallocate channel */
    dmadrv_dat.alloc_ch [ch] = FALSE;
    dmadrv_dat.num_alloc_ch--;

    /* Shut down channel */
    dmadrv_dat.pdma->dma_chan [ch].control = 0;
    dmadrv_dat.pdma->dma_chan [ch].config_ch = 0;
    dmadrv_dat.pdma->sync &= ~_BIT(ch);

    /* If no other DMA channels are enabled, then disable the DMA
       controller and disable the DMA clock */
    if (dmadrv_dat.num_alloc_ch == 0)
    {
      dmadrv_dat.pdma->config = 0;
      clkpwr_clk_en_dis(CLKPWR_DMA_CLK, 0);

      /* Disable DMA interrupt */
      int_install_irq_handler(IRQ_DMA, (PFV) NULL);
      int_disable(IRQ_DMA);
    }

    status = _NO_ERROR;
  }

  return status;
}

/***********************************************************************
 *
 * Function: dma_start_m2m
 *
 * Purpose: Start a M2M transfer on an allocated DMA channel
 *
 * Processing:
 *     A default memmory to memory DMA operation is performed with the
 *     passed channel, source, destination, and size. If the linked
 *     list pointer is not NULL, the DMA transfer is setup from the
 *     first linked list entry (including the control word).
 *
 * Parameters:
 *     ch    : Must be 0 to 7
 *     src   : Physical address of source data
 *     dest  : Physical address of destination data
 *     plli  : Physcial address of linked list (negates src and dest)
 *     trans : Number of transfers (when plli is NULL)
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the transfer was started, otherwise _ERROR
 *
 * Notes:
 *     This is a basic memory to memory transfer function. This starts
 *     the transfer and returns immediately before the transfer may be
 *     complete. No status about the ongoing transfer is returned to
 *     the caller. If more control or status is needed by the caller,
 *     it should monitor and check DMA channel status after the call to
 *     this function, or use an alternate function.
 *
 **********************************************************************/
STATUS dma_start_m2m(INT_32 ch,
                     void *src,
                     void *dest,
                     DMAC_LL_T *plli,
                     INT_32 trans)
{
  STATUS sts = _ERROR;

  /* Verify that the selected channel has been allocated */
  if (dmadrv_dat.alloc_ch [ch] == TRUE)
  {
    /* Setup source and destination and clear LLI */
    dmadrv_dat.pdma->dma_chan [ch].src_addr = (UNS_32) src;
    dmadrv_dat.pdma->dma_chan [ch].dest_addr = (UNS_32) dest;
    dmadrv_dat.pdma->dma_chan [ch].lli = (UNS_32) plli;

    /* Use linked list control word if available */
    if (plli != NULL)
    {
      dmadrv_dat.pdma->dma_chan [ch].control = plli->next_ctrl;
    }
    else
    {
      /* Setup channel configuration */
      dmadrv_dat.pdma->dma_chan [ch].control =
        (DMAC_CHAN_INT_TC_EN | DMAC_CHAN_DEST_AUTOINC |
         DMAC_CHAN_SRC_AUTOINC | DMAC_CHAN_DEST_AHB1 |
         DMAC_CHAN_SRC_AHB1 | DMAC_CHAN_DEST_WIDTH_32 |
         DMAC_CHAN_SRC_WIDTH_32 | DMAC_CHAN_DEST_BURST_4 |
         DMAC_CHAN_SRC_BURST_4 |
         DMAC_CHAN_TRANSFER_SIZE(trans));
    }

    /* Start channel transfer */
    dmadrv_dat.pdma->dma_chan [ch].config_ch =
      (DMAC_CHAN_FLOW_D_M2M | DMAC_CHAN_ENABLE | DMAC_CHAN_ITC |
      DMAC_CHAN_IE);

    sts = _NO_ERROR;
  }

  return sts;
}

/***********************************************************************
 *
 * Function: dma_setup_link_phy
 *
 * Purpose: Make a linked list entry (for physical addresses only)
 *
 * Processing:
 *     Place the source, destination, and DMA control word entries
 *     into the passed structure.
 *
 * Parameters:
 *     plink       : Pointer to linked list entry
 *     dmasrc_phy  : Physical address of source data
 *     dmadest_phy : Physical address of destination data
 *     dma_ctrl    : DMA control word for the transfer
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void dma_setup_link_phy(DMAC_LL_T *plink,
                        UNS_32 *dmasrc_phy,
                        UNS_32 *dmadest_phy,
                        UNS_32 dma_ctrl)
{
  plink->dma_src = (UNS_32) dmasrc_phy;
  plink->dma_dest = (UNS_32) dmadest_phy;
  plink->next_ctrl = dma_ctrl;
}

/***********************************************************************
 *
 * Function: dma_setup_virt_link
 *
 * Purpose: Make a linked list entry (for virtual addresses only)
 *
 * Processing:
 *     Convert the source and destination addresses to physical
 *     addresses, Call dma_setup_link_phy() to place the source,
 *     destination, and DMA control word entries into the passed
 *     structure.
 *
 * Parameters:
 *     plink        : Pointer to linked list entry
 *     dmasrc_virt  : Virtual address of source data
 *     dmadest_virt : Virtual address of destination data
 *     dma_ctrl     : DMA control word for the transfer
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void dma_setup_virt_link(DMAC_LL_T *plink,
                         UNS_32 *dmasrc_virt,
                         UNS_32 *dmadest_virt,
                         UNS_32 dma_ctrl)
{
  UNS_32 *src_phy, *dest_phy;

  src_phy = (UNS_32 *) cp15_map_virtual_to_physical(dmasrc_virt);
  dest_phy = (UNS_32 *) cp15_map_virtual_to_physical(dmadest_virt);

  dma_setup_link_phy(plink, src_phy, dest_phy, dma_ctrl);
}

/***********************************************************************
 *
 * Function: dma_get_base
 *
 * Purpose: Return pointer to DMA registers
 *
 * Processing:
 *     Convert the source and destination addresses to physical
 *     addresses, Call dma_setup_link_phy() to place the source,
 *     destination, and DMA control word entries into the passed
 *     structure.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Pointer to DMA base registers
 *
 * Notes: None
 *
 **********************************************************************/
DMAC_REGS_T *dma_get_base(void)
{
  return dmadrv_dat.pdma;
}
