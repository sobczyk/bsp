/***********************************************************************
 * $Id:: lpc32xx_sdcard_driver.c 4978 2010-09-20 22:32:52Z usb10132    $
 *
 * Project: LPC32XX SD card controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx SD card
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
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_sdcard_driver.h"

/***********************************************************************
 * SDcard controller driver package data
***********************************************************************/

/* Structure used for DMA control of data */
typedef struct
{
  BOOL_32 dma_enabled;    /* Used to monitor DMA flag state */
  BOOL_32 use_dma;        /* Data transfer DMA support flag */
  DMAC_REGS_T *pdmaregs;  /* Pointer to DMA registers */
  INT_32 dmach;           /* DMA channel */
} SDCARD_DMAC_T;

/* Structure used for data transfer control */
typedef struct
{
  UNS_32  blocksize;      /* Block transfer size in bytes */
  UNS_32  data_to;        /* Data transfer timeout in clocks */
  SD_XFER_T xferdat;      /* Data transfer control structure */
  INT_32  tosend;         /* Number of words to send */
  SD_CMDRESP_T resp;      /* Last command saved response */
} SDCARD_DXFER_T;

/* SDcard controller device configuration structure type */
typedef struct
{
  BOOL_32 init;           /* Device initialized flag */
  SDCARD_REGS_T *regptr;  /* Pointer to SDcard controller registers */
  PFV     sd0_cb;         /* Command callback func pointer */
  PFV     sd1_cb;         /* Data callback func pointer */
  SDCARD_DXFER_T dctrl;   /* Data control structure */
  SDCARD_DMAC_T dmact;    /* DMA control structure */
} SDCARD_CFG_T;

/* SDcard controller driver data */
static SDCARD_CFG_T sdcarddat;

/***********************************************************************
 * SDcard controller driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: sd_dma_enable
 *
 * Purpose: Allocate and enable the SD card controller DMA channel
 *
 * Processing:
 *     Attempt to get a DMA channel from the DMA allocation driver.
 *     If the channel was allocated ok, then clear any DMA interrupts
 *     and setup the channel into default clear state.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if DMA was setup correctly, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_dma_enable(void)
{
  STATUS err = _NO_ERROR;

  /* Get a channel and setup the callback for the DMA transfer */
  sdcarddat.dmact.dmach = dma_alloc_channel(-1, NULL);
  if (sdcarddat.dmact.dmach == _ERROR)
  {
    sdcarddat.dmact.use_dma = FALSE;
    err = _ERROR;
  }
  else
  {
    /* Get pointer to DMA registers */
    sdcarddat.dmact.pdmaregs = dma_get_base();

    /* Disable and clear DMA channel interrupts */
    sdcarddat.dmact.pdmaregs->int_tc_clear =
      _BIT(sdcarddat.dmact.dmach);
    sdcarddat.dmact.pdmaregs->int_err_clear =
      _BIT(sdcarddat.dmact.dmach);
    sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.dmact.dmach].
    config_ch = 0;

    /* Enable DMA in the SD card controller */
    sdcarddat.regptr->sd_dctrl |= SD_DMA_EN;
  }

  return err;
}

/***********************************************************************
 *
 * Function: sd_dma_disable
 *
 * Purpose: Disable and return the SD card controller DMA channel
 *
 * Processing:
 *     Free (and disable) the DMA channel allocated to this driver.
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
static void sd_dma_disable(void)
{
  sdcarddat.regptr->sd_dctrl &= ~SD_DMA_EN;
  dma_free_channel(sdcarddat.dmact.dmach);
}

/***********************************************************************
 *
 * Function: prep_cmd_resp
 *
 * Purpose: Prepare a command response register function
 *
 * Processing:
 *     The command state machine is setup with the expected response
 *     type adn returned to the caller.
 *
 * Parameters:
 *     pcmd : Pointer to a command control structure
 *
 * Outputs: None
 *
 * Returns: Prepared command register word
 *
 * Notes: None
 *
 **********************************************************************/
static UNS_32 prep_cmd_resp(SD_CMD_T *pcmd)
{
  UNS_32 resp;

  /* Generate response mask */
  if (pcmd->cmd_resp_size == 0)
  {
    resp = 0;
  }
  else
  {
    resp = (SD_CMD_RESP_RECEIVED | SD_CMD_TIMEOUT |
            SD_CMD_CRC_FAIL);
  }

  return resp;
}

/***********************************************************************
 *
 * Function: prep_cmd
 *
 * Purpose: Prepare a command register function
 *
 * Processing:
 *     The command state machine is setup with the passed argument,
 *     command, expected response type, and busy wait status. If a
 *     response is expected, then the command state machine and expected
 *     interrupts are setup the receive a response. The command
 *     interrupt is cleared and enabled.
 *
 * Parameters:
 *     pcmd : Pointer to a command control structure
 *
 * Outputs: None
 *
 * Returns: Prepared command register word
 *
 * Notes: None
 *
 **********************************************************************/
