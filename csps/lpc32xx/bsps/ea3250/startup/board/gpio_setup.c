/***********************************************************************
 * $Id:: gpio_setup.c 3376 2010-05-05 22:28:09Z usb10132               $
 *
 * Project: GPIO and MUX code
 *
 * Description:
 *     Provides MUX and GPIO setup for the board
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

#include "startup.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_gpio_driver.h"

/***********************************************************************
 *
 * Function: gpio_setup
 *
 * Purpose: Setup GPIO and MUX states
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
 * Notes: Changes these as needed.
 *
 **********************************************************************/
void gpio_setup(void)
{
  /* P2 group muxing
     GPIO_02 / KEY_ROW6 | (ENET_MDC)      ->KEY_ROW6 | (ENET_MDC)
     GPIO_03 / KEY_ROW7 | (ENET_MDIO)     ->KEY_ROW7 | (ENET_MDIO)
     GPO_21 / U4_TX | (LCDVD[3])          ->U4_TX | (LCDVD[3])
     EMC_D_SEL                            ->(D16 ..D31 used)
     GPIO_04 / SSEL1 | (LCDVD[22])        ->SSEL1 | (LCDVD[22])
     GPIO_05 / SSEL0                      ->GPIO_05 */
  GPIO->p2_mux_clr = (P2_SDRAMD19D31_GPIO | P2_GPO21_U4TX |
	  P2_GPIO03_KEYROW7 | P2_GPIO02_KEYROW6 | P2_GPIO05_SSEL0);
  GPIO->p2_dir_set = P2_DIR_GPIO(5);

  /* P_ group muxing
     I2S1TX_SDA / MAT3.1                  ->I2S1TX_SDA
     I2S1TX_CLK / MAT3.0                  ->I2S1TX_CLK
     I2S1TX_WS / CAP3.0                   ->I2S1TX_WS
     SPI2_DATIO / MOSI1 | (LCDVD[20])     ->MOSI1 | (LCDVD[20])
     SPI2_DATIN / MISO1 | (LCDVD[21])     ->MISO1 | (LCDVD[21])
     SPI2_CLK / SCK1 | (LCDVD[23])        ->SCK1 | (LCDVD[23])
     SPI1_DATIO / MOSI0                   ->MOSI0
     SPI1_DATIN / MISO0                   ->MISO0
     SPI1_CLK / SCK0                      ->SCK0
     (MS_BS) | MAT2.1                     ->(MS_BS)
     (MS_SCLK) | MAT2.0                   ->(MS_SCLK)
     U7_TX / MAT1.1 | (LCDVD[11])         ->MAT1.1 | (LCDVD[11])
     (MS_DIO3) | MAT0.3                   ->(MS_DIO3)
     (MS_DIO2) | MAT0.2                   ->(MS_DIO2)
     (MS_DIO1) | MAT0.1                   ->(MS_DIO1)
     (MS_DIO0) | MAT0.0                   ->(MS_DIO0) */
  GPIO->p_mux_set = (P_I2STXSDA1_MAT31 | P_I2STXCLK1_MAT30 |
	  P_I2STXWS1_CAP30 | P_MAT20 | P_MAT21 | P_MAT03 | P_MAT02 |
    P_MAT01 | P_MAT00 | P_SPI1DATAIO_SSP0_MOSI | P_SPI1DATAIN_SSP0_MISO |
	P_SPI1CLK_SCK0);
  GPIO->p_mux_clr = (P_SPI2DATAIO_MOSI1 | P_SPI2DATAIN_MISO1 |
	  P_SPI2CLK_SCK1 | P_U7TX_MAT11);

    /* P3 group muxing
     GPO_02 / MAT1.0 | (LCDVD[0])       ->GPO_02
     GPO_06 | (LCDVD[18])               ->PWM4.3 | (LCDVD[18])
     GPO_08 | (LCDVD[8])                ->PWM4.2 | (LCDVD[8])
     GPO_09 | (LCDVD[9])                ->PWM4.1 | (LCDVD[9])
     GPO_10 / MC2B | (LCDPWR)           ->PWM3.6 | (LCDPWR)
     GPO_12 / MC2A | (LCDLE)            ->PWM3.5 | (LCDLE)
     GPO_13 / MC1B | (LCDDCLK)          ->PWM3.4 | (LCDDCLK)
     GPO_15 / MC1A | (LCDFP)            ->PWM3.3 | (LCDFP)
     GPO_16 / MC0B | (LCDENAB/LCDM)     ->PWM3.2 | (LCDENAB/LCDM)
     GPO_18 / MC0A | (LCDLP)            ->PWM3.1 | (LCDLP) */
  GPIO->p3_mux_clr = (P3_GPO2_MAT10 | P3_GPO6 | P3_GPO8 | P3_GPO9 |
	  P3_GPO10_MC2B | P3_GPO12_MC2A | P3_GPO13_MC1B | P3_GPO15_MC1A |
	  P3_GPO16_MC0B | P3_GPO18_MC0A);

	/* Default mux configuation for P0 as follows:
	   P0_GPOP0_I2SRXCLK1 -> GPOP0 (Clear) (Output low)
	   P0_GPOP1_I2SRXWS1  -> GPOP1 (Clear) (Output low)
	   P0_GPOP2_I2SRXSDA0 -> GPOP2 (Clear) (Input)
	   P0_GPOP3_I2SRXCLK0 -> GPOP3 (Clear) (Input)
	   P0_GPOP4_I2SRXWS0  -> GPOP4 (Clear) (Input)
	   P0_GPOP5_I2STXSDA0 -> GPOP5 (Clear) (Input)
	   P0_GPOP6_I2STXCLK0 -> GPOP6 (Clear) (Input)
	   P0_GPOP7_I2STXWS0  -> GPOP7  (Clear) (Input) */
	GPIO->p0_mux_clr = (P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1 |
		P0_GPOP2_I2SRXSDA0 | P0_GPOP3_I2SRXCLK0 | P0_GPOP4_I2SRXWS0 |
		P0_GPOP5_I2STXSDA0 | P0_GPOP6_I2STXCLK0 | P0_GPOP7_I2STXWS0);
	gpio_set_p0_dir((P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1), 0);
	gpio_set_p0_state(0, (P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1));

  /* Muxing for address bus - full 24-bit address bus on
     EMC_A[0..23] */
  GPIO->p1_mux_clr = P1_ALL;

  /* Clear reset of ethernet device (GPO_04) */
  gpio_set_gpo_state(P3_STATE_GPO(4), 0);
}
