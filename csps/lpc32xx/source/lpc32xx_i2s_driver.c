/***********************************************************************
 * $Id:: lpc32xx_i2s_driver.c 2398 2009-10-28 00:21:31Z wellsk         $
 *
 * Project: LPC32xx I2S driver
 *
 * Description:
 *     This file contains driver support for the I2S module on the
 *     LPC32xx
 *
 * Notes:
 *     The number of different configurations supported by the I2S
 *     is beyond the scope of this driver. This driver provides the
 *     following basic functions:
 *     Sequenced analog to digital conversions (polled, interrupt)
 *     I2S status polling and general configuration
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

#include "lpc32xx_i2s_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"

/***********************************************************************
 * I2S driver private data and types
 **********************************************************************/

/* Size of the I2S receive ring buffer */
#define I2S_RING_BUFSIZE 64

/* I2S device configuration structure type */
typedef struct
{
  BOOL_32 init;                 /* Device initialized flag */
  INT_32 i2snum;                /* Used for array indicing, 0 = I2S0 */
  INT_32 i2s_w_sz;              /* i2s word size, 8,16 or 32 */
  I2S_REGS_T *regptr;           /* Pointer to I2S registers */
  UNS_32 rx[I2S_RING_BUFSIZE];  /* I2S data ring buffer */
  INT_32 rx_head;     	        /* I2S ring buffer head index */
  INT_32 rx_tail;               /* I2S ring buffer tail index */
} I2S_CFG_T;

/* I2S driver data */
STATIC I2S_CFG_T i2sdat [2];
					


/***********************************************************************
 *
 * Function: i2s_dma_init_dev
 *
 * Purpose: Initialize DMA for I2S
 *
 * Processing:
 *     See function
 *
 * Parameters:
 *          dmach:  DMA Channel number
 *          p_i2s_dma_prms: dma parameters
 *
 *
 * Outputs: None
 *
 * Returns:	if ok returns TRUE
 *
 *
 **********************************************************************/
INT_32 i2s_dma_init_dev(INT_32 devid, I2S_DMA_PRMS_T *p_i2s_dma_prms)

