/***********************************************************************
 * $Id:: mi2c_example.c 2095 2009-07-28 19:12:10Z wellsk               $
 *
 * Project: NXP PHY3250 I2C example
 *
 * Description:
 *     This file contains a I2C master mode example that queries a
 *     few I2C registers of the UDA1380. A register is changed, then
 *     queries again, then reset to it's original state.
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
#include "lpc_arm922t_cp15_driver.h"
#include "lpc_irq_fiq.h"
#include "lpc32xx_chip.h"
#include "phy3250_board.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_mstr_i2c_driver.h"

/***********************************************************************
 * I2C example private functions and data
 **********************************************************************/

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* I2C device handles */
static INT_32 mi2cdev;

#define I2CADDR 0x18

static UNS_16 read16(UNS_8 reg) {
  UNS_32 rxdata[8], txdata[8];
  I2C_MTXRX_SETUP_T xfer;
  UNS_32 status;

  txdata[0] = (I2CADDR << 1) | I2C_START | 0;
  txdata[1] = (UNS_32) reg;
  txdata[2] = (I2CADDR << 1) | I2C_START | 1;
  txdata[3] = 0xFF;
  txdata[4] = 0xFF | I2C_STOP;
  xfer.tx_data = txdata;
  xfer.rx_data = rxdata;
  xfer.tx_length = 5;
  xfer.clock_rate = 400000;
  xfer.cb = NULL;

  if (i2c_mstr_ioctl(mi2cdev, I2C_MSTR_TRANSFER, (INT_32) &xfer) ==
	  _NO_ERROR) {
		  status = i2c_mstr_ioctl(mi2cdev, I2C_MSTR_GET_STATUS, 0);
		  while ((status & (I2C_MSTR_STATUS_ARBF |
			  I2C_MSTR_STATUS_NOACKF | I2C_MSTR_STATUS_DONE)) == 0) {
				  status = i2c_mstr_ioctl(mi2cdev,
					  I2C_MSTR_GET_STATUS, 0);
		  }
  }
  
  return ((UNS_16) rxdata[1] | (((UNS_16) rxdata[0]) << 8));
}

static UNS_16 write16(UNS_8 reg, UNS_16 data) {
  UNS_32 rxdata[8], txdata[8];
  I2C_MTXRX_SETUP_T xfer;
  UNS_32 status;

  txdata[0] = (I2CADDR << 1) | I2C_START | 0;
  txdata[1] = (UNS_32) reg;
  txdata[2] = ((UNS_32) (data & 0xFF00) >> 8);
  txdata[3] = I2C_STOP | ((UNS_32) data & 0x00FF);
  xfer.tx_data = txdata;
  xfer.rx_data = rxdata;
  xfer.tx_length = 4;
  xfer.clock_rate = 400000;
  xfer.cb = NULL;

  if (i2c_mstr_ioctl(mi2cdev, I2C_MSTR_TRANSFER, (INT_32) &xfer) ==
	  _NO_ERROR) {
		  status = i2c_mstr_ioctl(mi2cdev, I2C_MSTR_GET_STATUS, 0);
		  while ((status & (I2C_MSTR_STATUS_ARBF |
			  I2C_MSTR_STATUS_NOACKF | I2C_MSTR_STATUS_DONE)) == 0) {
				  status = i2c_mstr_ioctl(mi2cdev,
					  I2C_MSTR_GET_STATUS, 0);
		  }
  }

  return data;
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
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void c_entry(void)
{
  UNS_16 data;

  /* Disable interrupts in ARM core */
  disable_irq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Install I2C interrupt handlers */
  int_install_irq_handler(IRQ_I2C_1, (PFV) i2c1_mstr_int_hanlder);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();
  int_enable(IRQ_I2C_1);

  /* Open I2C device */
  mi2cdev = i2c_mstr_open((void *) I2C1, 0);
  if (mi2cdev == 0)
	  return;

  /* Read register '0' */
  data = read16(0);

  /* Or' in bit 5 */
  write16(0, data | (1 << 5));

 /* Verify it */
  if ((data | (1 << 5)) != read16(0)) {
	  /* Failed */
	  goto exit1;
  }

  /* Restore original */
  write16(0, data);

exit1:
  /* close I2C device */    
  i2c_mstr_close(mi2cdev);
}
