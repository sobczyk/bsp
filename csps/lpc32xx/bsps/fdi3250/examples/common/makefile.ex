######################################################################## 
# $Id:: makefile.ex 1221 2008-10-21 16:13:22Z wellsk                   $
# 
# Project: Example makefile
#
# Notes:
#     This makefile generates an image that will load in SDRAM at
#     address 0x80000000. The image can be loaded and run with the
#     stage 1 loader or with a debugger.
#
# Description: 
#  Makefile
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

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(NXPMCU_SOFTWARE)\makerule\$(CSP)\make.$(CSP).$(TOOL)

########################################################################
#
# Pick up the assembler and C source files in the directory  
#
########################################################################
include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.ftypes

########################################################################
#
# GNU compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), gnu)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =
ENTRY       =
CFLAGS      +=-gdwarf-2
LDSCRIPT    =..\linker\ldscript_ram_$(TOOL).ld
ADDOBJS     += ..\common\crt0_gnu.o
endif

########################################################################
#
# Arm compiler/linker specific stuff
#
# ARM examples enter via __main and are linked at address 0x80000000
#
########################################################################

ifeq ($(TOOL), rvw)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =__main 
ENTRY       =--entry
CFLAGS      +=-g
AFLAGS      +=-g
LDESC       = --ro-base
LDSCRIPT    = 0x80000000 --first crt0_rvw.o
ADDOBJS     = ..\common\crt0_rvw.o
endif
ifeq ($(TOOL), keil)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     =__main 
ENTRY       =--entry
CFLAGS      +=-g
AFLAGS      +=-g
LDESC       = --ro-base
LDSCRIPT    = 0x80000000 --first crt0_rvw.o
ADDOBJS     = ..\common\crt0_rvw.o
endif

########################################################################
#
# IAR compiler/linker specific stuff
#
########################################################################

ifeq ($(TOOL), iar)
MEXT        =.map
MAPFILE     =$(EXECNAME)
C_ENTRY     = 
ENTRY       = 
CFLAGS      +=
AFLAGS      +=
LDESC       = --config
LDSCRIPT    = ..\linker\ldscript_ram_$(TOOL).ld
ADDOBJS     += ..\common\main_iar.o
endif

########################################################################
#
# Rules to build the executable 
#
########################################################################

default: $(OBJS) $(ADDOBJS) lpc_libs
	$(LD) $(OBJS) $(ADDOBJS) $(LDFLAGS) $(LK) $(SCAN) $(MAP) \
	$(MAPFILE)$(MEXT) $(LDESC) $(LDSCRIPT) $(ENTRY) $(C_ENTRY) \
	-o $(EXECNAME)$(EXT)
ifeq ($(TOOL), ads)
	$(ELFTOREC) $(EXECNAME)$(EXT) -o $(EXECNAME)$(REC)
	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EXECNAME).bin
endif

ifeq ($(TOOL), rvw)
	$(ELFTOREC) $(EXECNAME)$(EXT) --output $(EXECNAME)$(REC)
	$(ELFTOBIN) $(EXECNAME)$(EXT) --output $(EXECNAME).bin
endif

ifeq ($(TOOL), gnu)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EXECNAME).bin
endif

ifeq ($(TOOL), iar)
	$(ELFTOREC) $(EXECNAME)$(EXT) $(EXECNAME)$(REC)
	$(ELFTOBIN) $(EXECNAME)$(EXT) $(EXECNAME).bin
endif

ifeq ($(TOOL), keil)
	$(ELFTOREC) $(EXECNAME)$(EXT) --output $(EXECNAME)$(REC)
	$(ELFTOBIN) $(EXECNAME)$(EXT) --output $(EXECNAME).bin
endif

########################################################################
#
# Pick up the compiler and assembler rules
#
########################################################################

include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.build