{

  INT_32 DMAC_CHAN_DEST_WIDTH;
  INT_32 DMAC_CHAN_SRC_WIDTH ;
  INT_32 i2s_ww, i2sch;
  INT_32 dmach, dir, mem, sz;

  I2S_CFG_T *pi2s = (I2S_CFG_T *) devid;
  i2sch = pi2s->i2snum;
  i2s_ww = pi2s->i2s_w_sz ;

 
  dmach = p_i2s_dma_prms->dmach;
  dir   = p_i2s_dma_prms->dir;
  mem   = p_i2s_dma_prms->mem;
  sz    = p_i2s_dma_prms->sz;

  /* clear TC for the  selected dma channel */
  DMAC->int_tc_clear |= _SBF(0, dmach);

  /* Set the DMA src and dst word width based on I2S Word 
  width setting */
  if (i2s_ww == I2S_WW8)
  {
    DMAC_CHAN_DEST_WIDTH = DMAC_CHAN_DEST_WIDTH_8;
    DMAC_CHAN_SRC_WIDTH = DMAC_CHAN_SRC_WIDTH_8;
  }
  else if (i2s_ww == I2S_WW16)
  {
    DMAC_CHAN_DEST_WIDTH = DMAC_CHAN_DEST_WIDTH_16;
    DMAC_CHAN_SRC_WIDTH = DMAC_CHAN_SRC_WIDTH_16;
  }
  else
  {
    DMAC_CHAN_DEST_WIDTH = DMAC_CHAN_DEST_WIDTH_32;
    DMAC_CHAN_SRC_WIDTH = DMAC_CHAN_SRC_WIDTH_32;
  }

  /* Setup DMA for I2S Channel 0, DEST uses AHB1, SRC uses AHB0 */
  if (i2sch == I2S_CH0)
  {
    /* dma is flow controller */
    if (dir == DMAC_CHAN_FLOW_D_M2P)
    {
      DMAC->dma_chan[dmach].src_addr = mem;
      DMAC->dma_chan[dmach].dest_addr = (UNS_32) & I2S0->i2s_tx_fifo;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_TRANSFER_SIZE(sz) 
                                      | DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_DEST_AHB1
                                      | DMAC_CHAN_SRC_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_DEST_PERIP(DMA_PERID_I2S0_DMA0)
                              | DMAC_CHAN_FLOW_D_M2P | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;

    }
    /* peripheral is flow controller */
    else if (dir == DMAC_CHAN_FLOW_P_M2P)
    {
      DMAC->dma_chan[dmach].src_addr = mem;
      DMAC->dma_chan[dmach].dest_addr = (UNS_32) & I2S0->i2s_tx_fifo;
      DMAC->dma_chan[dmach].control =  DMAC_CHAN_SRC_BURST_4
                                       | DMAC_CHAN_DEST_BURST_4 
                                       | DMAC_CHAN_DEST_AHB1
                                       | DMAC_CHAN_SRC_AUTOINC 
                                       | DMAC_CHAN_INT_TC_EN
                                       | DMAC_CHAN_SRC_WIDTH 
                                       | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_DEST_PERIP(DMA_PERID_I2S0_DMA0)
                              | DMAC_CHAN_FLOW_P_M2P | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;

    }
    /* dma is flow controller */
    else if (dir == DMAC_CHAN_FLOW_D_P2M)
    {
      DMAC->dma_chan[dmach].src_addr = (UNS_32) & I2S0->i2s_rx_fifo;
      DMAC->dma_chan[dmach].dest_addr = mem;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_TRANSFER_SIZE(sz) 
                                      | DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_SRC_AHB1
                                      | DMAC_CHAN_DEST_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_SRC_PERIP(DMA_PERID_I2S0_DMA1)
                              | DMAC_CHAN_FLOW_D_P2M | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;
    }
    /* peripheral is flow controller */
    else if (dir == DMAC_CHAN_FLOW_P_P2M)
    {
      DMAC->dma_chan[dmach].src_addr = (UNS_32) & I2S0->i2s_rx_fifo;
      DMAC->dma_chan[dmach].dest_addr = mem;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_SRC_AHB1
                                      | DMAC_CHAN_DEST_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_SRC_PERIP(DMA_PERID_I2S0_DMA1)
                              | DMAC_CHAN_FLOW_P_P2M | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;
    }
  }
  /* Setup DMA for I2S Channel 1 */
  else if (i2sch == I2S_CH1)
  {
    /* dma is flow controller */
    if (dir == DMAC_CHAN_FLOW_D_M2P)
    {
      DMAC->dma_chan[dmach].src_addr = mem;
      DMAC->dma_chan[dmach].dest_addr = (UNS_32) & I2S1->i2s_tx_fifo;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_TRANSFER_SIZE(sz) 
                                      | DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_DEST_AHB1
                                      | DMAC_CHAN_SRC_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_DEST_PERIP(DMA_PERID_I2S1_DMA0)
                              | DMAC_CHAN_FLOW_D_M2P | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;
    }
    /* peripheral is flow controller */
    else if (dir == DMAC_CHAN_FLOW_P_M2P)
    {
      DMAC->dma_chan[dmach].src_addr = mem;
      DMAC->dma_chan[dmach].dest_addr = (UNS_32) & I2S1->i2s_tx_fifo;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_DEST_AHB1
                                      | DMAC_CHAN_SRC_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_DEST_PERIP(DMA_PERID_I2S1_DMA0)
                              | DMAC_CHAN_FLOW_P_M2P | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;
    }
    /* dma is flow controller */
    else if (dir == DMAC_CHAN_FLOW_D_P2M)
    {
      DMAC->dma_chan[dmach].src_addr = (UNS_32) & I2S1->i2s_rx_fifo;
      DMAC->dma_chan[dmach].dest_addr = mem;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_TRANSFER_SIZE(sz) 
                                      | DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_SRC_AHB1
                                      | DMAC_CHAN_DEST_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_SRC_PERIP(DMA_PERID_I2S1_DMA1)
                              | DMAC_CHAN_FLOW_D_P2M 
                              | DMAC_CHAN_IE
                              | DMAC_CHAN_ITC;
    }
    /* peripheral is flow controller */
    else if (dir == DMAC_CHAN_FLOW_P_P2M)
    {
      DMAC->dma_chan[dmach].src_addr = (UNS_32) & I2S1->i2s_rx_fifo;
      DMAC->dma_chan[dmach].dest_addr = mem;
      DMAC->dma_chan[dmach].control = DMAC_CHAN_SRC_BURST_4
                                      | DMAC_CHAN_DEST_BURST_4 
                                      | DMAC_CHAN_SRC_AHB1
                                      | DMAC_CHAN_DEST_AUTOINC 
                                      | DMAC_CHAN_INT_TC_EN
                                      | DMAC_CHAN_SRC_WIDTH 
                                      | DMAC_CHAN_DEST_WIDTH;

      DMAC->dma_chan[dmach].config_ch |= DMAC_CHAN_ENABLE 
                              | DMAC_SRC_PERIP(DMA_PERID_I2S1_DMA1)
                              | DMAC_CHAN_FLOW_P_P2M 
                              | DMAC_CHAN_IE 
                              | DMAC_CHAN_ITC;
    }
  }

  else
  {
    return (FALSE);
  }

  return(TRUE);
} 
/***********************************************************************
 *
 * Function: i2s_ptr_to_i2s_num
 *
 * Purpose: Convert a I2S register pointer to a I2S number
 *
 * Processing:
 *     Based on the passed I2S address, return the I2S number.
 *
 * Parameters:
 *     pi2s : Pointer to a I2S register set
 *
 * Outputs: None
 *
 * Returns: The I2S number (0 to 1) or -1 if register pointer is bad
 *
 *
 **********************************************************************/