static UNS_32 prep_cmd(SD_CMD_T *pcmd)
{
  UNS_32 resp, tmp;

  /* Write arg register first */
  sdcarddat.regptr->sd_arg = pcmd->arg;

  /* Generate command word */
  tmp = pcmd->cmd;
  resp = prep_cmd_resp(pcmd);
  switch (pcmd->cmd_resp_size)
  {
    case 0:
      /* No response expected */
      break;

    case 136:
      tmp |= (SD_RESPONSE | SD_LONGRESP_EN);
      break;

    default:
      tmp |= SD_RESPONSE;
      break;
  }

  /* Clear latched command statuses and enable command state
     machine interrupts */
  sdcarddat.regptr->sd_clear = (resp | SD_CMD_SENT);
  sdcarddat.regptr->sd_mask0 = (resp | SD_CMD_SENT);
  int_enable(IRQ_SD0);

  return tmp;
}

/***********************************************************************
 *
 * Function: sd1_dmarx_interrupt
 *
 * Purpose: DMA specific SD card data read interrupt handler
 *
 * Processing:
 *     See function.
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
static void sd1_dmarx_interrupt(void)
{
  UNS_32 dint;
  BOOL_32 done = FALSE;

  /* A data interrupt was received */
  dint = sdcarddat.regptr->sd_status;

  /* Has data transfer completed? */
  if ((dint & SD_DATA_END) != 0)
  {
    done = TRUE;
  }

  /* Has a data error occurred? */
  if ((dint & (SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
               SD_DATA_CRC_FAIL)) != 0)
  {
    done = TRUE;
  }

  if (done == TRUE)
  {
    int_disable(IRQ_SD1);
    sdcarddat.regptr->sd_dctrl &= ~SD_DATATRANSFER_EN;
    sdcarddat.regptr->sd_mask1 = 0;
    sdcarddat.dctrl.resp.data_status = dint;

    if (sdcarddat.sd1_cb != NULL)
    {
      sdcarddat.sd1_cb();
    }

    /* Disable DMA */
    sdcarddat.dmact.pdmaregs->sync &= ~DMA_PER_SDCARD;
    sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
                                        dmact.dmach].config_ch &=
										~DMAC_CHAN_ENABLE;
  }

  /* Clear checked statuses */
  sdcarddat.regptr->sd_clear = dint;
}

/***********************************************************************
 *
 * Function: sd1_dmatx_interrupt
 *
 * Purpose: DMA specific SD card data write interrupt handler
 *
 * Processing:
 *     See function.
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
static void sd1_dmatx_interrupt(void)
{
  UNS_32 dint;
  BOOL_32 done = FALSE;

  /* A data interrupt was received */
  dint = sdcarddat.regptr->sd_status;

  /* Has data transfer completed? */
  if ((dint & SD_DATA_END) != 0)
  {
    done = TRUE;
  }

  /* Has a data error occurred? */
  if ((dint & (SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
               SD_DATA_CRC_FAIL)) != 0)
  {
    done = TRUE;
  }

  if (done == TRUE)
  {
    int_disable(IRQ_SD1);
    sdcarddat.regptr->sd_dctrl &= ~SD_DATATRANSFER_EN;
    sdcarddat.regptr->sd_mask1 = 0;
    sdcarddat.dctrl.resp.data_status = dint;

    if (sdcarddat.sd1_cb != NULL)
    {
      sdcarddat.sd1_cb();
    }

    /* Disable DMA */
    sdcarddat.dmact.pdmaregs->sync &= ~DMA_PER_SDCARD;
    sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
                                        dmact.dmach].config_ch &=
										~DMAC_CHAN_ENABLE;
  }

  /* Clear checked statuses */
  sdcarddat.regptr->sd_clear = dint;
}

/***********************************************************************
 *
 * Function: sd1_rx_interrupt
 *
 * Purpose: Default SD card data read interrupt handler
 *
 * Processing:
 *     See function.
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
static void sd1_rx_interrupt(void)
{
  UNS_32 dint;
  INT_32 idx;
  BOOL_32 done = FALSE;

  /* A data interrupt was received */
  dint = sdcarddat.regptr->sd_status;
  sdcarddat.dctrl.resp.data_status = dint;

  /* Has data transfer completed? */
  if ((dint & SD_DATABLK_END) != 0)
  {
    /* No more data expected, so read what's left in the FIFO */
    while ((sdcarddat.regptr->sd_status &
            SD_FIFO_RXDATA_AVAIL) != 0)
    {
      *sdcarddat.dctrl.xferdat.buff =
        sdcarddat.regptr->sd_fifo [0];
      sdcarddat.dctrl.xferdat.buff++;
    }

    /* Only stop if this is the last block */
    if (sdcarddat.dctrl.xferdat.blocks == 0)
    {
      done = TRUE;
    }
    else
    {
      sdcarddat.dctrl.xferdat.blocks--;
    }
  }
  else if ((dint & SD_FIFO_RXDATA_HFULL) != 0)
  {
    /* The FIFO is at least half full, so read out 8 words
       of data */
    for (idx = 0; idx < 8; idx++)
    {
      *sdcarddat.dctrl.xferdat.buff =
        sdcarddat.regptr->sd_fifo [0];
      sdcarddat.dctrl.xferdat.buff++;
    }
  }

  /* Has a data error occurred? */
  if ((dint & (SD_STARTBIT_ERR | SD_FIFO_RXDATA_OFLOW |
               SD_DATA_TIMEOUT | SD_DATA_CRC_FAIL)) != 0)
  {
    done = TRUE;
  }

  if (done == TRUE)
  {
    int_disable(IRQ_SD1);
    sdcarddat.regptr->sd_dctrl &= ~SD_DATATRANSFER_EN;
    sdcarddat.regptr->sd_mask1 = 0;

    if (sdcarddat.sd1_cb != NULL)
    {
      sdcarddat.sd1_cb();
    }
  }

  /* Clear checked statuses */
  sdcarddat.regptr->sd_clear = dint;
}

