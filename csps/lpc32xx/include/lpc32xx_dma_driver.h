/***********************************************************************
 * $Id:: lpc32xx_dma_driver.h 974 2008-07-28 21:07:32Z wellsk          $
 *
 * Project: LPC32xx DMA driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx DMA controller.
 *
 * Notes:
 *     This driver is a helper driver only for DMA support. To use DMA
 *     in a peripheral will require writing some DMA support code for
 *     that peripheral in it's driver using these supporting functions.
 *     This driver provides the following DMA support functions:
 *       - Initial DMA controller and driver setup
 *       - Channel allocation and deallocation
 *       - DMA controller enable/disable
 *       - Automatic DMA clock control
 *       - Interrupt clearning and routing to user functions
 *     This driver uses physical addresses only.
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

#ifndef LPC32XX_DMA_DRIVER_H
#define LPC32XX_DMA_DRIVER_H

#include "lpc32xx_dmac.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * DMA driver functions
 **********************************************************************/

/* Initial DMA controller and driver */
STATUS dma_init(void);

/* Allocate a channel for DMA, use ch for selected channel (0 to 7) or
   -1 to use the highest priority available channel. Also sets up the
   user callback function for the channel's DMA interrupt. */
INT_32 dma_alloc_channel(INT_32 ch,
                         PFV cb);

/* Return (free) an allocated DMA channel */
STATUS dma_free_channel(INT_32 ch);

/* Start a M2M transfer on an allocated DMA channel */
STATUS dma_start_m2m(INT_32 ch,
                     void *src,
                     void *dest,
                     DMAC_LL_T *plli,
                     INT_32 trans);

/* Make a linked list entry (for virtual addresses only) */
void dma_setup_virt_link(DMAC_LL_T *plink,
                         UNS_32 *dmasrc_virt,
                         UNS_32 *dmadest_virt,
                         UNS_32 dma_ctrl);

/* Make a linked list entry (for physical addresses only) */
void dma_setup_link_phy(DMAC_LL_T *plink,
                        UNS_32 *dmasrc_phy,
                        UNS_32 *dmadest_phy,
                        UNS_32 dma_ctrl);

/* Enable or disable SYNC logic for a specific peripheral, periph must
   be a value of type DMA_PER_xxx, see the DMA peripheral header file */
void dma_enable_sync(UNS_32 periph,
                     BOOL_32 enable);

/* Return pointer to DMA registers */
DMAC_REGS_T *dma_get_base(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_DMA_DRIVER_H */