INT_32 i2s_ptr_to_i2s_num(I2S_REGS_T *pi2s)
{
  INT_32 i2snum = -1;

  if (pi2s == I2S0)
  {
    i2snum = 0; /* I2S0 */
  }
  else if (pi2s == I2S1)
  {
    i2snum = 1; /* I2S1  */
  }

  return i2snum;
}
/***********************************************************************
 *
 * Function: i2s_set_ctrl_params
 *
 * Purpose: Setup I2S controller parameters
 *
 * Processing:
 *     This function sets up the controller options for the I2S
 *     controller interface. Options include; 8, 16 or 32bit, slave, 
 *     master, mono/stereo, mute, reset and stop as well as tx/rx rates
 *	   The Tx and Rx channels are identically configured
 *
 * Parameters:
 *     pparms : Pointer to parameter config structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS i2s_set_ctrl_params(I2S_REGS_T *i2sregs, I2S_PRMS_T *pparms)

{
  UNS_32 tmp;
  INT_32 i2snum;

  /* determine which I2S channel is in use 0 or 1*/
  i2snum = i2s_ptr_to_i2s_num(i2sregs);

  /* set i2s word width in the config array */
  i2sdat[i2snum].i2s_w_sz = pparms->i2s_word_width;

  /* reset both Tx and Rx channels */
  i2sregs->i2s_dai = i2sregs->i2s_dao = I2S_RESET;

  /* read the current Tx channel configuration */
  tmp = i2sregs->i2s_dao;

  /* set the word width */
  tmp = tmp | pparms->i2s_word_width ;

  /* Based on the word width, set the WS half period */
  tmp = (tmp & ~WSMASK_HP);
  if (pparms->i2s_word_width == I2S_WW32)
  {
    tmp = tmp | I2S_WS_HP(I2S_WW32_HP);
  }
  else if (pparms->i2s_word_width == I2S_WW16)
  {
    tmp = tmp | I2S_WS_HP(I2S_WW16_HP);
  }
  else
  {
    tmp = tmp | I2S_WS_HP(I2S_WW8_HP);
  }

  /* mono or stereo channel */
  if (pparms->i2s_mono == TRUE)
  {
    tmp = tmp | I2S_MONO;
  }
  else
  {
    tmp = tmp & ~I2S_MONO;
  }

  /* stop */
  if (pparms->i2s_stop == TRUE)
  {
    tmp = tmp | I2S_STOP;
  }
  else
  {
    tmp = tmp & ~I2S_STOP;
  }

  /* Set Rx channel in Master(0) or Slave(1) mode */
  if (pparms->i2s_rx_slave == FALSE)
  {
    tmp = tmp & ~I2S_WS_SEL; // master mode
  }
  else
  {
    tmp = tmp  | I2S_WS_SEL; // slave mode
  }

  /* write the rx channel value */
  i2sregs->i2s_dai = tmp;

  /* Set Tx channel in Master or Slave mode */
  if (pparms->i2s_tx_slave == FALSE)
  {
    tmp = tmp & ~I2S_WS_SEL; // master mode
  }
  else
  {
    tmp = tmp | I2S_WS_SEL; // slave mode
  }

  /* write the tx channel value */
  if (pparms->i2s_mute == TRUE)
  {
    i2sregs->i2s_dao = tmp | I2S_MUTE ;
  }
  else
  {
    tmp = tmp & ~I2S_MUTE;
    i2sregs->i2s_dao = tmp ;
  }

  /* set the tx/rx rate */
  i2sregs->i2s_tx_rate = i2sregs->i2s_rx_rate = pparms->i2s_rate;

  /* set the DMA1 Config register */
  i2sregs->i2s_dma0 = pparms->i2s_dma0_con;

  /* set the DMA2 Config register */
  i2sregs->i2s_dma1 = pparms->i2s_dma1_con;


  return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: i2s_default
 *
 * Purpose: Places the ISC interface and controller in a default state
 *
 * Processing:
 *			  See Function
 * Parameters:
 *     i2sregs: Pointer to an I2S register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void i2s_default(I2S_REGS_T *i2sregs)
{

  /* Setup default state of I2S in DEFAULT mode, with all
     channels set up in a safe configuration, and all
     interrupts masked. 16Bit Stereo, 48KHz (assuming 104MHz HCLK)
   FIFO Interrupt depth set to 8 (not DMA) */

  /* Set transmit channel to default state, master mode, 16bit */
  i2sregs->i2s_dao = I2S_STOP | I2S_WW16 | I2S_WS_HP(I2S_WW16_HP);

  /* Set receive channel to default state, slave mode, 16bit  */
  i2sregs->i2s_dai = I2S_STOP | I2S_WW16 | I2S_WS_SEL 
                     | I2S_WS_HP(I2S_WW16_HP);

  /* reset Tx rate to default */
  i2sregs->i2s_tx_rate = A48KHZ104MHZ16BIT;

  /* reset Rx rate to default */
  i2sregs->i2s_rx_rate = A48KHZ104MHZ16BIT;

  /* clear the DMA configuration register */
  i2sregs->i2s_dma0 = i2sregs->i2s_dma1 = 0;

  /* Set the Tx/Rx FIFO Trigger levels */
  i2sregs->i2s_irq = (I2S_IRQ_TX_DEPTH(8) | I2S_IRQ_RX_DEPTH(8));

}


