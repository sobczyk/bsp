/***********************************************************************
 * $Id:: main_iar.c 1299 2008-10-31 21:22:24Z wellsk                   $
 *
 * Project: NXP PHY3250 simple ADC example
 *
 * Description:	This simple ADC example uses interrupts to measure the 
 *              level on ADIN2. The two clocking schemes are used
 *              --> RTC Clock and PERIPH_CLK 
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

#include "lpc_types.h"
#include "lpc32xx_clkpwr.h"

int main(void)
{
  /* Makre sure IRAM is mapped to 0x0 */
  CLKPWR->clkpwr_bootmap = CLKPWR_BOOTMAP_SEL_BIT;
  
  return c_entry();
}
