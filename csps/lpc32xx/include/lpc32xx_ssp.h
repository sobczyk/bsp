/***********************************************************************
 * $Id:: lpc32xx_ssp.h 1090 2008-08-18 22:17:00Z wellsk                $
 *
 * Project: LPC3250 SSP definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for LPC3250 component:
 *         Synchronous Serial Port
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

#ifndef LPC3250_SSP_H
#define LPC3250_SSP_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

/***********************************************************************
 * SSP Module Register Structure
 **********************************************************************/

/* SSP Module Register Structure */
typedef struct
{
  volatile UNS_32 cr0;     /* SSP control register 0 */
  volatile UNS_32 cr1;     /* SSP control register 1 */
  volatile UNS_32 data;    /* SSP data register */
  volatile UNS_32 sr;      /* SSP status register */
  volatile UNS_32 cpsr;    /* SSP clock prescale register */
  volatile UNS_32 imsc;    /* SSP interrupt mask register */
  volatile UNS_32 ris;     /* SSP raw interrupt status register */
  volatile UNS_32 mis;     /* SSP masked interrupt status register */
  volatile UNS_32 icr;     /* SSP interrupt clear register */
  volatile UNS_32 dmacr;   /* SSP DMA enable register */
} SSP_REGS_T;

/***********************************************************************
 * cr0 register definitions
 **********************************************************************/
/* SSP data size load macro, must be 4 bits to 16 bits */
#define SSP_CR0_DSS(n)   _SBF(0, (((n) - 1) & 0xF)) // Data Size Select
/* SSP control 0 Motorola SPI mode */
#define SSP_CR0_FRF_SPI  0x00000000
/* SSP control 0 TI synchronous serial mode */
#define SSP_CR0_FRF_TI   0x00000010
/* SSP control 0 National Microwire mode */
#define SSP_CR0_FRF_NS   0x00000020
/* SSP control 0 protocol mask */
#define SSP_CR0_PRT_MSK  0x00000030
/* SPI clock polarity bit (used in SPI mode only), (1) = maintains the
   bus clock high between frames, (0) = low */
#define SSP_CR0_CPOL(n)  _SBF(6, ((n) & 0x01))
/* SPI clock out phase bit (used in SPI mode only), (1) = captures data
   on the second clock transition of the frame, (0) = first */
#define SSP_CR0_CPHA(n)  _SBF(7, ((n) & 0x01))
/* SSP serial clock rate value load macro, divider rate is
   PERIPH_CLK / (cpsr * (SCR + 1)) */
#define SSP_CR0_SCR(n)   _SBF(8, ((n) & 0xFF))

/***********************************************************************
 * cr1 register definitions
 **********************************************************************/
/* SSP control 1 loopback mode enable bit */
#define SSP_CR1_LBM         _BIT(0)
/* SSP control 1 enable bit */
#define SSP_CR1_SSE(n)      _SBF(1, ((n) & 0x01))
#define SSP_CR1_SSP_ENABLE  _BIT(1)
#define SSP_CR1_SSP_DISABLE 0
/* SSP control 1 master/slave bit, (1) = master, (0) = slave */
#define SSP_CR1_MS       _BIT(2)
#define SSP_CR1_MASTER   0
#define SSP_CR1_SLAVE    _BIT(2)
/* SSP control 1 slave out disable bit, disables transmit line in slave
   mode */
#define SSP_CR1_SOD      _BIT(3)

/***********************************************************************
 * data register definitions
 **********************************************************************/
/* SSP data load macro */
#define SSP_DATAMASK(n)   ((n) & 0xFFFF)

/***********************************************************************
 * SSP status register (sr) definitions
 **********************************************************************/
/* SSP status TX FIFO Empty bit */
#define SSP_SR_TFE      _BIT(0)
/* SSP status TX FIFO not full bit */
#define SSP_SR_TNF      _BIT(1)
/* SSP status RX FIFO not empty bit */
#define SSP_SR_RNE      _BIT(2)
/* SSP status RX FIFO full bit */
#define SSP_SR_RFF      _BIT(3)
/* SSP status SSP Busy bit */
#define SSP_SR_BSY      _BIT(4)

/***********************************************************************
 * SSP clock prescaler register (cpsr) definitions
 **********************************************************************/
/* SSP clock prescaler load macro */
#define SSP_CPSR_CPDVSR(n) _SBF(0, (n) & 0xFE)

/***********************************************************************
 * SSP interrupt registers (imsc, ris, mis, icr) definitions
 **********************************************************************/
/* SSP interrupt bit for RX FIFO overflow */
#define SSP_IMSC_RORIM   _BIT(0)
#define SSP_RIS_RORRIS   _BIT(0)
#define SSP_MIS_RORMIS   _BIT(0)
#define SSP_ICR_RORIC    _BIT(0)
/* SSP interrupt bit for RX FIFO not empty and has a data timeout */
#define SSP_IMSC_RTIM    _BIT(1)
#define SSP_RIS_RTRIS    _BIT(1)
#define SSP_MIS_RTMIS    _BIT(1)
#define SSP_ICR_RTIC     _BIT(1)
/* SSP interrupt bit for RX FIFO half full */
#define SSP_IMSC_RXIM    _BIT(2)
#define SSP_RIS_RXRIS    _BIT(2)
#define SSP_MIS_RXMIS    _BIT(2)
/* SSP interrupt bit for TX FIFO half empty */
#define SSP_IMSC_TXIM    _BIT(3)
#define SSP_RIS_TXRIS    _BIT(3)
#define SSP_MIS_TXMIS    _BIT(3)

/***********************************************************************
 * SSP DMA enable register (dmacr) definitions
 **********************************************************************/
/* SSP bit for enabling RX DMA */
#define SSP_DMA_RXDMAEN  _BIT(0)
/* SSP bit for enabling TX DMA */
#define SSP_DMA_TXDMAEN  _BIT(1)

/* Macros pointing to SSP registers */
#define SSP0  ((SSP_REGS_T *)(SSP0_BASE))
#define SSP1  ((SSP_REGS_T *)(SSP1_BASE))

#endif /* LPC3250_SSP_H */