/***********************************************************************
 *
 * Function: sd1_tx_interrupt
 *
 * Purpose: Default SD card data write int handler
 *
 * Processing:
 *     See function.
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
static void sd1_tx_interrupt(void)
{
  UNS_32 dint;
  INT_32 cnt, idx;
  BOOL_32 done = FALSE;

  /* A data interrupt was received */
  dint = sdcarddat.regptr->sd_status;

  /* Has data transfer completed? */
  if ((dint & SD_DATABLK_END) != 0)
  {
    /* Only stop if this is the last block */
    if (sdcarddat.dctrl.xferdat.blocks == 0)
    {
      done = TRUE;
    }
    else
    {
      sdcarddat.dctrl.xferdat.blocks--;
    }
  }

  if ((sdcarddat.regptr->sd_status & SD_FIFO_TXDATA_HEMPTY) != 0)
  {
    /* The FIFO needs more data */
    while (((sdcarddat.regptr->sd_status &
             SD_FIFO_TXDATA_HEMPTY) != 0) &&
           (sdcarddat.dctrl.tosend > 0))
    {
      sdcarddat.regptr->sd_fifo [0] =
        *sdcarddat.dctrl.xferdat.buff;
      sdcarddat.dctrl.xferdat.buff++;
      sdcarddat.dctrl.tosend -= 1;
    }

    /* Up to 8 more words can be stuffed into the FIFO */
    cnt = sdcarddat.dctrl.tosend;
    if (cnt > 7)
    {
      cnt = 7;
    }
    for (idx = 0; idx < cnt; idx++)
    {
      sdcarddat.regptr->sd_fifo [0] =
        *sdcarddat.dctrl.xferdat.buff;
      sdcarddat.dctrl.xferdat.buff++;
    }
    sdcarddat.dctrl.tosend -= cnt;

    if (sdcarddat.dctrl.tosend == 0)
    {
      /* Disable FIFO related interrupts */
      sdcarddat.regptr->sd_mask1 &= ~(SD_FIFO_TXDATA_EMPTY |
                                      SD_FIFO_TXDATA_HEMPTY);
    }
  }

  /* Has a data error occurred? */
  if ((dint & (SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
               SD_DATA_CRC_FAIL)) != 0)
  {
    done = TRUE;
  }

  if (done == TRUE)
  {
    int_disable(IRQ_SD1);
    sdcarddat.regptr->sd_dctrl &= ~SD_DATATRANSFER_EN;
    sdcarddat.regptr->sd_mask1 = 0;
    sdcarddat.dctrl.resp.data_status = dint;

    if (sdcarddat.sd1_cb != NULL)
    {
      sdcarddat.sd1_cb();
    }
  }

  /* Clear checked statuses */
  sdcarddat.regptr->sd_clear = dint;
}

/***********************************************************************
 *
 * Function: sd0_cmd_interrupt
 *
 * Purpose: Default SD card command interrupt handler
 *
 * Processing:
 *     This function is called when an SD0 card interrupt is generated.
 *     This will save the command status and response if it is valid.
 *     The command state machine is cleared and any commands interrupts
 *     are cleared and masked. If the user defined command callback
 *     function exists, then it is called before exiting this function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: The user callback function is called in interrupt context.
 *
 **********************************************************************/
