/***********************************************************************
 * $Id:: nand_mlc_common.h 3388 2010-05-06 00:17:50Z usb10132          $
 *
 * Project: MLC support functions
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

#ifndef MLC_NAND_COMMON_H
#define MLC_NAND_COMMON_H

#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * MLC NAND support functions
 **********************************************************************/

/* Issue a command to the MLC NAND device */
void mlc_cmd(UNS_8 cmd);

/* Issue a address to the MLC NAND device */
void mlc_addr(UNS_8 addr);

/* Wait for device to go to the ready state */
void mlc_wait_ready(void);

/* Return the current NAND status */
UNS_8 mlc_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* MLC_NAND_COMMON_H */