/***********************************************************************
 * I2S driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: i2s_open
 *
 * Purpose: Open the I2S
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the I2S peripheral register set address,
 *     and initialize the I2S interface and controller to a default
 *     state by calling i2s_default(), and return a pointer to the I2S
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: I2S descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a I2S config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 i2s_open(void *ipbase,
                INT_32 arg)
{
  INT_32 status = 0;
  INT_32 i2snum;

  /* find a matching I2S number based on the passed pointer */
  i2snum = i2s_ptr_to_i2s_num((I2S_REGS_T *) ipbase);

  if (i2snum >= 0)
  {

    /* Has the I2S been previously initialized? */
    if (i2sdat[i2snum].init == FALSE)
    {

      /* Device is valid and not previously initialized */
      i2sdat[i2snum].init = TRUE;

      /* Device is valid and not previously initialized */
      i2sdat[i2snum].i2snum = i2snum;

      /* Default i2s word width is 16bit */
      i2sdat[i2snum].i2s_w_sz = I2S_WW16;

      /* Save and return address of peripheral block */
      i2sdat[i2snum].regptr = (I2S_REGS_T *) ipbase;

      /* Place I2S in a default state */
      i2s_default(i2sdat[i2snum].regptr);

      /* enable the I2S Channel pin muxing */
      i2s_pin_mux(i2snum, 1);

      /* Empty ring buffer and set conversion counter to 0 */
      i2sdat[i2snum].rx_head = i2sdat[i2snum].rx_tail = 0;

      /* Return pointer to I2S configuration structure */
      status = (INT_32) & i2sdat[i2snum];
    }
  }

  return status;

}

