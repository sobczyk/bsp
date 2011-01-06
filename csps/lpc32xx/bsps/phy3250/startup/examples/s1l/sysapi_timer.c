/***********************************************************************
 * $Id:: sysapi_timer.c 3530 2010-05-19 18:30:36Z usb10132             $
 *
 * Project: Time support functions
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         time_init
 *         time_reset
 *         time_start
 *         time_stop
 *         time_get
 *         time_get_rate
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

#include "s1l_sys_inf.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_timer_driver.h"

static UNS_64 base_rate;
static INT_32 tdev = 0;

/***********************************************************************
 *
 * Function: time_init
 *
 * Purpose: Initializes time system
 *
 * Processing: Initializes the system timer.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: 0 if the init failed, otherwise non-zero
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 time_init(void)
{
	TMR_PSCALE_SETUP_T pscale;

	/* Open timer driver */
	if (tdev == 0)
	{
		tdev = timer_open((void *) TIMER_CNTR0, 0);
		if (tdev != 0)
		{
			/* Use a prescale count to 100000 */
			pscale.ps_tick_val = 100000;
			pscale.ps_us_val = 0; /* Not needed when ps_tick_val != 0 */
			timer_ioctl(tdev, TMR_SETUP_PSCALE, (INT_32) &pscale);

			/* Get timer clock rate */
			base_rate = (UNS_64) timer_ioctl(tdev, TMR_GET_STATUS,
				TMR_GET_CLOCK);
		}
	}

	return tdev;
}

/***********************************************************************
 *
 * Function: time_reset
 *
 * Purpose: Resets system timer
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
void time_reset(void)
{
	if (tdev != 0)
	{
		timer_ioctl(tdev, TMR_RESET, 1);
	}
}

/***********************************************************************
 *
 * Function: time_start
 *
 * Purpose: Starts system timer
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
void time_start(void)
{
	if (tdev != 0)
	{
		timer_ioctl(tdev, TMR_ENABLE, 1);
	}
}

/***********************************************************************
 *
 * Function: time_stop
 *
 * Purpose: Stops system timer
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
void time_stop(void)
{
	if (tdev != 0)
	{
		timer_ioctl(tdev, TMR_ENABLE, 0);
	}
}

/***********************************************************************
 *
 * Function: time_get
 *
 * Purpose: Returns current system time value
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The number of ticks of the timer counter
 *
 * Notes: None
 *
 **********************************************************************/
UNS_64 time_get(void)
{
	TMR_COUNTS_T tcounts;
	UNS_64 ticks = 0;

	if (tdev != 0)
	{
		timer_ioctl(tdev, TMR_GET_COUNTS, (INT_32) &tcounts);

		/* Compute number of timer ticks */
		ticks = (UNS_64) tcounts.count_val * 100000;
		ticks = ticks + (UNS_64) tcounts.ps_count_val;
	}

	return ticks;
}

/***********************************************************************
 *
 * Function: time_get_rate
 *
 * Purpose:
 *     Returns base tick rate (ticks per second) of the time counter
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The timer tick rate (in ticks per second)
 *
 * Notes: None
 *
 **********************************************************************/
UNS_64 time_get_rate(void)
{
	return base_rate;
}

