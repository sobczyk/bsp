/***********************************************************************
 * $Id:: spi_burn.c 3404 2010-05-06 18:24:03Z usb10132                $
 *
 * Project: SPI burner for kickstart or stage 1
 *
 * Description:
 *     Burner for burning the kickstart ot stage 1 application into
 *     SPI FLASH.
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
  UNS_32 *a1;
  UNS_32 loadsize, loadcount;
  UNS_8 *p8, tmp [16];
  UNS_32 idx = 0;
  UNS_8 buffi[8];
  UNS_32 offset = 0;
  
  uart_output_init();

  /* Now to download the image */
  uart_output((UNS_8 *) "X");

  /* Wait for 'p' */
  while (!uart_input(&tmp[0], 1));

  /* Wait for 8 bytes from other side */
  idx = 0;
  while (idx < 8)
  {
	  idx += uart_input(&tmp[idx], (8 - idx));
  }

  /* Send 'o' after receving 8 bytes */
  uart_output((UNS_8 *) "o");

  /* Get size of secondary file */
  a1 = (UNS_32 *) tmp;
  loadsize = a1[1];

  /* Receive complete file using UART */
  loadcount = 0;
  p8 = (UNS_8 *) BURNER_LOAD_ADDR;
  while (loadsize > loadcount)
  {
	  loadcount += uart_input(p8 + loadcount, (loadsize - loadcount));
  }
  uart_output((UNS_8 *) "t");

  if (loadsize > (30 * 1024))
  {
   	uart_output("Image too large for kickstart, 54K max!\r\n");
	while (1);
  }

  /* Force SSP configuration, use GPIO_05 (SSP0_CS) in software
     control mode */
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p_mux_set = P_SPI1CLK_SCK0 | P_SPI1DATAIN_SSP0_MISO |
	  P_SPI1DATAIO_SSP0_MOSI;

  board_spi_config();

  /* Disable write protect */
  spi_flash_wp_disable();

  /* Write tags and size first */
  uart_output((UNS_8 *)"Writing SPI tag words\r\n");
  buffi[0] = 0xDF;
  buffi[1] = 0x9B;
  buffi[2] = 0x57;
  buffi[3] = 0x13;
  buffi[4] = (UNS_8) ((loadsize >> 0) & 0xFF);
  buffi[5] = (UNS_8) ((loadsize >> 8) & 0xFF);
  buffi[6] = (UNS_8) ((loadsize >> 16) & 0xFF);
  buffi[7] = (UNS_8) ((loadsize >> 24) & 0xFF);

  for (idx = 0; idx < 8; idx++)
  {
	board_spi_write(buffi[idx], idx);
	if (board_spi_read(idx) != buffi[idx])
    {
     	uart_output((UNS_8 *)"SPI tag verification failed.\r\n");
    }
  }

  uart_output((UNS_8 *)"Writing kickstart image data\r\n");
  offset = 8;
  for (idx = 0; idx < loadsize; idx++)
  {
	board_spi_write(p8[idx], offset + idx);
	if (board_spi_read(idx + offset) != p8[idx])
    {
		uart_output((UNS_8 *)"SPI image data verification failed.\r\n");
    }
  }
  uart_output((UNS_8 *)"Successfully flashed kickstart image\r\n");

  /* Loop forever */
  while (1);
}