/***********************************************************************
 *
 * Function: i2s_close
 *
 * Purpose: Close the I2S
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the I2S
 *     interface and controller and return
 *     _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to I2S config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS i2s_close(INT_32 devid)
{
  I2S_CFG_T *pi2s = (I2S_CFG_T *) devid;
  STATUS status = _ERROR;
  UNS_32 tmp;

  /* Close and disable device if it was previously initialized */
  if (pi2s->init == TRUE)
  {
    /* Place I2S in a default state */
    pi2s->regptr->i2s_dao = I2S_MUTE | I2S_STOP | I2S_RESET;
    pi2s->regptr->i2s_dai = I2S_STOP | I2S_RESET;

    /* reset Tx rate to default */
    pi2s->regptr->i2s_tx_rate = 0;

    /* reset Rx rate to default */
    pi2s->regptr->i2s_rx_rate = 0;

    /* clear the DMA */
    pi2s->regptr->i2s_dma0 = 0;

    /* clear the DMA */
    pi2s->regptr->i2s_dma1 = 0;

    /* Disable interrupts */
    pi2s->regptr->i2s_irq = 0;

    /* Free I2S and Disable clock */
    pi2s->init = FALSE;

    if (pi2s->i2snum == 0)
    {
      clkpwr_clk_en_dis(CLKPWR_I2S0_CLK, 0);
    }
    else
    {
      clkpwr_clk_en_dis(CLKPWR_I2S1_CLK, 0);
    }

    /* relaese the pins muxing */
    i2s_pin_mux(pi2s->i2snum, 0);

    /* return the I2S1 DMA to default */
    if (pi2s->i2snum == I2S_CH1)
    {
      tmp = CLKPWR->clkpwr_i2s_clk_ctrl;
      tmp = tmp & ~CLKPWR_I2SCTRL_I2S1_USE_DMA;
      CLKPWR->clkpwr_i2s_clk_ctrl = tmp;
    }
    
    status = _NO_ERROR;       
   
  }

  return status;
}

