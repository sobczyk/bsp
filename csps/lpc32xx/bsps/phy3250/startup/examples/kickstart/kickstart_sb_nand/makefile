######################################################################## 
# $Id:: makefile 3448 2010-05-10 17:39:56Z usb10132                    $
# 
# Project: Kickstart makefile for small block NAND
# 
######################################################################## 
# Software that is described herein is for illustrative purposes only  
# which provides customers with programming information regarding the  
# products. This software is supplied "AS IS" without any warranties.  
# NXP Semiconductors assumes no responsibility or liability for the 
# use of the software, conveys no license or title under any patent, 
# copyright, or mask work right to the product. NXP Semiconductors 
# reserves the right to make changes in the software without 
# notification. NXP Semiconductors also make no representation or 
# warranty that such application will be suitable for the specified 
# use without further testing or modification. 
########################################################################

EXECNAME    =kickstart_nand_small_block_$(TOOL)

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(NXPMCU_SOFTWARE)\makerule\$(CSP)\make.$(CSP).$(TOOL)
include $(NXPMCU_SOFTWARE)\csps\$(CSP)\bsps\$(BSP)\startup\examples\buildfiles\make.env

########################################################################
# ARM Realview
########################################################################
ifeq ($(TOOL), rvw)

# NO MMU and NO board code. The linked image size for the kickstart
# with MMU and board support exceeds 15.5K and won't be bootable by
# the LPC32x0 with small block FLASH. Your stage 1 application will
# need to do this instead.
#AFLAGS += --predefine "USE_MMU SETL {TRUE}"
#AFLAGS += --predefine "USE_BOARD_INIT SETL {TRUE}"
#AFLAGS += --predefine "USE_ALL_STACKS SETL {TRUE}"
#AFLAGS += --predefine "RW_RELOC SETL {TRUE}"

# The kickstart loader loads and executes at address 0x0 in IRAM.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)\startup\examples\buildfiles\ldscript_iram_rvw.ld

endif

########################################################################
# Keil MDK justs uses the ARM Realview defines
########################################################################
ifeq ($(TOOL), keil)

# NO MMU and NO board code. The linked image size for the kickstart
# with MMU and board support exceeds 15.5K and won't be bootable by
# the LPC32x0 with small block FLASH. Your stage 1 application will
# need to do this instead.
#AFLAGS += --predefine "USE_MMU SETL {TRUE}"
#AFLAGS += --predefine "USE_BOARD_INIT SETL {TRUE}"
#AFLAGS += --predefine "USE_ALL_STACKS SETL {TRUE}"
#AFLAGS += --predefine "RW_RELOC SETL {TRUE}"

# The kickstart loader loads and executes at address 0x0 in IRAM.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)\startup\examples\buildfiles\ldscript_iram_rvw.ld

endif

########################################################################
# GNU
########################################################################
ifeq ($(TOOL), gnu)

# NO MMU and NO board code. The linked image size for the kickstart
# with MMU and board support exceeds 15.5K and won't be bootable by
# the LPC32x0 with small block FLASH. Your stage 1 application will
# need to do this instead.
#AFLAGS += --defsym USE_MMU=1
#AFLAGS += --defsym USE_BOARD_INIT=1
#AFLAGS += --defsym USE_ALL_STACKS=1
#AFLAGS += --defsym RW_RELOC=1

# The kickstart loader loads and executes at address 0x0 in IRAM.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)\startup\examples\buildfiles\ldscript_iram_gnu.ld

endif

########################################################################
#
# Compiler\linker specific stuff
#
########################################################################

ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
endif

ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
endif

ifeq ($(TOOL), gnu)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
endif

########################################################################
#
# Pick up the assembler and C source files in the directory  
#
########################################################################
include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.ftypes

########################################################################
#
# Kickstart code object files 
#
########################################################################

OBJS += cache_support.o startup_entry.o

default: $(OBJS) lpc_libs elf bin hex

elf:
	$(LD) $(OBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) $(MAPFILE)$(MEXT) \
	$(LDESC) $(LDSCRIPT) $(ENTRY) $(C_ENTRY) -o $(EXECNAME)$(EXT)

ifeq ($(TOOL), rvw)
bin: 
	$(ELFTOBIN) $(EXECNAME)$(EXT) --output $(EXECNAME).bin

hex:
	$(ELFTOREC) $(EXECNAME)$(EXT) --output $(EXECNAME)$(REC)
endif

ifeq ($(TOOL), keil)
bin: 
	$(ELFTOBIN) $(EXECNAME)$(EXT) --output $(EXECNAME).bin

hex:
	$(ELFTOREC) $(EXECNAME)$(EXT) --output $(EXECNAME)$(REC)
endif

ifeq ($(TOOL), gnu)
bin: 
	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EXECNAME).bin

hex:
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
endif

########################################################################
#
# Pick up the compiler and assembler rules
#
########################################################################

include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.build

.PHONY: debug bin

