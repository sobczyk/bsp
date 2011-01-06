$Id:: LPC3220_porting_readme.txt 2646 2009-12-18 18:18:08Z usb10132    $

The LPC3220 device contains 128K of IRAM, while the LPC3230, LPC3240,
and LPC3250 devices have 256K.

For developers porting the startup code to a LPC3220 based device, some
modifications are needed to fit the code in IRAM. The recommended
initial mapping is shown below for LPC32x0 systems.

************************************************************************
Organization of IRAM for the LPC32x0 startup code for LPC3220 based
systems:
Address            Size      Section
------------------ --------- -------------------------------------------
0x08000000         105K*     Code, rw data, zi data, other
0x0801A400         7K*       ARM mode stacks
0x0801C000         16K       MMU section table
0x08020000         NA        End of IRAM

************************************************************************
Organization of IRAM for the LPC32x0 startup code for LPC3230, LPC3240,
and LPC3250 based systems:
Address            Size      Section
------------------ --------- -------------------------------------------
0x08000000         233K*     Code, rw data, zi data, other
0x0803A400         7K*       ARM mode stacks
0x0803C000         16K       MMU section table
0x08040000         NA        End of IRAM

*The stacks sizes can be adjusted in the code to better suit
your environment by adjusting the *_STACK_SIZE definitions in
the phy3250_startup_entry.s (Realview, Keil),
phy3250_startup_entry.asm (GNU), and
phy3250_startup_entry.s79 (IAR) files.

THE DEFAULT STARTUP CODE IS CONFIGURED FOR SYSTEMS WITH 256k OF IRAM.
IF YOU ARE USING A LPC3220 BASED SYSTEM AND ARE PORTING THE STARTUP
CODE TO YOUR SYSTEM, MAKE THE FOLLOWING CHANGE BELOW OR THE STARTUP
CODE WILL NOT RUN.

128K IRAM code modifications:
-----------------------------

************************************************************************
* Modification required for IAR compilers
************************************************************************
If you are using IAR to compile your code, located the 
phy3250_startup_entry.s79 file and make the following change:

/*; End of internal RAM*/
/*END_OF_IRAM	EQU 0x08040000*/
END_OF_IRAM	EQU 0x08020000

************************************************************************
* Modification required for Realview and Keil compilers
************************************************************************
If you are using Realview or Keil to compile your code, located the 
phy3250_startup_entry.s file and make the following change:

; End of internal RAM
;END_OF_IRAM EQU 0x08040000
END_OF_IRAM EQU 0x08020000

************************************************************************
* Modification required for GNU compilers
************************************************************************
If you are using GNU to compile your code, located the
phy3250_startup_entry.asm file and make the following change:

/*; End of internal RAM*/
/*.EQU END_OF_IRAM, 0x08040000*/
.EQU END_OF_IRAM, 0x08020000
