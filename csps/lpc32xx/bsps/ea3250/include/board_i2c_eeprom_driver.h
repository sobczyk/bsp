/***********************************************************************
 * $Id:: board_spi_flash_driver.h 3388 2010-05-06 00:17:50Z usb10132   $
 *
 * Project: Simple I2C functions for I2C EEPROM access
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

#ifndef BOARD_I2C_EEPROM_DRIVER_H
#define BOARD_I2C_EEPROM_DRIVER_H

#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Serial EEPROM Slave address */
#define EEPROM_SLAVE_ADDR		0xA0

/* Internal FIFO Depth of EEPROM */
#define EEPROM_FIFO_DEPTH		0x40		

/* EEPROM BASE ADDRESS*/
#define EEPROM_BASE_ADDRESS		0x0000
#define EEPROM_LENGTH			0x8000

/* I2C init, read, and write functions for byte access */
void board_i2c_init(UNS_32 id);
int board_i2c_write(UNS_32 addr, UNS_8 *buffer, UNS_32 length);
int board_i2c_write_byte(UNS_32 addr, UNS_8 data);
int board_i2c_read(UNS_32 addr, UNS_8 *buffer, UNS_32 length);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_I2C_EEPROM_DRIVER_H */
