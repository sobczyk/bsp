/***********************************************************************
 * $Id:: ssp_example.c 2393 2009-10-28 00:18:31Z wellsk                $
 *
 * Project: SSP driver example
 *
 * Description:
 *     A simple SSP polled driver example.
 *
 * Notes:
 *     This examples has no direct output. This code must be executed
 *     with a debugger to see how it works. The write functionality
 *     has been disabled by default to prevent unintended writes to
 *     the serial EEPROM. To enable it, uncomment the SSPWRITE define.
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
#include "lpc_irq_fiq.h"
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_ssp_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* SSP device handles */
static INT_32 sspdev;

/* Uncomment me to enable SSP serial EEPROM writes, this will change
   data on the serial EEPROM. Use with care */
//#define SSPWRITE

/* Number of serial EEPROM bytes to write and read and index of
   location to write/read */
#define SSPBYTES 16
#define SSPINDEX 100

/***********************************************************************
 *
 * Function: eeprom_write
 *
 * Purpose: Write a byte to a serial EEPROM address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr : Serial EEPROM byte address to write
 *     dat8 : Data to write
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void eeprom_write(UNS_16 addr, UNS_8 dat8)
{
  UNS_8 data [8];
  int bytes;

  data [0] = 0x06; /* WREN */
  data [1] = 0xFF;
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);
  ssp_write(sspdev, data, 2);
  bytes = 0;
  while (bytes < 2)
  {
    bytes += ssp_read(sspdev, &data [bytes], 1);
  }
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  data [0] = 0x05; /* RDSR */
  data [1] = 0xFF;
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);
  ssp_write(sspdev, data, 2);
  bytes = 0;
  while (bytes < 2)
  {
    bytes += ssp_read(sspdev, &data [bytes], 1);
  }
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  data [0] = 0x02; /* Write */
  data [1] = (addr >> 8) & 0xFF;
  data [2] = (addr >> 0) & 0xFF;
  data [3] = dat8;
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);
  ssp_write(sspdev, data, 4);
  bytes = 0;
  while (bytes < 4)
  {
    bytes += ssp_read(sspdev, &data [bytes], 1);
  }
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  data [0] = 0x05; /* RDSR */
  data [1] = 0xFF;
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);
  ssp_write(sspdev, data, 2);
  bytes = 0;
  while (bytes < 2)
  {
    bytes += ssp_read(sspdev, &data [bytes], 1);
  }
  GPIO->p3_outp_set = P3_STATE_GPIO(5);
}

/***********************************************************************
 *
 * Function: eeprom_read
 *
 * Purpose: Read a byte from a serial EEPROM address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr : Serial EEPROM byte address to write
 *
 * Outputs: None
 *
 * Returns: The byte read from the pass address
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 eeprom_read(UNS_16 addr)
{
  UNS_8 data [8];
  int bytes;

  data [0] = 0x03; /* Read */
  data [1] = (addr >> 8) & 0xFF;
  data [2] = (addr >> 0) & 0xFF;
  data [3] = 0xFF;
  GPIO->p3_outp_clr = P3_STATE_GPIO(5);
  ssp_write(sspdev, data, 4);
  addr++;
  bytes = 0;
  while (bytes < 4)
  {
    bytes += ssp_read(sspdev, &data [bytes], 1);
  }
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  return data [3];
}

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
int c_entry(void)
{
  SSP_CONFIG_T sspcfg;
  UNS_8 data;
  UNS_16 addr;
  int bytes;

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* The SSP can control the SSEL signal, but will assert the signal
     at the start of a single FIFO entry's transfer and de-assert it
     after the FIFO entry is sent. Some blocks need the chip select to
     remain asserted for the entire cycle (such as this serial EEPROM)
     so a GPIO will be used instead. */
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  /* The MISO, MOSI, and SCK signals are controlled by the SSP */
  GPIO->p_mux_set = (P_SPI1DATAIO_SSP0_MOSI |
                        P_SPI1DATAIN_SSP0_MISO | P_SPI1CLK_SCK0);

  /* Open SSP */
  sspcfg.databits = 8;
  sspcfg.mode = SSP_CR0_FRF_SPI;
  sspcfg.highclk_spi_frames = FALSE;
  sspcfg.usesecond_clk_spi = FALSE;
  sspcfg.ssp_clk = 5000000;
  sspcfg.master_mode = TRUE;
  sspdev = ssp_open(SSP0, (INT_32) & sspcfg);
  if (sspdev == 0)
  {
    /* Error */
    return -1;
  }
  ssp_ioctl(sspdev, SSP_ENABLE, 1);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Send data to the serial EEPROM */
  addr = SSPINDEX;
  bytes = SSPBYTES;
  data = (UNS_8)(addr & 0xFF);
  while (bytes > 0)
  {
#ifdef SSPWRITE
    data++;
    eeprom_write(addr, data);
#endif
    data = eeprom_read(addr);
    addr++;
  }

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close SSP */
  ssp_close(sspdev);

  return 1;
}
