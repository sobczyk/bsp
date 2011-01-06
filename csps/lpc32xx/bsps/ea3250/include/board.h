/***********************************************************************
 * $Id:: board_config.h 3388 2010-05-06 00:17:50Z usb10132             $
 *
 * Project: EA3250 Board related functions
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

#ifndef BOARD_H
#define BOARD_H

#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Toggles LED on the board based on the on value */
void ea3250_toggle_led(BOOL_32 on);

/* Sets the ea3250 core voltage to the passed value in mV */
int ea3250_vcore_set(UNS_32 vcore);

/* Reset the board */
void board_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
