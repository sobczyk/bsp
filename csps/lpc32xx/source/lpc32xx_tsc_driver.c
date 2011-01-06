/***********************************************************************
 * $Id:: lpc32xx_tsc_driver.c 774 2008-05-22 21:04:51Z kendwyer        $
 *
 * Project: LPC32xx TSC driver
 *
 * Description:
 *     This file contains driver support for the TSC module on the
 *     LPC32xx
 *
 * Notes:
 *     The number of different configurations supported by the TSC
 *     is beyond the scope of this driver. This driver provides the
 *     following basic functions:
 *         Sequenced analog to digital conversions (polled, interrupt)
 *         TSC status polling and general configuration
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

#include "lpc32xx_tsc_driver.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_intc_driver.h"

/***********************************************************************
 * TSC driver private data and types
 **********************************************************************/

/* Size of the TSC receive ring buffer */
#define TSC_RING_BUFSIZE 16

/* Function prototype used for polled and interrupt driven reads */
typedef INT_32(*TSC_RFUNC_T)(void *, void *, INT_32);

/* TSC device configuration structure type */
typedef struct
{
  BOOL_32 init;                         /* Device initialized flag */
  LPC3250_ADCTSC_REGS_T *regptr; 	/* Pointer to ADC/TSC registers */
  UNS_32 rx[TSC_RING_BUFSIZE];  	/* TSC data ring buffer */
  INT_32 rx_head;                       /* TSC ring buffer head index */
  INT_32 rx_tail;                       /* TSC ring buffer tail index */
} TSC_CFG_T;

/* TSC device configuration structure */
STATIC TSC_CFG_T tsccfg;

/***********************************************************************
 * TSC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: tsc_default
 *
 * Purpose: Places the TSC interface and controller in a default state
 *
 * Processing: 
 *     Setup default state of TSC to Auto mode, 16 word FIFO,
 *     default timing register serttings, in a safe configuration, 
 *     and all interrupts masked
 *
 * Parameters:
 *     tscregs: Pointer to an TSC register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void tsc_default(LPC3250_ADCTSC_REGS_T *tscregs)
{
  /* Setup default state of TSC to Auto mode, 16 word FIFO,
  default timing register serttings, in a safe configuration, 
  and all interrupts masked */

  /* Ensure TSC/ADC is not powered */
  tscregs->adc_con = 0;

  /* set TSC SEL to default state */
  tscregs->adc_sel = ADC_SEL_MASK;

  /* Set the TSC FIFO depth */
  tscregs->adc_con |= TSC_ADCCON_IRQ_TO_FIFO_16;

  /* Delay time register */
  tscregs->tsc_dtr = 0x10;
  /* Rise time register */
  tscregs->tsc_rtr = 0x10;
  /* Update time register */
  tscregs->tsc_utr = 0x10;
  /* Touch time register */
  tscregs->tsc_ttr = 0x10;
  /* Drain X plate time register */
  tscregs->tsc_dxp = 0x10;

  /* min x register */
  tscregs->tsc_min_x = 0;
  /* max x register */
  tscregs->tsc_max_x = 0;
  /* max y register */
  tscregs->tsc_max_y = 0;
  /* min y register */
  tscregs->tsc_min_y = 0;

  /* Aux update time register */
  tscregs->tsc_aux_utr = 0;
  /* Aux minimum value register */
  tscregs->tsc_aux_min = 0;
  /* Aux maximum value register */
  tscregs->tsc_aux_max = 0;

}


/***********************************************************************
 * TSC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: tsc_open
 *
 * Purpose: Open the TSC
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the TSC peripheral register set address,
 *     and initialize the TSC interface and controller to a default
 *     state by calling adc_default(), and return a pointer to the TSC
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: TSC descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a TSC config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 tsc_open(void *ipbase,
                INT_32 arg)
{
  INT_32 status = 0;


  if ((tsccfg.init == FALSE) && 
      ((LPC3250_ADCTSC_REGS_T *) ipbase == TSC))
  {
    /* Device is valid and not previously initialized */
    tsccfg.init = TRUE;

    /* Save and return address of peripheral block */
    tsccfg.regptr = (LPC3250_ADCTSC_REGS_T *) ipbase;

    /* Place ADC in a default state */
    tsc_default(tsccfg.regptr);

    /* Empty ring buffer and set conversion counter to 0 */
    tsccfg.rx_head = tsccfg.rx_tail = 0;

    /* Return pointer to ADC configuration structure */
    status = (INT_32) & tsccfg;
  }


  return status;
}

