$Id:: phytec_readme.txt 2619 2009-12-17 20:36:15Z usb10132             $

Generic Board Support Package version 2.0

See the documentation included in the DOC directory for how to use
this BSP.

************************************************************************
************************************************************************
* Tested tools
************************************************************************
************************************************************************

Sourcery G++ Lite 2010q1-188 (GCC)
Arm Realview v3.1

This is untested with Keil MDK, but should work with it correctly, as
they are very similar to the Realview 3.1 tools.

Tool | Examples   | Startup code
-----+------------+-------------
GCC  | All        | Yes
Keil | Untested   | Untested
Arm  | All        | Yes
IAR  | Untested   | Untested(1)

(1) Although examples provide support for IAR, the startup code does
not yet support IAR.

************************************************************************
************************************************************************
* Package history
************************************************************************
************************************************************************
Version 2.00
 Initial release

Version 2.01
 Corrected an cache flush/invalidaiton issue with NAND read/write commands
 Moved EMC timeout setup after SDRAM init
 Revised NAND spare area and ECC layout
 Fixed an issue with interrupts and cache that could cause lockup