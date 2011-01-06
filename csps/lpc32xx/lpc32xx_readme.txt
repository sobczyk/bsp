$Id:: lpc32xx_readme.txt 4364 2010-08-20 16:53:14Z usb10132            $

NXP LPC32xx Chip Support Package version 1.02

************************************************************************
************************************************************************
* SUpported chip peripherals of the CSP
************************************************************************
************************************************************************
This CSP contains driver support for the following chip peripherals:
 * LPC32xx ADC/Touchscreen
 * LPC32xx Color LCD controller
 * LPC32xx Clock and power controller
 * LPC32xx DMA controller (channel allocation driver only)
 * LPC32xx GPIO controller
 * LPC32xx I2S audio
 * LPC32xx Interrupt controller
 * LPC32xx Keyboard scanner
 * LPC32xx Millisecond timer
 * LPC32xx Real time clock (RTC)
 * LPC3xxx SD card controller
 * LPC32xx SSP 0/1
 * LPC32xx standard timers 0/1/2/3
 * LPC32xx UARTs 3/4/5/6
 * LPC32xx PWM
 * LPC32xx SLC NAND controller
 * LPC32xx MLC NAND controller
 * LPC32xx I2c (master mode only)

More complex peripherals such as the network controller (MAC) and
USB host/device controllers are not part of this CSP. Reference drivers
for those interfaces can be found in the Linux or WinCE ports.

************************************************************************
************************************************************************
* Toolchain support
************************************************************************
************************************************************************
* ARM Realview support
************************************************************************
This package works under ARM Realview 3.x.

************************************************************************
* CodeSourcery GCC support
************************************************************************
This package works under CodeSourcery GCC.

************************************************************************
* ARM Developer's Suite support
************************************************************************
Support for Arm Developer's Suite has not been fully tested and may
only be partially available.

************************************************************************
* IAR Embedded Workbench support
************************************************************************
This package has been verified with an earlier versions of IAR Embedded
Workbench (pre-5.3). It should work with the latest versions.

************************************************************************
* Keil support
************************************************************************
This package works under Keil uVision3.

************************************************************************
************************************************************************
* Known issues
************************************************************************
************************************************************************
No known issues

************************************************************************
************************************************************************
* Package history
************************************************************************
************************************************************************
Version 1.00
 Initial NXP release.

Version 1.01
 Serial loader tool added to BSP (and associated documentation)

Version 1.02
 Updated default interrupt value for MIC_APR_DEFAULT
 Added support for chip unique ID (uid)
 Added standard UART FIFO pop after RX FIFO flush (required)
 Updated GPIO macro names to match LPC32XX user guide names
 Tweaked various drivers to updated GPIO macros
 Modified code in several drivers to supress warnings
 Added a new master only I2C driver
 Added a NOR loader that will allow you to build images that boot from
     NOR FLASH

Version 1.03
 Fixed several issues related to UART clock selection masks

Version 1.04
 Fixed an issue where the latched INT states wouldn't get cleared in
     the hsuart driver
