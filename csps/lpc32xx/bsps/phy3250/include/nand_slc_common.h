/***********************************************************************
 * $Id:: nand_slc_common.h 3388 2010-05-06 00:17:50Z usb10132          $
 *
 * Project: SLC support functions
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

#ifndef SLC_NAND_COMMON_H
#define SLC_NAND_COMMON_H

#include "lpc_types.h"
#include "lpc32xx_dmac.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * SLC NAND support functions
 **********************************************************************/

/* Issue a command to the MLC NAND device */
void slc_cmd(UNS_8 cmd);

/* Issue a address to the MLC NAND device */
void slc_addr(UNS_8 addr);

/* Wait for device to go to the ready state */
void slc_wait_ready(void);

/* Assert or deassert NAND chip select state */
void slc_sb_set_cs(BOOL_32 low);

/* Return the current NAND status */
UNS_8 slc_get_status(void);

/* Copy ECC from/to buffers */
INT_32 slc_ecc_copy_to_buffer(UNS_8 * spare, const UNS_32 * ecc, INT_32 count);
INT_32 slc_ecc_copy_from_buffer(const UNS_8 * spare, UNS_32 * ecc, INT_32 count);

/* Locate the ECC error and correct it (256 byte block), returns 1
   if corrected, 0 = no correction needed, <0 if uncorrectable */
INT_32 nand_slc_correct_ecc(UNS_32 *ecc_gen, UNS_32 *ecc_stored,
                            UNS_8 *buf);

#ifdef __cplusplus
}
#endif

#endif /* SLC_NAND_COMMON_H */