static void sd0_cmd_interrupt(void)
{
  int idx;
  UNS_32 cmd;

  /* Save status */
  sdcarddat.dctrl.resp.cmd_status = sdcarddat.regptr->sd_status;

  /* Save response */
  sdcarddat.dctrl.resp.cmd_resp [0] = sdcarddat.regptr->sd_respcmd;
  for (idx = 0; idx < 4; idx++)
  {
    sdcarddat.dctrl.resp.cmd_resp [idx + 1] =
      sdcarddat.regptr->sd_resp [idx];
  }

  /* Stop command state machine */
  sdcarddat.regptr->sd_cmd &= ~SD_CPST_EN;

  /* Clear pending command statuses */
  sdcarddat.regptr->sd_clear = (SD_CMD_CRC_FAIL | SD_CMD_TIMEOUT |
    SD_CMD_RESP_RECEIVED | SD_CMD_SENT);
  sdcarddat.regptr->sd_mask0 = 0;

  if (sdcarddat.dctrl.xferdat.dataop == SD_DATAOP_WRITE)
  {
    /* Start data state machine */
    sdcarddat.regptr->sd_dctrl |= SD_DATATRANSFER_EN;

    /* Clear FIFO conditions */
    sdcarddat.regptr->sd_clear = (SD_FIFO_TXDATA_HEMPTY |
      SD_DATABLK_END | SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
                                  SD_DATA_CRC_FAIL);

    if (sdcarddat.dmact.dma_enabled == TRUE)
    {
      /* Set DMA receive handler */
      int_install_irq_handler(IRQ_SD1, (PFV) sd1_dmatx_interrupt);

      /* Setup FIFO control conditions to interrupt when data
         block has been received, start bit errors, data FIFO
         overflow, data timeout, or data CRC error */
      sdcarddat.regptr->sd_mask1 = (SD_DATA_END |
        SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
                                    SD_DATA_CRC_FAIL);
      sdcarddat.regptr->sd_dctrl |= SD_DMA_EN;

      /* Setup DMA transfer */
      if (sdcarddat.dctrl.xferdat.buff != NULL)
      {
        /* Setup source */
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
          dmact.dmach].src_addr = (UNS_32) sdcarddat.dctrl.xferdat.buff;
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
                                            dmact.dmach].lli = 0;
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
          dmact.dmach].dest_addr = (UNS_32) & SDCARD->sd_fifo;
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
          dmact.dmach].control = (DMAC_CHAN_SRC_AUTOINC |
		  DMAC_CHAN_SRC_AHB1 | DMAC_CHAN_DEST_WIDTH_32 |
		  DMAC_CHAN_SRC_WIDTH_32 | DMAC_CHAN_DEST_BURST_8 |
		  DMAC_CHAN_SRC_BURST_8);
      }

      /* Set DMA initial control */
      sdcarddat.dmact.pdmaregs->sync |= DMA_PER_SDCARD;

      /* Setup DMA config and start DMA controller */
      sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
        dmact.dmach].config_ch = (DMAC_CHAN_IE | DMAC_CHAN_FLOW_P_M2P |
		DMAC_DEST_PERIP(DMA_PERID_SDCARD) | DMAC_CHAN_ENABLE);
    }
    else
    {
      /* If not DMA, use FIFO mode */
      /* Setup FIFO control conditions to interrupt when FIFO is
         empty or half empty, data block has been trasmitted, data
         has ended, data underflow, or data CRC error */
      sdcarddat.regptr->sd_mask1 = (SD_FIFO_TXDATA_HEMPTY |
		  SD_DATABLK_END | SD_FIFO_TXDATA_UFLOW | SD_DATA_TIMEOUT |
                                    SD_DATA_CRC_FAIL);

      /* Enable DMA if needed */
      sdcarddat.regptr->sd_dctrl &= ~SD_DMA_EN;

      /* Fill transmit FIFO */
      while ((sdcarddat.regptr->sd_status &
              SD_FIFO_TXDATA_FULL) == 0)
      {
        sdcarddat.regptr->sd_fifo [0] =
          *sdcarddat.dctrl.xferdat.buff;
        sdcarddat.dctrl.xferdat.buff++;
        sdcarddat.dctrl.tosend -= 1;
      }
    }
  }

  /* If a pending command is being used to stop this transfer,
     then set it up now */
  if (sdcarddat.dctrl.xferdat.usependcmd == TRUE)
  {
    sdcarddat.dctrl.xferdat.usependcmd = FALSE;
    cmd = prep_cmd(&sdcarddat.dctrl.xferdat.pendcmd);
    sdcarddat.regptr->sd_cmd = (cmd | 0 |
                                SD_CMDPEND_WAIT);
  }
  else
  {
    /* Disable SDMMC interrupt for now */
    int_disable(IRQ_SD0);

    /* Call command callback function if it exists */
    if (sdcarddat.sd0_cb != NULL)
    {
      sdcarddat.sd0_cb();
    }
  }
}

/***********************************************************************
 *
 * Function: sd_start_data_write
 *
 * Purpose: Start data write transfer operations to a card
 *
 * Processing:
 *     If the number of blocks to transfer is 0, return an error status
 *     to the caller. Install the standard data transmit interrupt
 *     handler. Set the data block size and data timeout clocks in the
 *     controller. Clear any latched data interrupts. Fill the transmit
 *     FIFO and decrement the amount to send by the filled amount.
 *     Enable data transfer interrupts as needed by the transmit
 *     interrupt handler. Decrement the number of blocks to transfer in
 *     the interrupt handler.
 *
 * Parameters:
 *     cmdreg : Command control register value
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_start_data_write(UNS_32 cmdreg)
{
  UNS_32 datalen;
  STATUS status = _ERROR;

  /* Determine data length from block count */
  datalen = sdcarddat.dctrl.blocksize *
            sdcarddat.dctrl.xferdat.blocks;
  if ((datalen > 0) && (datalen <= 65024))
  {
    /* Number of words to send */
    sdcarddat.dctrl.tosend = datalen >> 2;

    /* Set standard transmit handler */
    int_install_irq_handler(IRQ_SD1, (PFV) sd1_tx_interrupt);

    /* Decrement block count for start of transfer */
    sdcarddat.dctrl.xferdat.blocks--;

    /* Setup blocksize, data length, data timeout, and direction */
    sdcarddat.regptr->sd_dtimer = sdcarddat.dctrl.data_to;
    sdcarddat.regptr->sd_dlen = datalen;
    sdcarddat.regptr->sd_dctrl &= ~SD_DIR_FROMCARD;

    /* Start command state machine */
    sdcarddat.regptr->sd_cmd = (cmdreg | SD_CPST_EN);

    int_enable(IRQ_SD1);

    status = _NO_ERROR;
  }

  return status;
}

