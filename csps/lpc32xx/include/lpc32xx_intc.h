/***********************************************************************
* $Id:: lpc32xx_intc.h 2376 2009-10-27 17:57:20Z wellsk               $
*
* Project: LPC32XX interrupt controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         Master Interrupt Controller, Sub-Interrupt Controller 1
*         & Sub-Interrupt Controller 2
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

#ifndef LPC32XX_INTC_H
#define LPC32XX_INTC_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
* Interrupt controller Register Structures
**********************************************************************/

/* Interrupt Controller Module Register Structure */
typedef struct
{
  volatile UNS_32 er;
  volatile UNS_32 rsr;
  volatile UNS_32 sr;
  volatile UNS_32 apr;
  volatile UNS_32 atr;
  volatile UNS_32 itr;
} INTC_REGS_T;

/***********************************************************************
* Interrupt source defines & enumerations
*
* Flat IRQ_numbers are generated for all three (MIC, SIC1 & SIC2)
* interrupt controller sources using the following scheme.
* MIC  irqs =  0 - 31
* SIC1 irqs = 32 - 63
* SIC2 irqs = 64 - 95
**********************************************************************/
#define IRQ_MIC_BASE  0
#define IRQ_SIC1_BASE 32
#define IRQ_SIC2_BASE 64
/* Interrupt source enumerations */
typedef enum
{
  /* MIC interrupts */
  IRQ_SUB1IRQ = 0,						/*interrupts from SIC1*/
  IRQ_SUB2IRQ,							/*interrupts from SIC1*/
  IRQ_PWM3 = 3,							/*PWM3 irq*/
  IRQ_PWM4,								/*PWM4 irq*/
  IRQ_HSTIMER,							/*High Speed Timer irq*/
  IRQ_WATCH,							/*Watchdog Timer irq */
  IRQ_UART_IIR3,						/*UART3 irq */
  IRQ_UART_IIR4,						/*UART4 irq */
  IRQ_UART_IIR5,						/*UART5 irq */
  IRQ_UART_IIR6,						/*UART6 irq */
  IRQ_FLASH,							/*NAND flash irq */
  IRQ_SD1 = 13,							/*SD card irq 1 */
  IRQ_LCD = 14,							/*LCD irq */
  IRQ_SD0 = 15,							/*SD card irq 0 */
  IRQ_TIMER0,							/*Timer 0 IRQ */
  IRQ_TIMER1,							/*Timer 1 IRQ */
  IRQ_TIMER2,							/*Timer 2 IRQ */
  IRQ_TIMER3,							/*Timer 3 IRQ */
  IRQ_SSP0,								/*SSP 0 IRQ */
  IRQ_SSP1,								/*SSP 1 IRQ */
  IRQ_I2S0,								/*I2S 0 IRQ */
  IRQ_I2S1,								/*I2S 1 IRQ */
  IRQ_UART_IIR7,						/*UART7 irq */
  IRQ_UART_IIR2,						/*UART2 irq */
  IRQ_UART_IIR1,						/*UART1 irq */
  IRQ_MSTIMER,							/*Millisecond Timer irq */
  IRQ_DMA,								/*DMA irq */
  IRQ_ETHERNET,							/*Ethernet irq */
  IRQ_SUB1FIQ,							/*FIQs from SUB 1*/
  IRQ_SUB2FIQ,							/*FIQs from SUB 2 */

  /* SIC1 interrupts */
  IRQ_JTAG_COMM_TX = IRQ_SIC1_BASE + 1,	/*JTAG TX irq */
  IRQ_JTAG_COMM_RX,						/*JTAG RX irq */
  IRQ_GPI_11 = IRQ_SIC1_BASE + 4,		/*GPI pin 11 irq */
  IRQ_TS_P = IRQ_SIC1_BASE + 6,			/*Touchscreen pendown irq */
  IRQ_TS_IRQ = IRQ_SIC1_BASE + 7,		/*ADC and Touchscreen irq */
  IRQ_TS_AUX = IRQ_SIC1_BASE + 8,		/*Touchscreen aux irq */
  IRQ_SPI2 = IRQ_SIC1_BASE + 12,		/*SPI2 irq */
  IRQ_PLLUSB,							/*USB PLL lock irq */
  IRQ_PLLHCLK,							/*HCLK PLL lock irq */
  IRQ_PLL397 = IRQ_SIC1_BASE + 17,		/*397 PLL lock irq */
  IRQ_I2C_2,							/*I2C2 irq */
  IRQ_I2C_1,							/*I2C1 irq */
  IRQ_RTC,								/*RTC irq */
  IRQ_KEY = IRQ_SIC1_BASE + 22,			/*Keyscan irq */
  IRQ_SPI1,								/*SPI1 irq */
  IRQ_SW,								/*software Timer irq */
  IRQ_USB_OTG_TIMER,					/*USB OTG Timer irq */
  IRQ_USB_OTG_ATX,						/*USB transceiver irq */
  IRQ_USB_HOST,							/*USB Host irq */
  IRQ_USB_DEV_DMA,						/*USB DMA irq */
  IRQ_USB_DEV_LP,						/*USB low priority irq */
  IRQ_USB_DEV_HP,						/*USB high priority irq */
  IRQ_USB_I2C,							/*USB I2C irq */

  /* SIC1 interrupts */
  IRQ_GPIO_00 = IRQ_SIC2_BASE,			/*GPIO pin 00 irq */
  IRQ_GPIO_01,							/*GPIO pin 01 irq */
  IRQ_GPIO_02,							/*GPIO pin 02 irq */
  IRQ_GPIO_03,							/*GPIO pin 03 irq */
  IRQ_GPIO_04,							/*GPIO pin 04 irq */
  IRQ_GPIO_05,							/*GPIO pin 05 irq */
  IRQ_SPI2_DATAIN,						/*SPI2 DATAIN irq */
  IRQ_U2_HCTS,							/*UART2 HCTS irq */
  IRQ_P0_P1_IRQ,						/*OR'ed P0/P1 GPIO irq */
  IRQ_GPI_08,							/*GPI pin 08 irq */
  IRQ_GPI_09,							/*GPI pin 09 irq */
  IRQ_GPI_10,							/*GPI pin 10 irq */
  IRQ_U7_HCTS,							/*UART7 HCTS irq */
  IRQ_GPI_07 = IRQ_SIC2_BASE + 15,		/*GPI pin 07 irq */
  IRQ_SDIO = IRQ_SIC2_BASE + 18,		/*SDIO irq */
  IRQ_U5_RX,							/*UART5 RX irq */
  IRQ_SPI1_DATAIN,						/*SPI1 DATAIN irq */
  IRQ_GPI_00 = IRQ_SIC2_BASE + 22,		/*GPI pin 00 irq */
  IRQ_GPI_01,							/*GPI pin 01 irq */
  IRQ_GPI_02,							/*GPI pin 02 irq */
  IRQ_GPI_03,							/*GPI pin 03 irq */
  IRQ_GPI_04,							/*GPI pin 04 irq */
  IRQ_GPI_05,							/*GPI pin 05 irq */
  IRQ_GPI_06,							/*GPI pin 06 irq */
  IRQ_SYSCLK = IRQ_SIC2_BASE + 31,		/*SYSCLK Mux irq */
  IRQ_END_OF_INTERRUPTS
} INTERRUPT_SOURCE_T;

