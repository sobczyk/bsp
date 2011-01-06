/**********************************************************************
* $Id:: lpc32xx_gpio.h 2396 2009-10-28 00:20:32Z wellsk               $
*
* Project: LPC32XX chip family GPIO and MUX definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family components:
*         General Purpose Input/Output controller
*         Signal muxing
*
* Notes:
*     There are a lot of available GPIOs on the LPC32xx that are muxed
*     with other system peripherals. In some cases, enabling a specific
*     peripheral will disable the GPIOs (ie, LCD) muxed with that
*     peripheral, but in other cases GPIO functions can be selected as
*     part of the mux control. See the Users guide for more information
*     on GPIOs and muxing. The LPC32xx has the following basic GPIO
*     support:
*       P0[7..0]    inputs/outputs (P0.0 to P0.7)
*       P1[23..0]   inputs/outputs muxed with address bus
*                   (P1.0 to P1.23)
*       P2[12..0]   inputs/outputs muxed with data bus D[31..19]
*                   (P2.0 to P2.12)
*       GPI[9..0]   inputs GPI_09 to GPI_00
*       GPI[23..15] inputs GPI_15 to GPI_23
*       GPI[25]     input GPI_25
*       GPI[23..15] inputs GPI_15 to GPI_23
*       GPO[23..0]  outputs GPO_00 to GPO_23
*       GPIO[..0]   inputs/outputs GIPO_00 to GPIO_05
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

#ifndef LPC32XX_GPIO_H
#define LPC32XX_GPIO_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

/**********************************************************************
* GPIO Module Register Structure
**********************************************************************/

/* GPIO Module Register Structure */
typedef struct
{
  volatile UNS_32 p3_inp_state;   /* Input pin state register */
  volatile UNS_32 p3_outp_set;    /* Output pin set register */
  volatile UNS_32 p3_outp_clr;    /* Output pin clear register */
  volatile UNS_32 p3_outp_state;  /* Output pin state register */
  volatile UNS_32 p2_dir_set;     /* GPIO direction set register */
  volatile UNS_32 p2_dir_clr;     /* GPIO direction clear register */
  volatile UNS_32 p2_dir_state;   /* GPIO direction state register */
  volatile UNS_32 p2_inp_state; /* SDRAM-Input pin state register*/
  volatile UNS_32 p2_outp_set;  /* SDRAM-Output pin set register */
  volatile UNS_32 p2_outp_clr;  /* SDRAM-Output pin clear register*/
  volatile UNS_32 p2_mux_set;     /* PIO mux control set register*/
  volatile UNS_32 p2_mux_clr;     /* PIO mux control clear register*/
  volatile UNS_32 p2_mux_state;   /* PIO mux state register */
  volatile UNS_32 reserved1 [3];
  volatile UNS_32 p0_inp_state;    /* P0 GPIOs pin read register */
  volatile UNS_32 p0_outp_set;    /* P0 GPIOs output set register */
  volatile UNS_32 p0_outp_clr;    /* P0 GPIOs output clear register */
  volatile UNS_32 p0_outp_state;  /* P0 GPIOs output state register */
  volatile UNS_32 p0_dir_set;     /* P0 GPIOs direction set reg */
  volatile UNS_32 p0_dir_clr;     /* P0 GPIOs direction clear reg */
  volatile UNS_32 p0_dir_state;   /* P0 GPIOs direction state reg */
  volatile UNS_32 reserved2;
  volatile UNS_32 p1_inp_state;    /* P1 GPIOs pin read register */
  volatile UNS_32 p1_outp_set;    /* P1 GPIOs output set register */
  volatile UNS_32 p1_outp_clr;    /* P1 GPIOs output clear register */
  volatile UNS_32 p1_outp_state;  /* P1 GPIOs output state register */
  volatile UNS_32 p1_dir_set;     /* P1 GPIOs direction set reg */
  volatile UNS_32 p1_dir_clr;     /* P1 GPIOs direction clear reg */
  volatile UNS_32 p1_dir_state;   /* P1 GPIOs direction state reg */
  volatile UNS_32 reserved3;
  volatile UNS_32 reserved4 [32];
  volatile UNS_32 p_mux_set;     /* PIO mux2 control set register*/
  volatile UNS_32 p_mux_clr;     /* PIO mux2 control clear register*/
  volatile UNS_32 p_mux_state;   /* PIO mux2 state register */
  volatile UNS_32 reserved5;
  volatile UNS_32 p3_mux_set;     /* PIO mux3 control set register*/
  volatile UNS_32 p3_mux_clr;     /* PIO mux3 control clear register*/
  volatile UNS_32 p3_mux_state;   /* PIO mux3 state register */
  volatile UNS_32 reserved6;
  volatile UNS_32 p0_mux_set;       /* P0 mux control set register*/
  volatile UNS_32 p0_mux_clr;       /* P0 mux control clear register*/
  volatile UNS_32 p0_mux_state;     /* P0 mux state register */
  volatile UNS_32 reserved7;
  volatile UNS_32 p1_mux_set;       /* P1 mux control set register*/
  volatile UNS_32 p1_mux_clr;       /* P1 mux control clear register*/
  volatile UNS_32 p1_mux_state;     /* P1 mux state register */
} GPIO_REGS_T;

