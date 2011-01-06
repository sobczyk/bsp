$Id:: phytec_readme.txt 5071 2010-09-29 16:40:49Z usb10132             $

Phytec LPC3250 Board Support Package version 2.01

See the documentation included in the DOC directory for how to use
this BSP.

************************************************************************
************************************************************************
* Package configuration - MUST READ!
************************************************************************
************************************************************************
Before building and using these packages, be sure to configure your
system defines to match your hardware revisions. See the defines and
possible values in the comments in the phy3250_board.h file included
with this BSP. Failure to properly set these defines to the correct
value may prevent some of the code and examples from working correctly!
The possible define values are below:
 PHY3250_CARRIERBOARD_1305_X
 PHY3250_MODULEBOARD_1304_X
 PHY3250_LCD_1307_X

************************************************************************
************************************************************************
* Board revision notes
************************************************************************
************************************************************************
LCD modules
LCD modules have a board number of 1307.x, where x = 0 or 1. The changes
between .0 and .1 LCD module are shown in the table below:
LCD module revision   Differences
.0                    Initial board
.1                    Active HIGH backlight signal, different wiring
Because of differences in the connector wiring of .0 and .1 LCD modules,
.0 LCD modules should only be used with .0 carrier boards, while .1 LCD
modules should only be used with .2 or greater carrier boards.

Carrier boards
Carrier boards have a revision number of 1305.x, where x = 0 to 3. The
changes between the boards module are shown in the table below:
Carrier board rev     Differences
.0/.1                 Initial board
.2/.3                 USB peripheral VBUS routed to USB_VBUS signal
.2/.3                 GPI4 used to VBUS detection for USB peripheral
.2/.3                 USB_ADR/SW used to control USB_ host power

Module boards
Carrier boards have a revision number of 1304.x, where x = 0 to 1. The
changes between the module boards module are shown in the table below:
Module board rev      Differences
.0                    Initial board
.1                    USB ISP1301 I2C address changed from 0x2C to 0x2D

************************************************************************
************************************************************************
* Other notes
************************************************************************
************************************************************************
The dram_selfrefresh and NORFlashLoader examples will only work with
the Keil and Realview toolchains.

Although the IAR toolchain should work with most of these examples,
the examples have not been tested with the latest IAR toolchains.

S1L built with the GNU toolchain does not work correctly. Kickstart
built with the GNU toolchain exceeds the memory size bootable with NAND
FLASH. Use Keil or Realview for rebuilding the S1L or kickstart examples
or remove parts of the code to make it smaller. (It may be possible to
tweak Linux compiler options to get a smaller image too.)

************************************************************************
************************************************************************
* Tested tools
************************************************************************
************************************************************************

Sourcery G++ Lite 2010q1-188 (GCC)
Arm Realview v3.1
Keil uVision 4 - Realview MDK v4.01
IAR Embedded Workbench 5.41

Tool | Examples   | Startup code
-----+------------+-------------
Arm  | Yes, 2     | Yes
GCC  | Yes        | Yes
IAR  | Most, 2, 3 | 1
Keil | Yes        | Yes

(1) IAR startup code may compile, but not run.
(2) Some examples are specifically for Keil/Realview.
(3) IAR examples may not correctly return to the S1L prompt when done.

************************************************************************
************************************************************************
* Package history
************************************************************************
************************************************************************
Version 1.00
 Initial release

Version 1.10
 All examples now build a binary in addition to an S-record file
 Made the NAND_DRVDATA_T data public in the NAND driver
 The serial loader tool and it's documentation have been moved to
     the LPC32x0 CSP
 Added full GNU support for S1L
 Corrected an issue in the GNU startup code when clearing the BSS
     segment (fixes issues with GNU builds of startup code)
 Lots of small typecast fixes in the S1L startup example code to fix
     various warning messages with GNU
 Made the "nandrs" command output easier to understand
 Added S1L core code to the PHY3250 BSP
 Added additional code to the "Restoration startup example" that
     outputs some messages that may help debugging if the NAND FLASH
     detection locks up.
 Added support for carrier board, CPU module, and LCD revisions. The
     selected system revision defines (PHY3250_CARRIERBOARD_1305_X,
     PHY3250_MODULEBOARD_1304_X, PHY3250_LCD_1307_X) must match the
     system for which the code
 Added conditonal compilation of code based on revisions of carrier,
     CPU, and LCD boards
 Updated boot loader and BSP documentation, serial loader tool docs
      is now part of the CSP
 Moved backlight enable/disable code to phy3250_board.c and updated
      LCD based examples to call backlight function.
 Added S1L support and updates:
     New nexdump command to dump spare area of NAND FLASH sectors
     info command now dumps NAND device ID and SDRAM type/size
     Lots of core S1L updates and fixes
     Improved bad block management and marking
 Added several pre-built IDE examples for IAR and Realview 3.x
 Added ECC support and correction to SLC read/write operations

Version 1.20
 Various updates and enhancements to S1L - Support for scripting and
     history, support for more SD/MMC card types, fixed a NAND boot
     issue, documentation update, and various smaller fixes.
 Added a master only I2C driver example talking to the UDA1380.
 GPIO macro names changes due to a change in the CSP (various files)
 Fixed a warning in the Keil/Realview startup assembly code
 Lowered timeout values for EMC AHB bus timeouts - this should help
     with very large displays
 Removed GPIO configuration from kickstart to make image smaller
 LED toggling added during SD accesses
 Updated S1L bootloader document
 Improved HSTIMER example with background LED flash
 Added NOR FLASH example
 Added NOR FLASH loader tool
 Added background LED toggle for simple startup example
 Update restore.bin, stage1.bin, and kickstart.bin binaries

Version 1.22
 Updated Keil projects
 Various example tweaks

Version 2.00
 Major update to startup code - completely revised based on generic startup code
 S1L update to the latest version (not update and maddr commands are gone)
 Added different variants of kickstart, can boot u-boot with S1L now
 Added burner and erase utilitiesfor NOR, NAND, and SPI FLASH

version 2.01
 Added bad block forced erase support for S1L v1.x to v2.x updates
 Corrected an cache flush/invalidation issue with NAND read/write commands
 Moved EMC timeout setup after SDRAM init
 Fixed an compilation issue with bnon-DEBUG builds (startup code, S1L)
 Added a reset command to reset the board
 Keil script updates
 Revised NAND spare area and ECC layout
 nanderase (burner app) now erases first 25 blocks
 Fixed an issue with interrupts and cache that could cause lockup
 Slightly changed default virtual memory layout to be inline with other boards