/***********************************************************************
 *
 * Function: sd_start_data_read
 *
 * Purpose: Start data read transfer operations from a card
 *
 * Processing:
 *     If the number of blocks to transfer is 0, return an error status
 *     to the caller. Install the standard data receive interrupt
 *     handler. Empty the data FIFO. Enable data transfer interrupts as
 *     needed by the recieve interrupt handler. Set the data block size
 *     and data timeout clocks in the controller. Decrement the number
 *     of blocks to transfer in the interrupt handler.
 *
 * Parameters:
 *     cmdreg : Command control register value
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_start_data_read(UNS_32 cmdreg)
{
  volatile UNS_32 tmp;
  UNS_32 datalen;
  STATUS status = _ERROR;

  /* Determine data length from block count */
  datalen = sdcarddat.dctrl.blocksize *
            sdcarddat.dctrl.xferdat.blocks;
  if ((datalen > 0) && (datalen <= 65024))
  {
    /* Make sure data FIFO is empty */
    while ((sdcarddat.regptr->sd_status &
            SD_FIFO_RXDATA_AVAIL) != 0)
    {
      tmp = sdcarddat.regptr->sd_fifo [0];
    }

    /* Enable DMA if needed */
    if (sdcarddat.dmact.dma_enabled == TRUE)
    {
      /* Set DMA receive handler */
      int_install_irq_handler(IRQ_SD1, (PFV) sd1_dmarx_interrupt);

      /* Setup FIFO control conditions to interrupt when data
         block has been received, start bit errors, data FIFO
         overflow, data timeout, or data CRC error */
      sdcarddat.regptr->sd_clear = (SD_DATA_END |
		  SD_STARTBIT_ERR | SD_FIFO_RXDATA_OFLOW | SD_DATA_TIMEOUT |
                                    SD_DATA_CRC_FAIL);
      sdcarddat.regptr->sd_mask1 = (SD_DATA_END |
		  SD_STARTBIT_ERR | SD_FIFO_RXDATA_OFLOW | SD_DATA_TIMEOUT |
                                    SD_DATA_CRC_FAIL);
      sdcarddat.regptr->sd_dctrl |= SD_DMA_EN;

      /* Setup DMA transfer */
      if (sdcarddat.dctrl.xferdat.buff != NULL)
      {
        /* Setup destination and clear LLI */
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
			dmact.dmach].dest_addr =
            (UNS_32) sdcarddat.dctrl.xferdat.buff;
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
                                            dmact.dmach].lli = 0;
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
			dmact.dmach].src_addr = (UNS_32) & SDCARD->sd_fifo;

        /* Setup DMA config and start DMA controller */
        sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
			dmact.dmach].control = (DMAC_CHAN_DEST_AUTOINC |
			DMAC_CHAN_SRC_AHB1 | DMAC_CHAN_DEST_WIDTH_32 |
			DMAC_CHAN_SRC_WIDTH_32 | DMAC_CHAN_DEST_BURST_8 |
			DMAC_CHAN_SRC_BURST_8);
      }

      /* Set DMA initial control */
      sdcarddat.dmact.pdmaregs->sync |= DMA_PER_SDCARD;
      sdcarddat.dmact.pdmaregs->dma_chan [sdcarddat.
		  dmact.dmach].config_ch = (DMAC_CHAN_IE |
		  DMAC_CHAN_FLOW_P_P2M | DMAC_SRC_PERIP(DMA_PERID_SDCARD) |
		  DMAC_CHAN_ENABLE);
    }
    else
    {
      /* Set standard receive handler */
      int_install_irq_handler(IRQ_SD1, (PFV) sd1_rx_interrupt);

      /* Setup FIFO control conditions to interrupt when FIFO
         is half full, data block has been received, start bit
         errors, data FIFO overflow, data timeout, or data CRC
         error */
      sdcarddat.regptr->sd_clear = (SD_FIFO_RXDATA_HFULL |
		  SD_DATABLK_END | SD_STARTBIT_ERR | SD_FIFO_RXDATA_OFLOW |
                                    SD_DATA_TIMEOUT | SD_DATA_CRC_FAIL);
      sdcarddat.regptr->sd_mask1 = (SD_FIFO_RXDATA_HFULL |
		  SD_DATABLK_END | SD_STARTBIT_ERR | SD_FIFO_RXDATA_OFLOW |
                                    SD_DATA_TIMEOUT | SD_DATA_CRC_FAIL);
      sdcarddat.regptr->sd_dctrl &= ~SD_DMA_EN;
    }
    int_enable(IRQ_SD1);

    /* Setup blocksize, data length, data timeout, and direction */
    sdcarddat.regptr->sd_dtimer = sdcarddat.dctrl.data_to;
    sdcarddat.regptr->sd_dlen = datalen;
    sdcarddat.regptr->sd_dctrl |= SD_DIR_FROMCARD;

    /* Decrement block count for start of transfer */
    sdcarddat.dctrl.xferdat.blocks--;

    /* Start command state machine */
    sdcarddat.regptr->sd_cmd = (cmdreg | SD_CPST_EN);

    /* Start data state machine */
    sdcarddat.regptr->sd_dctrl |= SD_DATATRANSFER_EN;

    status = _NO_ERROR;
  }

  return status;
}