/***********************************************************************
 *
 * Function: tsc_close
 *
 * Purpose: Close the TSC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the ADC
 *     interface and controller by calling adc_default(), and return
 *     _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to TSC config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS tsc_close(INT_32 devid)
{

  TSC_CFG_T *tsccfgptr = (TSC_CFG_T *) devid;
  STATUS status = _ERROR;

  if (tsccfgptr->init == TRUE)
  {
    /* Restore TSC to a default state */
    tsc_default(tsccfg.regptr);

    /* Ensure TSC/ADC is powered off */
    tsccfgptr->regptr->adc_con = 0;

    status = _NO_ERROR;
    tsccfgptr->init = FALSE;
  }

  return status;
}

/***********************************************************************
 *
 * Function: tsc_ioctl
 *
 * Purpose: TSC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate TSC parameter.
 *
 * Parameters:
 *     devid: Pointer to TSC config structure
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
STATUS tsc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{

  LPC3250_ADCTSC_REGS_T *tscregsptr;
  TSC_CFG_T *tsccfgptr = (TSC_CFG_T *) devid;
  STATUS status = _ERROR;
  volatile UNS_32  temp;

  if (tsccfgptr->init == TRUE)
  {
    status = _NO_ERROR;
    tscregsptr = tsccfgptr->regptr;

    switch (cmd)
    {
        /* Set the TSC FIFO depth */
      case SET_FIFO_DEPTH:

        /* Clear previous FIFO level */
        temp = tscregsptr->adc_con;
        temp = temp & ~TSC_FIFO_MSK;
        tscregsptr->adc_con =  temp;

        if (arg == 1)
        {
          tscregsptr->adc_con |= TSC_ADCCON_IRQ_TO_FIFO_1 ;
        }
        if (arg == 4)
        {
          tscregsptr->adc_con |= TSC_ADCCON_IRQ_TO_FIFO_4;
        }
        if (arg == 8)
        {
          tscregsptr->adc_con |= TSC_ADCCON_IRQ_TO_FIFO_8 ;
        }
        if (arg == 16)
        {
          tscregsptr->adc_con |= TSC_ADCCON_IRQ_TO_FIFO_16;
        }

        break;

      case TSC_NUM_BITS:
        /* Set the number of bits to convert */
        if (arg != 0)
        {
          tscregsptr->adc_con |= TSC_ADCCON_X_SAMPLE_SIZE(arg);
          tscregsptr->adc_con |= TSC_ADCCON_Y_SAMPLE_SIZE(arg);
        }
        /* default to 10bits */
        else
        {
          tscregsptr->adc_con |= TSC_ADCCON_X_SAMPLE_SIZE(10);
          tscregsptr->adc_con |= TSC_ADCCON_Y_SAMPLE_SIZE(10);
        }


      case TSC_AUTO_EN:
        /* enable AUTO mode */
        if (arg != 0)
        {
          tscregsptr->adc_con |= TSC_ADCCON_AUTO_EN;
        }
        /* disable AUTO mode */
        else
        {
          tscregsptr->adc_con &= ~TSC_ADCCON_AUTO_EN;
        }
        break;

        /* Enable auto position mode, then auto_en bit must be set*/
      case TSC_AUTO_POS:
        /* enable AUTO Position mode */
        if (arg != 0)
        {
          tscregsptr->adc_con |= TSC_ADCCON_POS_DET 
                                | TSC_ADCCON_AUTO_EN;
        }
        /* disable AUTO Position mode */
        else
        {

          tscregsptr->adc_con = 0;
        }
        break;

        /* Enable AUX mode measurement, position dectect 
           must also be enabled */
      case TSC_AUX_MODE:

        if (arg != 0)
        {
          /* enable AUX mode */
          tscregsptr->adc_con |= TSC_ADCCON_AUTO_EN;
          tscregsptr->adc_con |= TSC_ADCCON_TS_AUX_EN;
          tscregsptr->adc_con |= TSC_ADCCON_POS_DET;
        }
        /* disable AUX mode, and position detect mode*/
        else
        {
          tscregsptr->adc_con &= ~TSC_ADCCON_AUTO_EN;
          tscregsptr->adc_con &= ~TSC_ADCCON_TS_AUX_EN;
          tscregsptr->adc_con &= ~TSC_ADCCON_POS_DET;
        }

        break;

      case TSC_GET_STATUS:

        switch (arg)
        {
          case TSC_FIFO_EMPTY_ST:
            /* Return fifo empty bit state */
            if ((tscregsptr->tsc_stat & TSC_STAT_FIFO_EMPTY) != 0)
            {
              status = 1;
            }
            else
            {
              status = 0;
            }
            break;

          case TSC_FIFO_OVRUN_ST:
            /* Return fifo overrun bit state */
            if ((tscregsptr->tsc_stat & TSC_STAT_FIFO_OVRRN) != 0)
            {
              status = 1;
            }
            else
            {
              status = 0;
            }
            break;

          default:
            /* Unsupported parameter */
            status = LPC_BAD_PARAMS;
        }
        break;

        /* set the minimum x value to accept */
      case TSC_MIN_X:
        tscregsptr->tsc_min_x = TSC_DTR_MINX_VALUE(arg);
        break;

        /* set the minimum y value to accept */
      case TSC_MIN_Y:
        tscregsptr->tsc_min_y = TSC_DTR_MINY_VALUE(arg);
        break;

        /* set the maximum x value to accept */
      case TSC_MAX_X:
        tscregsptr->tsc_max_x = TSC_DTR_MAXX_VALUE(arg);
        break;

        /* set the maximum y value to accept */
      case TSC_MAX_Y:
        tscregsptr->tsc_max_y = TSC_DTR_MAXY_VALUE(arg);
        break;

        /* Aux update time register */
      case TSC_AUX_UTR:
        tscregsptr->tsc_aux_utr = TSC_AUX_UTR_UPDATE_TIME(arg);
        break;

        /* Aux minimum value register */
      case TSC_AUX_MIN:
        tscregsptr->tsc_aux_min = TSC_AUX_MIN_VALUE(arg);
        break;

        /* Aux maximum value register */
      case TSC_AUX_MAX:
        tscregsptr->tsc_aux_max = TSC_AUX_MAX_VALUE(arg);
        break;

      default:
        /* Unsupported parameter */
        status = LPC_BAD_PARAMS;
    }
  }


  return status;
}

