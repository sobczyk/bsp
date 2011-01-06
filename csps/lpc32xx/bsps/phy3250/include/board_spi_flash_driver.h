/***********************************************************************
 * $Id:: board_spi_flash_driver.h 3388 2010-05-06 00:17:50Z usb10132   $
 *
 * Project: Simple SPI functions for SPI FLASH access
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

#ifndef BOARD_SPI_FLASH_DRIVER_H
#define BOARD_SPI_FLASH_DRIVER_H

#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* SPI config, read, and write functions for byte access */
void board_spi_config(void);
void board_spi_write(UNS_8 byte, int index);
UNS_8 board_spi_read(int index);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_SPI_FLASH_DRIVER_H */
