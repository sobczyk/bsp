######################################################################## 
# $Id:: make.lpc32xx.gnu 5049 2010-09-27 20:49:37Z usb10132            $
# 
# Project: LH7A404 toolset rules for GNU toolset
# 
# Description: 
#     Make rules for the GNU toolset
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

include $(NXPMCU_SOFTWARE)\makerule\common\make.rules.environment

CPU      = arm926ej-s
CFLAGS   = -mcpu=arm926ej-s -Wall -Os
CFLAGS   += -mno-sched-prolog -fno-hosted -mno-thumb-interwork
CFLAGS   += -I$(CSP_INC_DIR) -I$(BSP_INC_DIR) -I$(GEN_INC_DIR)
AFLAGS   = -mcpu=arm926ej-s
AFLAGS   += -I$(CSP_INC_DIR) -I$(BSP_INC_DIR) -I$(GEN_INC_DIR)
CC       = arm-none-eabi-gcc
AS       = arm-none-eabi-as
AR       = arm-none-eabi-ar -r
LD       = arm-none-eabi-gcc
NM       = arm-none-eabi-nm
OBJDUMP  = arm-none-eabi-objdump
OBJCOPY  = arm-none-eabi-objcopy
READELF  = arm-none-eabi-readelf
LDFLAGS  += -Wl,--gc-sections

LK       =  -static
LK       += -Wl,--start-group $(TARGET_CSP_LIB) $(TARGET_BSP_LIB) $(TARGET_GEN_LIB)
LK       +=  -lgcc -lc -lg -lm -lstdc++ -lsupc++ 
LK       += -Wl,--end-group
MAP      = -Xlinker -Map -Xlinker
LDESC    = -Xlinker -T  
ENTRY    = -e
BIN      = -bin
EXT      = .elf
LEXT     = 
ELFTOREC =arm-none-eabi-objcopy -O srec --strip-all --verbose
ELFTOBIN =arm-none-eabi-objcopy -I elf32-littlearm -O binary --strip-all --verbose
REC      =.srec
