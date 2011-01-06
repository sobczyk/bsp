######################################################################## 
# $Id:: makefile 3447 2010-05-10 17:38:23Z usb10132                    $
# 
# Project: S1L makefile for booting from kickstart (IRAM at offset 32k)
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

EXECNAME    =s1l_from_kick_$(TOOL)

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(NXPMCU_SOFTWARE)/makerule/$(CSP)/make.$(CSP).$(TOOL)
include $(NXPMCU_SOFTWARE)/csps/$(CSP)/bsps/$(BSP)/startup/examples/buildfiles/make.env

########################################################################
# ARM Realview
########################################################################
ifeq ($(TOOL), rvw)

# S1L runs in IRAM and assumes kickstart has already setup MMU and
# board specific functions. S1L needs all the stacks.
#AFLAGS += --predefine "USE_MMU SETL {TRUE}"
#AFLAGS += --predefine "USE_BOARD_INIT SETL {TRUE}"
AFLAGS += --predefine "USE_ALL_STACKS SETL {TRUE}"
#AFLAGS += --predefine "RW_RELOC SETL {TRUE}"

# The S1L loader loads and executes at address 0x8000 in IRAM when it
# loads from kickstart. This allows kickstart to use the lower 32K of
# IRAM for it's code and data.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)/startup/examples/buildfiles/ldscript_iram32k_rvw.ld

endif

########################################################################
# Keil MDK justs uses the ARM Realview defines
########################################################################
ifeq ($(TOOL), keil)

# S1L runs in IRAM and assumes kickstart has already setup MMU and
# board specific functions. S1L needs all the stacks.
#AFLAGS += --predefine "USE_MMU SETL {TRUE}"
#AFLAGS += --predefine "USE_BOARD_INIT SETL {TRUE}"
AFLAGS += --predefine "USE_ALL_STACKS SETL {TRUE}"
#AFLAGS += --predefine "RW_RELOC SETL {TRUE}"

# The S1L loader loads and executes at address 0x8000 in IRAM when it
# loads from kickstart. This allows kickstart to use the lower 32K of
# IRAM for it's code and data.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)/startup/examples/buildfiles/ldscript_iram32k_rvw.ld

endif

########################################################################
# GNU
########################################################################
ifeq ($(TOOL), gnu)

# S1L runs in IRAM and assumes kickstart has already setup MMU and
# board specific functions. S1L needs all the stacks.
#AFLAGS += --defsym USE_MMU=1
#AFLAGS += --defsym USE_BOARD_INIT=1
AFLAGS += --defsym USE_ALL_STACKS=1
#AFLAGS += --defsym RW_RELOC=1

# The S1L loader loads and executes at address 0x8000 in IRAM when it
# loads from kickstart. This allows kickstart to use the lower 32K of
# IRAM for it's code and data.
# Suitable for NAND FLASH and SPI boot modes
LDSCRIPT =$(BASE_BSP)/startup/examples/buildfiles/ldscript_iram32k_gnu.ld

endif

########################################################################
#
# Compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      += -I $(NXPMCU_SOFTWARE)/ip/s1l/include
endif

ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      += -I $(NXPMCU_SOFTWARE)/ip/s1l/include
endif

ifeq ($(TOOL), gnu)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      += -I $(NXPMCU_SOFTWARE)/ip/s1l/include
endif

VPATH = $(NXPMCU_SOFTWARE)/ip/s1l/source;
vpath = %.c $(NXPMCU_SOFTWARE)/ip/s1l/source

########################################################################
#
# Pick up the assembler and C source files in the directory  
#
########################################################################
include $(NXPMCU_SOFTWARE)/makerule/common/make.rules.ftypes

########################################################################
#
# Kickstart code object files 
#
########################################################################

sl1src = s1l_bootmgr.o s1l_cmds.o s1l_cmds_core.o s1l_cmds_flash.o
sl1src += s1l_cmds_image.o s1l_except.o s1l_fat.o s1l_image_mngt.o
sl1src += s1l_line_input.o s1l_memtests.o s1l_movdat.o s1l_sys.o

OBJS += $(patsubst %.o, $(NXPMCU_SOFTWARE)/ip/s1l/source/%.o, $(sl1src))

sl1src2 = sys_mmu_cmd_group.o sysapi_misc.o sysapi_blkdev.o
sl1src2 += sysapi_timer.o sysapi_termio.o sys_hw.o cfg_save.o
sl1src2 += sysapi_flash.o

OBJS += $(patsubst %.o, ../%.o, $(sl1src2))

OBJS += $(NXPMCU_SOFTWARE)/csps/lpc32xx/bsps/fdi3250/startup/examples/common/write_protect_disable.o

OBJS += $(NXPMCU_SOFTWARE)/csps/lpc32xx/bsps/fdi3250/startup/sdram/sdram_common.o $(NXPMCU_SOFTWARE)/csps/lpc32xx/bsps/fdi3250/startup/sdram/ddr_lp_sdram.o $(NXPMCU_SOFTWARE)/csps/lpc32xx/bsps/fdi3250/startup/board/mem_setup.o
OBJS += $(NXPMCU_SOFTWARE)/csps/lpc32xx/bsps/fdi3250/startup/cache_support.o startup_entry.o

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

include $(NXPMCU_SOFTWARE)/makerule/common/make.rules.build

.PHONY: debug bin