/**********************************************************************
* p3_inp_state, p3_outp_set, p3_outp_clr, p3_outp_state register
* defines
**********************************************************************/
/* p3_inp_state selection defines */
#define P3_IN_STATE_GPI_00	  _BIT(0)
#define P3_IN_STATE_GPI_01	  _BIT(1)
#define P3_IN_STATE_GPI_02	  _BIT(2)
#define P3_IN_STATE_GPI_03	  _BIT(3)
#define P3_IN_STATE_GPI_04	  _BIT(4)
#define P3_IN_STATE_GPI_05	  _BIT(5)
#define P3_IN_STATE_GPI_06	  _BIT(6)
#define P3_IN_STATE_GPI_07	  _BIT(7)
#define P3_IN_STATE_GPI_08	  _BIT(8)
#define P3_IN_STATE_GPI_09	  _BIT(9)
#define P3_IN_STATE_GPIO_00	  _BIT(10)
#define P3_IN_STATE_GPIO_01	  _BIT(11)
#define P3_IN_STATE_GPIO_02	  _BIT(12)
#define P3_IN_STATE_GPIO_03	  _BIT(13)
#define P3_IN_STATE_GPIO_04	  _BIT(14)
#define P3_IN_STATE_U1_RX	  _BIT(15)
#define P3_IN_STATE_U2_HCTS	  _BIT(16)
#define P3_IN_STATE_U2_RX		  _BIT(17)
#define P3_IN_STATE_U3_RX		  _BIT(18)
#define P3_IN_STATE_GPI_19_U4RX _BIT(19)
#define P3_IN_STATE_U5_RX		  _BIT(20)
#define P3_IN_STATE_U6_IRRX	  _BIT(21)
#define P3_IN_STATE_U7_HCTS	  _BIT(22)
#define P3_IN_STATE_U7_RX		  _BIT(23)
#define P3_IN_STATE_GPIO_05	  _BIT(24)
#define P3_IN_STATE_SPI1_DATIN  _BIT(25)
#define P3_IN_STATE_SPI2_DATIN  _BIT(27)
#define P3_IN_STATE_GPI_28_U3RI _BIT(28)

/* Following macro is used to determine bit position for GPO pin in
*  p3_outp_set, p3_outp_clr & p3_outp_state registers.
*  Where pin = {0-23}
*/
#define P3_STATE_GPO(pin)	  _BIT((pin))

/* Following macro is used to determine bit position for GPIO pin in
*  p3_outp_set, p3_outp_clr & p3_outp_state registers.
*  Where pin = {0-5}
*/
#define P3_STATE_GPIO(pin)  _BIT(((pin) + 25))

/**********************************************************************
* p2_dir_set, p2_dir_clr, p2_dir_state, p2_inp_state, p2_outp_set,
* p2_outp_clr register defines
**********************************************************************/
/* Following macro is used to determine bit position for GPIO pins in
*  p2_dir_set, p2_dir_clr & p2_dir_state registers.
*  Where pin = {0-5}
*/
#define P2_DIR_GPIO(pin)	 _BIT(((pin) + 25))

/* Following macro is used to determine bit position for EMC_D pins in
*  p2_dir_set, p2_dir_clr, p2_dir_state, p2_inp_state,
*  p2_outp_set, & p2_outp_clr.
*  Where pin = {19-31}
*/
#define P2_SDRAM_DIR_PIN(pin) _BIT(((pin) - 19))