/***********************************************************************
 *
 * Function: i2s_ioctl
 *
 * Purpose: I2S configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate I2S parameter.
 *
 * Parameters:
 *     devid: Pointer to I2S config structure
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
STATUS i2s_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  I2S_REGS_T *i2sregsptr;
  I2S_CFG_T *i2scfgptr = (I2S_CFG_T *) devid;
  STATUS status = _ERROR;

  if (i2scfgptr->init == TRUE)
  {
    status = _NO_ERROR;
    i2sregsptr = i2scfgptr->regptr;

    switch (cmd)
    {
      /* setup I2S chennal with default parameters */  
      case I2S_SETUP_PARAMS:
        i2s_set_ctrl_params((I2S_REGS_T *)i2sregsptr, 
                            (I2S_PRMS_T *) arg);
        break;

      case I2S_DO_MUTE:
        if (arg != 0)
        {
          /* Mute the transmit channel */
          i2sregsptr->i2s_dao |= I2S_MUTE;
        }
        else
        {
          /* Disable transmit channel mute */
          i2sregsptr->i2s_dao &= ~I2S_MUTE;
        }
        break;

      case I2S_DO_RESET:
        if (arg != 0)
        {
          /* Reset the transmit channel */
          i2sregsptr->i2s_dao |= I2S_RESET;
        }
        else
        {
          /* Clear the transmit channel reset */
          i2sregsptr->i2s_dao &= ~I2S_RESET;
        }
        break;

      case I2S_DI_RESET:
        if (arg != 0)
        {
          /* Reset the receive channel */
          i2sregsptr->i2s_dai |= I2S_RESET;
        }
        else
        {
          /* Clear the receive channel reset  */
          i2sregsptr->i2s_dai &= ~I2S_RESET;
        }
        break;

      case I2S_DO_STOP:
        if (arg != 0)
        {
          /* Stop the transmit channel FIFO filling */
          i2sregsptr->i2s_dao |= I2S_STOP;
        }
        else
        {
          /* Allow the transmit channel FIFO to fill*/
          i2sregsptr->i2s_dao &= ~I2S_STOP;
        }
        break;

      case I2S_DI_STOP:
        if (arg != 0)
        {
          /* STOP filling the receive channel FIFO*/
          i2sregsptr->i2s_dai |= I2S_STOP;
        }
        else
        {
          /* Allow the receive channel FIFO to fill */
          i2sregsptr->i2s_dai &= ~I2S_STOP;
        }
        break;

      case I2S_TX_RATE:
        if (arg != 0)
        {
          /* Set the I2S Tx rate */
          i2sregsptr->i2s_tx_rate = arg;
        }
        break;

      case I2S_RX_RATE:
        if (arg != 0)
        {
          /* Set the I2S Rx rate */
          i2sregsptr->i2s_rx_rate = arg;
        }
        break;

      case I2Sx_RX_IRQ_EN:
        if (arg != 0)
        {
          /* I2S Rx Interrupt Enable */
          i2sregsptr->i2s_irq |= I2S_RX_IRQ_EN;
        }
        else
        {
          /* I2S Rx Interrupt Disable */
          i2sregsptr->i2s_irq &= ~I2S_RX_IRQ_EN;
        }

        break;

      case I2Sx_TX_IRQ_EN:

        if (arg != 0)
        {
          /* I2S Rx Interrupt Enable */
          i2sregsptr->i2s_irq |= I2S_TX_IRQ_EN;
        }
        else
        {
          /* I2S Rx Interrupt Disable */
          i2sregsptr->i2s_irq &= ~I2S_TX_IRQ_EN;
        }
        break;

      case I2Sx_DMA0_TX_EN:

        if (arg != 0)
        {
          /* I2S Tx DMA Enable */
          i2sregsptr->i2s_dma0 |= I2S_DMA0_TX_EN;
        }
        else
        {
          /* I2S Tx DMA Disable */
          i2sregsptr->i2s_dma0 &= ~I2S_DMA0_TX_EN;
        }
        break;

      case I2Sx_DMA0_RX_EN:

        if (arg != 0)
        {
          /* I2S Rx DMA Enable */
          i2sregsptr->i2s_dma0 |= I2S_DMA0_RX_EN;
        }
        else
        {
          /* I2S Rx DMA Disable */
          i2sregsptr->i2s_dma0 &= ~I2S_DMA0_RX_EN;
        }
        break;

      case I2Sx_DMA1_TX_EN:

        if (arg != 0)
        {
          /* I2S Tx DMA Enable */
          i2sregsptr->i2s_dma1 |= I2S_DMA1_TX_EN;
        }
        else
        {
          /* I2S Tx DMA Disable */
          i2sregsptr->i2s_dma1 &= ~I2S_DMA1_TX_EN;
        }
        break;

      case I2Sx_DMA1_RX_EN:

        if (arg != 0)
        {
          /* I2S Rx DMA Enable */
          i2sregsptr->i2s_dma1 |= I2S_DMA1_RX_EN;
        }
        else
        {
          /* I2S Rx DMA Disable */
          i2sregsptr->i2s_dma1 &= ~I2S_DMA1_RX_EN;
        }
        break;

      default:
        /* Unsupported parameter */
        status = LPC_BAD_PARAMS;
        break;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: i2s_read_result
 *
 * Purpose: I2S read function (stub only)
 *
 * Processing:
 *     Returns 0 to the caller.
 *
 * Parameters:
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 i2s_read_result(INT_32 devid,
                       void *buffer)
{
  INT_32  bytes;
  bytes = 0;

  return bytes;
}