/***********************************************************************
 *
 * Function: sd_start_data_xfer
 *
 * Purpose: Start data transfer operations to/from a card
 *
 * Processing:
 *     Save the data control structure. If the data operation type is
 *     read, then call the sd_start_data_read function, else call the
 *     sd_start_data_write function.
 *
 * Parameters:
 *     cmdreg : Command control register value
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_start_data_xfer(UNS_32 cmdreg)
{
  STATUS status = _NO_ERROR;

  /* Streaming enabled? */
  if (sdcarddat.dctrl.xferdat.stream == TRUE)
  {
    sdcarddat.regptr->sd_dctrl |= SD_STREAM_EN;
  }
  else
  {
    sdcarddat.regptr->sd_dctrl &= ~SD_STREAM_EN;
  }

  /* Determine direction for transfer */
  if (sdcarddat.dctrl.xferdat.dataop == SD_DATAOP_READ)
  {
    /* Start data read */
    status = sd_start_data_read(cmdreg);
  }
  else
  {
    /* Start data write */
    status = sd_start_data_write(cmdreg);
  }

  return status;
}

/***********************************************************************
 *
 * Function: sd_issue_cmd
 *
 * Purpose: Start command or data transfer operations to/from a card
 *
 * Processing:
 *     The command state machine is setup with the passed argument,
 *     command, expected response type, and busy wait status. If a
 *     response is expected, then the command state machine and expected
 *     interrupts are setup the receive a response. The command
 *     interrupt is cleared and enabled. If a data transfer is expected
 *     with this command, the command state machine is started in
 *     pending mode and the sd_start_data_xfer() function is called to
 *     start the data operation. If no data is expected, then the
 *     the command state machine is started immediately.
 *
 * Parameters:
 *     pcmd : Pointer to a command and data control structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_issue_cmd(SD_CMDDATA_T *pcmd)
{
  UNS_32 tmp;
  STATUS status = _NO_ERROR;

  tmp = prep_cmd(&pcmd->cmd);

  /* Save transfer information */
  sdcarddat.dctrl.xferdat = pcmd->data;

  /* If this function also has data associated with it, then have the
     command state machine kicked off with the data state machine */
  if (pcmd->data.dataop != SD_DATAOP_NONE)
  {
    /* Setup data transfer and start data state machine */
    status = sd_start_data_xfer(tmp);
  }
  else
  {
    /* Start command state machine */
    sdcarddat.dctrl.xferdat.usependcmd = FALSE;
    sdcarddat.regptr->sd_cmd = (tmp | SD_CPST_EN);
  }

  return status;
}