/***********************************************************************
 *
 * Function: tsc_read_result
 *
 * Purpose: Reads data directly from the TSC FIFO
 *
 * Processing:
 *     If the device is not initialized, return 0 to the caller.
 *     Otherwise, while the number of bytes read is less than max_bytes
 *     and the TSC FIFO is not empty, read a FIFO entry from the TSC
 *     TSC and place it into the passed user buffer. Increment bytes
 *     by 2. When the loop is exited, return the number of bytes read
 *     to the caller.
 *
 * Parameters:
 *     devid:     Pointer to an TSC configuration structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: The number of bytes read from the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 tsc_read_result(INT_32 devid,
                       void *buffer,
                       INT_32 max_bytes)
{

  TSC_CFG_T *tsccfgptr = (TSC_CFG_T *) devid;
  LPC3250_ADCTSC_REGS_T *tscregsptr = tsccfgptr->regptr;
  UNS_16 *tscbuf = (UNS_16 *) buffer;
  INT_32 bytes = 0;

  if (tsccfgptr->init == TRUE)
  {
    /* Loop until until max_bytes expires */
    while (max_bytes > 0)
    {
	    /* read until FIFO is empty */
        while ((tscregsptr->tsc_stat & TSC_STAT_FIFO_EMPTY) 
              != TSC_STAT_FIFO_EMPTY)
        {
          *tscbuf = (UNS_32) TSC->tsc_fifo;
          bytes++;
        }
    
    max_bytes= max_bytes-2;
    
    }
  }

  return bytes;
}



