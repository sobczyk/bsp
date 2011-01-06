/***********************************************************************
 * $Id:: lpc32xx_adc_driver.c 1022 2008-08-06 22:23:42Z wellsk         $
 *
 * Project: LPC32xx ADC driver
 *
 * Description:
 *     This file contains driver support for the ADC module on the
 *     LPC32xx
 *
 * Notes:
 *     The number of different configurations supported by the ADC
 *     is beyond the scope of this driver. This driver provides the
 *     following basic functions:
 *         Sequenced analog to digital conversions (polled, interrupt)
 *         ADC status polling and general configuration
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

#include "lpc32xx_adc_driver.h"

/***********************************************************************
 * ADC driver private data and types
 **********************************************************************/

/* Size of the ADC receive ring buffer */
#define ADC_RING_BUFSIZE 4

/* Function prototype used for polled and interrupt driven reads */
typedef INT_32(*ADC_RFUNC_T)(void *, void *, INT_32);

/* ADC device configuration structure type */
typedef struct
{
  BOOL_32 init;       			    /* Device initialized flag */
  LPC3250_ADCTSC_REGS_T *regptr; 	/* Pointer to ADC registers */
  UNS_16 rx[ADC_RING_BUFSIZE];  	/* ADC data ring buffer */
  INT_32 rx_head;     			    /* ADC ring buffer head index */
  INT_32 rx_tail;     			    /* ADC ring buffer tail index */
} ADC_CFG_T;

/* ADC device configuration structure */
STATIC ADC_CFG_T adccfg;

/***********************************************************************
 * ADC driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: adc_default
 *
 * Purpose: Places the ADC interface and controller in a default state
 *
 * Processing:
 *     Setup default state of ADC, channel 0 selected, ADC powered up
 *
 * Parameters:
 *     adcregs: Pointer to an ADC register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void adc_default(LPC3250_ADCTSC_REGS_T *adcregs)
{
  /* Setup default state of ADC, channel 0 selected, ADC powered up */

  /* set ADC SEL to default state */
  adcregs->adc_sel = ADC_SEL_MASK;

  /* set ADC power up */
  adcregs->adc_con = TSC_ADCCON_POWER_UP;

}


/***********************************************************************
 * ADC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: adc_open
 *
 * Purpose: Open the ADC
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the ADC peripheral register set address,
 *     and initialize the ADC interface and controller to a default
 *     state by calling adc_default(), and return a pointer to the ADC
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: ADC descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a ADC config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 adc_open(void *ipbase,
                INT_32 arg)
{
  INT_32 status = 0;

  if ((adccfg.init == FALSE) && 
     ((LPC3250_ADCTSC_REGS_T *) ipbase == ADC))
  {
    /* Device is valid and not previously initialized */
    adccfg.init = TRUE;

    /* Save and return address of peripheral block */
    adccfg.regptr = (LPC3250_ADCTSC_REGS_T *) ipbase;

    /* Place ADC in a default state */
    adc_default(adccfg.regptr);

    /* Empty ring buffer and set conversion counter to 0 */
    adccfg.rx_head = adccfg.rx_tail = 0;

    /* set the channel for conversion */
    adccfg.regptr->adc_sel =  ADC_SEL_MASK;

    /* Return pointer to ADC configuration structure */
    status = (INT_32) & adccfg;
  }

  return status;
}

/***********************************************************************
 *
 * Function: adc_close
 *
 * Purpose: Close the ADC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the ADC
 *     interface and controller by calling adc_default(), and return
 *     _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS adc_close(INT_32 devid)
{
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  STATUS status = _ERROR;

  if (adccfgptr->init == TRUE)
  {
    /* set ADC power OFF */
    adccfgptr->regptr->adc_con = 0;

    /* put adc select register to reset state */
    adccfgptr->regptr->adc_con = 0x4;

    status = _NO_ERROR;
    adccfgptr->init = FALSE;
  }

  return status;
}

/***********************************************************************
 *
 * Function: adc_ioctl
 *
 * Purpose: ADC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate ADC parameter.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
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
STATUS adc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  LPC3250_ADCTSC_REGS_T *adcregsptr;
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  STATUS status = _ERROR;

  if (adccfgptr->init == TRUE)
  {
    status = _NO_ERROR;
    adcregsptr = adccfgptr->regptr;

    switch (cmd)
    {

      case START_CONVERSION:
        /* Start the ADC conversion on the supplied channel */
        adcregsptr->adc_con |= (TSC_ADCCON_ADC_STROBE);
        break;

      case ADC_CH_SELECT:
        /* arg holds the channel to set for convertion */
        adcregsptr->adc_sel =  ADC_SEL_MASK;
        adcregsptr->adc_sel |= arg;

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
 * Function: adc_read_result
 *
 * Purpose: Reads data directly from the ADC FIFO
 *
 * Processing:
 *     If the device is not initialized, return 0 to the caller.
 *     Otherwise read the ADC data, return the number of bytes read
 *     to the caller.
 *
 * Parameters:
 *     devid:     Pointer to an ADC configuration structure
 *     buffer:    Pointer to data buffer to copy to
 *
 * Outputs: None
 *
 * Returns: The number of bytes read (will always be 1)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 adc_read_result(INT_32 devid,
                       void *buffer)
{
  ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
  LPC3250_ADCTSC_REGS_T *adcregptr = adccfgptr->regptr;
  UNS_16 *adcbuf = (UNS_16 *) buffer;
  INT_32 bytes = 0;

  if (adccfgptr->init == TRUE)
  {
    *adcbuf = (UNS_16) adcregptr->adc_dat;
	bytes++;
  }

  return bytes;
}



/***********************************************************************
 *
 * Function: adc_write
 *
 * Purpose: ADC write function (stub only)
 *
 * Processing:
 *     Returns 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
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
INT_32 adc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}


/***********************************************************************
 *
 * Function: adc_ring_fill
 *
 * Purpose: Move data from the ADC DAT to the driver ring buffer
 *
 * Processing:
 *     While there is ADC Data, copy an entry from the ADC DAT into
 *     the ADC driver ring buffer. Increment the ring buffer
 *     head pointer. If it exceeds the ring buffer size, reset the
 *     head pointer to 0.
 *
 * Parameters:
 *     adccfgptr: Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void adc_ring_fill(ADC_CFG_T *adccfgptr)
{
  LPC3250_ADCTSC_REGS_T *adcregsptr = 
      (LPC3250_ADCTSC_REGS_T *) adccfgptr->regptr;

  /* read in the 10 bit value */
  adccfgptr->rx[adccfgptr->rx_head] =
    (UNS_16) adcregsptr->adc_dat & TSC_ADCDAT_VALUE_MASK;

  /* Increment receive ring buffer head pointer */
  adccfgptr->rx_head++;
  if (adccfgptr->rx_head >= ADC_RING_BUFSIZE)
  {
    adccfgptr->rx_head = 0;
  }

}
/***********************************************************************
 *
 * Function: adc_interrupt
 *
 * Purpose: ADC interrupt handler
 *
 * Processing:
 *     On an interrupt call the adc_ring_fill() to move
 *     the data from the ADC data register to the ADC driver ring buffer.
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

void adc_interrupt(void)
{
  /* A read of the ADC data register will clear the ADC interrupt */
  /* Read the ADC result into a buffer */
  adc_ring_fill(&adccfg);
}