/***********************************************************************
 *
 * Function: sd_set_ctrl_params
 *
 * Purpose: Setup SD controller parameters
 *
 * Processing:
 *     This function sets up the controller options for the SD card
 *     controller interface. Options include pullup configuration,
 *     open drain or push-pull modes, card clock rate, power saving
 *     mode selection, and 1-bit or 4-bit bus operation.
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
static STATUS sd_set_ctrl_params(SDC_PRMS_T *pparms)
{
  UNS_32 sd_div, sd_clk, tmp;

  /* Setup pullups */
  clkpwr_setup_mcard_ctrlr(1, pparms->pullup23, pparms->pullup1,
                           pparms->pullup0, 1);

  /* Setup power control and bus mode */
  tmp = 0;
  if (pparms->opendrain == TRUE)
  {
    tmp = SD_OPENDRAIN_EN;
  }
  sdcarddat.regptr->sd_power = (tmp | pparms->powermode);

  /* Get current SD clock rate */
  sd_clk = clkpwr_get_clock_rate(CLKPWR_MSCARD_CLK);

  /* Find best divider to generate target clock rate */
  sd_div = 0;
  tmp = 0;
  while ((sd_clk / (2 * (sd_div + 1))) >= pparms->sdclk_rate)
  {
    sd_div++;
  }
  if (sd_div > SD_CLKDIV_MASK)
  {
    /* Limit to maximum supported divider */
    sd_div = SD_CLKDIV_MASK;
  }
  else if (sd_div == 0)
  {
    /* May have to use the clock bypass instead */
    if (pparms->sdclk_rate >= sd_clk)
    {
      tmp |= SD_SDCLK_BYPASS;
    }
  }

  /*  Set SD clocking and bus width */
  if (pparms->use_wide == TRUE)
  {
    tmp |= SD_WIDEBUSMODE_EN;
  }
  if (pparms->pwrsave == TRUE)
  {
    tmp |= SD_SDCLK_PWRSAVE;
  }
  sdcarddat.regptr->sd_clock = (tmp | SD_SDCLK_EN | sd_div);

  return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: sd_set_data_params
 *
 * Purpose: Setup SD data transfer parameters
 *
 * Processing:
 *     This function sets up the data transfer options for SD card
 *     controller block transfers. Options setup include the block
 *     transfer size and data transfer timeout in clocks. If DMA is
 *     used, this also sets up the DMA controller for SD card DMA
 *     used. The command and data callbacks required by the user
 *     application are also setup here.
 *
 * Parameters:
 *     pparms : Pointer to parameter data structure
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the configuration was ok, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
static STATUS sd_set_data_params(SDC_XFER_SETUP_T *pparms)
{
  STATUS sts = _NO_ERROR;

  /* Save command and data callback function pointers */
  sdcarddat.sd0_cb = pparms->cmd_callback;
  sdcarddat.sd1_cb = pparms->data_callback;

  /* Translate block size to correct register value */
  sdcarddat.dctrl.blocksize = pparms->blocksize;
  switch (pparms->blocksize)
  {
    case 1:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_1BYTE;
      break;

    case 2:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_2BYTES;
      break;

    case 4:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_4BYTES;
      break;

    case 8:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_8BYTES;
      break;

    case 16:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_16BYTES;
      break;

    case 32:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_32BYTES;
      break;

    case 64:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_64BYTES;
      break;

    case 128:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_128BYTES;
      break;

    case 256:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_256BYTES;
      break;

    case 512:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_512BYTES;
      break;

    case 1024:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_1024BYTES;
      break;

    case 2048:
      sdcarddat.regptr->sd_dctrl = SD_BLKSIZE_2048BYTES;
      break;

    default:
      sts = _ERROR;
      break;
  }

  if (sts != _ERROR)
  {
    /* Set timeout */
    sdcarddat.dctrl.data_to = pparms->data_to;
    sdcarddat.regptr->sd_dtimer = pparms->data_to;

    /* If DMA was perviously disabled and DMA is being enabled, then
       setup DMA now */
    sdcarddat.dmact.use_dma = pparms->use_dma;
    if (sdcarddat.dmact.dma_enabled != sdcarddat.dmact.use_dma)
    {
      if (sdcarddat.dmact.dma_enabled == TRUE)
      {
        /* DMA is being disabled */
        sd_dma_disable();
      }
      else
      {
        /* DMA is being enabled */
        sts = sd_dma_enable();
      }

      sdcarddat.dmact.dma_enabled = sdcarddat.dmact.use_dma;
    }
  }

  return sts;
}