/***********************************************************************
 *
 * Function: i2s_write
 *
 * Purpose: I2S write function (stub only)
 *
 * Processing:
 *     Returns 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to I2S config structure
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
INT_32 i2s_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: i2s_read_ring
 *
 * Purpose: Reads data from the I2S ring buffer
 *
 * Processing:
 *     If the init flag for the I2S structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the I2S interrupts and
 *     disable the I2S interrupts. Loop until max_bytes equals 0 or
 *     until the receive ring buffer is empty, whichever comes
 *     first. Read the data from the ring buffer  indexed by the tail
 *     pointer and place it into the user buffer. Increment the tail
 *     pointer and user buffer pointer. If the tail pointer exceeds the
 *     buffer size, set the tail pointer to 0. Increment bytes, and
 *     decrement max_bytes. Exit the loop based on the loop conditions,
 *     re-enable the receive interrupts, and return the number of bytes
 *     read to the caller.
 *     Depending on the I2S Word Width, the function will read 8/16 or
 *     32bits of data at a time into the ring buffer and increment
 *     as appropriate
 *
 * Parameters:
 *     devid:     Pointer to an I2S configuration structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually read from the ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 i2s_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes)
{

  I2S_CFG_T *i2scfgptr = (I2S_CFG_T *) devid;
  INT_32 bytes = 0;

  UNS_32 *data = (UNS_32 *) buffer;

  if (i2scfgptr->init == TRUE)
  {
    /* Temporarily lock out I2S interrupts during this
       read so the I2S receive interrupt won't cause problems
       with the ring buffer index values */
    if (i2scfgptr->i2snum == I2S_CH0)
    {
      int_disable(IRQ_I2S0);
    }
    else if (i2scfgptr->i2snum == I2S_CH1)
    {
      int_disable(IRQ_I2S1);
    }

    /* Loop until receive ring buffer is empty or until max_bytes
       expires */
    while ((max_bytes > 0) &&
           (i2scfgptr->rx_tail != i2scfgptr->rx_head))
    {
      /* Read data from ring buffer into user buffer */
      *data = i2scfgptr->rx[i2scfgptr->rx_tail];
      data++;

      /* Update tail pointer */
      i2scfgptr->rx_tail++;

      /* Make sure tail didn't overflow */
      if (i2scfgptr->rx_tail >= I2S_RING_BUFSIZE)
      {
        i2scfgptr->rx_tail = 0;
      }

      /* Increment data count and decrement buffer size count */
      bytes = bytes + 4;
      max_bytes = max_bytes - 4;
    }

    /* Re-enable I2S receive interrupt(s) */
    if (i2scfgptr->i2snum == 0)
    {
      int_enable(IRQ_I2S0);
    }
    else if (i2scfgptr->i2snum == 1)
    {
      int_enable(IRQ_I2S1);
    }

  }

  return bytes;

}
/***********************************************************************
 *
 * Function: i2s_ring_fill
 *
 * Purpose: Move data from the I2S DAT to the driver ring buffer
 *
 * Processing:
 *     While there is I2S FIFO data, copy an entry from the I2S FIFO into
 *     the IS2 driver ring buffer. Increment the ring buffer
 *     head pointer. If it exceeds the ring buffer size, reset the
 *     head pointer to 0.
 *
 * Parameters:
 *     i2scfgptr: Pointer to I2S config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void i2s_ring_fill(I2S_CFG_T *i2scfgptr)
{
  I2S_REGS_T *i2sregsptr = i2scfgptr->regptr;

  /* Read from Rx FIFO until empty */
  while ((i2sregsptr->i2s_stat & I2S_RX_STATE_MASK) != 0)
  {
    i2scfgptr->rx[i2scfgptr->rx_head] = i2sregsptr->i2s_rx_fifo;

    /* Increment receive ring buffer head pointer */
    i2scfgptr->rx_head++;
    if (i2scfgptr->rx_head >= I2S_RING_BUFSIZE)
    {
      i2scfgptr->rx_head = 0;
    }
  }
}
/***********************************************************************
 *
 * Function: i2s0_interrupt
 *
 * Purpose: I2S0 interrupt handler
 *
 * Processing:
 *     On an interrupt call the i2s_ring_fill() to move
 *     the data from the I2S
 *     FIFO to the I2S driver ring buffer.
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

void i2s0_interrupt(void)
{
  /* read the I2S result into a buffer */
  i2s_ring_fill(&i2sdat[I2S_CH0]);
}
/***********************************************************************
 *
 * Function: i2s1_interrupt
 *
 * Purpose: I2S1 interrupt handler
 *
 * Processing:
 *     On an interrupt call the i2s_ring_fill() to move
 *     the data from the I2S
 *     FIFO to the I2S driver ring buffer.
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

void i2s1_interrupt(void)
{
  /* read the I2S1 result into a buffer */
  i2s_ring_fill(&i2sdat[I2S_CH1]);
}

