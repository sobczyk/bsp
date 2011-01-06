/***********************************************************************
 * $Id:: spi_erase.c 3452 2010-05-10 18:33:56Z usb10132               $
 *
 * Project: Simple SPI erase program
 *
 * Description:
 *     Erases the start of SPI FLASH to prevent SPI boot.
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
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_uart_driver.h"
#include "board_spi_flash_driver.h"
#include "misc_config.h"
#include "common_funcs.h"

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns 1, or <0 on an error
 *
 * Notes: None
 *
 **********************************************************************/
void c_entry(void)
{
  UNS_32 idx;

  uart_output_init();

  /* Force SSP configuration, use GPIO_05 (SSP0_CS) in software
     control mode */
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p_mux_set = P_SPI1CLK_SCK0 | P_SPI1DATAIN_SSP0_MISO |
	  P_SPI1DATAIO_SSP0_MOSI;

  board_spi_config();

  /* Disable write protect */
  spi_flash_wp_disable();

  /* Fill first locations with 0's */
  for (idx = 0; idx < 8; idx++)
  {
	board_spi_write(0, idx);
  }

  uart_output((UNS_8 *)"SPI erased\r\n");

  /* Loop forever */
  while (1);
}
