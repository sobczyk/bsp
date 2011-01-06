/***********************************************************************
 * $Id:: i2c_example.c 1332 2008-11-19 21:20:07Z tangdz                $
 *
 * Project: NXP PHY3250 I2C example
 *
 * Description:
 *     This file contains a I2C code example that uses the on-board
 *     RTC-8564. The RTC is intialized and set to generate an alarm.
 *     LED1 is blinking while RTC is polled and LED2 is turned-on
 *     when an alarm is detected. Only I2C1 interface is used in this 
 *     example. I2C1_SCL and I2C1_SDA signals can be observed on the 
 *     extension board pins 10F and 11C.
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
#include "lpc32xx_hstimer_driver.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_i2c_driver.h"

/***********************************************************************
 * I2C example private functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: delay
 *
 * Purpose: generate a delay
 *
 * Processing:
 *     A local software counter counts up to the specified count.
 *
 * Parameters:
 *    cnt : number to be counted
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void delay(UNS_32 cnt)
{
    UNS_32 i = cnt;
    while (i != 0) i--;
    return;
}

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* I2C device handles */
static INT_32 i2cdev1, i2cdev2;

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
  static I2C_SETUP_T i2c_setup[2];
  static I2C_MTX_SETUP_T   i2c_mtx_setup;
  static I2C_MTXRX_SETUP_T i2c_mtxrx_setup;
  static I2C_MRX_SETUP_T   i2c_mrx_setup;
  UNS_8  rx_data[16], tx_data[16];

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

  /* Enable I2C1/2 clock */
  clkpwr_clk_en_dis(CLKPWR_I2C1_CLK,1);
  clkpwr_clk_en_dis(CLKPWR_I2C2_CLK,1);

  /* install default I2C1 & I2C2 interrupt handlers */
  int_install_ext_irq_handler(IRQ_I2C_1, 
                              (PFV) i2c1_user_interrupt, ACTIVE_LOW, 1);
  int_install_ext_irq_handler(IRQ_I2C_2, 
                              (PFV) i2c2_user_interrupt, ACTIVE_LOW, 1);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* open I2C1 */
  i2cdev1 = i2c_open(I2C1, 0);

  /* formally assign a 7-bit slave address 0x50 to I2C1    */
  /* I2C1 clock is 100 kHz, 50% duty cycle, high pin drive */
  i2c_setup[0].addr_mode  = ADDR7BIT;
  i2c_setup[0].sl_addr    = 0x50;
  i2c_setup[0].rate_option= I2C_RATE_RELATIVE;
  i2c_setup[0].rate       = 100000;
  i2c_setup[0].low_phase  = 50;
  i2c_setup[0].high_phase = 50;
  i2c_setup[0].pins_drive = I2C_PINS_HIGH_DRIVE;
  i2c_ioctl((UNS_32) i2cdev1, I2C_SETUP, (UNS_32) &i2c_setup[0]);

  /* stop the RTC */
  tx_data[0] = 0x00;
  tx_data[1] = 1<<5;
  i2c_mtx_setup.addr_mode = ADDR7BIT;
  i2c_mtx_setup.sl_addr = 0x51;
  i2c_mtx_setup.tx_data = &tx_data[0];
  i2c_mtx_setup.tx_length = 2;
  i2c_mtx_setup.retransmissions_max = 5;
  i2c_ioctl(i2cdev1, I2C_MASTER_TX, (INT_32) &i2c_mtx_setup);
  while (( i2c_mtx_setup.status & I2C_SETUP_STATUS_DONE) == 0);
  delay(10000);

  /* initialize the RTC to Saturday, Aug 16 2008, 14:18:35 */
  /* set alarm for Saturday, 14:19                         */
  tx_data[ 0] = 0x01;             //write from RTC address 1
  tx_data[ 1] = 0x00;             //no ints
  tx_data[ 2] = 0x35;             //seconds
  tx_data[ 3] = 0x18;             //minutes
  tx_data[ 4] = 0x14;             //hours
  tx_data[ 5] = 0x16;             //days
  tx_data[ 6] = 0x06;             //weekdays
  tx_data[ 7] = 0x08;             //months
  tx_data[ 8] = 0x08;             //years
  tx_data[ 9] = 0x19;             //alarm minutes
  tx_data[10] = 0x14;             //alarm hours
  tx_data[11] = 0x16;             //alarm days
  tx_data[12] = 0x06;             //alarm weekdays
  tx_data[13] = 0x00;             //disable clockout
  tx_data[14] = 0x00;             //disable simple timer
  i2c_mtx_setup.tx_length = 15;
  i2c_ioctl(i2cdev1, I2C_MASTER_TX, (INT_32) &i2c_mtx_setup);
  while (( i2c_mtx_setup.status & I2C_SETUP_STATUS_DONE) == 0);
  delay(10000);

  /* let the RTC run */
  tx_data[0] = 0x00;              //write as of RTC address 0
  tx_data[1] = 0x00;              //write 0x00
  i2c_mtx_setup.tx_length = 2;
  i2c_ioctl(i2cdev1, I2C_MASTER_TX, (INT_32) &i2c_mtx_setup);
  while (( i2c_mtx_setup.status & I2C_SETUP_STATUS_DONE) == 0);
  delay(10000);

  gpio_set_gpo_state(0x00, 1<<14 | 1<<1);  //turn-off LED1 & LED2

  /* Loop until an alaram is reported */
  do
  {
    /* read from address 0x01; perform 2 reads */
	tx_data[0] = 0x01;                     //read as of RTC address 0x01
    i2c_mtxrx_setup.addr_mode = ADDR7BIT;
    i2c_mtxrx_setup.sl_addr = 0x51;
    i2c_mtxrx_setup.tx_data = &tx_data[0];
    i2c_mtxrx_setup.tx_length = 1;
    i2c_mtxrx_setup.rx_data = &rx_data[0];
    i2c_mtxrx_setup.rx_length = 2;
    i2c_mtxrx_setup.retransmissions_max = 5;
    i2c_ioctl(i2cdev1, I2C_MASTER_TXRX, (INT_32) &i2c_mtxrx_setup);
    while (( i2c_mtxrx_setup.status & I2C_SETUP_STATUS_DONE) == 0);

    /* control LED1 */
    if ((rx_data[1] & 0x01) == 0)
    {
      gpio_set_gpo_state(0x00, 1<<1);
    }
    else
    {
      gpio_set_gpo_state(1<<1, 0x00);
    }
	delay(10000);
  }
  while ((rx_data[0] & 0x08) == 0x00);

  /* turn-on LED2 */
  gpio_set_gpo_state(1<<14, 0x00);

  /* close I2C1 */    
  i2c_close(i2cdev1);
}
