
This CDL provides the following BSPs and support:
EA3250       : Embedded Artists 3250 board code
GENERIC_32x0 : Generic 3250 board code package
PHY3250      : Phytec 3250 board code
FDI3250      : Future Designs 3250 board code
VAL3250_2    : VAL3250 rev2 board code

All BSPs in this CDL share similar code and support structure.
They all use similar boot loaders and startup code with only
slight variations. S1L source and support code for various
non-volatile memory types is also provided. If your new to
this code, it is highly recommended to start with the
documentation in the generic_32x0/docs directory to get an
understanding on how startup code, burners, boot loaders,
and S1L work and how they are organized.

The EA3250 and PHY3250 boards are commercially available boards.
The BSPs included here allow the building of simple examples,
startup code source, boot loaders, and burner software.

The VAL3250_2 board is not commercially available. The code is
provded here as a reference only.

Also included in the CDL is a generic set of code for boards
based on the LPC32x0. This generic code based provides a good
starting point for developing system startup code, boot loader
support, and SDRAM initialization support. The code is generic
enough to use on most any LPC32x0 based board, although tweaks
may be needed to work fully on any specific board.
