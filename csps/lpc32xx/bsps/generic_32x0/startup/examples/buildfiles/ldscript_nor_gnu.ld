 /*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; $Id:: ldscript_nor_gnu.ld 4983 2010-09-21 09:13:04Z ing03005        $
 ; 
 ; Project: 32x0 linker file for GNU toolset for NOR boot
 ; 
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  
 ; Software that is described herein is for illustrative purposes only  
 ; which provides customers with programming information regarding the  
 ; products. This software is supplied "AS IS" without any warranties.  
 ; NXP Semiconductors assumes no responsibility or liability for the 
 ; use of the software, conveys no license or title under any patent, 
 ; copyright, or mask work right to the product. NXP Semiconductors 
 ; reserves the right to make changes in the software without 
 ; notification. NXP Semiconductors also make no representation or 
 ; warranty that such application will be suitable for the specified 
 ; use without further testing or modification. 
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;*/

/* Define the supported output formats - elf32-littlearm is the
   default */
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/* Define the target architecture */
OUTPUT_ARCH(arm)

/* Define the system entry point */
ENTRY(arm926ejs_reset)

/* Define the memory layout for the board */
SECTIONS
{
    /* Set up the location counter */
    . = 0xE0000004;

    /* Define a execution region that resides at ram base */
    .text :
    {
        /* Start of .text segment */
        startup_entry.o *(.text .text.*) 		
    }

    /* Define a readonly data section in flash that resides at the
       end of the .rom execution region */
    . = ALIGN(4);
    .rodata : 
    { 
        /* Place all .rodata regions in this section. */
        *(.rodata* .rodata.*); 
    }

	.ARM.exidx :
	{
		*(.ARM.exidx*)
	}
	__exidx_end = .;

	__TEXT_END = .;
    PROVIDE (__gnu_roend = .);
    /* (Real) End of .text segment */

    /* Set up the location counter for the data area */
    .data 0x00001000 : AT (__TEXT_END)
    {
        /* Start of .bss segment */
        PROVIDE (__gnu_rwstart = .);

        *(.data .data.*); 

        /* Start of .bss segment */
        PROVIDE (__gnu_rwend = .);
    }

    /* Define a ZI section at the end of the .data section */
    .bss : AT (__TEXT_END + SIZEOF (.data))
    {
        /* Start of .bss segment */
        PROVIDE (__gnu_bssstart = .);

        *(.bss .bss.*) *(COMMON); 

        /* End of .bss segment */
        PROVIDE (__gnu_bssend = .);
    }

    /* Define linker variable to reference end address of this
       section - required by gcc */
    PROVIDE (end = .);

    /* DWARF debug sections
       Symbols in the DWARF debugging sections are relative 
       to the beginning of the section so we begin them at 0 */
  	
    /* DWARF 1 */
    .debug          0 	: { *(.debug) }
    .line           0 	: { *(.line) }
  	
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 	: { *(.debug_srcinfo) }
    .debug_sfnames  0 	: { *(.debug_sfnames) }
  	
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 	: { *(.debug_aranges) }
    .debug_pubnames 0 	: { *(.debug_pubnames) }
  	
    /* DWARF 2 */
    .debug_info     0 	: { *(.debug_info) }
    .debug_abbrev   0 	: { *(.debug_abbrev) }
    .debug_line     0 	: { *(.debug_line) }
    .debug_frame    0 	: { *(.debug_frame) }
    .debug_str      0 	: { *(.debug_str) }
    .debug_loc      0 	: { *(.debug_loc) }
    .debug_macinfo  0 	: { *(.debug_macinfo) }
}