/***********************************************************************
 * SDcard controller driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: sdcard_open
 *
 * Purpose: Open the SD card controller driver
 *
 * Processing:
 *     If the passed driver pointer is valid and the driver hasn't been
 *     previously opened, then enabled the SD card controller clocks
 *     and setup the SD card controller in a safe default configuration.
 *     Setup the default command handler and set the default data for
 *     data and command control structure to initial values.
 *
 * Parameters:
 *     ipbase: Pointer to a SD card controller peripheral block
 *     arg   : Always 0
 *
 * Outputs: None
 *
 * Returns: The pointer to a SD card driver config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdcard_open(void *ipbase,
                   INT_32 arg)
{
  SDC_PRMS_T sdparams_ctrl;
  SDC_XFER_SETUP_T sdparams_dat;
  INT_32 tptr = (INT_32) NULL;

  if ((SDCARD_REGS_T *) ipbase == SDCARD)
  {
    /* Has SD card controller driver been previously initialized? */
    if (sdcarddat.init == FALSE)
    {
      /* Device not initialized and it usable, so set it to
         used */
      sdcarddat.init = TRUE;

      /* Save address of register block */
      sdcarddat.regptr = SDCARD;

      /* Enable SD clock at IP clock divided by 2 */
      clkpwr_clk_en_dis(CLKPWR_MSCARD_CLK, 1);
      sdcarddat.regptr->sd_clock = SD_SDCLK_EN;

      /* Setup SDCARD interface and controller. Enable SD
         controller and pullups, setup clock rate to 390KHz,
         disable wide bus, power off mode */
      sdparams_ctrl.sdclk_rate = 390000;
      sdparams_ctrl.use_wide = FALSE;
      sdparams_ctrl.pwrsave = FALSE;
      sdparams_ctrl.opendrain = TRUE;
      sdparams_ctrl.powermode = SD_POWER_OFF_MODE;
      sdparams_ctrl.pullup0 = 1;
      sdparams_ctrl.pullup1 = 1;
      sdparams_ctrl.pullup23 = 1;
      sd_set_ctrl_params(&sdparams_ctrl);

      /* Disable command and data state machines */
      sdcarddat.regptr->sd_cmd = 0;
      sdcarddat.regptr->sd_dctrl = 0;

      /* Disable all interrupts */
      sdcarddat.regptr->sd_mask0 = 0;
      sdcarddat.regptr->sd_mask1 = 0;

      /* Clear any latched statuses */
      sdcarddat.regptr->sd_clear = 0x7FF;

      /* Install default command interrupt handler */
      int_install_irq_handler(IRQ_SD0, (PFV) sd0_cmd_interrupt);

      /* DMA was not previously enabled */
      sdcarddat.dmact.dma_enabled = FALSE;

      /* Set data transfer defaults for the interface, 512 byte
         blocksize, data timeout of maximum clocks, no default
         callbacks */
      sdparams_dat.data_callback = NULL;
      sdparams_dat.cmd_callback = NULL;
      sdparams_dat.blocksize = 512;
      sdparams_dat.data_to = 0xFFFFFFFF;
      sdparams_dat.use_dma = FALSE;
      sd_set_data_params(&sdparams_dat);

      /* Clean up some data values */
      sdcarddat.dctrl.tosend = 0;
      sdcarddat.dctrl.resp.cmd_status = 0;
      sdcarddat.dctrl.resp.data_status = 0;

      tptr = (INT_32) & sdcarddat;
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: sdcard_close
 *
 * Purpose: Close the SD card controller driver
 *
 * Processing:
 *     If the passed device ID is valid and the device has been
 *     previously initialized, then safely shut down the device and the
 *     associated clocks. Disable interrupts and DMA for teh device.
 *
 * Parameters:
 *     devid: Pointer to SD card controller structure (from sdcard_open)
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS sdcard_close(INT_32 devid)
{
  STATUS status = _ERROR;

  if ((SDCARD_CFG_T *) devid == &sdcarddat)
  {
    if (sdcarddat.init == TRUE)
    {
      /* Disable SD controller interrupts */
      int_disable(IRQ_SD0);
      int_install_irq_handler(IRQ_SD0, (PFV) NULL);
      int_disable(IRQ_SD1);
      int_install_irq_handler(IRQ_SD1, (PFV) NULL);

      /* Disable command and data state machines */
      sdcarddat.regptr->sd_cmd = 0;
      sdcarddat.regptr->sd_dctrl = 0;

      /* Disable all interrupts and place in power-off state */
      sdcarddat.regptr->sd_mask0 = 0;
      sdcarddat.regptr->sd_mask1 = 0;
      sdcarddat.regptr->sd_power =
        (SD_POWER_OFF_MODE | SD_OPENDRAIN_EN);

      /* Disable DMA for the device if it was enabled */
      if (sdcarddat.dmact.dma_enabled == TRUE)
      {
        sd_dma_disable();
      }

      /* Disable SD controller clock */
      sdcarddat.regptr->sd_clock = 0;

      /* Disable SD clock */
      clkpwr_clk_en_dis(CLKPWR_MSCARD_CLK, 0);

      /* Set device as uninitialized */
      sdcarddat.init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: sdcard_ioctl
 *
 * Purpose: SD card controller driver configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, perform the associated SD card driver
 *     function.
 *
 * Parameters:
 *     devid: Pointer to KeySD controller driver structure
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
STATUS sdcard_ioctl(INT_32 devid,
                    INT_32 cmd,
                    INT_32 arg)
{
  SD_CMDRESP_T *pCmdResp;
  UNS_32 tmp1, tmp2;
  INT_32 status = _ERROR;

  if ((SDCARD_CFG_T *) devid == &sdcarddat)
  {
    if (sdcarddat.init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case SD_SETUP_PARAMS:
          status = sd_set_ctrl_params(
                     (SDC_PRMS_T *) arg);
          break;

        case SD_SETUP_DATA_XFER:
          status = sd_set_data_params(
                     (SDC_XFER_SETUP_T *) arg);
          break;

        case SD_ISSUE_CMD:
          status = sd_issue_cmd((SD_CMDDATA_T *) arg);
          break;

        case SD_GET_CMD_RESP:
          /* Get last saved response and status */
          pCmdResp = (SD_CMDRESP_T *) arg;
          *pCmdResp = sdcarddat.dctrl.resp;
          break;

        case SD_GET_STATUS:
          /* Return a SDCARD status */
          switch (arg)
          {
            case SD_RAW_STATUS:
              status = (INT_32)
                       sdcarddat.regptr->sd_status;
              break;

            case SD_GET_DMA_CHANNEL:
              status = _ERROR;
              if (sdcarddat.dmact.dma_enabled == TRUE)
              {
                status = sdcarddat.dmact.dmach;
              }
              break;

            case SD_GET_CLOCK:
              tmp1 = clkpwr_get_clock_rate(
                       CLKPWR_MSCARD_CLK);
              tmp2 = sdcarddat.regptr->sd_clock &
                     SD_CLKDIV_MASK;
              status = (STATUS)(tmp1 / (2 * (tmp2 + 1)));
              break;

            default:
              /* Unsupported parameter */
              status = LPC_BAD_PARAMS;
              break;
          }
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
 * Function: sdcard_read
 *
 * Purpose: SD card controller driver read function (stub only)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     devid:     Pointer to SD controller driver descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes read, or _NO_ERROR on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdcard_read(INT_32 devid,
                   void *buffer,
                   INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: sdcard_write
 *
 * Purpose: SD card controller driver write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to SD controller driver descriptor
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
INT_32 sdcard_write(INT_32 devid,
                    void *buffer,
                    INT_32 n_bytes)
{
  return 0;
}
