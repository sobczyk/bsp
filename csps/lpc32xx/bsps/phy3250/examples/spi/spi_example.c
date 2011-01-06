/***********************************************************************
 * $Id:: spi_example.c 2391 2009-10-28 00:17:59Z wellsk                $
 *
 * Project: SPI driver example
 *
 * Description:
 *     A simple SPI polled driver example.
 *
 * Notes:
 *     This examples has no direct output. This code must be executed
 *     with a debugger to see how it works. The write functionality
 *     has been disabled by default to prevent unintended writes to
 *     the serial EEPROM. To enable it, uncomment the SPIWRITE define.
 *     Only SPI1 interface is used in this example. GPIO_05, SPI1_CLK,
 *     SPI1_DATIO, and SPI1_DATIN pins can be observed on the extention
 *     board test points 17B, 16F, 17E, and 17A.
 *     Data programmed into the EEPROM and read from it can be found in
 *     eewr_data[] and eerd_data[] arrays.
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
#include "lpc32xx_spi_driver.h"
#include "lpc32xx_clkpwr.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* SPI device handles */
static INT_32 spidev;

/* SPI device configuration */
static SPI_CONFIG_T spicfg;

/* Uncomment me to enable SPI serial EEPROM writes, this will change
   data on the serial EEPROM. Use with care */
//#define SPIWRITE

/* Number of serial EEPROM bytes to write and read and index of
   location to write/read */
#define SPIBYTES 3
#define SPIINDEX 100

/***********************************************************************
 *
 * Function: eeprom_rdsr
 *
 * Purpose: Read the EEPROM Ready Status Register (RDSR)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr : none
 *
 * Outputs: None
 *
 * Returns: The byte read from RDSR
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 eeprom_rdsr(void)
{
  UNS_8 data;

  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  data = 0x05;                                      //prepare RDSR
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);
  spi_write(spidev, &data, 1);
  spi_ioctl(spidev, SPI_DELAY, 1);

  spicfg.transmitter = FALSE;                       //SPI1 is a Rx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  spi_write(spidev, &data, 1);                      //dummy write
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);

  spi_read(spidev, &data, 1);                       //read the data
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);

  return data;
}


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

  /* wait for EEPROM to become ready - use RDSR */
  /* loop until RDSR bit 0 becomes 0            */
  while ((eeprom_rdsr() & 0x01) != 0x00);

  /* send WREN command */
  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  data [0] = 0x06;                                  //prepare WREN
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);
  spi_write(spidev, data, 1);
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);

  /* wait for EEPROM to become ready and write enabled - use RDSR */
  /* loop until RDSR bit 1 becomes 1 and RDSR bit 0 becomes 0     */
  while ((eeprom_rdsr() & 0x03) != 0x02);

  /* write data - WRITE */
  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  data [0] = 0x02;                                  //prepare WRITE
  data [1] = (addr >> 8) & 0xFF;
  data [2] = (addr >> 0) & 0xFF;
  data [3] = dat8;
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);
  spi_write(spidev, data, 4);
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);

  return;
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
 *     addr : Serial EEPROM byte address to read
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
  UNS_8 data [3];

  /* wait for EEPROM to become ready - use RDSR */
  /* loop until RDSR bit 0 becomes 0            */
  while ((eeprom_rdsr() & 0x01) != 0x00);

  spicfg.transmitter = TRUE;                        //SPI1 is a Tx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  /* read data from the EEPROM - use READ */
  data [0] = 0x03;                                  //prepare READ
  data [1] = (addr >> 8) & 0xFF;
  data [2] = (addr >> 0) & 0xFF;

  spi_ioctl(spidev, SPI_DRIVE_SSEL, 0);
  spi_write(spidev, data, 3);
  spi_ioctl(spidev, SPI_DELAY, 1);

  spicfg.transmitter = FALSE;                       //SPI1 is a Rx
  spi_ioctl(spidev, SPI_TXRX, (INT_32) & spicfg);

  spi_write(spidev, data, 1);                       //dummy write
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_DRIVE_SSEL, 1);

  spi_read(spidev, &data [0], 1);                   //read data
  spi_ioctl(spidev, SPI_DELAY, 1);
  spi_ioctl(spidev, SPI_CLEAR_RX_BUFFER, 0);

  return data [0];
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
  UNS_8 data, eewr_data[SPIBYTES], eerd_data[SPIBYTES];
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

  /* The SPI can not control the SSEL signal, it will be driven
     by write/read routines as needed and accessed as an output
     GPIO pin. This EEPROM can be use with SPI mode 3 when the
     chip select must remain asserted for the entire cycle.     */
  GPIO->p2_mux_clr = P2_GPIO05_SSEL0;
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  /* Select SPI1 pins */
  GPIO->p_mux_clr = P_SPI1DATAIO_SSP0_MOSI |
                    P_SPI1DATAIN_SSP0_MISO | P_SPI1CLK_SCK0;
  CLKPWR->clkpwr_spi_clk_ctrl |= CLKPWR_SPICLK_USE_SPI1;

  /* Open SPI */
  spicfg.databits = 8;
  spicfg.highclk_spi_frames = TRUE;
  spicfg.usesecond_clk_spi = TRUE;
  spicfg.spi_clk = 5000000;
  spicfg.msb = FALSE;
  spicfg.transmitter = TRUE;
  spicfg.busy_halt = FALSE;
  spicfg.unidirectional = TRUE;
  spidev = spi_open(SPI1, (INT_32) & spicfg);
  if (spidev == 0)
  {
    /* Error */
    return -1;
  }
  spi_ioctl(spidev, SPI_ENABLE, 1);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Send data to the serial EEPROM */
  addr = SPIINDEX;
  bytes = SPIBYTES;
  data = (UNS_8)(addr & 0xFF);
  while (bytes > 0)
  {
#ifdef SPIWRITE
    data++;
    eewr_data[SPIBYTES-bytes] = data;
    eeprom_write(addr, data);
#endif
    data = eeprom_read(addr);
    eerd_data[SPIBYTES-bytes] = data;
    addr++;
    bytes--;
  }

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close SPI */
  spi_close(spidev);

  return 1;
}
