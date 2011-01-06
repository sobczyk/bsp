/***********************************************************************
* $Id:: lpc32xx_i2s.h 924 2008-07-23 17:57:39Z wellsk                 $
*
* Project: LPC32xx i2s controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC3xxx chip family component:
*         I2S controller
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

#ifndef LPC32xx_I2S_H
#define LPC32xx_I2S_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* I2S controller register structures
**********************************************************************/

/* I2S controller module register structures */
typedef struct
{
  volatile UNS_32 i2s_dao;      /* Digital Audio output register */
  volatile UNS_32 i2s_dai;      /* Digital Audio input register */
  volatile UNS_32 i2s_tx_fifo;  /* Transmit FIFO register */
  volatile UNS_32 i2s_rx_fifo;  /* Receive FIFO register */
  volatile UNS_32 i2s_stat;     /* I2S Status register */
  volatile UNS_32 i2s_dma0;     /* DMA0 Configuration register*/
  volatile UNS_32 i2s_dma1;     /* DMA1 Configuration register*/
  volatile UNS_32 i2s_irq;      /* Interrupt Request Control register*/
  volatile UNS_32 i2s_tx_rate;  /* Transmit bit rate register*/
  volatile UNS_32 i2s_rx_rate;  /* Receive bitrate register*/
} I2S_REGS_T;

/**********************************************************************
* i2s_daO i2s_dai register definitions
**********************************************************************/
#define I2S_WW8      _SBF(0,0) /* Word width is 8bit*/
#define I2S_WW16     _SBF(0,1) /* Word width is 16bit*/
#define I2S_WW32     _SBF(0,3) /* Word width is 32bit*/
#define I2S_MONO     _BIT(2)   /* Mono */
#define I2S_STOP     _BIT(3)   /* Stop, diables the access to FIFO, 
                                  mutes the channel */
#define I2S_RESET    _BIT(4)   /* Reset the channel */
#define I2S_WS_SEL   _BIT(5)   /* Channel Master(0) or slave(1) 
                                  mode select*/
#define I2S_WS_HP(s) _SBF(6,s) /* Word select half period - 1 */

#define I2S_MUTE     _BIT(15)  /* Mute the channel, 
                                  Transmit channel only */

#define I2S_WW32_HP  0x1f /* Word select half period for 32bit 
                             word width */
#define I2S_WW16_HP  0x0f /* Word select half period for 16bit 
                             word width */
#define I2S_WW8_HP   0x7  /* Word select half period for 8bit
                             word width */

#define WSMASK_HP	  0X7FC /* Mask for WS half period bits */

/**********************************************************************
* i2s_tx_fifo register definitions
**********************************************************************/
#define I2S_FIFO_TX_WRITE(d)              (d)

/**********************************************************************
* i2s_rx_fifo register definitions
**********************************************************************/
#define I2S_FIFO_RX_WRITE(d)              (d)

/**********************************************************************
* i2s_stat register definitions
**********************************************************************/
#define I2S_IRQ_STAT     _BIT(0)
#define I2S_DMA0_REQ     _BIT(1)
#define I2S_DMA1_REQ     _BIT(2)

#define I2S_RX_STATE_MASK	0x0000ff00
#define I2S_TX_STATE_MASK	0x00ff0000

/**********************************************************************
* i2s_dma0 Configuration register definitions
**********************************************************************/
#define I2S_DMA0_RX_EN     _BIT(0)       /* Enable RX DMA1*/
#define I2S_DMA0_TX_EN     _BIT(1)       /* Enable TX DMA1*/
#define I2S_DMA0_RX_DEPTH(s)  _SBF(8,s)  /* Set the level for DMA1 
                                            RX Request */
#define I2S_DMA0_TX_DEPTH(s)  _SBF(16,s) /* Set the level for DMA1 
                                            TX Request */

/**********************************************************************
* i2s_dma1 Configuration register definitions
**********************************************************************/
#define I2S_DMA1_RX_EN     _BIT(0)       /* Enable RX DMA1*/
#define I2S_DMA1_TX_EN     _BIT(1)       /* Enable TX DMA1*/
#define I2S_DMA1_RX_DEPTH(s)  _SBF(8,s)	 /* Set the level for DMA1 
                                            RX Request */
#define I2S_DMA1_TX_DEPTH(s)  _SBF(16,s) /* Set the level for DMA1 
                                            TX Request */

/**********************************************************************
* i2s_irq register definitions
**********************************************************************/
#define I2S_RX_IRQ_EN     _BIT(0)       /* Enable RX IRQ*/
#define I2S_TX_IRQ_EN     _BIT(1)       /* Enable TX IRQ*/
#define I2S_IRQ_RX_DEPTH(s)  _SBF(8,s)  /* valid values ar 0 to 7 */
#define I2S_IRQ_TX_DEPTH(s)  _SBF(16,s) /* valid values ar 0 to 7 */

/**********************************************************************
* define audio rates for i2s_tx_rate/i2s_rx_rate register definitions
**********************************************************************/

#define A96KHZ104MHZ8BIT 0x7ed  // 7, 237
#define A48KHZ104MHZ8BIT 0x3cb  // 3, 203
#define A44KHZ104MHZ8BIT 0x14a  // 1, 74
#define A32KHZ104MHZ8BIT 0x5fe	// 5, 254
#define A22KHZ104MHZ8BIT 0x194  // 1, 148
#define A16KHZ104MHZ8BIT 0x1cb	// 1, 203

#define A96KHZ104MHZ16BIT 0xeed	//  14, 237
#define A48KHZ104MHZ16BIT 0x7ED //  7, 237
#define A44KHZ104MHZ16BIT 0x6dd //  6, 221	
#define A32KHZ104MHZ16BIT 0x5fe //  5, 254
#define A22KHZ104MHZ16BIT 0x14a //  1, 74
#define A16KHZ104MHZ16BIT 0x2cb //  2, 203

#define A96KHZ104MHZ32BIT 0x1ced// 28, 237
#define A48KHZ104MHZ32BIT 0xeed // 14, 237 
#define A44KHZ104MHZ32BIT 0xdf0 // 13, 240 
#define A32KHZ104MHZ32BIT 0x57f	// 5, 127
#define A22KHZ104MHZ32BIT 0x125 // 1, 37 
#define A16KHZ104MHZ32BIT 0x5fe // 5, 254 

/**********************************************************************
* i2s_tx_rate register definitions
**********************************************************************/
#define I2S_TX_RATE(d)              (d)

/**********************************************************************
* i2s_rx_rate register definitions
**********************************************************************/
#define I2S_RX_RATE(d)              (d)

/**********************************************************************
* i2s channel select
**********************************************************************/
#define I2S_CH0	0
#define I2S_CH1	1

/* Macro pointing to I2S controller registers */
#define I2S0  ((I2S_REGS_T  *)(I2S0_BASE))
#define I2S1  ((I2S_REGS_T  *)(I2S1_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_I2S_H */
