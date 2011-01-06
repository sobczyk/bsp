/***********************************************************************
 * $Id:: board_spi_flash_driver.c 3380 2010-05-05 23:54:23Z usb10132   $
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

#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_ssp_driver.h"
#include "board_config.h"

/* Serial EEPROM commands (SPI via SSP) */
#define SEEPROM_WREN          0x06
#define SEEPROM_WRDI          0x04
#define SEEPROM_RDSR          0x05
#define SEEPROM_WRSR          0x01
#define SEEPROM_READ          0x03
#define SEEPROM_WRITE         0x02

/* SSP driver flag */
static INT_32 sspid;

/***********************************************************************
 *
 * Function: board_spi_transfer
 *
 * Purpose: Transfer data to/from the serial EEPROM
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     out   : Output data
 *     in    : Input data
 *     bytes : Number of bytes to send/receive
 *
 * Outputs: None
 *
 * Returns: TRUE if the byte was transferred
 *
 * Notes: Do not use this function to transfer more than 8 bytes!
 *
 **********************************************************************/
static BOOL_32 board_spi_transfer(UNS_8 *out, UNS_8 *in, int bytes)
{
  INT_32 rbytes = 0;

  /* Asset chip select */
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);

  ssp_write(sspid, out, bytes);
  while (rbytes < bytes)
  {
    rbytes += ssp_read(sspid, &in [rbytes], 1);
  }

  /* De-assert chip select */
  GPIO->p3_outp_set = P3_STATE_GPIO(5);
 
  return TRUE;
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: board_spi_config
 *
 * Purpose: Config SSP for correct mode
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
 * Notes:
 *     The SSP0 CS should be configured as a GPIO output
 *
 **********************************************************************/
void board_spi_config(void)
{
	SSP_CONFIG_T sspcfg;

    /* Try to open SSP driver */
    sspcfg.databits           = 8;
    sspcfg.mode               = SSP_CR0_FRF_SPI;
    sspcfg.highclk_spi_frames = FALSE;
    sspcfg.usesecond_clk_spi  = FALSE;
    sspcfg.ssp_clk            = SPICLKRATE;
    sspcfg.master_mode        = TRUE;
    sspid = ssp_open(SSP0, (INT_32) & sspcfg);
    ssp_ioctl(sspid, SSP_ENABLE, 1);
}

/***********************************************************************
 *
 * Function: board_spi_write
 *
 * Purpose: Write a byte to the serial EEPROM
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
 * Notes: Will handle up to 64KBytes devices.
 *
 **********************************************************************/
void board_spi_write(UNS_8 byte, int index)
{
  UNS_8 prog, datai [8], datao [8];

  /* Write enable */
  datao [0] = SEEPROM_WREN;
  datao [1] = 0xFF;
  board_spi_transfer(datao, datai, 2);

  /* Write byte */
  datao [0] = SEEPROM_WRITE;
  datao [1] = (UNS_8)((index >> 8) & 0xFF);
  datao [2] = (UNS_8)((index >> 0) & 0xFF);
  datao [3] = byte;
  board_spi_transfer(datao, datai, 4);

  /* Wait for device to finish programming */
  prog = 0xFF;
  while ((prog & 0x1) != 0)
  {
    /* Read status */
    datao [0] = SEEPROM_RDSR;
    datao [1] = 0xFF;
    board_spi_transfer(datao, datai, 2);
    prog = datai [1];
  }
}

/***********************************************************************
 *
 * Function: board_spi_read
 *
 * Purpose: Read a byte from the serial EEPROM at the passed index
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     index : Index into serial device to read index
 *
 * Outputs: None
 *
 * Returns: Byte read from passed index
 *
 * Notes: Will handle up to 64KBytes devices.
 *
 **********************************************************************/
UNS_8 board_spi_read(int index)
{
  UNS_8 datai [8], datao [8];
  UNS_8 byte = 0;

  /* Read byte */
  datao [0] = SEEPROM_READ;
  datao [1] = (UNS_8)((index >> 8) & 0xFF);
  datao [2] = (UNS_8)((index >> 0) & 0xFF);
  datao [3] = 0xFF;
  board_spi_transfer(datao, datai, 4);
  byte = datai [3];

  return byte;
}
