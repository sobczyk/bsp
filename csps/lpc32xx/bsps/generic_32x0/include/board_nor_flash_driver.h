/***********************************************************************
 * $Id:: board_nor_flash_driver.h 3398 2010-05-06 18:06:52Z usb10132   $
 *
 * Project: Simple functions for NOR FLASH program
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

#ifndef BOARD_NOR_FLASH_DRIVER_H
#define BOARD_NOR_FLASH_DRIVER_H

#include "lpc_types.h"
#include "lpc32xx_emc.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Base address of NOR FLASH */
#define BOARD_NOR_FLASH_BASE_ADDR EMC_CS0_BASE

/* NOR FLASH program and verify functions */
void board_nor_init(UNS_8 memory_width);
UNS_32 board_nor_verify(UNS_32 adr, UNS_8 *buf, UNS_32 sz);
INT_32 board_nor_prg(UNS_32 adr, UNS_8 *buf, UNS_32 sz);
INT_32 board_nor_erase_sector(UNS_32 adr);
INT_32 board_nor_erase_chip(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_NOR_FLASH_DRIVER_H */