#ifdef __cplusplus
}
#endif

/***********************************************************************
* Interrupt Controller Register Bit Fields
**********************************************************************/
/* Default value represeting the Activation polarity of all internal
*  interrupt sources
*/
/* Except BITs 31, 30, 1 & 0 all are active high */
#define MIC_APR_DEFAULT		0x3FF0EFE0

/* Except BITs 26 & 4 & 6 all are active high */
#define SIC1_APR_DEFAULT	0xFBD27186

/* All internal interrupts 6, 7, 12, 19, 20 & 31 are set active high
*  All reserved bit are set active high
*  All GPI and GPIO bits are set to active low
*/
#define SIC2_APR_DEFAULT	0x801810C0

/* Default value represeting the Activation Type of all internal
*  interrupt sources. All are level senesitive.
*/
#define MIC_ATR_DEFAULT		0x00000000
#define SIC1_ATR_DEFAULT	0x00026000
#define SIC2_ATR_DEFAULT	0x00000000

/* Macro pointing to interrupt controller registers */
#define MIC  ((INTC_REGS_T *)(MIC_BASE))
#define SIC1 ((INTC_REGS_T *)(SIC1_BASE))
#define SIC2 ((INTC_REGS_T *)(SIC2_BASE))

#endif /* LPC32XX_INTC_H */