/***********************************************************************
 *
 * Function: tsc_write
 *
 * Purpose: TSC write function (stub only)
 *
 * Processing:
 *     Returns 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to TSC config structure
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
INT_32 tsc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: tsc_read_ring
 *
 * Purpose: Reads data from the TSC ring buffer
 *
 * Processing:
 *     If the init flag for the TSC structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the TSC interrupts and
 *     disable the TSC interrupts. Loop until max_bytes equals 0 or
 *     until the receive ring buffer is empty, whichever comes
 *     first. Read the data from the ring buffer  indexed by the tail
 *     pointer and place it into the user buffer. Increment the tail
 *     pointer and user buffer pointer. If the tail pointer exceeds the
 *     buffer size, set the tail pointer to 0. Increment bytes, and
 *     decrement max_bytes. Exit the loop based on the loop conditions,
 *     re-enable the receive interrupts, and return the number of bytes
 *     read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to an TSC configuration structure
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
INT_32 tsc_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes)
{

  TSC_CFG_T *tsccfgptr = (TSC_CFG_T *) devid;
  INT_32 bytes = 0;

  UNS_16 *data = (UNS_16 *) buffer;

    if (tsccfgptr->init == TRUE)
    {
        /* Temporarily lock out TSC interrupts during this
           read so the TSC receive interrupt won't cause problems
           with the ring buffer index values */
        int_disable(IRQ_TS_IRQ);

        /* Loop until receive ring buffer is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            (tsccfgptr->rx_tail != tsccfgptr->rx_head))
        {
            /* Read data from ring buffer into user buffer */
            *data = tsccfgptr->rx[tsccfgptr->rx_tail];
            data++;

            /* Update tail pointer */
            tsccfgptr->rx_tail++;

            /* Make sure tail didn't overflow */
            if (tsccfgptr->rx_tail >= TSC_RING_BUFSIZE)
            {
                tsccfgptr->rx_tail = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes = bytes + 2;
            max_bytes = max_bytes - 2;
        }

        /* Re-enable TSC receive interrupt(s) */
        int_enable(IRQ_TS_IRQ);
    }

    return bytes;

}

/***********************************************************************
 *
 * Function: tsc_ring_fill
 *
 * Purpose: Move data from the TSC DAT to the driver ring buffer
 *
 * Processing:
 *     While there is TSC FIFO data, copy an entry from the TSC FIFO into
 *     the TSC driver ring buffer. Increment the ring buffer
 *     head pointer. If it exceeds the ring buffer size, reset the
 *     head pointer to 0.
 *
 * Parameters:
 *     adccfgptr: Pointer to TSC config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void tsc_ring_fill(TSC_CFG_T *tsccfgptr)
{

  LPC3250_ADCTSC_REGS_T *tscregsptr = tsccfgptr->regptr;

  /* Read the full FIFO until the FIFO Empty status bit is set */
  while ((tscregsptr->tsc_stat & TSC_STAT_FIFO_EMPTY) 
        != TSC_STAT_FIFO_EMPTY)
  {
    /* read in the 32 bit value */
    tsccfgptr->rx[tsccfgptr->rx_head] = (UNS_32) tscregsptr->tsc_fifo;
    /* Check that the TSC is pressed, sample valid */
    if ((tsccfgptr->rx[tsccfgptr->rx_head] & TSC_FIFO_TS_P_LEVEL) 
       != TSC_FIFO_TS_P_LEVEL)
    {
      /* Increment receive ring buffer head pointer */
      tsccfgptr->rx_head++;
      if (tsccfgptr->rx_head >= TSC_RING_BUFSIZE)
      {
        tsccfgptr->rx_head = 0;
      }
    }
  }
}
/***********************************************************************
 *
 * Function: tsc_interrupt
 *
 * Purpose: TSC interrupt handler
 *
 * Processing:
 *     On an interrupt call the tsc_ring_fill() to move
 *     the data from the TSC
 *     FIFO to the TSC driver ring buffer.
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

void tsc_interrupt(void)
{
  /* a read the TSC fifo register will clear the TSC interrupt */
  /* read the TSC result into a buffer */
  tsc_ring_fill(&tsccfg);
}

