/***********************************************************************
 * $Id:: lpc32xx_adc_driver.h 715 2008-05-08 14:53:06Z kendwyer        $
 *
 * Project: LPC32xx ADC driver
 *
 * Description:
 *     This file contains driver support for the ADC module on the
 *     LPC3250
 *
 * Notes:
 *     See the lpc32xx_adc_driver.c file for more information on this
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

#ifndef LPC32xx_ADC_DRIVER_H
#define LPC32xx_ADC_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc32xx_tsc.h" /* Note that the ADC and TSC 
                            share the same register set */

/***********************************************************************
 * ADC device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* ADC device commands (IOCTL commands) */
typedef enum
{

  START_CONVERSION,    /* Start the ADC conversion on 
                          the selected channel */

  ADC_CH_SELECT,       /* Select the ADC channel for ADC conversion,
						   valid values are 0,1 or 2 */

} ADC_IOCTL_CMD_T;

/***********************************************************************
 * ADC driver API functions
 **********************************************************************/

/* Open the ADC */
INT_32 adc_open(void *ipbase,
                INT_32 arg);

/* Close the ADC */
STATUS adc_close(INT_32 devid);

/* ADC configuration block */
STATUS adc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* ADC read function */
INT_32 adc_read_result(INT_32 devid,
                       void *buffer);

/* ADC write function (stub only) */
INT_32 adc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

/***********************************************************************
 * ADC driver miscellaneous functions
 **********************************************************************/

/* ADC interrupt handler */
void adc_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_ADC_DRIVER_H */
