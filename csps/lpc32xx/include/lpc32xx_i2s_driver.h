/***********************************************************************
 * $Id:: lpc32xx_i2s_driver.h 809 2008-06-11 20:29:59Z kendwyer        $
 *
 * Project: LPC32xx I2S driver
 *
 * Description:
 *     This file contains driver support for the I2S module on the
 *     LPC3250
 *
 * Notes:
 *     See the lpc32xx_i2s_driver.c file for more information on this
 *     driver.
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

#ifndef LPC32xx_I2S_DRIVER_H
#define LPC32xx_I2S_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc32xx_i2s.h"

/* General I2S controller parameters */
typedef struct
{
  UNS_32  i2s_word_width;   /* Tx/Rx channel word width */
  BOOL_32 i2s_mono;         /* Set channel stereo or mono operation */
  BOOL_32 i2s_stop;         /* STOP I2S channel */
  BOOL_32 i2s_tx_slave;     /* Master(0) or slave(1) mode for 
                               I2S TX Channel */
  BOOL_32 i2s_rx_slave;	    /* Master(0) or slave(1) mode for 
                               I2S RX Channel */
  BOOL_32 i2s_mute;         /* I2S Mute Tx channel */
  UNS_32  i2s_rate;         /* I2S clock rate for tx and rx channel */
  UNS_32  i2s_dma0_con;     /* I2S DMA1 configuration register */
  UNS_32  i2s_dma1_con;     /* I2S DMA1 configuration register */

} I2S_PRMS_T;

/* General I2S controller parameters for DMA */
typedef struct
{
  INT_32 dmach;    /* dma channel */
  INT_32 dir;      /* dma direction (M2P, P2M and Flow Control) */   
  INT_32 mem;	   /* memory src/dst */
  INT_32 sz ;	   /* size of transfer */

} I2S_DMA_PRMS_T;

/***********************************************************************
 * I2S device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* I2S device commands (IOCTL commands) */
typedef enum
{

  I2S_SETUP_PARAMS,     /* Configure the I2S Channel. The Tx and Rx 
                           channels are assumed identicle 'arg' contains
                           the attributes to write to the I2S channel */

  I2S_DO_MUTE,          /* Mute the Transmit (DAO) channel */

  I2S_DO_STOP,	    	/* When 1, Transmit FIFO stops draining,
                           Channel muted */
  I2S_DI_STOP,	    	/* When 1, Receive FIFO will stop filling */

  I2S_DO_RESET,	        /* Asynchronously reset the transmit channel 
                           and FIFO */
  I2S_DI_RESET,	        /* Asynchronously reset the transmit channel 
                           and FIFO */

  I2Sx_RX_IRQ_EN,       /* When 1, enables RX interrupt*/
  I2Sx_TX_IRQ_EN,       /* When 1, enables TX interrupt */

  I2Sx_DMA0_TX_EN,      /* When 1, enables DMA0 for I2S transmit */
  I2Sx_DMA0_RX_EN,      /* When 1, enables DMA0 for I2S receive */

  I2Sx_DMA1_TX_EN,      /* When 1, enables DMA1 for I2S transmit */
  I2Sx_DMA1_RX_EN,      /* When 1, enables DMA1 for I2S receive */

  I2S_TX_RATE,          /* Divider for the I2S Tx clock rate */
  I2S_RX_RATE,          /* Divider for the I2S Rx clock rate */


} I2S_IOCTL_CMD_T;

/***********************************************************************
 * I2S driver API functions
 **********************************************************************/

/* Open the I2S */
INT_32 i2s_open(void *ipbase,
                INT_32 arg);

/* Close the I2S */
STATUS i2s_close(INT_32 devid);

/* I2S configuration block */
STATUS i2s_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* I2S read function (interrupt, ring buffer) */
INT_32 i2s_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes);

/* I2S write function (stub only) */
INT_32 i2s_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

/***********************************************************************
 * I2S driver miscellaneous functions
 **********************************************************************/
INT_32 i2s_dma_init_dev(INT_32 devid,
                        I2S_DMA_PRMS_T *p_i2s_dma_prms);

/* I2S interrupt handlers */
void i2s0_interrupt(void); /* used to read the I2S fifo */
void i2s1_interrupt(void); /* used to read the I2S fifo */

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_I2S_DRIVER_H */
