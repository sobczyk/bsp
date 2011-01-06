$Id:: phytec_readme.txt 2619 2009-12-17 20:36:15Z usb10132             $

Embedded Artists EA3250 Board Support Package version 2.01

See the documentation included in the DOC directory for how to use
this BSP.

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
Arm  | Yes        | Yes
GCC  | Yes        | Yes
IAR  | Untested   | Untested
Keil | Yes        | Yes

************************************************************************
************************************************************************
* Package history
************************************************************************
************************************************************************
Version 2.00
 Initial release

Version 2.01
 Corrected an cache flush/invalidation issue with NAND read/write
     commands
 Moved EMC timeout setup after SDRAM init
 Setup default core voltage to 1.35v prior to DDR init
 Added a new command (vcore) for adjusting core voltage (use with care)
  Set voltage will remain across reset cycles, but not power cycles
 Added bberase command for force erasing bad blocks
 Moved S1L config saved and load to NAND blocks 23 and 24 - much faster
 Added a reset command
 Added a command to change default clock speed - clock speed is saved
     across power cycles. using the command (clock) will reset the board
     to initialize it for the new clock speeds. This command is NOT
     included in the default build and is considered unstable.
 Fixed an issue with DQSIN delay getting a wrong value at low clock
     speeds
 Revised NAND spare area and ECC layout
 NAND erase application now erases first 25 blocks (burners)
 NAND blocks repartitioned - Stage 1 app now only uses 4 blocks
 Fixed an issue with interrupts and cache that could cause lockup