/**********************************************************************
* p2_mux_set, p2_mux_clr, p2_mux_state register defines
**********************************************************************/
/* Muxed PIO#2 pin state defines */
#define P2_GPIO05_SSEL0			_BIT(5)
#define P2_GPIO04_SSEL1			_BIT(4)
#define P2_SDRAMD19D31_GPIO		_BIT(3)
#define P2_GPO21_U4TX	        _BIT(2)
#define P2_GPIO03_KEYROW7		_BIT(1)
#define P2_GPIO02_KEYROW6		_BIT(0)

/**********************************************************************
* p0_inp_state, p0_outp_set, p0_outp_clr, p0_outp_state, p0_dir_set,
* p0_dir_clr, p0_dir_state register defines
**********************************************************************/
/* Following macro is used to determine bit position for a P0 GPIO pin
   used with the p0_xxx registers for pins P0_0 to P0_7*/
#define P0_STATE_GPIO(pin)		 _BIT((pin))

/* P0 pin mux defines (0 = GPIO, 1 = alternate function) */
#define P0_GPOP0_I2SRXCLK1		  _BIT(0)
#define P0_GPOP1_I2SRXWS1	      _BIT(1)
#define P0_GPOP2_I2SRXSDA0	      _BIT(2)
#define P0_GPOP3_I2SRXCLK0	      _BIT(3)
#define P0_GPOP4_I2SRXWS0	      _BIT(4)
#define P0_GPOP5_I2STXSDA0	      _BIT(5)
#define P0_GPOP6_I2STXCLK0	      _BIT(6)
#define P0_GPOP7_I2STXWS0	      _BIT(7)

/**********************************************************************
* p1_inp_state, p1_outp_set, p1_outp_clr, p1_outp_state, p1_dir_set,
* p1_dir_clr, p1_dir_state register defines
**********************************************************************/
/* Following macro is used to determine bit position for a P1 GPIO pin
   used with the p1_xxx registers for pins P1_0 to P1_23*/
#define P1_STATE_GPIO(pin)		  _BIT((pin))

/**********************************************************************
* p_mux_set, p_mux_clr, p_mux_state register defines
**********************************************************************/
/* Muxed P0 pin state defines */
#define P_I2STXSDA1_MAT31     _BIT(2)
#define P_I2STXCLK1_MAT30     _BIT(3)
#define P_I2STXWS1_CAP30      _BIT(4)
#define P_SPI2DATAIO_MOSI1    _BIT(5)
#define P_SPI2DATAIN_MISO1    _BIT(6)
#define P_SPI2CLK_SCK1        _BIT(8)
#define P_SPI1DATAIO_SSP0_MOSI _BIT(9)
#define P_SPI1DATAIN_SSP0_MISO _BIT(10)
#define P_SPI1CLK_SCK0        _BIT(12)
#define P_MAT21               _BIT(13)
#define P_MAT20               _BIT(14)
#define P_U7TX_MAT11          _BIT(15)
#define P_MAT03               _BIT(17)
#define P_MAT02               _BIT(18)
#define P_MAT01               _BIT(19)
#define P_MAT00               _BIT(20)

/**********************************************************************
* p3_mux_set, p3_mux_clr, p3_mux_state register defines
**********************************************************************/
/* Muxed p2 pin state macros */
#define P3_GPO2_MAT10          _BIT(2)
#define P3_GPO6                _BIT(6)
#define P3_GPO8                _BIT(8)
#define P3_GPO9                _BIT(9)
#define P3_GPO10_MC2B          _BIT(10)
#define P3_GPO12_MC2A          _BIT(12)
#define P3_GPO13_MC1B          _BIT(13)
#define P3_GPO15_MC1A          _BIT(15)
#define P3_GPO16_MC0B          _BIT(16)
#define P3_GPO18_MC0A          _BIT(18)

/**********************************************************************
* p0_mux_set, p0_mux_clr, p0_mux_state register defines
**********************************************************************/
/* See section on (p0_inp_state) above. */

/**********************************************************************
* p1_mux_set, p1_mux_clr, p1_mux_state register defines
**********************************************************************/
/* Mask for all GPIO P1 bits */
#define P1_ALL                   0x00FFFFFF

/* Macro pointing to GPIO registers */
#define GPIO  ((GPIO_REGS_T *)(GPIO_BASE))

#endif /* LPC32XX_GPIO_H */
